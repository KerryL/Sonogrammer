// File:  audioFile.cpp
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Audio file object.

// Local headers
#include "audioFile.h"
#include "soundData.h"
#include "libCallWrapper.h"
#include "audioUtilities.h"
#include "resampler.h"

// FFmpeg headers
extern "C"
{
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif

#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/opt.h>

#ifdef _WIN32
#pragma warning(pop)
#endif
}

// Standard C++ headers
#include <algorithm>

AudioFile::AudioFile(const std::string& fileName) : fileName(fileName)
{
	av_register_all();
	if (ProbeAudioFile())
		ExtractSoundData();
}

int AudioFile::CheckStreamSpecifier(AVFormatContext* s, AVStream* st, const char* spec)
{
    const int ret(avformat_match_stream_specifier(s, st, spec));
	LibCallWrapper::FFmpegErrorCheck(ret, "Failed to match stream specifier");
    return ret;
}

AVDictionary* AudioFile::FilterCodecOptions(AVDictionary* opts, AVCodecID codec_id,
	AVFormatContext* s, AVStream* st, AVCodec *codec)
{
	AVDictionary* ret(nullptr);
	AVDictionaryEntry* t(nullptr);
	int flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM : AV_OPT_FLAG_DECODING_PARAM;
	char prefix(0);
	const AVClass* cc(avcodec_get_class());
	
	if (!codec)
	    codec = s->oformat ? avcodec_find_encoder(codec_id) : avcodec_find_decoder(codec_id);
	
	switch (st->codecpar->codec_type)
	{
	case AVMEDIA_TYPE_VIDEO:
		prefix  = 'v';
		flags  |= AV_OPT_FLAG_VIDEO_PARAM;
		break;

	case AVMEDIA_TYPE_AUDIO:
		prefix  = 'a';
		flags  |= AV_OPT_FLAG_AUDIO_PARAM;
		break;

	case AVMEDIA_TYPE_SUBTITLE:
		prefix  = 's';
		flags  |= AV_OPT_FLAG_SUBTITLE_PARAM;
		break;
	}

	while (t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX), t)
	{
		char *p(strchr(t->key, ':'));
		if (p)
			switch (CheckStreamSpecifier(s, st, p + 1))
			{
			case  1: *p = 0;
				break;

			case  0:
				continue;

			default:
				return nullptr;
			}

		if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) || !codec ||
			(codec->priv_class && av_opt_find(&codec->priv_class, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ)))
			av_dict_set(&ret, t->key, t->value, 0);
		else if (t->key[0] == prefix && av_opt_find(&cc, t->key + 1, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ))
			av_dict_set(&ret, t->key + 1, t->value, 0);

		if (p)
			*p = ':';
    }

    return ret;
}

AVDictionary** AudioFile::FindStreamInfoOptions(AVFormatContext* s, AVDictionary* codecOptions)
{
	if (s->nb_streams == 0)
		return nullptr;

	AVDictionary **options;
	options = static_cast<AVDictionary**>(av_mallocz_array(s->nb_streams, sizeof(*options)));

	if (LibCallWrapper::AllocationFailed(options, "Failed to allocate stream options dictionary"))
		return nullptr;

	unsigned int i;
	for (i = 0; i < s->nb_streams; i++)
		options[i] = FilterCodecOptions(codecOptions, s->streams[i]->codecpar->codec_id, s, s->streams[i], nullptr);
	return options;
}

