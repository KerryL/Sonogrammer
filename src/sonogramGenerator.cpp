// File:  sonogramGenerator.cpp
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Sound data object.

// Local headers
#include "sonogramGenerator.h"
#include "dataset2D.h"

// wxWidgets headers
#include <wx/bitmap.h>
#include <wx/rawbmp.h>

// Standard C++ headers
#include <algorithm>

SonogramGenerator::SonogramGenerator(const SoundData& soundData,
	const FFTParameters& parameters) : soundData(soundData), parameters(parameters)
{
	ComputeFrequencyInformation();
}

wxColor SonogramGenerator::ComputeContrastingMarkerColor(const ColorMap& m)
{
	return wxColor(255, 0, 0);
	// TODO:  Make algorithm below better
	double minMag(1.0);
	wxColor minColor;
	for (const auto& c : m)
	{
		if (c.magnitude < minMag)
		{
			minColor = c.color;
			minMag = c.magnitude;
		}
	}

	const double perceivedLuminance((0.299 * minColor.Red() + 0.587 * minColor.Green() + 0.114 * minColor.Blue()) / 255.0);
	if (perceivedLuminance < 0.5)
		return wxColor(255, 255, 255);// black
	return wxColor(0, 0, 0);// white
}

wxImage SonogramGenerator::GetImage(ColorMap colorMap) const
{
	std::sort(colorMap.begin(), colorMap.end());
	
	const unsigned int colorDepth(24);
	wxImage sonogram(frequencyData.size(), frequencyData.front().size(), colorDepth);
	wxImagePixelData pixels(sonogram);
	int w;
	for (w = 0; w < sonogram.GetWidth(); ++w)
	{
		int h;
		for (h = 0; h < sonogram.GetHeight(); ++h)
		{
			const wxColor c(GetColorFromMap(frequencyData[w][h], colorMap));
			wxImagePixelData::Iterator p(pixels);
			p.Offset(pixels, w, sonogram.GetHeight() - h - 1);
			p.Red() = c.Red();
			p.Green() = c.Green();
			p.Blue() = c.Blue();
		}
	}

	return sonogram;
}
#include <iostream>
wxColor SonogramGenerator::GetScaledColorFromMap(const DatasetType& scaledMagnitude, const ColorMap& colorMap)
{
	//std::cout << "scaledMag = " << scaledMagnitude << std::endl;
	MagnitudeColor lower, upper;
	for (size_t i = 0; i < colorMap.size() - 1; ++i)
	{
		if (scaledMagnitude <= colorMap[i + 1].magnitude)
		{
			lower = colorMap[i];
			upper = colorMap[i + 1];
			//std::cout << "match; range = " << lower.magnitude << " to " << upper.magnitude << std::endl;
			break;
		}
	}
//std::cout << "here"<<std::endl;
	return GetInterpolatedColor(lower.color, lower.magnitude, upper.color, upper.magnitude, scaledMagnitude);
}

wxColor SonogramGenerator::GetColorFromMap(const DatasetType& magnitude, const ColorMap& colorMap) const
{
	const DatasetType scaledMagnitude(GetScaledMagnitude(magnitude));
	return GetScaledColorFromMap(scaledMagnitude, colorMap);
}

wxColor SonogramGenerator::GetInterpolatedColor(const wxColor& lowerColor, const double& lowerValue,
	const wxColor& upperColor, const double& upperValue, const double& value)
{
	assert(value >= lowerValue && value <= upperValue);

	double lowerH, lowerS, lowerV;
	GetHSV(lowerColor, lowerH, lowerS, lowerV);

	double upperH, upperS, upperV;
	GetHSV(upperColor, upperH, upperS, upperV);

	const double minHue(std::min(lowerH, upperH));
	const double maxHue(std::max(lowerH, upperH));
	double resultHue;
	if (minHue + 1.0 - maxHue < maxHue - minHue)
	{
		resultHue = minHue + (maxHue - (minHue + 1.0)) * value / (upperValue - lowerValue);
		if (resultHue < 0.0)
			resultHue += 1.0;
	}
	else
		resultHue = minHue + (maxHue - minHue) * value / (upperValue - lowerValue);

	return ColorFromHSV(resultHue,
		lowerS + (upperS - lowerS) * (value - lowerValue) / (upperValue - lowerValue),
		lowerV + (upperV - lowerV) * (value - lowerValue) / (upperValue - lowerValue));
}

DatasetType SonogramGenerator::GetScaledMagnitude(const DatasetType& magnitude) const
{
	const float minRef(1.0e-10);
	assert(maxMagnitude >= minMagnitude);
	if (maxMagnitude == minMagnitude)
		return 0.0;
	return (log10(std::max(minRef, magnitude)) - log10(std::max(minRef, minMagnitude))) / (log10(maxMagnitude) - log10(std::max(minRef, minMagnitude)));
}

