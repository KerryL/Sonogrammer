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
	bool AddStream(Encoder& encoder, std::queue<AVPacket>& packetQueue);

	AVFormatContext* GetOutputFormatContext() const { return outputFormatContext; }

	std::vector<AVCodecID> GetAudioCodecs() const;
	std::vector<AVCodecID> GetVideoCodecs() const;

	bool WriteHeader();
	bool WriteTrailer();

	bool WriteNextFrame();

private:
	std::ostream& outStream;
	std::string fileName;

	AVFormatContext* outputFormatContext = nullptr;
	
	struct Stream
	{
		Encoder* e;
		std::queue<AVPacket>* q;
	};
	
	std::vector<Stream> streams;

	std::vector<AVCodecID> GetCodecList(const AVMediaType& type) const;
};

#endif// MUXER_H_
