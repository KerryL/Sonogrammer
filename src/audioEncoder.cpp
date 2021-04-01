// File:  audioEncoder.cpp
// Date:  4/1/2021
// Auth:  K. Loux
// Desc:  Audio decoder wrapper.

// Local headers
#include "audioEncoder.h"
#include "libCallWrapper.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

AudioEncoder::AudioEncoder(std::ostream& outStream) : Encoder(outStream)
{
}

bool AudioEncoder::Initialize(AVFormatContext* outputFormatContext, const int& channels, const int& sampleRate, const AVSampleFormat& format, const AVCodecID& codecId)
{
	if (encoderContext)
		avcodec_free_context(&encoderContext);

	encoder = avcodec_find_encoder(codecId);
	if (!encoder)
	{
		outStream << "Failed to find audio encoder" << std::endl;
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

	encoderContext->sample_fmt = format;
	encoderContext->channels = channels;
	encoderContext->profile = FF_PROFILE_AAC_LOW;// TODO:  Shouldn't be hard-coded - also understand why we would choose this one?
	encoderContext->sample_rate = sampleRate;
	encoderContext->time_base.num = 1;
	encoderContext->time_base.den = sampleRate;

	if (LibCallWrapper::FFmpegErrorCheck(avcodec_open2(encoderContext, encoder, nullptr), "Failed to open encoder"))
		return false;

	if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
		encoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	return true;
}

unsigned int AudioEncoder::GetFrameSize() const
{
	return encoderContext->frame_size;
}
