// File:  encoder.h
// Date:  4/1/2021
// Auth:  K. Loux
// Desc:  Encoder base class.

#ifndef ENCODER_H_
#define ENCODER_H_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavcodec/avcodec.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// Standard C++ headers
#include <ostream>

class Encoder
{
public:
	Encoder(std::ostream& outStream);
	virtual ~Encoder();

	// Derived classes are responsible for allocating the encoders and contexts

	AVPacket* Encode(const AVFrame& Frame);

	AVCodec* GetCodec() { return encoder; }

protected:
	std::ostream& outStream;

	AVCodecContext* encoderContext = nullptr;
	AVCodec* encoder = nullptr;
	AVPacket outputPacketA, outputPacketB;
};

#endif// ENCODER_H_
