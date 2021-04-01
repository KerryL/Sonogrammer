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

class VideoMaker
{
public:
	VideoMaker(const unsigned int& width, const unsigned int& height) : width(width), height(height) {}

	bool MakeVideo(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
		const std::set<SonogramGenerator::MagnitudeColor>& colorMap, const std::string& fileName);

	const std::string GetErrorString() const { return errorString; }

private:
	const unsigned int width;
	const unsigned int height;
	static const double frameRate;// [Hz]

	std::string errorString;

	wxImage GetFrameImage(const wxImage& wholeSonogram, const double& time, const double& secondsPerPixel, const wxColor& lineColor) const;
	AVFrame* ImageToAVFrame(const wxImage& image) const;
	AVFrame* SoundToAVFrame(const unsigned int& startSample, const SoundData& soundData, const unsigned int& frameSize) const;
};

#endif// VIDEO_MAKER_H_
