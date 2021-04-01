// File:  videoEncoder.cpp
// Date:  3/10/2021
// Auth:  K. Loux
// Desc:  Wrapper around FFmpeg video encoder.

// Local headers
#include "videoEncoder.h"
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

// Standard C++ headers
#include <cassert>

VideoEncoder::VideoEncoder(std::ostream& outStream) : Encoder(outStream)
{
}

bool VideoEncoder::Initialize(AVFormatContext* outputFormatContext, const unsigned int& width, const unsigned int& height,
	const double& frameRate, const AVPixelFormat& pixelFormat, const AVCodecID& codecId)
{
	if (encoderContext)
		avcodec_free_context(&encoderContext);

	pixelFormatConversionContext = sws_getContext(width, height, AV_PIX_FMT_RGB24, width, height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, nullptr, nullptr, nullptr);

	AVDictionary* videoOptions(nullptr);
	encoder = avcodec_find_encoder(codecId);
	/*av_dict_set(&videoOptions, "preset", "slow", 0);
	av_dict_set(&videoOptions, "crf", "20", 0);
	avcodec_get_context_defaults3(stream->codec, videoCodec);*/
	encoderContext = avcodec_alloc_context3(encoder);
	AVStream* videoStream(avformat_new_stream(outputFormatContext, encoder));
	encoderContext->width = width;
	encoderContext->height = height;
	encoderContext->pix_fmt = pixelFormat;

	avcodec_open2(encoderContext, encoder, &videoOptions);
	videoStream->time_base.num = 1;
	videoStream->time_base.den = static_cast<int>(frameRate);// TODO:  Better to specify with AVRational?
	avcodec_parameters_from_context(videoStream->codecpar, encoderContext);

	/*av_dump_format(outputContext, 0, fileName.c_str(), 1);
	avio_open(&outputContext->pb, fileName.c_str(), AVIO_FLAG_WRITE);
	avformat_write_header(outputContext, &videoOptions);*/
	av_dict_free(&videoOptions);

	AVFrame* rgbFrame;
	rgbFrame = av_frame_alloc();
	rgbFrame->format = AV_PIX_FMT_RGB24;
	rgbFrame->width = width;
	rgbFrame->height = height;
	av_frame_get_buffer(rgbFrame, 1);

	AVFrame* convertedFrame;
	convertedFrame = av_frame_alloc();
	convertedFrame->format = pixelFormat;
	convertedFrame->width = width;
	convertedFrame->height = height;
	av_frame_get_buffer(convertedFrame, 1);

	/*if (LibCallWrapper::FFmpegErrorCheck(avcodec_open2(encoderContext, encoder, nullptr), "Failed to open encoder"))
		return false;*/

	if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
		encoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	return true;
}

AVFrame* VideoEncoder::ConvertFrame(AVFrame* in)
{
	pixelFormatConversionContext;
	// TODO:  Implement
	return in;
}
