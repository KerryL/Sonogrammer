// File:  waveFormGenerator.h
// Date:  4/6/2021
// Auth:  K. Loux
// Desc:  Class for generating images of waveforms.

// Local headers
#include "waveFormGenerator.h"
#include "soundData.h"

// wxWidgets headers
#include <wx/image.h>
#include <wx/dcmemory.h>

WaveFormGenerator::WaveFormGenerator(const SoundData& soundData) : soundData(soundData)
{
}

wxImage WaveFormGenerator::GetImage(const unsigned int& width, const unsigned int& height, const wxColor& backgroundColor, const wxColor& lineColor) const
{
	const unsigned int colorDepth(24);
	wxBitmap waveForm(width, height, colorDepth);

	const double pointsPerSlice(static_cast<double>(soundData.GetData().GetNumberOfPoints()) / width);// [samples/px]
	if (pointsPerSlice > 1.0)
	{
		// Create a list of points to create a polygon that describes the min/max within a slice
		std::vector<wxPoint> pointList(width * 2);
		for (unsigned int i = 0; i < width; ++i)
		{
			double minValue(0.0), maxValue(0.0);
			for (unsigned int j = 0; j < pointsPerSlice; ++j)
			{
				if (i * pointsPerSlice + j > soundData.GetData().GetNumberOfPoints())
					break;

				if (soundData.GetData().GetY()[i * pointsPerSlice + j] > maxValue)
					maxValue = soundData.GetData().GetY()[i * pointsPerSlice + j];
				else if (soundData.GetData().GetY()[i * pointsPerSlice + j] < minValue)
					minValue = soundData.GetData().GetY()[i * pointsPerSlice + j];
			}

			pointList[i] = wxPoint(i, height * 0.5 * (maxValue + 1.0));
			pointList[2 * width - i - 1] = wxPoint(i, height * 0.5 * (minValue + 1.0));
		}

		{
			wxMemoryDC dc;
			dc.SelectObject(waveForm);
			dc.SetBackground(backgroundColor);
			dc.Clear();

			wxPen pen(lineColor);
			dc.SetPen(pen);
			wxBrush brush(lineColor);
			dc.SetBrush(brush);

			dc.DrawPolygon(pointList.size(), pointList.data());
		}
	}
	else
	{
		{
			wxMemoryDC dc;
			dc.SelectObject(waveForm);
			dc.SetBackground(backgroundColor);
			dc.Clear();

			wxPen pen(lineColor);
			dc.SetPen(pen);
			wxBrush brush(lineColor);
			dc.SetBrush(brush);

			for (unsigned int x = 1; x < soundData.GetData().GetNumberOfPoints(); ++x)
				dc.DrawLine((x - 1) / pointsPerSlice + 0.5, height * 0.5 * (soundData.GetData().GetY()[x - 1] + 1.0),
					x / pointsPerSlice + 0.5, height * 0.5 * (soundData.GetData().GetY()[x] + 1.0));
		}
	}

	return waveForm.ConvertToImage();
}
