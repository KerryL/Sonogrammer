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

SonogramGenerator::SonogramGenerator(const SoundData& soundData,
	const FFTParameters& parameters) : soundData(soundData), parameters(parameters)
{
	ComputeFrequencyInformation();
}

wxBitmap SonogramGenerator::GetBitmap(const ColorMap& colorMap) const
{
	const unsigned int colorDepth(24);
	wxBitmap sonogram(frequencyData.size(), frequencyData.front().size(), colorDepth);
	wxNativePixelData pixels(sonogram);
	int w;
	for (w = 0; w < sonogram.GetWidth(); ++w)
	{
		int h;
		for (h = 0; h < sonogram.GetHeight(); ++h)
		{
			const wxColor c(GetColorFromMap(frequencyData[w][h], colorMap));
			wxNativePixelData::Iterator p(pixels);
			p.Offset(pixels, w, sonogram.GetHeight() - h);
			p.Red() = c.Red();
			p.Green() = c.Green();
			p.Blue() = c.Blue();
		}
	}

	return sonogram;
}

wxColor SonogramGenerator::GetColorFromMap(const double& magnitude, const ColorMap& colorMap)
{
	wxColor lowerColor, upperColor;
	bool foundLower(false);
	for (const auto& c : colorMap)
	{
		if (!foundLower && magnitude >= c.magnitude)
		{
			lowerColor = c.color;
			foundLower = true;
		}
		else if (foundLower)
		{
			upperColor = c.color;
			break;
		}
	}

	double lowerH, lowerS, lowerV;
	GetHSV(lowerColor, lowerH, lowerS, lowerV);

	double upperH, upperS, upperV;
	GetHSV(upperColor, upperH, upperS, upperV);

	const double minHue(std::min(lowerH, upperH));
	const double maxHue(std::max(lowerH, upperH));
	double resultHue;
	if (minHue + 1.0 - maxHue < maxHue - minHue)
		resultHue = 0.5 * (minHue + 1.0 + maxHue);
	else
		resultHue = 0.5 * (maxHue + minHue);

	return ColorFromHSV(resultHue, 0.5 * (lowerS + upperS), 0.5 * (lowerV + upperV));
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
