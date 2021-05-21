// File:  muxer.cpp
// Date:  4/1/2021
// Auth:  K. Loux
// Desc:  Wrapper around FFmpeg muxer.

// Local headers
#include "muxer.h"
#include "encoder.h"
#include "libCallWrapper.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

Muxer::Muxer(std::ostream& outStream) : outStream(outStream)
{
}

Muxer::~Muxer()
{
	/*while (!streams.empty())
	{
		avcodec_close();// or something?
	}*/
	
	if (outputFormatContext)
		avformat_free_context(outputFormatContext);
}

bool Muxer::Initialize(const std::string& format, const std::string& outputFileName)
{
	fileName = outputFileName;

	auto outputFormat = av_guess_format(format.c_str(), nullptr, nullptr);
	if (!outputFormat)
	{
		outStream << "Failed to guess output format" << std::endl;
		return false;
	}

	avformat_alloc_output_context2(&outputFormatContext, outputFormat, nullptr, nullptr);
	if (!outputFormatContext)
	{
		outStream << "Failed to allocate output context" << std::endl;
		return false;
	}

	return true;
}

bool Muxer::AddStream(Encoder& encoder, std::queue<AVPacket>& packetQueue)
{
	Stream stream;
	stream.e = &encoder;
	stream.q = &packetQueue;
	streams.push_back(stream);
	return true;
}

std::vector<AVCodecID> Muxer::GetCodecList(const AVMediaType& type) const
{
	std::vector<AVCodecID> encoderList;
	AVCodec* c(nullptr);
	while (c = av_codec_next(c))
	{
		auto e(avcodec_find_encoder(c->id));
		if (e && e->type == type && avformat_query_codec(outputFormatContext->oformat, c->id, FF_COMPLIANCE_STRICT) == 1)
			encoderList.push_back(c->id);
	}

	return encoderList;
}

// I don't completely understand why sometimes we get a specific codec ID and sometimes we don't, even when a container supports multiple codecs
// This seems to work but can probably be improved
std::vector<AVCodecID> Muxer::GetAudioCodecs() const
{
	if (outputFormatContext->oformat->audio_codec == AV_CODEC_ID_FIRST_AUDIO)
		return GetCodecList(AVMEDIA_TYPE_AUDIO);
	return std::vector<AVCodecID>(1, outputFormatContext->oformat->audio_codec);
}

std::vector <AVCodecID> Muxer::GetVideoCodecs() const
{
	/*if (outputFormatContext->oformat->audio_codec == ????)// when?
		return GetCodecList(AVMEDIA_TYPE_VIDEO);*/
	return std::vector<AVCodecID>(1, outputFormatContext->oformat->video_codec);
}

bool Muxer::WriteHeader()
{
	if (LibCallWrapper::FFmpegErrorCheck(avio_open(&outputFormatContext->pb, fileName.c_str(), AVIO_FLAG_WRITE), "Failed to open output file"))
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(avformat_write_header(outputFormatContext, nullptr), "Failed to write header"))
		return false;

	return true;
}

bool Muxer::WriteTrailer()
{
	if (LibCallWrapper::FFmpegErrorCheck(av_interleaved_write_frame(outputFormatContext, nullptr), "Failed to flush buffer"))
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(av_write_trailer(outputFormatContext), "Failed to write trailer"))
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(avio_close(outputFormatContext->pb), "Failed to close output file"))
		return false;

	return true;
}

bool Muxer::WriteNextFrame()
{
	unsigned int minPTSi(0);
	for (unsigned int i = 1; i < streams.size(); ++i)
	{
		if (streams[i].q->empty())
			continue;
		if (streams[i].q->front().pts < streams[minPTSi].q->front().pts)
			minPTSi = i;
	}
	
	if (streams[minPTSi].q->empty())
		return false;

	auto packet(streams[minPTSi].q->front());
	streams[minPTSi].q->pop();

	packet.stream_index = streams[minPTSi].e->stream->index;

	if (LibCallWrapper::FFmpegErrorCheck(av_interleaved_write_frame(outputFormatContext, &packet), "Failed to write frame"))
		return false;
		
	av_packet_unref(&packet);

	return true;
}
