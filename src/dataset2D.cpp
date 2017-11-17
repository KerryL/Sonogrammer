/*=============================================================================
                                   LibPlot2D
                       Copyright Kerry R. Loux 2011-2016

                  This code is licensed under the GPLv2 License
                    (http://opensource.org/licenses/GPL-2.0).
=============================================================================*/

// File:  dataset2D.cpp
// Date:  5/2/2011
// Auth:  K. Loux
// Desc:  Container for x and y-data series for plotting.

// Standard C++ headers
#include <fstream>
#include <utility>
#include <algorithm>
#include <cassert>
#include <numeric>

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "dataset2D.h"

//=============================================================================
// Class:			Dataset2D
// Function:		Dataset2D
//
// Description:		Constructor for the Dataset class.
//
// Input Arguments:
//		numberOfPoints = const unsigned int &
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//=============================================================================
Dataset2D::Dataset2D(const unsigned int &numberOfPoints)
{
	Resize(numberOfPoints);
}

//=============================================================================
// Class:			Dataset2D
// Function:		Reverse
//
// Description:		Reverses the order of the Y-data.  X-data remains unchanged.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//=============================================================================
void Dataset2D::Reverse()
{
	std::reverse(mYData.begin(), mYData.end());
}

//=============================================================================
// Class:			Dataset2D
// Function:		Resize
//
// Description:		Resizes the arrays.  Deletes all existing data before
//					resizing.
//
// Input Arguments:
//		numberOfPoints = const unsigned int &
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//=============================================================================
void Dataset2D::Resize(const unsigned int &numberOfPoints)
{
	mXData.resize(numberOfPoints);
	mYData.resize(numberOfPoints);
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator+=
//
// Description:		Overloaded operator (+=).
//
// Input Arguments:
//		target	= const Dataset2D& to add to this
//
// Output Arguments:
//		None
//
// Return Value:
//		Dataset2D& reference to this object
//
//=============================================================================
Dataset2D& Dataset2D::operator+=(const Dataset2D &target)
{
	assert(mYData.size() == target.mYData.size());

	unsigned int i;
	for (i = 0; i < mYData.size(); ++i)
		mYData[i] += target.mYData[i];

	return *this;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator-=
//
// Description:		Overloaded operator (-=).
//
// Input Arguments:
//		target	= const Dataset2D& to subtract from this
//
// Output Arguments:
//		None
//
// Return Value:
//		Dataset2D& reference to this object
//
//=============================================================================
Dataset2D& Dataset2D::operator-=(const Dataset2D &target)
{
	assert(mYData.size() == target.mYData.size());

	unsigned int i;
	for (i = 0; i < mYData.size(); ++i)
		mYData[i] -= target.mYData[i];

	return *this;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator*=
//
// Description:		Overloaded operator (*=).
//
// Input Arguments:
//		target	= const Dataset2D& to multiply by this
//
// Output Arguments:
//		None
//
// Return Value:
//		Dataset2D& reference to this object
//
//=============================================================================
Dataset2D& Dataset2D::operator*=(const Dataset2D &target)
{
	assert(mYData.size() == target.mYData.size());

	unsigned int i;
	for (i = 0; i < mYData.size(); ++i)
		mYData[i] *= target.mYData[i];

	return *this;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator/=
//
// Description:		Overloaded operator (/=).
//
// Input Arguments:
//		target	= const Dataset2D& divide into this
//
// Output Arguments:
//		None
//
// Return Value:
//		Dataset2D& reference to this object
//
//=============================================================================
Dataset2D& Dataset2D::operator/=(const Dataset2D &target)
{
	assert(mYData.size() == target.mYData.size());

	unsigned int i;
	for (i = 0; i < mYData.size(); ++i)
		mYData[i] /= target.mYData[i];

	return *this;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator+
//
// Description:		Overloaded operator (+).
//
// Input Arguments:
//		target	= const Dataset2D& to add to this
//
// Output Arguments:
//		None
//
// Return Value:
//		const Dataset2D& containing desired sum
//
//=============================================================================
const Dataset2D Dataset2D::operator+(const Dataset2D &target) const
{
	Dataset2D result = *this;
	result += target;

	return result;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator-
//
// Description:		Overloaded operator (-).
//
// Input Arguments:
//		target	= const Dataset2D& to subtract from this
//
// Output Arguments:
//		None
//
// Return Value:
//		const Dataset2D& containing desired difference
//
//=============================================================================
const Dataset2D Dataset2D::operator-(const Dataset2D &target) const
{
	Dataset2D result = *this;
	result -= target;

	return result;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator*
//
// Description:		Overloaded operator (*).
//
// Input Arguments:
//		target	= const Dataset2D& to multiply with this
//
// Output Arguments:
//		None
//
// Return Value:
//		const Dataset2D& containing desired product
//
//=============================================================================
const Dataset2D Dataset2D::operator*(const Dataset2D &target) const
{
	Dataset2D result = *this;
	result *= target;

	return result;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator/
//
// Description:		Overloaded operator (/).
//
// Input Arguments:
//		target	= const Dataset2D& to divide into this
//
// Output Arguments:
//		None
//
// Return Value:
//		const Dataset2D& containing desired ratio
//
//=============================================================================
const Dataset2D Dataset2D::operator/(const Dataset2D &target) const
{
	Dataset2D result = *this;
	result /= target;

	return result;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator+=
//
// Description:		Overloaded operator (+=).
//
// Input Arguments:
//		target	= const DatasetType& to add to this
//
// Output Arguments:
//		None
//
// Return Value:
//		Dataset2D& reference to this
//
//=============================================================================
Dataset2D& Dataset2D::operator+=(const DatasetType &target)
{
	for (auto& y : mYData)
		y += target;

	return *this;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator-=
//
// Description:		Overloaded operator (-=).
//
// Input Arguments:
//		target	= const DatasetType& to subract from this
//
// Output Arguments:
//		None
//
// Return Value:
//		Dataset2D& reference to this
//
//=============================================================================
Dataset2D& Dataset2D::operator-=(const DatasetType &target)
{
	for (auto& y : mYData)
		y -= target;

	return *this;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator*=
//
// Description:		Overloaded operator (*=).
//
// Input Arguments:
//		target	= const DatasetType& to multiply with this
//
// Output Arguments:
//		None
//
// Return Value:
//		Dataset2D& reference to this
//
//=============================================================================
Dataset2D& Dataset2D::operator*=(const DatasetType &target)
{
	for (auto& y : mYData)
		y *= target;

	return *this;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator/=
//
// Description:		Overloaded operator (/=).
//
// Input Arguments:
//		target	= const DatasetType& to divide into this
//
// Output Arguments:
//		None
//
// Return Value:
//		Dataset2D& reference to this
//
//=============================================================================
Dataset2D& Dataset2D::operator/=(const DatasetType &target)
{
	for (auto& y : mYData)
		y /= target;

	return *this;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator+
//
// Description:		Overloaded operator (+).
//
// Input Arguments:
//		target	= const DatasetType& to add to this
//
// Output Arguments:
//		None
//
// Return Value:
//		const Dataset2D& containing desired sum
//
//=============================================================================
const Dataset2D Dataset2D::operator+(const DatasetType &target) const
{
	Dataset2D result(*this);
	result += target;

	return result;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator-
//
// Description:		Overloaded operator (-).
//
// Input Arguments:
//		target	= const DatasetType& to subtract from this
//
// Output Arguments:
//		None
//
// Return Value:
//		const Dataset2D& containing desired difference
//
//=============================================================================
const Dataset2D Dataset2D::operator-(const DatasetType &target) const
{
	Dataset2D result(*this);
	result -= target;

	return result;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator*
//
// Description:		Overloaded operator (*).
//
// Input Arguments:
//		target	= const DatasetType& to multiply with this
//
// Output Arguments:
//		None
//
// Return Value:
//		const Dataset2D& containing desired product
//
//=============================================================================
const Dataset2D Dataset2D::operator*(const DatasetType &target) const
{
	Dataset2D result(*this);
	result *= target;

	return result;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator/
//
// Description:		Overloaded operator (/).
//
// Input Arguments:
//		target	= const DatasetType& to divide into this
//
// Output Arguments:
//		None
//
// Return Value:
//		const Dataset2D& containing desired ratio
//
//=============================================================================
const Dataset2D Dataset2D::operator/(const DatasetType &target) const
{
	Dataset2D result(*this);
	result /= target;

	return result;
}

//=============================================================================
// Class:			Dataset2D
// Function:		operator%
//
// Description:		Overloaded operator (%).
//
// Input Arguments:
//		target	= const DatasetType& to divide into this
//
// Output Arguments:
//		None
//
// Return Value:
//		const Dataset2D& containing desired ratio
//
//=============================================================================
const Dataset2D Dataset2D::operator%(const DatasetType &target) const
{
	Dataset2D result(*this);
	unsigned int i;
	for (i = 0; i < mYData.size(); ++i)
		result.mYData[i] = fmod(mYData[i], target);

	return result;
}

//=============================================================================
// Class:			Dataset2D
// Function:		ComputeYMean
//
// Description:		Computes the average of the Y-data.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		DatasetType
//
//=============================================================================
DatasetType Dataset2D::ComputeYMean() const
{
	return std::accumulate(mYData.cbegin(), mYData.cend(), static_cast<DatasetType>(0.0))
		/ mYData.size();
}

//=============================================================================
// Class:			Dataset2D
// Function:		GetAverageDeltaX
//
// Description:		Computes the average spacing of the X-values.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		DatasetType
//
//=============================================================================
DatasetType Dataset2D::GetAverageDeltaX() const
{
	DatasetType sum(0.0);
	unsigned int i;
	for (i = 1; i < mXData.size(); ++i)
		sum += mXData[i] - mXData[i - 1];

	return sum / (mXData.size() - 1.0);
}
