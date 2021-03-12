// File:  videoEncoder.cpp
// Date:  3/10/2021
// Auth:  K. Loux
// Desc:  Wrapper around FFmpeg video encoder.

// Local headers
#include "videoEncoder.h"
#include "libCallWrapper.h"

// Standard C++ headers
#include <cassert>

VideoEncoder::VideoEncoder(std::ostream& outStream) : outStream(outStream)
{
	av_init_packet(&outputPacketA);
	av_init_packet(&outputPacketB);
}

VideoEncoder::~VideoEncoder()
{
	if (encoderContext)
		avcodec_free_context(&encoderContext);

	av_packet_unref(&outputPacketA);
	av_packet_unref(&outputPacketB);
}

bool VideoEncoder::InitializeEncoder(const unsigned int& width, const unsigned int& height, const double& frameRate, const int& bitRate, const AVPixelFormat& format, const std::string& codecName)
{
	if (encoderContext)
	{
		avcodec_free_context(&encoderContext);
		av_packet_unref(&outputPacketA);
		av_packet_unref(&outputPacketB);
	}

	avcodec_register_all();
	if (codecName == "H264")
		encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
	else
		assert(false && "Requested unsupported codec");

	if (!encoder)
	{
		outStream << "Failed to find encoder" << std::endl;
		return false;
	}

	encoderContext = avcodec_alloc_context3(encoder);
	if (!encoderContext)
	{
		outStream << "Failed to allocate encoder context" << std::endl;
		return false;
	}

	if (LibCallWrapper::FFmpegErrorCheck(av_opt_set_int(encoderContext, "refcounted_frames", 1, 0),
		"Failed to set encoder context to reference count"))
		return false;

	encoderContext->framerate.num = static_cast<int>(frameRate);// TODO:  Be more robust with non-integers
	encoderContext->framerate.den = 1;
	encoderContext->pix_fmt = format;
	encoderContext->width = width;
	encoderContext->height = height;

	if (LibCallWrapper::FFmpegErrorCheck(avcodec_open2(encoderContext, encoder, nullptr), "Failed to open encoder"))
		return false;

	av_init_packet(&outputPacketA);
	av_init_packet(&outputPacketB);

	return true;
}

AVPacket* VideoEncoder::EncodeVideo(const AVFrame& inputFrame)
{
	if (LibCallWrapper::FFmpegErrorCheck(avcodec_send_frame(encoderContext, &inputFrame),
		"Error sending audio frame to encoder"))
		return nullptr;

	AVPacket* lastOutputPacket, *nextOutputPacket(nullptr);
	bool nextPacketIsA(true);
	int returnCode;
	do
	{
		// Loop to ensure we're not growing a buffer inside ffmpeg
		lastOutputPacket = nextOutputPacket;
		nextPacketIsA = !nextPacketIsA;
		if (nextPacketIsA)
			nextOutputPacket = &outputPacketA;
		else
			nextOutputPacket = &outputPacketB;

		returnCode = avcodec_receive_packet(encoderContext, nextOutputPacket);
	} while (returnCode == 0);

	if (returnCode != AVERROR(EAGAIN) || !lastOutputPacket)
	{
		LibCallWrapper::FFmpegErrorCheck(returnCode, "Failed to receive packet from encoder");
		return nullptr;
	}

	return lastOutputPacket;
}
