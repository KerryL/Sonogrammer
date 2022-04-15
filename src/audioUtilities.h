// File:  audioUtilities.h
// Date:  3/17/2017
// Auth:  K. Loux
// Desc:  Utilities for working with audio.

#ifndef AUDIO_UTILITIES_H_
#define AUDIO_UTILITIES_H_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libswresample/swresample.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// Standard C++ headers
#include <string>
#include <iostream>

// Windows forward declarations
struct timeval;

namespace AudioUtilities
{

std::string ListAudioDevices(const bool& input);

AVFrame* CopyAVFrame(const AVFrame* frameIn);

int GetChannelLayoutFromCount(const int& count);

struct timeval AddTime(const struct timeval& a, const struct timeval& b);
struct timeval AddTime(const struct timeval& a, const unsigned int& bUsec);

bool SetResamplerOptions(SwrContext* swrContext,
#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(57, 24, 100)
	const int& inputSampleRate, const uint64_t& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
	const int& outputSampleRate, const uint64_t& outputChannelLayout, const AVSampleFormat& outputSampleFormat);
#else
	const int& inputSampleRate, const AVChannelLayout& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
	const int& outputSampleRate, const AVChannelLayout& outputChannelLayout, const AVSampleFormat& outputSampleFormat);
#endif

uint32_t GetSystemTimeMilliseconds();

}// namespace AudioUtilities

#endif// AUDIO_UTILITIES_H_
