// File:  audioEncoderInterface.h
// Date:  5/19/2021
// Auth:  K. Loux
// Desc:  Helper for audio encoding to automatically generate the proper FFmpeg objects to pass to the AudioEncoder object.

#ifndef AUDIO_ENCODER_INTERFACE_H_
#define AUDIO_ENCODER_INTERFACE_H_

// Local headers
#include "audioEncoder.h"

// Standard C++ headers
#include <queue>
#include <memory>

// Local forward declarations
class SoundData;

// FFmpeg forward declarations
struct AVPacket;

class AudioEncoderInterface
{
public:
	bool Encode(const std::string& outputFileName, const std::unique_ptr<SoundData>& soundData, const int& bitRate);

private:
	void FreeQueuedPackets(std::queue<AVPacket*>& q);
	void SoundToAVFrame(const unsigned int& startSample, const SoundData& soundData, const unsigned int& frameSize, AVFrame*& frame) const;
};

#endif// AUDIO_ENCODER_INTERFACE_H_
