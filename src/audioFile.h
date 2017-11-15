// File:  audioFile.h
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Audio file object.

#ifndef AUDIO_FILE_H_
#define AUDIO_FILE_H_

// Local headers
#include "soundData.h"

// FFmpeg headers
extern "C"
{
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif

#include <libavcodec/avcodec.h>

#ifdef _WIN32
#pragma warning(pop)
#endif
}

// Standard C++ headers
#include <string>
#include <memory>

// FFmpeg forward declarations
struct AVFormatContext;
struct AVStream;

class AudioFile
{
public:
	explicit AudioFile(const std::string& fileName);

	SoundData& GetSoundData() const { return *data; }

	inline double GetDuration() const { return fileInfo.duration; }
	inline int64_t GetBitRate() const { return fileInfo.bitRate; }
	inline std::string GetChannelFormat() const { return fileInfo.channelFormat; }
	inline unsigned int GetSampleRate() const { return fileInfo.sampleRate; }
	inline std::string GetSampleFormat() const { return fileInfo.sampleFormat; }

private:
	const std::string fileName;

	void ExtractSoundData();
	std::unique_ptr<SoundData> data;

	struct AudioFileInformation
	{
		double duration = 0.0;// [sec]
		int64_t bitRate = 0;// [bit/sec]
		std::string channelFormat;
		unsigned int sampleRate = 0;// [Hz]
		std::string sampleFormat;
	};

	AudioFileInformation fileInfo;

	void ProbeAudioFile();
	static std::string GetChannelFormatString(const uint64_t& layout);
	static std::string GetSampleFormatString(const AVSampleFormat& format);

	// Taken from ffprobe.c and cmdutils.c files
	static AVDictionary** FindStreamInfoOptions(AVFormatContext* s, AVDictionary* codecOptions);
	static int CheckStreamSpecifier(AVFormatContext* s, AVStream* st, const char* spec);
	static AVDictionary *FilterCodecOptions(AVDictionary* opts, AVCodecID codec_id,
		AVFormatContext* s, AVStream* st, AVCodec* codec);
};

#endif// AUDIO_FILE_H_
