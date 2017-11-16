// File:  audioUtilities.cpp
// Date:  3/17/2017
// Auth:  K. Loux
// Desc:  Utilities for working with audio.

// Local headers
#include "audioUtilities.h"
#include "libCallWrapper.h"

// SDL headers
#include <SDL.h>

#pragma warning(push)
#pragma warning(disable:4244)

// FFmpeg headers
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#pragma warning(pop)

// Windows headers
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>// for struct timeval

// Standard C++ headers
#include <sstream>
#include <cassert>
#include <chrono>

namespace AudioUtilities
{

std::string ListAudioDevices(const bool& input)
{
	std::ostringstream ss;
	const unsigned int count(SDL_GetNumAudioDevices(static_cast<int>(input)));
	unsigned int i;
	for (i = 0; i < count; ++i)
		ss << i << " : " << SDL_GetAudioDeviceName(i, static_cast<int>(input)) << '\n';

	return ss.str();
}

AVFrame* CopyAVFrame(const AVFrame* frameIn)
{
	AVFrame* outFrame(av_frame_alloc());
	if (LibCallWrapper::AllocationFailed(outFrame, "Failed to allocate AVFrame"))
		return nullptr;

	outFrame->format = frameIn->format;
	outFrame->width = frameIn->width;
	outFrame->height = frameIn->height;
	outFrame->channels = frameIn->channels;
	outFrame->channel_layout = frameIn->channel_layout;
	outFrame->nb_samples = frameIn->nb_samples;

	if (LibCallWrapper::FFmpegErrorCheck(av_frame_get_buffer(outFrame, 32), "Failed to get frame buffer"))
	{
		av_frame_free(&outFrame);
		return nullptr;
	}

	if (LibCallWrapper::FFmpegErrorCheck(av_frame_copy(outFrame, frameIn), "Failed to copy frame"))
	{
		av_frame_free(&outFrame);
		return nullptr;
	}

	if (LibCallWrapper::FFmpegErrorCheck(av_frame_copy_props(outFrame, frameIn), "Failed to copy frame properties"))
	{
		av_frame_free(&outFrame);
		return nullptr;
	}

	return outFrame;
}

int GetChannelLayoutFromCount(const int& count)
{
	if (count == 1)
		return AV_CH_LAYOUT_MONO;
	else if (count == 2)
		return AV_CH_LAYOUT_STEREO;

	assert(false && "unsupported channel count");
	return 0;
}

struct timeval AddTime(const struct timeval& a, const struct timeval& b)
{
	struct timeval result;

	result.tv_sec = a.tv_sec + b.tv_sec;
	result.tv_usec = a.tv_usec + b.tv_usec;

	if (result.tv_usec > 1000000)
	{
		result.tv_usec -= 1000000;
		result.tv_sec += 1;
	}

	return result;
}

struct timeval AddTime(const struct timeval& a, const unsigned int& bUsec)
{
	struct timeval b;
	b.tv_sec = bUsec / 1000000;
	b.tv_usec = bUsec % 1000000;
	return AddTime(a, b);
}

bool SetResamplerOptions(SwrContext* swrContext,
	const int& inputSampleRate, const uint64_t& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
	const int& outputSampleRate, const uint64_t& outputChannelLayout, const AVSampleFormat& outputSampleFormat)
{
	if (LibCallWrapper::FFmpegErrorCheck(av_opt_set_int(swrContext, "in_channel_layout", inputChannelLayout, 0),
		"Failed to set renderer resampler input channel layout") != 0)
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(av_opt_set_int(swrContext, "in_sample_rate", inputSampleRate, 0),
		"Failed to set renderer resampler input sample rate") != 0)
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(av_opt_set_int(swrContext, "in_sample_fmt", inputSampleFormat, 0),
		"Failed to set renderer resampler input sample format") != 0)
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(av_opt_set_int(swrContext, "out_channel_layout", outputChannelLayout, 0),
		"Failed to set renderer resampler output channel layout") != 0)
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(av_opt_set_int(swrContext, "out_sample_rate", outputSampleRate, 0),
		"Failed to set renderer resampler output sample rate") != 0)
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(av_opt_set_int(swrContext, "out_sample_fmt", outputSampleFormat, 0),
		"Failed to set renderer resampler output sample format") != 0)
		return false;

	return true;
}

uint32_t GetSystemTimeMilliseconds()
{
	return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

}// namespace AudioUtilities
