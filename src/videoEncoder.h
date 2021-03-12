// File:  videoEncoder.h
// Date:  3/10/2021
// Auth:  K. Loux
// Desc:  Wrapper around FFmpeg video encoder.

#ifndef VIDEO_ENCODER_H_
#define VIDEO_ENCODER_H_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// Standard C++ headers
#include <string>

// FFmpeg forward declarations
struct AVCodec;
struct AVCodecContext;
struct AVFrame;

class VideoEncoder
{
public:
	VideoEncoder(std::ostream& outStream);
	~VideoEncoder();

	bool InitializeEncoder(const unsigned int& width, const unsigned int& height, const double& frameRate, const int& bitRate, const AVPixelFormat& format, const std::string& codecName);

	AVPacket* EncodeVideo(const AVFrame& inputFrame);

private:
	std::ostream& outStream;

	AVCodecContext* encoderContext = nullptr;
	AVCodec* encoder = nullptr;
	AVPacket outputPacketA, outputPacketB;
};

#endif// VIDEO_ENCODER_H_
