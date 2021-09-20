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
	outputPacket = av_packet_alloc();
}

Encoder::~Encoder()
{
	if (encoderContext)
		avcodec_free_context(&encoderContext);

	av_packet_unref(outputPacket);
	av_packet_free(&outputPacket);
	
	if (inputFrame)
		av_frame_free(&inputFrame);
}

bool Encoder::DoBasicInitialization(AVFormatContext* outputFormatContext, const AVCodecID& codecId)
{
	encoder = avcodec_find_encoder(codecId);
	if (!encoder)
	{
		outStream << "Failed to find codec" << std::endl;
		return false;
	}
	
	stream = avformat_new_stream(outputFormatContext, encoder);
	if (!stream)
	{
		outStream << "Failed to allocate stream" << std::endl;
		return false;
	}
	stream->id = outputFormatContext->nb_streams - 1;
	
	encoderContext = avcodec_alloc_context3(encoder);
	if (!encoderContext)
	{
		outStream << "Failed to allocate encoder context" << std::endl;
		return false;
	}
	
	if (LibCallWrapper::FFmpegErrorCheck(avcodec_parameters_to_context(encoderContext, stream->codecpar), "Failed to copy parameters to context"))
		return false;
	
	ptsCounter = 0;
	
	return true;
}

Encoder::Status Encoder::Encode(AVPacket& encodedPacket)
{
	if (inputFrame)
	{
		if (inputFrame->nb_samples == 0)// video
			inputFrame->pts = ptsCounter++;
		else// audio
		{
			inputFrame->pts = ptsCounter;
			ptsCounter += inputFrame->nb_samples;
		}
	}

	if (!flushing && LibCallWrapper::FFmpegErrorCheck(avcodec_send_frame(encoderContext, inputFrame),
		"Error sending frame to encoder"))
		return Status::Error;
	
	if (!inputFrame)
		flushing = true;

	int returnCode(avcodec_receive_packet(encoderContext, outputPacket));
	if (returnCode == AVERROR(EAGAIN))
		return Status::NeedMoreInput;
	else if (returnCode == AVERROR_EOF)
		return Status::Done;
	else if (returnCode == 0)
	{
		av_packet_rescale_ts(outputPacket, encoderContext->time_base, stream->time_base);
		av_packet_ref(&encodedPacket, outputPacket);
		return Status::HavePacket;
	}
	
	LibCallWrapper::FFmpegErrorCheck(returnCode, "Failed to receive packet from encoder");
	return Status::Error;
}
