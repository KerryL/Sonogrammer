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

VideoEncoder::~VideoEncoder()
{
	if (rgbFrame)
		av_frame_free(&rgbFrame);
		
	if (pixelFormatConversionContext)
		sws_freeContext(pixelFormatConversionContext);
}

bool VideoEncoder::Initialize(AVFormatContext* outputFormatContext, const unsigned int& width, const unsigned int& heightIn,
	const double& frameRate, const AVPixelFormat& pixelFormat, const AVCodecID& codecId)
{
	if (!DoBasicInitialization(outputFormatContext, codecId))
		return false;
		
	height = heightIn;

	pixelFormatConversionContext = sws_getContext(width, height, AV_PIX_FMT_RGB24, width, height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, nullptr, nullptr, nullptr);

	AVDictionary* videoOptions(nullptr);
	/*av_dict_set(&videoOptions, "preset", "slow", 0);
	av_dict_set(&videoOptions, "crf", "20", 0);*/

	encoderContext->width = width;
	encoderContext->height = height;
	encoderContext->pix_fmt = pixelFormat;
	encoderContext->bit_rate = 400000;// TODO:  Justify choice or don't hardcode?
	encoderContext->gop_size = 12;// TODO:  Justify choice or don't hardcode?  Has something to do with max spacing betwee "intraframes"
	encoderContext->time_base.num = 1;
	encoderContext->time_base.den = static_cast<int>(frameRate);// TODO:  Better to specify with AVRational?

	if (LibCallWrapper::FFmpegErrorCheck(avcodec_open2(encoderContext, encoder, &videoOptions), "Failed to open video encoder"))
		return false;

	av_dict_free(&videoOptions);
	
	const int align(32);

	rgbFrame = av_frame_alloc();
	if (!rgbFrame)
	{
		outStream << "Failed to allocated RGB frame" << std::endl;
		return false;
	}
		
	rgbFrame->format = AV_PIX_FMT_RGB24;
	rgbFrame->width = width;
	rgbFrame->height = height;
	if (LibCallWrapper::FFmpegErrorCheck(av_frame_get_buffer(rgbFrame, align), "Failed to allocate rgb frame buffer"))
		return false;

	inputFrame = av_frame_alloc();
	if (!inputFrame)
	{
		outStream << "Failed to allocated video input frame" << std::endl;
		return false;
	}
	
	inputFrame->format = pixelFormat;
	inputFrame->width = width;
	inputFrame->height = height;
	if (LibCallWrapper::FFmpegErrorCheck(av_frame_get_buffer(inputFrame, align), "Failed to allocate converted frame buffer"))
		return false;

	if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
		encoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		
	if (LibCallWrapper::FFmpegErrorCheck(avcodec_parameters_from_context(stream->codecpar, encoderContext), "Failed to copy parameters to stream"))
		return false;

	return true;
}

bool VideoEncoder::ConvertFrame()
{
	return !LibCallWrapper::FFmpegErrorCheck(sws_scale(pixelFormatConversionContext, rgbFrame->data, rgbFrame->linesize, 0,
		height, inputFrame->data, inputFrame->linesize), "Failed to convert image");
}
