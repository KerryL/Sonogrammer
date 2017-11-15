// File:  sonogramGenerator.h
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Sound data object.

#ifndef SONOGRAM_GENERATOR_H_
#define SONOGRAM_GENERATOR_H_

// Local headers
#include "soundData.h"
#include "fft.h"

// wxWidgets headers
#include <wx/colour.h>

// Standard C++ headers
#include <vector>
#include <map>

// wxWidgets forward declarations
class wxBitmap;

class SonogramGenerator
{
public:
	struct FFTParameters
	{
		FastFourierTransform::WindowType windowFunction;
		unsigned int windowSize;
		double overlap;
	};

	SonogramGenerator(const SoundData& soundData, const FFTParameters& parameters);

	typedef std::map<double, wxColor> ColorMap;
	wxBitmap GetBitmap(const unsigned int& width, const unsigned int& height, const ColorMap& colorMap) const;

private:
	const SoundData& soundData;
	const FFTParameters parameters;

	std::vector<std::vector<double>> frequencyData;// first index time, second index frequency
	void ComputeFrequencyInformation();
	std::vector<double> ComputeTimeSliceFFT(const Dataset2D& sliceData) const;
};

#endif// SONOGRAM_GENERATOR_H_
