// File:  resampler.h
// Date:  7/21/2017
// Auth:  K. Loux
// Desc:  Wrapper around libswresample calls to make interface easier to use.

#ifndef _RESAMPLER_H_
#define _RESAMPLER_H_

// Standard C++ headers
#include <ostream>
#include <cstdint>

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

class Resampler
{
public:
	Resampler();
	~Resampler();

#if LIBSWRESAMPLE_VERSION_INT < AV_VERSION_INT(4, 5, 100)
	bool Initialize(const int& inputSampleRate, const uint64_t& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
		const int& outputSampleRate, const uint64_t& outputChannelLayout, const AVSampleFormat& outputSampleFormat);
#else
	bool Initialize(const int& inputSampleRate, const AVChannelLayout& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
		const int& outputSampleRate, const AVChannelLayout& outputChannelLayout, const AVSampleFormat& outputSampleFormat);
#endif

	AVFrame* Resample(const AVFrame* frame);// Must be called twice if the sample rate changes
	int GetMaxOutputSamples() const { return maxOutputSampleCount; }
	bool NeedsSecondResample() const { return !sampleRatesMatch; }

private:
	bool initialized = false;
	SwrContext* context = nullptr;

	bool sampleRatesMatch = true;
	int maxOutputSampleCount = 0;
	uint8_t** audioOutBuffer = nullptr;
	AVFrame* resampledFrame;

	int outputChannelCount = 0;
	int outputFrequency = 0;
	int inputFrequency = 0;
	AVSampleFormat outputFormat = AV_SAMPLE_FMT_NONE;
	int numberOfOutputPlanes = 0;

	static const int frameSize;

	bool CallResampler(const uint8_t** rawData, const unsigned int inputSampleCount, const bool& resetSampleCount);

	void FreeAudioBuffer();
};

#endif// _RESAMPLER_H_
