// File:  videoMaker.cpp
// Date:  3/1/2021
// Auth:  K. Loux
// Desc:  Tool for turning sonograms into videos.

// Local headers
#include "videoMaker.h"
#include "videoEncoder.h"

// wxWidgets headers
#include <wx/image.h>
#include <wx/font.h>
#include <wx/dcmemory.h>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// Standard C++ headers
#include <sstream>

const double VideoMaker::frameRate(30.0);// [Hz]
const int VideoMaker::footerHeight(24);
const int VideoMaker::xAxisHeight(20);
const int VideoMaker::yAxisWidth(20);

wxImage VideoMaker::PrepareSonogram(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
	const std::set<SonogramGenerator::MagnitudeColor>& colorMap, wxImage& footer) const
{
	const unsigned int sonogramWidth(width - yAxisWidth);
	const unsigned int sonogramHeight(height - xAxisHeight - footerHeight);
	const unsigned int sonogramWithXAxisHeight(height - footerHeight);
	
	// Create the sonogram (one pixel for every FFT slice)
	SonogramGenerator generator(*soundData, parameters);
	auto wholeSonogram(generator.GetImage(colorMap));
	wholeSonogram.Rescale(std::max(static_cast<unsigned int>(wholeSonogram.GetWidth()), sonogramWidth), sonogramHeight, wxIMAGE_QUALITY_HIGH);
	
	// Scale the wholeSonogram for use as a footer in each frame
	footer = wxImage(wholeSonogram.GetWidth() + sonogramWidth, wholeSonogram.GetHeight());
	footer.SetRGB(wxRect(0, 0, footer.GetWidth(), footer.GetHeight()), 255, 255, 255);
	footer.Paste(wholeSonogram, sonogramWidth / 2, 0);
	footer.Rescale(width, footerHeight);
	
	// Add time scale across top of wholeSonogram
	// Also, extend beginning and end of sonogram with white so cursor stays in the middle throughout playback
	wxImage sonogramWithXAxis(wholeSonogram.GetWidth() + sonogramWidth, sonogramWithXAxisHeight);
	sonogramWithXAxis.SetRGB(wxRect(0, 0, sonogramWithXAxis.GetWidth(), sonogramWithXAxis.GetHeight()), 255, 255, 255);
	sonogramWithXAxis.Paste(wholeSonogram, sonogramWidth / 2, xAxisHeight);
	
	wxFont labelFont(xAxisHeight / 2, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	wxBitmap temp(sonogramWithXAxis);
	{
		wxMemoryDC dc;
		dc.SelectObject(temp);
		dc.SetBackground(*wxWHITE);
		dc.SetFont(labelFont);
		dc.SetTextBackground(*wxWHITE);
		dc.SetTextForeground(*wxBLACK);
		const auto pixelsPerSecond(static_cast<int>(wholeSonogram.GetWidth() / soundData->GetDuration() + 0.5));
		
		// Lines are exactly at the second mark; labels are to the left of the line
		int time(0);
		for (unsigned int x = sonogramWidth / 2; x < temp.GetWidth() - sonogramWidth / 2; x += pixelsPerSecond)
		{
			const auto label(wxString::Format(_T("%d:%02d"), time / 60, time % 60));
			const auto extents(dc.GetTextExtent(label));

			dc.DrawLine(x, 0, x, xAxisHeight);			
			dc.DrawText(label, x - extents.GetWidth() - 2, (xAxisHeight - extents.GetHeight()) / 2);
			time += 1;
		}
	}

	return temp.ConvertToImage();
}

wxImage VideoMaker::CreateYAxisLabel(const SonogramGenerator::FFTParameters& parameters)
{
	const unsigned int sonogramHeight(height - footerHeight - xAxisHeight);
	wxBitmap yAxisLabel(yAxisWidth, sonogramHeight);
	wxFont labelFont(xAxisHeight / 3, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	
	{
		wxMemoryDC dc;
		dc.SelectObject(yAxisLabel);
		dc.SetBackground(*wxWHITE);
		dc.Clear();
		dc.SetFont(labelFont);
		dc.SetTextBackground(*wxWHITE);
		dc.SetTextForeground(*wxBLACK);
		const auto kHzLabel(_T("kHz"));
		const auto kHzExtents(dc.GetTextExtent(kHzLabel));
		dc.DrawText(kHzLabel, (yAxisWidth - kHzExtents.GetWidth()) / 2, sonogramHeight - xAxisHeight / 2);
		
		// No line for zero, but we can include a line for max, so we'll draw a line at exactly the correct pixel and label below the line
		// We'll aim to have 5 graduations to nearest kHz
		const int graduations(5);
		const int graduationIncrement(static_cast<int>((parameters.maxFrequency - parameters.minFrequency) / graduations / 1000.0 + 0.5));// [kHz]
		const int pixelsPerGraduation(sonogramHeight * graduationIncrement / (parameters.maxFrequency - parameters.minFrequency) * 1000);
		
		labelFont.SetPointSize(xAxisHeight / 2);
		dc.SetFont(labelFont);
		int frequency(static_cast<int>(parameters.minFrequency / 1000 + 0.5) + graduationIncrement);// [kHz]
		for (unsigned int y = pixelsPerGraduation; y <= sonogramHeight; y += pixelsPerGraduation)
		{
			dc.DrawLine(0, y, yAxisWidth, y);
			const auto label(wxString::Format(_T("%d"), frequency));
			const auto extents(dc.GetTextExtent(label));
			dc.DrawText(label, (yAxisWidth - extents.GetWidth()) / 2, sonogramHeight - y);
			frequency += graduationIncrement;
		}
	}

	return yAxisLabel.ConvertToImage();
}

bool VideoMaker::MakeVideo(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
	const std::set<SonogramGenerator::MagnitudeColor>& colorMap)
{
	wxInitAllImageHandlers();

	wxImage footer;
	const auto wholeSonogram(PrepareSonogram(soundData, parameters, colorMap, footer));
	const auto yAxisLabel(CreateYAxisLabel(parameters));
	
	wxImage baseFrame(width, height);
	baseFrame.SetRGB(wxRect(0, 0, baseFrame.GetWidth(), baseFrame.GetHeight()), 255, 255, 255);
	baseFrame.Paste(yAxisLabel, 0, xAxisHeight);
	baseFrame.Paste(footer, 0, wholeSonogram.GetHeight());

	std::ostringstream errorStream;
	VideoEncoder encoder(errorStream);
	/*if (!encoder.InitializeEncoder(width, height, frameRate, 0, AV_PIX_FMT_YUV420P, "H264"))
		return false;*/

	// Let's try encoding the video first
	double time(0.0);
	const double secondsPerPixel(soundData->GetDuration() / (wholeSonogram.GetWidth() - width + yAxisWidth));
	const auto lineColor(SonogramGenerator::ComputeContrastingMarkerColor(colorMap));
	std::vector<AVPacket*> encodedPackets;
	int i(0);
	while (time <= soundData->GetDuration())
	{
		const auto frame(GetFrameImage(wholeSonogram, baseFrame, time, secondsPerPixel, lineColor));
		time += 1.0 / frameRate;

		frame.SaveFile(wxString::Format("/home/kerry/Projects/a/img%06d.jpg", i++));

		// TODO:  Convert to AVFrame
		/*AVFrame inputFrame;

		encodedPackets.push_back(encoder.EncodeVideo(inputFrame));*/
	}

	/*AVFormatContext &outFmtCtx;

	// write to file for debugging
	avio_open(&outFmtCtx->pb, "test.h264", AVIO_FLAG_WRITE);
	avformat_write_header(outFmtCtx, nullptr);*/

	return false;
}

wxImage VideoMaker::GetFrameImage(const wxImage& wholeSonogram, const wxImage& baseFrame,
	const double& time, const double& secondsPerPixel, const wxColor& lineColor) const
{
	wxImage frame(baseFrame);
	
	// In this method, we:
	// 1.  Extract the correct portion of the sonogram image
	// 2.  Add the cursor
	// 3.  Grey-out the portions of the footer that do not correspond to the current frame
	
	const int lineWidth(1);// [px]
	
	const unsigned int sonogramWidth(width - yAxisWidth);

	const auto leftPixel(static_cast<unsigned int>(time / secondsPerPixel));
	auto image(wholeSonogram.GetSubImage(wxRect(std::min(wholeSonogram.GetWidth() - sonogramWidth, leftPixel), 0, sonogramWidth, wholeSonogram.GetHeight())));
	frame.Paste(image, yAxisWidth, 0);
	frame.SetRGB(wxRect(sonogramWidth / 2 + yAxisWidth, 0, lineWidth, wholeSonogram.GetHeight()), lineColor.Red(), lineColor.Green(), lineColor.Blue());
	
	const int rightPixel(leftPixel + width);
	const int leftFooter(leftPixel * frame.GetWidth() / wholeSonogram.GetWidth());
	const int rightFooter(leftFooter + width * sonogramWidth / wholeSonogram.GetWidth());
	
	// Grey-out the appropriate portions of the footer
	wxBitmap temp(frame);
	{
		wxMemoryDC dc;
		dc.SelectObject(temp);
		wxBrush fill(wxColor(170, 170, 170, 80), wxBRUSHSTYLE_SOLID);
		dc.SetBrush(fill);
		dc.SetPen(wxNullPen);
		if (leftFooter > 0)
			dc.DrawRectangle(0, wholeSonogram.GetHeight(), leftFooter, footerHeight);
		if (rightFooter < frame.GetWidth())
			dc.DrawRectangle(rightFooter, wholeSonogram.GetHeight(), frame.GetWidth(), footerHeight);
	}
	return temp.ConvertToImage();
}
