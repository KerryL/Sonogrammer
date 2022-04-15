// File:  resampler.cpp
// Date:  7/21/2017
// Auth:  K. Loux
// Desc:  Wrapper around libswresample calls to make interface easier to use.

// Local headers
#include "resampler.h"
#include "audioUtilities.h"
#include "libCallWrapper.h"

// Standard C++ headers
#include <cassert>

const int Resampler::frameSize(4096);

Resampler::Resampler()
{
	resampledFrame = av_frame_alloc();
}

Resampler::~Resampler()
{
	FreeAudioBuffer();

	if (context)
		swr_free(&context);

	if (resampledFrame)
	{
		av_channel_layout_uninit(&resampledFrame->ch_layout);
		av_frame_free(&resampledFrame);
	}
}

void Resampler::FreeAudioBuffer()
{
	if (audioOutBuffer)
		av_freep(&audioOutBuffer[0]);
	av_freep(&audioOutBuffer);
}

#if LIBSWRESAMPLE_VERSION_INT < AV_VERSION_INT(4, 5, 100)
bool Resampler::Initialize(const int& inputSampleRate, const uint64_t& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
	const int& outputSampleRate, const uint64_t& outputChannelLayout, const AVSampleFormat& outputSampleFormat)
#else
bool Resampler::Initialize(const int& inputSampleRate, const AVChannelLayout& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
	const int& outputSampleRate, const AVChannelLayout& outputChannelLayout, const AVSampleFormat& outputSampleFormat)
#endif
{
	if (context)
		swr_free(&context);

	context = swr_alloc();
	if (LibCallWrapper::AllocationFailed(context, "Failed to allocate resampler context"))
	{
		initialized = false;
		return false;
	}

	if (!AudioUtilities::SetResamplerOptions(context, inputSampleRate, inputChannelLayout, inputSampleFormat,
		outputSampleRate, outputChannelLayout, outputSampleFormat))
	{
		initialized = false;
		return false;
	}

	if (LibCallWrapper::FFmpegErrorCheck(swr_init(context),
		"Failed to initialize resampler context"))
	{
		initialized = false;
		return false;
	}

	sampleRatesMatch = inputSampleRate == outputSampleRate;
#if LIBSWRESAMPLE_VERSION_INT < AV_VERSION_INT(4, 5, 100)
	outputChannelCount = av_get_channel_layout_nb_channels(outputChannelLayout);
#else
	outputChannelCount = outputChannelLayout.nb_channels;
#endif
	outputFormat = outputSampleFormat;
	outputFrequency = outputSampleRate;
	inputFrequency = inputSampleRate;

	maxOutputSampleCount = static_cast<int>(av_rescale_rnd(swr_get_delay(context, inputSampleRate)
		+ frameSize, outputSampleRate, inputSampleRate, AV_ROUND_UP));

	FreeAudioBuffer();
	if (LibCallWrapper::FFmpegErrorCheck(av_samples_alloc_array_and_samples(&audioOutBuffer, nullptr,
		outputChannelCount, static_cast<int>(maxOutputSampleCount), outputFormat, 0),
		"Failed to allocate audio output sample buffer"))
		return false;

#if LIBSWRESAMPLE_VERSION_INT < AV_VERSION_INT(4, 5, 100)
	resampledFrame->channels = outputChannelCount;
	resampledFrame->channel_layout = outputChannelLayout;
#else
	if (LibCallWrapper::FFmpegErrorCheck(av_channel_layout_copy(&resampledFrame->ch_layout, &outputChannelLayout),
		"Failed to copy channel layout to output frame"))
		return false;
#endif
	resampledFrame->format = outputFormat;
	resampledFrame->sample_rate = outputSampleRate;

	if (av_sample_fmt_is_planar(outputFormat) == 0)
		numberOfOutputPlanes = 1;
	else
		numberOfOutputPlanes = outputChannelCount;

	initialized = true;
	return true;
}

AVFrame* Resampler::Resample(const AVFrame* frame)
{
	assert(initialized);

	if (frame)
	{
		if (!CallResampler(const_cast<const uint8_t**>(frame->data), frame->nb_samples, true))
			return nullptr;
	}
	else
	{
		assert(!sampleRatesMatch && "Shouldn't call Resample() without a valid frame unless we're flushing the resampler due to mismatched sample rates");
		// If the sample rate has changed, we need to call swr_convert again with NULL input in order to get all converted samples
		if (!CallResampler(nullptr, 0, false))
			return nullptr;
	}

	return resampledFrame;
}

bool Resampler::CallResampler(const uint8_t** rawData, const unsigned int inputSampleCount,
	const bool& resetSampleCount)
{
	const int sampleCount(swr_convert(context, audioOutBuffer, static_cast<int>(maxOutputSampleCount),
		rawData, inputSampleCount));
	if (LibCallWrapper::FFmpegErrorCheck(sampleCount, "Failed to convert audio format"))
		return false;

	if (sampleCount == 0)
	{
		//outStream << "Resampled frame contains no samples" << std::endl;// Spams when resampling from file (don't know why - file seems to stream just fine)
		return false;
	}

	const int usedOutputBufferSize(av_samples_get_buffer_size(nullptr, outputChannelCount,
		static_cast<int>(sampleCount), outputFormat, 1));
	if (LibCallWrapper::FFmpegErrorCheck(usedOutputBufferSize, "Failed to determine output buffer size"))
		return false;

	if (resetSampleCount)
	{
		resampledFrame->nb_samples = sampleCount;
		resampledFrame->linesize[0] = usedOutputBufferSize;
	}
	else
	{
		resampledFrame->nb_samples += sampleCount;
		resampledFrame->linesize[0] += usedOutputBufferSize;
	}

	int i;
	for (i = 0; i < numberOfOutputPlanes; ++i)
		resampledFrame->data[i] = audioOutBuffer[i];

	return true;
}
