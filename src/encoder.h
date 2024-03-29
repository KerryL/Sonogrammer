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
	const AVCodec* encoder = nullptr;
	AVCodecContext* encoderContext = nullptr;
	
	AVFrame* inputFrame = nullptr;
	
	enum class Status
	{
		HavePacket,
		NeedMoreInput,
		Done,
		Error
	};

	Status Encode(AVPacket& encodedPacket);

protected:
	std::ostream& outStream;
	bool flushing = false;

	AVPacket* outputPacket;
	int64_t ptsCounter;
	
	bool DoBasicInitialization(AVFormatContext* outputFormatContext, const AVCodecID& codecId);// Must be called by derived class initialization methods
};

#endif// ENCODER_H_
