// File:  audioFile.cpp
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Audio file object.

// Local headers
#include "audioFile.h"
#include "soundData.h"

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

AudioFile::AudioFile(const std::string& fileName) : fileName(fileName)
{
	av_register_all();
	ProbeAudioFile();
	// TODO:  Extract sound data
}

SoundData AudioFile::GetSoundData() const
{
	// TODO:  Implement
	return SoundData();
}

int AudioFile::CheckStreamSpecifier(AVFormatContext* s, AVStream* st, const char* spec)
{
    const int ret(avformat_match_stream_specifier(s, st, spec));
	if (ret < 0)
	{
		// TODO:  Message
	}

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
				exit(1);// TODO:  Can't have this
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

	if (!options)
	{
		// TODO:  Message
		return nullptr;
	}

	unsigned int i;
	for (i = 0; i < s->nb_streams; i++)
		options[i] = FilterCodecOptions(codecOptions, s->streams[i]->codecpar->codec_id, s, s->streams[i], nullptr);
	return options;
}

void AudioFile::ProbeAudioFile()
{
	fileInfo = AudioFileInformation();

	AVFormatContext* formatContext(avformat_alloc_context());
	if (!formatContext)
	{
		// TODO:  Message
		return;
	}

	if (avformat_open_input(&formatContext, fileName.c_str(), nullptr, nullptr) != 0)
	{
		// TODO:  Message
		return;
	}

	AVDictionary *codecOptions(nullptr);
	AVDictionary **options(FindStreamInfoOptions(formatContext, codecOptions));

	if (avformat_find_stream_info(formatContext, options) < 0)
	{
		// TODO:  Message
		avformat_close_input(&formatContext);
		return;
	}

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
		fileInfo.channelFormat = GetChannelFormatString(s->codecpar->channel_layout);
		fileInfo.bitRate = s->codecpar->bit_rate;
		fileInfo.sampleFormat = GetSampleFormatString(static_cast<AVSampleFormat>(s->codecpar->format));
		break;
	}

	avformat_close_input(&formatContext);
}

std::string AudioFile::GetChannelFormatString(const uint64_t& layout)
{
	if (layout == AV_CH_LAYOUT_MONO)
		return "Mono";
	else if (layout == AV_CH_LAYOUT_STEREO)
		return "Stereo";
	else if (layout == AV_CH_LAYOUT_QUAD)
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