void SonogramGenerator::ComputeFrequencyInformation()
{
	const DatasetType sliceWidth((parameters.windowSize + 1) / soundData.GetSampleRate());// [sec]
	frequencyData.resize(ComputeNumberOfSlices());

	minMagnitude = std::numeric_limits<DatasetType>::max();
	maxMagnitude = 0.0;

	const double resolution(soundData.GetSampleRate() / parameters.windowSize);// [Hz]
	const unsigned int minFrequencyIndex(parameters.minFrequency / resolution);
	const unsigned int maxFrequencyIndex(parameters.maxFrequency / resolution);
	assert(maxFrequencyIndex > minFrequencyIndex);

	DatasetType startTime(0.0);
	const DatasetType startIncrement(sliceWidth * (1.0 - parameters.overlap));
	for (auto& sliceFrequency : frequencyData)
	{
		if (startTime >= soundData.GetDuration())
		{
			sliceFrequency = std::vector<DatasetType>(maxFrequencyIndex - minFrequencyIndex, 0.0);
			minMagnitude = 0.0;
			startTime += startIncrement;
			continue;
		}

		Dataset2D slice(soundData.ExtractSegment(startTime, std::min(startTime + sliceWidth, soundData.GetDuration()))->GetData());
		if (slice.GetNumberOfPoints() < parameters.windowSize)
		{
			sliceFrequency = std::vector<DatasetType>(maxFrequencyIndex - minFrequencyIndex, 0.0);
			minMagnitude = 0.0;
			startTime += startIncrement;
			continue;
		}

		startTime += startIncrement;
		if (minFrequencyIndex == 0 && maxFrequencyIndex == slice.GetNumberOfPoints() - 1)
			sliceFrequency = std::move(ComputeTimeSliceFFT(slice));
		else
		{
			auto fftData(std::move(ComputeTimeSliceFFT(slice)));
			sliceFrequency = std::vector<DatasetType>(fftData.begin() + minFrequencyIndex, fftData.begin() + maxFrequencyIndex);
		}

		const double maxElement(*std::max_element(sliceFrequency.begin(), sliceFrequency.end()));
		if (maxElement > maxMagnitude)
			maxMagnitude = maxElement;

		const double minElement(*std::min_element(sliceFrequency.begin(), sliceFrequency.end()));
		if (minElement < minMagnitude)
			minMagnitude = minElement;
	}
}

unsigned int SonogramGenerator::ComputeNumberOfSlices() const
{
	assert(soundData.GetDuration() * soundData.GetSampleRate() > parameters.windowSize);
	return (soundData.GetDuration() * soundData.GetSampleRate() - parameters.windowSize)
		/ (parameters.windowSize * (1.0 - parameters.overlap)) + 1;
}

std::vector<DatasetType> SonogramGenerator::ComputeTimeSliceFFT(const Dataset2D& sliceData) const
{
	return FastFourierTransform::ComputeFFT(sliceData,
		parameters.windowFunction, parameters.windowSize, 0.0, true)->GetY();
}

void SonogramGenerator::GetHSV(const wxColor& c, double& hue, double& saturation, double& value)
{
	const double red(c.Red() / 255.0);
	const double green(c.Green() / 255.0);
	const double blue(c.Blue() / 255.0);

	value = std::max(std::max(red, green), blue);
	const double delta(value - std::min(std::min(red, green), blue));

	if (delta == 0)
		hue = 0.0;
	else if (value == red)
		hue = fmod((green - blue) / delta, 6.0) / 6.0;
	else if (value == green)
		hue = ((blue - red) / delta + 2.0) / 6.0;
	else// if (value == blue)
		hue = ((red - green) / delta + 4.0) / 6.0;

	if (hue < 0.0)
		hue += 1.0;
	assert(hue >= 0.0 && hue <= 1.0);

	if (value == 0.0)
		saturation = 0.0;
	else
		saturation = delta / value;
}

wxColor SonogramGenerator::ColorFromHSV(const double& hue, const double& saturation, const double& value)
{
	assert(hue >= 0.0 && hue <= 1.0);
	assert(saturation >= 0.0 && saturation <= 1.0);
	assert(value >= 0.0 && value <= 1.0);

	const double c(value * saturation);
	const double x(c * (1.0 - fabs(fmod(hue * 6.0, 2.0) - 1.0)));
	const double m(value - c);

	if (hue < 1.0 / 6.0)
		return wxColor(static_cast<unsigned char>((c + m) * 255), static_cast<unsigned char>((x + m) * 255), static_cast<unsigned char>(m * 255));
	else if (hue < 2.0 / 6.0)
		return wxColor(static_cast<unsigned char>((x + m) * 255), static_cast<unsigned char>((c + m) * 255), static_cast<unsigned char>(m * 255));
	else if (hue < 3.0 / 6.0)
		return wxColor(static_cast<unsigned char>(m * 255), static_cast<unsigned char>((c + m) * 255), static_cast<unsigned char>((x + m) * 255));
	else if (hue < 4.0 / 6.0)
		return wxColor(static_cast<unsigned char>(m * 255), static_cast<unsigned char>((x + m) * 255), static_cast<unsigned char>((c + m) * 255));
	else if (hue < 5.0 / 6.0)
		return wxColor(static_cast<unsigned char>((x + m) * 255), static_cast<unsigned char>(m * 255), static_cast<unsigned char>((c + m) * 255));
	//else if (hue < 6.0 / 6.0)
		return wxColor(static_cast<unsigned char>((c + m) * 255), static_cast<unsigned char>(m * 255), static_cast<unsigned char>((x + m) * 255));
}
