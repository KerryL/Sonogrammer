// File:  audioEncoder.h
// Date:  4/1/2021
// Auth:  K. Loux
// Desc:  Audio decoder wrapper.

#ifndef AUDIO_ENCODER_H_
#define AUDIO_ENCODER_H_

// Local headers
#include "encoder.h"

// FFmpeg forward declarations
struct AVFormatContext;

class AudioEncoder : public Encoder
{
public:
	explicit AudioEncoder(std::ostream& outStream);

	bool Initialize(AVFormatContext* outputFormatContext, const int& channels, const int& sampleRate, const AVSampleFormat& format, const AVCodecID& codecId);

	unsigned int GetFrameSize() const;
};

#endif// AUDIO_ENCODER_H_
