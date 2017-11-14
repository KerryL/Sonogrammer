// File:  soundData.cpp
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Sound data object.

// Local headers
#include "soundData.h"
#include "filter.h"

SoundData SoundData::ExtractSegment(const double& startTime, const double& endTime) const
{
	// TODO:  Implement
	return SoundData();
}

SoundData SoundData::ApplyFilter(Filter& filter) const
{
	SoundData filteredData(*this);

	filter.Initialize(filteredData.data.GetY().front());
	for (auto& v : filteredData.data.GetY())
		v = filter.Apply(v);

	return filteredData;
}
