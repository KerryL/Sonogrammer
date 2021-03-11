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
	const unsigned int width;
	const unsigned int height;
};

#endif// VIDEO_MAKER_H_