bool AudioFile::ProbeAudioFile()
{
	fileInfo = AudioFileInformation();

	AVFormatContext* formatContext(avformat_alloc_context());
	if (LibCallWrapper::AllocationFailed(formatContext, "Failed to allocate format context"))
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(avformat_open_input(&formatContext, fileName.c_str(), nullptr, nullptr),
		"Failed to open audio file"))
		return false;

	AVDictionary *codecOptions(nullptr);
	AVDictionary **options(FindStreamInfoOptions(formatContext, codecOptions));

	if (LibCallWrapper::FFmpegErrorCheck(avformat_find_stream_info(formatContext, options),
		"Failed to get stream information"))
	{
		avformat_close_input(&formatContext);
		return false;
	}

	streamIndex = -1;

	unsigned int i;
	for (i = 0; i < formatContext->nb_streams; ++i)
        av_dict_free(&options[i]);
    av_freep(&options);

	for (i = 0; i < formatContext->nb_streams; ++i)
	{
		const AVStream* s(formatContext->streams[i]);
		if (s->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
			continue;

		fileInfo.duration = s->duration * static_cast<double>(s->time_base.num) / s->time_base.den;
		fileInfo.sampleRate = s->codecpar->sample_rate;
		fileInfo.channelFormat = GetChannelFormatString(s->codecpar->channel_layout, s->codecpar->channels);
		fileInfo.bitRate = s->codecpar->bit_rate;
		fileInfo.sampleFormat = GetSampleFormatString(static_cast<AVSampleFormat>(s->codecpar->format));
		streamIndex = i;
		break;
	}

	avformat_close_input(&formatContext);
	return true;
}

std::string AudioFile::GetChannelFormatString(const uint64_t& layout, const int& channelCount)
{
	if (layout == AV_CH_LAYOUT_MONO || channelCount == 1)
		return "Mono";
	else if (layout == AV_CH_LAYOUT_STEREO || channelCount == 2)
		return "Stereo";
	else if (layout == AV_CH_LAYOUT_QUAD || channelCount == 4)
		return "Quad";

	return "Multi";
}

std::string AudioFile::GetSampleFormatString(const AVSampleFormat& format)
{
	switch (format)
	{
	case AV_SAMPLE_FMT_U8:
		return "U8";

	case AV_SAMPLE_FMT_S16:
		return "S16";

	case AV_SAMPLE_FMT_S32:
		return "S32";

	case AV_SAMPLE_FMT_FLT:
		return "FLT";

	case AV_SAMPLE_FMT_DBL:
		return "DBL";

	case AV_SAMPLE_FMT_U8P:
		return "U8P";

	case AV_SAMPLE_FMT_S16P:
		return "S16P";

	case AV_SAMPLE_FMT_S32P:
		return "S32P";

	case AV_SAMPLE_FMT_FLTP:
		return "FLTP";

	case AV_SAMPLE_FMT_DBLP:
		return "DBLP";

	case AV_SAMPLE_FMT_S64:
		return "S64";

	case AV_SAMPLE_FMT_S64P:
		return "S64P";
	}

	return "Unknown";
}

void AudioFile::ExtractSoundData()
{
	assert(fileInfo.duration > 0.0);

	data = std::make_unique<SoundData>(static_cast<DatasetType>(fileInfo.sampleRate), static_cast<DatasetType>(fileInfo.duration));

	AVFormatContext* formatContext(nullptr);
	AVCodecContext* codecContext(nullptr);
	do
	{
		if (!OpenAudioFile(formatContext, codecContext))
			break;

		Resampler resampler;
		if (!CreateResampler(*codecContext, resampler))
			break;

		if (!ReadAudioFile(*formatContext, *codecContext, resampler))
			break;
	} while (false);

	avcodec_free_context(&codecContext);
	avformat_close_input(&formatContext);
}

bool AudioFile::OpenAudioFile(AVFormatContext*& formatContext, AVCodecContext*& codecContext)
{
	if (LibCallWrapper::FFmpegErrorCheck(avformat_open_input(&formatContext,
		fileName.c_str(), nullptr, nullptr), "Failed to open audio file"))
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(avformat_find_stream_info(formatContext, nullptr),
		"Failed to get stream information"))
		return false;

	assert(streamIndex >= 0);

	if (!CreateCodecContext(*formatContext, codecContext))
		return false;

	return true;
}

