// File:  videoEncoder.h
// Date:  3/10/2021
// Auth:  K. Loux
// Desc:  Wrapper around FFmpeg video encoder.

#ifndef VIDEO_ENCODER_H_
#define VIDEO_ENCODER_H_

// Local headers
#include "encoder.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavformat/avformat.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// Standard C++ headers
#include <string>

// FFmpeg forward declarations
struct SwsContext;

class VideoEncoder : public Encoder
{
public:
	VideoEncoder(std::ostream& outStream);

	bool Initialize(AVFormatContext* outputFormat, const unsigned int& width, const unsigned int& height,
		const double& frameRate, const AVPixelFormat& pixelFormat, const AVCodecID& codecId);

	AVFrame* ConvertFrame(AVFrame* in);

private:
	SwsContext* pixelFormatConversionContext;
};

#endif// VIDEO_ENCODER_H_
