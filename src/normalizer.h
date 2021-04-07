// File:  normalizer.h
// Date:  4/7/2021
// Auth:  K. Loux
// Desc:  Class for applying normalization to SoundData.

#ifndef NORMALIZER_H_
#define NORMALIZER_H_

// Local forward declarations
class SoundData;

class Normalizer
{
public:
	enum Method
	{
		Peak,// without weighting
		PeakAWeighted// Approximation to equal-loudness curves in ISO 226
	};

	double ComputeGainFactor(const SoundData& soundData, double targetDecibels, const Method& method) const;
	void Normalize(SoundData& soundData, const float& gainFactor) const;

private:
	double GetPeakAmplitude(const SoundData& soundData) const;
};

#endif// NORMALIZER_H_
