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

// FFmpeg forward declarations
struct AVStream;
struct AVFormatContext;

class Encoder
{
public:
	Encoder(std::ostream& outStream);
	virtual ~Encoder();

	// Derived classes are responsible for allocating the encoders and contexts
	
	AVStream* stream = nullptr;
	AVCodec* encoder = nullptr;
	AVFrame* inputFrame = nullptr;

	AVPacket* Encode();

protected:
	std::ostream& outStream;

	AVCodecContext* encoderContext = nullptr;
	AVPacket outputPacketA, outputPacketB;
	
	bool DoBasicInitialization(AVFormatContext* outputFormatContext, const AVCodecID& codecId);// Must be called by derived class initialization methods
};

#endif// ENCODER_H_
