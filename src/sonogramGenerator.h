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
#include <set>

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

	struct MagnitudeColor
	{
		MagnitudeColor() = default;
		MagnitudeColor(const double& magnitude, const wxColor& color) : magnitude(magnitude), color(color) {}

		double magnitude;
		wxColor color;

		bool operator<(const MagnitudeColor& mc) const { return magnitude < mc.magnitude; }
		bool operator>(const MagnitudeColor& mc) const { return magnitude > mc.magnitude; }
	};

	typedef std::set<MagnitudeColor> ColorMap;
	wxBitmap GetBitmap(const ColorMap& colorMap) const;

private:
	const SoundData& soundData;
	const FFTParameters parameters;

	std::vector<std::vector<double>> frequencyData;// first index time, second index frequency
	void ComputeFrequencyInformation();
	std::vector<double> ComputeTimeSliceFFT(const Dataset2D& sliceData) const;

	static wxColor GetColorFromMap(const double& magnitude, const ColorMap& colorMap);
	static void GetHSV(const wxColor& c, double& hue, double& saturation, double& value);
	static wxColor ColorFromHSV(const double& hue, const double& saturation, const double& value);
};

#endif// SONOGRAM_GENERATOR_H_
