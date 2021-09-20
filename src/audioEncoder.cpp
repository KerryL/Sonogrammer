// File:  audioEncoder.cpp
// Date:  4/1/2021
// Auth:  K. Loux
// Desc:  Audio decoder wrapper.

// Local headers
#include "audioEncoder.h"
#include "libCallWrapper.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

AudioEncoder::AudioEncoder(std::ostream& outStream) : Encoder(outStream)
{
}

bool AudioEncoder::Initialize(AVFormatContext* outputFormatContext, const int& channels, const int& sampleRate, const int& bitRate, const AVSampleFormat& format, const AVCodecID& codecId)
{
	if (!DoBasicInitialization(outputFormatContext, codecId))
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(av_opt_set_int(encoderContext, "refcounted_frames", 1, 0),
		"Failed to set encoder context to reference count"))
		return false;

	encoderContext->sample_fmt = format;
	encoderContext->channels = channels;
	encoderContext->channel_layout = AV_CH_LAYOUT_MONO;
	encoderContext->sample_rate = sampleRate;
	encoderContext->time_base.num = 1;
	encoderContext->time_base.den = sampleRate;
	encoderContext->bit_rate = bitRate;

	if (LibCallWrapper::FFmpegErrorCheck(avcodec_open2(encoderContext, encoder, nullptr), "Failed to open audio encoder"))
		return false;

	if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
		encoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		
	inputFrame = av_frame_alloc();
	if (!inputFrame)
	{
		outStream << "Failed to allocated audio input frame" << std::endl;
		return false;
	}
	
	inputFrame->channels = 1;
	inputFrame->channel_layout = AV_CH_LAYOUT_MONO;
	inputFrame->sample_rate = sampleRate;
	if (encoderContext->frame_size > 0)
		inputFrame->nb_samples = encoderContext->frame_size;
	else
		inputFrame->nb_samples = 1024;
	inputFrame->format = AV_SAMPLE_FMT_FLT;
	const int align(32);
	if (LibCallWrapper::FFmpegErrorCheck(av_frame_get_buffer(inputFrame, align), "Failed to allocate audio buffer"))
		return false;
		
	if (LibCallWrapper::FFmpegErrorCheck(avcodec_parameters_from_context(stream->codecpar, encoderContext), "Failed to copy parameters to stream"))
		return false;

	return true;
}

unsigned int AudioEncoder::GetFrameSize() const
{
	if (encoderContext->frame_size > 0)
		return encoderContext->frame_size;
	return inputFrame->nb_samples;
}
