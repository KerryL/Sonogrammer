// File:  videoMaker.cpp
// Date:  3/1/2021
// Auth:  K. Loux
// Desc:  Tool for turning sonograms into videos.

// Local headers
#include "videoMaker.h"

// wxWidgets headers
#include <wx/image.h>

bool VideoMaker::MakeVideo(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
	const std::set<SonogramGenerator::MagnitudeColor>& colorMap)
{
	// Create the sonogram (one pixel for every FFT slice)
	SonogramGenerator generator(*soundData, parameters);
	auto wholeSonogram(generator.GetImage(colorMap));
	wholeSonogram.Rescale(std::max(static_cast<unsigned int>(wholeSonogram.GetWidth()), width), height, wxIMAGE_QUALITY_HIGH);

	// Use the first "width" pixels of the sonogram for every frame until the time marker is at width / 2.
	// Then extract a new section of the sonogram to keep the time marker in the middle, until we get near the end.
	// Then use the last "width" pixels of the sonogram for every frame until we reach the end.

	// TODO:  At some point, would be nice to include labeled scales for x and y axes

	return false;
}
