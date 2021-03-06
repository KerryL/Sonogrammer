// File:  soundData.cpp
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Sound data object.

// Local headers
#include "soundData.h"
#include "filter.h"

// Standard C++ headers
#include <cassert>
#include <algorithm>

SoundData::SoundData(const DatasetType& sampleRate, const DatasetType& duration) : sampleRate(sampleRate),
	duration(duration), data(static_cast<unsigned int>(sampleRate * duration))
{
}

SoundData::SoundData(const SoundData& sd) : sampleRate(sd.sampleRate), duration(sd.duration), data(sd.data)
{
}

SoundData::SoundData(SoundData&& sd) : sampleRate(sd.sampleRate), duration(sd.duration), data(std::move(sd.data))
{
}

std::unique_ptr<SoundData> SoundData::ExtractSegment(const DatasetType& startTime, const DatasetType& endTime) const
{
	assert(endTime > startTime);
	//assert(endTime <= duration);// Too strict - values formatted with %f are truncated, so when we come back in here, duration could be longer out past 6th decimal place
	const DatasetType segmentDuration(std::min(duration, endTime) - startTime);
	auto segment(std::make_unique<SoundData>(sampleRate, segmentDuration));

	// Because our data has a constant sample rate, we can calculate the indices
	const auto firstGoodIndex(static_cast<std::vector<DatasetType>::size_type>(startTime * sampleRate));
	const auto newPointCount(static_cast<std::vector<DatasetType>::size_type>(segmentDuration * sampleRate));

	const auto firstX(data.GetX().begin() + firstGoodIndex);
	const auto lastX(firstX + newPointCount);
	const auto firstY(data.GetY().begin() + firstGoodIndex);
	const auto lastY(firstY + newPointCount);

	segment->data.GetX() = std::vector<DatasetType>(firstX, lastX);
	segment->data.GetY() = std::vector<DatasetType>(firstY, lastY);

	return segment;
}

std::unique_ptr<SoundData> SoundData::ApplyFilter(Filter& filter) const
{
	auto filteredData(std::make_unique<SoundData>(*this));

	filter.Initialize(filteredData->data.GetY().front());
	for (auto& v : filteredData->data.GetY())
		v = static_cast<DatasetType>(filter.Apply(v));

	return filteredData;
}
