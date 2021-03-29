// File:  videoMaker.h
// Date:  3/1/2021
// Auth:  K. Loux
// Desc:  Tool for turning sonograms into videos.

#ifndef VIDEO_MAKER_H_
#define VIDEO_MAKER_H_

// Local headers
#include "sonogramGenerator.h"

class VideoMaker
{
public:
	VideoMaker(const unsigned int& width, const unsigned int& height) : width(width), height(height) {}

	bool MakeVideo(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
		const std::set<SonogramGenerator::MagnitudeColor>& colorMap);

private:
	// Dimensions apply to sonogram itself; axis and footer add to the total size
	const unsigned int width;
	const unsigned int height;
	
	static const double frameRate;// [Hz]
	static const int footerHeight;
	static const int xAxisHeight;
	static const int yAxisWidth;

	wxImage PrepareSonogram(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
		const std::set<SonogramGenerator::MagnitudeColor>& colorMap, wxImage& footer) const;
	wxImage CreateYAxisLabel(const SonogramGenerator::FFTParameters& parameters);
	wxImage GetFrameImage(const wxImage& wholeSonogram, const wxImage& baseFrame,
		const double& time, const double& secondsPerPixel, const wxColor& lineColor) const;
};

#endif// VIDEO_MAKER_H_
