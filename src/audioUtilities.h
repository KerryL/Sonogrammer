// File:  audioUtilities.h
// Date:  3/17/2017
// Auth:  K. Loux
// Desc:  Utilities for working with audio.

#ifndef AUDIO_UTILITIES_H_
#define AUDIO_UTILITIES_H_

// Standard C++ headers
#include <string>
#include <iostream>

// FFmpeg forward declarations
struct AVFrame;
enum AVSampleFormat;
struct SwrContext;

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
	const int& inputSampleRate, const uint64_t& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
	const int& outputSampleRate, const uint64_t& outputChannelLayout, const AVSampleFormat& outputSampleFormat);

uint32_t GetSystemTimeMilliseconds();

}// namespace AudioUtilities

#endif// AUDIO_UTILITIES_H_
