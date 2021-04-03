// File:  videoMaker.cpp
// Date:  3/1/2021
// Auth:  K. Loux
// Desc:  Tool for turning sonograms into videos.

// Local headers
#include "videoMaker.h"
#include "videoEncoder.h"
#include "audioEncoder.h"
#include "muxer.h"

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
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// Standard C++ headers
#include <sstream>
#include <iostream>
#include <fstream>

const double VideoMaker::frameRate(30.0);// [Hz]
const int VideoMaker::footerHeight(24);
const int VideoMaker::xAxisHeight(20);
const int VideoMaker::yAxisWidth(20);

wxImage VideoMaker::PrepareSonogram(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
	const std::set<SonogramGenerator::MagnitudeColor>& colorMap, wxImage& footer) const
{
	const unsigned int sonogramWidth(width - yAxisWidth);
	const unsigned int sonogramWithXAxisHeight(height - footerHeight);
	
	// Create the sonogram (one pixel for every FFT slice)
	SonogramGenerator generator(*soundData, parameters);
	auto wholeSonogram(generator.GetImage(colorMap));
	
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
			dc.DrawLine(0, sonogramHeight - y, yAxisWidth, sonogramHeight - y);
			const auto label(wxString::Format(_T("%d"), frequency));
			const auto extents(dc.GetTextExtent(label));
			dc.DrawText(label, (yAxisWidth - extents.GetWidth()) / 2, sonogramHeight - y);
			frequency += graduationIncrement;
		}
	}

	return yAxisLabel.ConvertToImage();
}

bool VideoMaker::MakeVideo(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
	const std::set<SonogramGenerator::MagnitudeColor>& colorMap, const std::string& fileName)
{
	wxInitAllImageHandlers();

	wxImage footer;
	const auto wholeSonogram(PrepareSonogram(soundData, parameters, colorMap, footer));
	const auto yAxisLabel(CreateYAxisLabel(parameters));
	
	wxImage baseFrame(width, height);
	baseFrame.SetRGB(wxRect(0, 0, baseFrame.GetWidth(), baseFrame.GetHeight()), 255, 255, 255);
	baseFrame.Paste(yAxisLabel, 0, xAxisHeight);
	baseFrame.Paste(footer, 0, wholeSonogram.GetHeight());

	auto maskedFooter(footer);
	const unsigned char grey(200);
	const unsigned char alpha(50);
	for (int x = 0; x < maskedFooter.GetWidth(); ++x)
	{
		for (int y = 0; y < maskedFooter.GetHeight(); ++y)
		{
			unsigned char r(maskedFooter.GetRed(x, y));
			unsigned char g(maskedFooter.GetGreen(x, y));
			unsigned char b(maskedFooter.GetBlue(x, y));
			ComputeMaskedColor(grey, alpha, r, g, b);
			maskedFooter.SetRGB(x, y, r, g, b);
		}
	}

	std::ostream& errorStream(std::cerr);

	Muxer muxer(errorStream);
	if (!muxer.Initialize("mp4", fileName))
		return false;

	assert(muxer.GetVideoCodec() != AV_CODEC_ID_NONE);
	assert(muxer.GetAudioCodec() != AV_CODEC_ID_NONE);

	// TODO:  Allow user to specify video and audio bitrates?
	VideoEncoder videoEncoder(errorStream);
	if (!videoEncoder.Initialize(muxer.GetOutputFormatContext(), width, height, frameRate, 130000, AV_PIX_FMT_YUV420P, muxer.GetVideoCodec()))
		return false;

	AudioEncoder audioEncoder(errorStream);
	if (!audioEncoder.Initialize(muxer.GetOutputFormatContext(), 1, soundData->GetSampleRate(), 64000, AV_SAMPLE_FMT_FLTP, muxer.GetAudioCodec()))
		return false;

	std::queue<AVPacket> encodedVideo;
	std::queue<AVPacket> encodedAudio;
	if (!muxer.AddStream(videoEncoder, encodedVideo) || !muxer.AddStream(audioEncoder, encodedAudio))
		return false;

	if (!muxer.WriteHeader())
		return false;

	// Encode the video
	double time(0.0);
	const double secondsPerPixel(soundData->GetDuration() / (wholeSonogram.GetWidth() - width + yAxisWidth));
	const auto lineColor(SonogramGenerator::ComputeContrastingMarkerColor(colorMap));
	while (true)
	{
		if (time <= soundData->GetDuration())
		{
			const auto image(GetFrameImage(wholeSonogram, baseFrame, maskedFooter, time, secondsPerPixel, lineColor));
			ImageToAVFrame(image, videoEncoder.rgbFrame);
			time += 1.0 / frameRate;

			if (!videoEncoder.ConvertFrame())
			{
				FreeQueuedPackets(encodedVideo);
				return false;
			}
		}
		else
			videoEncoder.inputFrame = nullptr;// Begin flushing
			
		AVPacket packet;
		av_init_packet(&packet);
		const auto status(videoEncoder.Encode(packet));
		if (status == Encoder::Status::Error)
		{
			av_packet_unref(&packet);
			FreeQueuedPackets(encodedVideo);
			return false;
		}
		
		if (status == Encoder::Status::HavePacket)
			encodedVideo.push(packet);
		else
			av_packet_unref(&packet);
		
		if (status == Encoder::Status::Done)
			break;
	}

	// Encode the audio
	unsigned int startSample(0);
	while (true)
	{
		if (startSample <= soundData->GetData().GetNumberOfPoints())
		{
			SoundToAVFrame(startSample, *soundData, audioEncoder.GetFrameSize(), audioEncoder.inputFrame);
			startSample += audioEncoder.GetFrameSize();
		}
		else
			audioEncoder.inputFrame = nullptr;// Begin flushing

		AVPacket packet;
		av_init_packet(&packet);
		const auto status(audioEncoder.Encode(packet));
		if (status == Encoder::Status::Error)
		{
			av_packet_unref(&packet);
			FreeQueuedPackets(encodedAudio);
			FreeQueuedPackets(encodedVideo);
			return false;
		}
		if (status == Encoder::Status::HavePacket)
			encodedAudio.push(packet);
		else
			av_packet_unref(&packet);
			
		if (status == Encoder::Status::Done)
			break;
	}

	while (!encodedAudio.empty() || !encodedVideo.empty())
	{
		if (!muxer.WriteNextFrame())
		{
			FreeQueuedPackets(encodedAudio);
			FreeQueuedPackets(encodedVideo);
			return false;
		}
	}

	if (!muxer.WriteTrailer())
		return false;

	return true;
}

