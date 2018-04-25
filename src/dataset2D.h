/*=============================================================================
                                   LibPlot2D
                       Copyright Kerry R. Loux 2011-2016

                  This code is licensed under the GPLv2 License
                    (http://opensource.org/licenses/GPL-2.0).
=============================================================================*/

// File:  dataset2D.h
// Date:  5/2/2011
// Auth:  K. Loux
// Desc:  Container for x and y-data series for plotting.

#ifndef DATASET_H_
#define DATASET_H_

// Standard C++ headers
#include <cassert>
#include <cstdlib>
#include <vector>
#include <memory>

// wxWidgets forward declarations
class wxString;

typedef float DatasetType;

/// Class for representing paired x-y data.
class Dataset2D
{
public:
	Dataset2D() = default;

	/// Constructor.
	///
	/// \param numberOfPoints Initial size of the buffers.
	explicit Dataset2D(const size_t &numberOfPoints);

	/// Resizes the internal buffers to hold the specified number of points.
	///
	/// \param numberOfPoints New size of internal buffers.
	void Resize(const size_t &numberOfPoints);

	/// Reverses the order of the data stored in this object.
	void Reverse();

	/// Computes the mean of the y-data.
	/// \returns The mean of the y-data.
	DatasetType ComputeYMean() const;

	/// Computes the average of the delta between adjacent x-data.
	/// \returns The average of the delta between adjacent x-data.
	DatasetType GetAverageDeltaX() const;

	/// Gets the number of points stored in this object.
	/// \returns The number of points stored in this object.
	size_t GetNumberOfPoints() const { return mXData.size(); }

	/// \name Private data accessors
	/// @{

	const std::vector<DatasetType>& GetX() const { return mXData; };
	const std::vector<DatasetType>& GetY() const { return mYData; };
	std::vector<DatasetType>& GetX() { return mXData; };
	std::vector<DatasetType>& GetY() { return mYData; };

	/// @}

	/// \name Overloaded operators
	/// @{

	Dataset2D& operator+=(const Dataset2D &target);
	Dataset2D& operator-=(const Dataset2D &target);
	Dataset2D& operator*=(const Dataset2D &target);
	Dataset2D& operator/=(const Dataset2D &target);

	const Dataset2D operator+(const Dataset2D &target) const;
	const Dataset2D operator-(const Dataset2D &target) const;
	const Dataset2D operator*(const Dataset2D &target) const;
	const Dataset2D operator/(const Dataset2D &target) const;

	Dataset2D& operator+=(const DatasetType &target);
	Dataset2D& operator-=(const DatasetType &target);
	Dataset2D& operator*=(const DatasetType &target);
	Dataset2D& operator/=(const DatasetType &target);

	const Dataset2D operator+(const DatasetType &target) const;
	const Dataset2D operator-(const DatasetType &target) const;
	const Dataset2D operator*(const DatasetType &target) const;
	const Dataset2D operator/(const DatasetType &target) const;
	const Dataset2D operator%(const DatasetType &target) const;

	/// @}

private:
	std::vector<DatasetType> mXData, mYData;
};

#endif// DATASET_H_
