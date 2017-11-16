// File:  resampler.cpp
// Date:  7/21/2017
// Auth:  K. Loux
// Desc:  Wrapper around libswresample calls to make interface easier to use.

// Local headers
#include "resampler.h"
#include "audioUtilities.h"
#include "libCallWrapper.h"

#pragma warning(push)
#pragma warning(disable:4244)

// FFmpeg headers
extern "C"
{
#include <libswresample/swresample.h>
}

#pragma warning(pop)

// Standard C++ headers
#include <cassert>

const int Resampler::frameSize(1024);

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
		av_frame_free(&resampledFrame);
}

void Resampler::FreeAudioBuffer()
{
	if (audioOutBuffer)
		av_freep(&audioOutBuffer[0]);
	av_freep(&audioOutBuffer);
}

bool Resampler::Initialize(const int& inputSampleRate, const uint64_t& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
	const int& outputSampleRate, const uint64_t& outputChannelLayout, const AVSampleFormat& outputSampleFormat)
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
	outputChannelCount = av_get_channel_layout_nb_channels(outputChannelLayout);
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

	resampledFrame->channels = outputChannelCount;
	resampledFrame->channel_layout = outputChannelLayout;
	resampledFrame->format = outputFormat;
	resampledFrame->sample_rate = outputSampleRate;

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
		assert(!sampleRatesMatch && "Shouldn't be calling Resample() without a valid frame unless we're flushing the resampler due to mismatched sample rates");
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

	unsigned int i;
	for (i = 0; i < 8; ++i)
		resampledFrame->data[i] = audioOutBuffer[i];// TODO:  Is this a memory leak?

	return true;
}
