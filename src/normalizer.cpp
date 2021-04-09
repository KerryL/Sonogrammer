// File:  normalizer.cpp
// Date:  4/7/2021
// Auth:  K. Loux
// Desc:  Class for applying normalization to SoundData.

// Local headers
#include "normalizer.h"
#include "soundData.h"
#include "filter.h"

// Standard C++ headers
#include <cmath>

void Normalizer::Normalize(SoundData& soundData, const float& gainFactor) const
{
	for (auto& v : soundData.GetData().GetY())
	{
		v *= gainFactor;
		if (v > 1.0f)
			v = 1.0f;
		else if (v < -1.0f)
			v = -1.0f;
	}
}

double Normalizer::ComputeGainFactor(const SoundData& soundData, double targetDecibels, const Method& method) const
{
	if (targetDecibels > 0.0)
		targetDecibels = 0.0;

	double peakAmplitude;
	if (method == Method::PeakAWeighted)
	{
		const auto numerator(Filter::CoefficientsFromString("7397050000*s^4"));
		const auto denominator(Filter::CoefficientsFromString("(s+129.4)^2 * (s+676.7) * (s+4636) * (s+76655)^2"));
		Filter aWeightingFilter(soundData.GetSampleRate(), numerator, denominator);
		auto weighted(soundData.ApplyFilter(aWeightingFilter));
		peakAmplitude = GetPeakAmplitude(*weighted);
	}
	else
		peakAmplitude = GetPeakAmplitude(soundData);

	const double targetAmplitude(pow(10.0, targetDecibels / 20.0));
	return targetAmplitude / peakAmplitude;
}

double Normalizer::GetPeakAmplitude(const SoundData& soundData) const
{
	double minValue(0.0), maxValue(0.0);
	for (auto& v : soundData.GetData().GetY())
	{
		if (v < minValue)
			minValue = v;
		else if (v > maxValue)
			maxValue = v;
	}

	if (maxValue > -minValue)
		return maxValue;
	return -minValue;
}
