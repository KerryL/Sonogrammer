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
class wxImage;

class SonogramGenerator
{
public:
	struct FFTParameters
	{
		FastFourierTransform::WindowType windowFunction;
		unsigned int windowSize;
		double overlap;
		double minFrequency;
		double maxFrequency;
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
	wxImage GetImage(const ColorMap& colorMap) const;

	static wxColor ComputeContrastingMarkerColor(const ColorMap& m);

	static wxColor GetInterpolatedColor(const wxColor& lowerColor, const double& lowerValue,
		const wxColor& upperColor, const double& upperValue, const double& value);

private:
	const SoundData& soundData;
	const FFTParameters parameters;

	DatasetType minMagnitude;
	DatasetType maxMagnitude;

	std::vector<std::vector<DatasetType>> frequencyData;// first index time, second index frequency
	void ComputeFrequencyInformation();
	std::vector<DatasetType> ComputeTimeSliceFFT(const Dataset2D& sliceData) const;

	DatasetType GetScaledMagnitude(const DatasetType& magnitude) const;
	wxColor GetColorFromMap(const DatasetType& magnitude, const ColorMap& colorMap) const;

	unsigned int ComputeNumberOfSlices() const;

	static void GetHSV(const wxColor& c, double& hue, double& saturation, double& value);
	static wxColor ColorFromHSV(const double& hue, const double& saturation, const double& value);
};

#endif// SONOGRAM_GENERATOR_H_
