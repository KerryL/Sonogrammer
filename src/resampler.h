// File:  resampler.h
// Date:  7/21/2017
// Auth:  K. Loux
// Desc:  Wrapper around libswresample calls to make interface easier to use.

#ifndef _RESAMPLER_H_
#define _RESAMPLER_H_

// Standard C++ headers
#include <ostream>
#include <cstdint>

// FFmpeg forward declarations
struct AVFrame;
struct SwrContext;
enum AVSampleFormat;

class Resampler
{
public:
	Resampler();
	~Resampler();

	bool Initialize(const int& inputSampleRate, const uint64_t& inputChannelLayout, const AVSampleFormat& inputSampleFormat,
		const int& outputSampleRate, const uint64_t& outputChannelLayout, const AVSampleFormat& outputSampleFormat);

	AVFrame* Resample(const AVFrame* frame);// Must be called twice if the sample rate changes
	int GetMaxOutputSamples() const { return maxOutputSampleCount; }
	bool NeedsSecondResample() const { return !sampleRatesMatch; }

private:
	bool initialized = false;
	SwrContext* context = nullptr;

	bool sampleRatesMatch;
	int maxOutputSampleCount = 0;
	uint8_t** audioOutBuffer = nullptr;
	AVFrame* resampledFrame;

	int outputChannelCount;
	int outputFrequency;
	int inputFrequency;
	AVSampleFormat outputFormat;

	static const int frameSize;

	bool CallResampler(const uint8_t** rawData, const unsigned int inputSampleCount, const bool& resetSampleCount);

	void FreeAudioBuffer();
};

#endif// _RESAMPLER_H_
