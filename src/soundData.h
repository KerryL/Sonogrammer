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
	SoundData(const DatasetType& sampleRate, const DatasetType& duration);
	explicit SoundData(const SoundData& sd);
	explicit SoundData(SoundData&& sd);
	SoundData& operator=(const SoundData& sd) = delete;
	SoundData& operator=(SoundData&& sd) = delete;

	std::unique_ptr<SoundData> ExtractSegment(const DatasetType& startTime, const DatasetType& endTime) const;
	std::unique_ptr<SoundData> ApplyFilter(Filter& filter) const;

	inline DatasetType GetSampleRate() const { return sampleRate; }
	inline DatasetType GetDuration() const { return duration; }
	inline Dataset2D GetData() const { return data; }

private:
	friend AudioFile;

	const DatasetType sampleRate;// [Hz]
	const DatasetType duration;// [sec]
	Dataset2D data;
};

#endif// SOUND_DATA_H_
