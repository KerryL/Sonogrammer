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

/// Class for representing paired x-y data.
class Dataset2D
{
public:
	Dataset2D() = default;

	/// Constructor.
	///
	/// \param numberOfPoints Initial size of the buffers.
	explicit Dataset2D(const unsigned int &numberOfPoints);

	/// Resizes the internal buffers to hold the specified number of points.
	///
	/// \param numberOfPoints New size of internal buffers.
	void Resize(const unsigned int &numberOfPoints);

	/// Reverses the order of the data stored in this object.
	void Reverse();

	/// Computes the mean of the y-data.
	/// \returns The mean of the y-data.
	double ComputeYMean() const;

	/// Computes the average of the delta between adjacent x-data.
	/// \returns The average of the delta between adjacent x-data.
	double GetAverageDeltaX() const;

	/// Gets the number of points stored in this object.
	/// \returns The number of points stored in this object.
	unsigned int GetNumberOfPoints() const { return mXData.size(); }

	/// \name Private data accessors
	/// @{

	const std::vector<double>& GetX() const { return mXData; };
	const std::vector<double>& GetY() const { return mYData; };
	std::vector<double>& GetX() { return mXData; };
	std::vector<double>& GetY() { return mYData; };

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

	Dataset2D& operator+=(const double &target);
	Dataset2D& operator-=(const double &target);
	Dataset2D& operator*=(const double &target);
	Dataset2D& operator/=(const double &target);

	const Dataset2D operator+(const double &target) const;
	const Dataset2D operator-(const double &target) const;
	const Dataset2D operator*(const double &target) const;
	const Dataset2D operator/(const double &target) const;
	const Dataset2D operator%(const double &target) const;

	/// @}

private:
	std::vector<double> mXData, mYData;
};

#endif// DATASET_H_
