// File:  videoMaker.h
// Date:  3/1/2021
// Auth:  K. Loux
// Desc:  Tool for turning sonograms into videos.

#ifndef VIDEO_MAKER_H_
#define VIDEO_MAKER_H_

// Local headers
#include "sonogramGenerator.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// Standard C++ headers
#include <queue>

// FFmpeg forward declarations
struct AVPacket;

class VideoMaker
{
public:
	VideoMaker(const unsigned int& width, const unsigned int& height, unsigned int audioBitRate, unsigned int videoBitRate)
		: width(width), height(height), audioBitRate(audioBitRate), videoBitRate(videoBitRate) {}

	bool MakeVideo(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
		const SonogramGenerator::ColorMap& colorMap, const std::string& fileName);

	const std::string GetErrorString() const { return errorString; }

private:
	// Dimensions apply to sonogram itself; axis and footer add to the total size
	const unsigned int width;// [px]
	const unsigned int height;// [px]
	const unsigned int audioBitRate;// [b/s]
	const unsigned int videoBitRate;// [b/s]
	
	static const double frameRate;// [Hz]
	static const int footerHeight;
	static const int xAxisHeight;
	static const int yAxisWidth;

	wxImage PrepareSonogram(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
		const SonogramGenerator::ColorMap& colorMap, wxImage& footer) const;
	wxImage CreateYAxisLabel(const SonogramGenerator::FFTParameters& parameters);
	wxImage GetFrameImage(const wxImage& wholeSonogram, const wxImage& baseFrame, const wxImage& maskedFooter,
		const double& time, const double& secondsPerPixel, const wxColor& lineColor) const;

	static void ComputeMaskedColor(const unsigned char& grey, const unsigned char& alpha, unsigned char& r, unsigned char& g, unsigned char& b);

	std::string errorString;

	void ImageToAVFrame(const wxImage& image, AVFrame*& frame) const;
	void SoundToAVFrame(const unsigned int& startSample, const SoundData& soundData, const unsigned int& frameSize, AVFrame*& frame) const;
	
	static void FreeQueuedPackets(std::queue<AVPacket*>& q);
};

#endif// VIDEO_MAKER_H_