void VideoMaker::FreeQueuedPackets(std::queue<AVPacket>& q)
{
	while (!q.empty())
	{
		av_packet_unref(&q.front());
		q.pop();
	}
}

void VideoMaker::ImageToAVFrame(const wxImage& image, AVFrame*& frame) const
{
	const int align(32);
	av_image_fill_arrays(frame->data, frame->linesize, image.GetData(), AV_PIX_FMT_RGB24, width, height, align);
}

void VideoMaker::SoundToAVFrame(const unsigned int& startSample, const SoundData& soundData, const unsigned int& frameSize, AVFrame*& frame) const
{
	if (startSample + frameSize > soundData.GetData().GetNumberOfPoints())
	{
		memset(frame->data[0], 0, frameSize * sizeof(float));
		const auto valuesToCopy(soundData.GetData().GetNumberOfPoints() - startSample);
		memcpy(frame->data[0], soundData.GetData().GetY().data() + startSample, valuesToCopy * sizeof(float));
	}
	else
		memcpy(frame->data[0], soundData.GetData().GetY().data() + startSample, frameSize * sizeof(float));
}

void VideoMaker::ComputeMaskedColor(const unsigned char& grey, const unsigned char& alpha, unsigned char& r, unsigned char& g, unsigned char& b)
{
	r = alpha * grey / 255 + (255 - alpha) * r / 255;
	g = alpha * grey / 255 + (255 - alpha) * g / 255;
	b = alpha * grey / 255 + (255 - alpha) * b / 255;
}

wxImage VideoMaker::GetFrameImage(const wxImage& wholeSonogram, const wxImage& baseFrame, const wxImage& maskedFooter,
	const double& time, const double& secondsPerPixel, const wxColor& lineColor) const
{
	wxImage frame(baseFrame);
	
	// In this method, we:
	// 1.  Extract the correct portion of the sonogram image
	// 2.  Add the cursor
	// 3.  Grey-out the portions of the footer that do not correspond to the current frame
	
	const int lineWidth(1);// [px]
	
	const unsigned int sonogramWidth(width - yAxisWidth);
	const unsigned int sonogramHeight(height - footerHeight);

	const auto leftPixel(static_cast<unsigned int>(time / secondsPerPixel));
	auto image(wholeSonogram.GetSubImage(wxRect(std::min(wholeSonogram.GetWidth() - sonogramWidth, leftPixel), 0, sonogramWidth, wholeSonogram.GetHeight())));
	frame.Paste(image, yAxisWidth, 0);
	frame.SetRGB(wxRect(sonogramWidth / 2 + yAxisWidth, 0, lineWidth, wholeSonogram.GetHeight()), lineColor.Red(), lineColor.Green(), lineColor.Blue());
	
	// Grey-out the appropriate portions of the footer
	const int leftFooter(leftPixel * frame.GetWidth() / wholeSonogram.GetWidth());
	const int rightFooter(leftFooter + width * sonogramWidth / wholeSonogram.GetWidth());
	
	if (leftFooter > 0)
		frame.Paste(maskedFooter.GetSubImage(wxRect(0, 0, leftFooter, maskedFooter.GetHeight())), 0, sonogramHeight);
	
	if (rightFooter < frame.GetWidth())
		frame.Paste(maskedFooter.GetSubImage(wxRect(rightFooter, 0, maskedFooter.GetWidth() - rightFooter, maskedFooter.GetHeight())), rightFooter, sonogramHeight);

	return frame;
}
