// File:  soundData.h
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Sound data object.

#ifndef SOUND_DATA_H_
#define SOUND_DATA_H_

// Local headers
#include "dataset2D.h"

// Local forward declarations
class Filter;
class AudioFile;

class SoundData
{
public:
	SoundData ExtractSegment(const double& startTime, const double& endTime) const;
	SoundData ApplyFilter(Filter& filter) const;

private:
	friend AudioFile;

	Dataset2D data;
};

#endif// SOUND_DATA_H_
