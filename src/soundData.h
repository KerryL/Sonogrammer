// File:  soundData.h
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Sound data object.

#ifndef SOUND_DATA_H_
#define SOUND_DATA_H_

// Local headers
#include "dataset2D.h"

// Standard C++ headers
#include <memory>

// Local forward declarations
class Filter;
class AudioFile;

class SoundData
{
public:
	SoundData(const double& sampleRate, const double& duration);
	SoundData(const SoundData& sd);
	SoundData(SoundData&& sd);
	SoundData& operator=(const SoundData& sd) = delete;
	SoundData& operator=(SoundData&& sd) = delete;

	std::unique_ptr<SoundData> ExtractSegment(const double& startTime, const double& endTime) const;
	std::unique_ptr<SoundData> ApplyFilter(Filter& filter) const;

private:
	friend AudioFile;

	const double sampleRate;// [Hz]
	const double duration;// [sec]
	Dataset2D data;
};

#endif// SOUND_DATA_H_