bool AudioFile::CreateCodecContext(AVFormatContext& formatContext, AVCodecContext*& codecContext)
{
	AVCodec* codec;
	codec = avcodec_find_decoder(formatContext.streams[streamIndex]->codecpar->codec_id);
	if (LibCallWrapper::AllocationFailed(codec, "Failed to find decoder"))
		return false;

	codecContext = avcodec_alloc_context3(codec);
	if (LibCallWrapper::AllocationFailed(codecContext, "Failed to allocate decoder context"))
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(avcodec_parameters_to_context(codecContext,
		formatContext.streams[streamIndex]->codecpar), "Failed to convert parameters to context"))
		return false;

	codecContext->channel_layout = AudioUtilities::GetChannelLayoutFromCount(codecContext->channels);
	codecContext->frame_size = 1024;// Is this OK?  TODO

	if (LibCallWrapper::FFmpegErrorCheck(avcodec_open2(codecContext, codec, nullptr),
		"Failed to open codec"))
		return false;

	if (LibCallWrapper::FFmpegErrorCheck(av_opt_set_int(codecContext, "refcounted_frames", 1, 0),
		"Failed to set decoder context to reference count"))
		return false;

	return true;
}

bool AudioFile::CreateResampler(const AVCodecContext& codecContext, Resampler& resampler)
{
	if (!resampler.Initialize(codecContext.sample_rate, codecContext.channel_layout, codecContext.sample_fmt,
		codecContext.sample_rate, AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_FLTP))
		return false;

	return true;
}

bool AudioFile::ReadPacketFromFile(AVFormatContext& formatContext, AVPacket& packet) const
{
	do
	{
		const int returnCode(av_read_frame(&formatContext, &packet));
		if (returnCode != AVERROR_EOF)
		{
			if (LibCallWrapper::FFmpegErrorCheck(returnCode,
				"Failed to read packet from file"))
				return false;
		}
		else
		{
			packet.size = 0;
			packet.data = nullptr;
			break;
		}
	} while (packet.stream_index != streamIndex);

	return true;
}

bool AudioFile::ReadAudioFile(AVFormatContext& formatContext, AVCodecContext& codecContext, Resampler& resampler)
{
	AVFrame* frame(av_frame_alloc());
	if (LibCallWrapper::AllocationFailed(frame, "Failed to allocate frame buffer"))
		return false;

	AVPacket packet;
	av_init_packet(&packet);

	if (!ReadPacketFromFile(formatContext, packet))
	{
		av_frame_free(&frame);
		return false;
	}

	dataInsertionPoint = 0;
	/*std::generate(data->data.GetX().begin(), data->data.GetX().end(), [n = 0.0, this]() mutable
	{
		return n++ / fileInfo.sampleRate;
	});*/// Apparently, this bit isn't required

	int returnCode(0);
	while (returnCode != AVERROR_EOF)
	{
		if (LibCallWrapper::FFmpegErrorCheck(avcodec_send_packet(&codecContext, &packet),
			"Error sending packet from file to decoder"))
		{
			av_frame_free(&frame);
			return false;
		}

		bool gotAFrame(false);
		do
		{
			returnCode = avcodec_receive_frame(&codecContext, frame);
			if (returnCode == 0)
			{
				AVFrame *resampledFrame(resampler.Resample(frame));
				if (resampledFrame)
					AppendFrame(*resampledFrame);

				if (resampler.NeedsSecondResample())
				{
					resampledFrame = resampler.Resample(nullptr);
					if (resampledFrame)
						AppendFrame(*resampledFrame);
				}
			}
			else if (returnCode != AVERROR(EAGAIN) && !gotAFrame && returnCode != AVERROR_EOF)
			{
				LibCallWrapper::FFmpegErrorCheck(returnCode,
					"Error receiving file frame from decoder");
			}
		} while (returnCode == 0);

		if (!ReadPacketFromFile(formatContext, packet))
		{
			av_frame_free(&frame);
			return false;
		}
	}

	av_frame_free(&frame);
	return true;
}

#include <fstream>// TODO:  Remove
std::ofstream t("test.csv");// TODO:  Remove

void AudioFile::AppendFrame(const AVFrame& frame)
{
	const float* floatData(reinterpret_cast<float*>(frame.data[0]));// Because we resampled to FLTP
	unsigned int i;// TODO:  Remove
	for (i = 0; i < frame.linesize[0] / sizeof(float); ++i)// TODO:  Remove
		t << floatData[i] << '\n';// TODO:  Remove
	std::copy(floatData, floatData + frame.linesize[0] / sizeof(float),
		data->data.GetY().begin() + dataInsertionPoint);
	dataInsertionPoint += frame.nb_samples;
}
