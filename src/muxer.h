// File:  muxer.h
// Date:  4/1/2021
// Auth:  K. Loux
// Desc:  Wrapper around FFmpeg muxer.

#ifndef MUXER_H_
#define MUXER_H_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavcodec/avcodec.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// Standard C++ headers
#include <string>
#include <ostream>
#include <vector>
#include <queue>

// FFmpeg forward declarations
struct AVStream;
struct AVFormatContext;

// Local forward declarations
class Encoder;

class Muxer
{
public:
	Muxer(std::ostream& outStream);
	~Muxer();

	bool Initialize(const std::string& format, const std::string& outputFileName);
	bool AddStream(Encoder& encoder);

	AVFormatContext* GetOutputFormatContext() const { return outputFormatContext; }

	AVCodecID GetAudioCodec() const;
	AVCodecID GetVideoCodec() const;

	bool WriteHeader();
	bool WriteTrailer();

	bool WriteNextFrame(std::vector<std::queue<AVPacket*>*>& queues);

private:
	std::ostream& outStream;
	std::string fileName;

	AVFormatContext* outputFormatContext = nullptr;
	std::vector<AVStream*> streams;
};

#endif// MUXER_H_
