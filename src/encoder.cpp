// File:  encoder.cpp
// Date:  4/1/2021
// Auth:  K. Loux
// Desc:  Encoder base class.

// Local headers
#include "encoder.h"
#include "libCallWrapper.h"

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

Encoder::Encoder(std::ostream& outStream) : outStream(outStream)
{
	av_init_packet(&outputPacketA);
	av_init_packet(&outputPacketB);
}

Encoder::~Encoder()
{
	if (encoderContext)
		avcodec_free_context(&encoderContext);

	av_packet_unref(&outputPacketA);
	av_packet_unref(&outputPacketB);
}

AVPacket* Encoder::Encode(const AVFrame& inputFrame)
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
