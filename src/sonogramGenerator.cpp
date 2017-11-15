// File:  sonogramGenerator.cpp
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Sound data object.

// Local headers
#include "sonogramGenerator.h"
#include "dataset2D.h"

// wxWidgets headers
#include <wx/bitmap.h>

SonogramGenerator::SonogramGenerator(const SoundData& soundData,
	const FFTParameters& parameters) : soundData(soundData), parameters(parameters)
{
	ComputeFrequencyInformation();
}

wxBitmap SonogramGenerator::GetBitmap(const unsigned int& width, const unsigned int& height, const ColorMap& colorMap) const
{
	// TODO:  Implement
	return wxBitmap(width, height);
}

void SonogramGenerator::ComputeFrequencyInformation()
{
	const double sliceWidth(parameters.windowSize * soundData.GetSampleRate());// [sec]
	const unsigned int numberOfSlices((1.0 + parameters.overlap) * soundData.GetDuration() * soundData.GetSampleRate() / parameters.windowSize);
	frequencyData.resize(numberOfSlices);

	unsigned int i;
	for (i = 0; i < numberOfSlices; ++i)
	{
		const double startTime(i * sliceWidth * (1.0 - parameters.overlap));
		Dataset2D slice(soundData.ExtractSegment(startTime, std::min(startTime + sliceWidth, soundData.GetDuration()))->GetData());
		frequencyData[i] = ComputeTimeSliceFFT(slice);
	}
}

std::vector<double> SonogramGenerator::ComputeTimeSliceFFT(const Dataset2D& sliceData) const
{
	return FastFourierTransform::ComputeFFT(sliceData,
		parameters.windowFunction, parameters.windowSize, 0.0, true)->GetY();
}
