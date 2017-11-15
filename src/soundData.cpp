// File:  soundData.cpp
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Sound data object.

// Local headers
#include "soundData.h"
#include "filter.h"

// Standard C++ headers
#include <cassert>

SoundData::SoundData(const double& sampleRate, const double& duration) : sampleRate(sampleRate),
	duration(duration), data(static_cast<unsigned int>(sampleRate * duration))
{
}

SoundData::SoundData(const SoundData& sd) : sampleRate(sd.sampleRate), duration(sd.duration), data(sd.data)
{
}

SoundData::SoundData(SoundData&& sd) : sampleRate(sd.sampleRate), duration(sd.duration), data(std::move(sd.data))
{
}

std::unique_ptr<SoundData> SoundData::ExtractSegment(const double& startTime, const double& endTime) const
{
	assert(endTime > startTime);
	const double segmentDuration(endTime - startTime);
	auto segment(std::make_unique<SoundData>(sampleRate, segmentDuration));

	// Because our data has a constant sample rate, we can calculate the indices
	typedef std::vector<double>::size_type DataSizeType;
	const DataSizeType firstGoodIndex(static_cast<DataSizeType>(startTime * sampleRate));
	const DataSizeType newPointCount(static_cast<DataSizeType>(segmentDuration * sampleRate));
	segment->data.GetX().erase(segment->data.GetX().begin(), segment->data.GetX().begin() + firstGoodIndex);
	segment->data.GetY().erase(segment->data.GetY().begin(), segment->data.GetY().begin() + firstGoodIndex);
	segment->data.GetX().erase(segment->data.GetX().begin() + newPointCount, segment->data.GetX().end());
	segment->data.GetY().erase(segment->data.GetY().begin() + newPointCount, segment->data.GetY().end());

	return segment;
}

std::unique_ptr<SoundData> SoundData::ApplyFilter(Filter& filter) const
{
	auto filteredData(std::make_unique<SoundData>(*this));

	filter.Initialize(filteredData->data.GetY().front());
	for (auto& v : filteredData->data.GetY())
		v = filter.Apply(v);

	return filteredData;
}
