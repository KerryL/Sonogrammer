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

bool VideoMaker::MakeVideo(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
	const std::set<SonogramGenerator::MagnitudeColor>& colorMap, const std::string& fileName)
{
	wxInitAllImageHandlers();

	// Create the sonogram (one pixel for every FFT slice)
	SonogramGenerator generator(*soundData, parameters);
	auto wholeSonogram(generator.GetImage(colorMap));
	wholeSonogram.Rescale(std::max(static_cast<unsigned int>(wholeSonogram.GetWidth()), width), height, wxIMAGE_QUALITY_HIGH);

	std::ostream& errorStream(std::cerr);

	Muxer muxer(errorStream);
	if (!muxer.Initialize("mp4", fileName))
		return false;

	assert(muxer.GetVideoCodec() != AV_CODEC_ID_NONE);
	assert(muxer.GetAudioCodec() != AV_CODEC_ID_NONE);

	VideoEncoder videoEncoder(errorStream);
	if (!videoEncoder.Initialize(muxer.GetOutputFormatContext(), width, height, frameRate, AV_PIX_FMT_YUV420P, muxer.GetVideoCodec()))
		return false;

	AudioEncoder audioEncoder(errorStream);
	if (!audioEncoder.Initialize(muxer.GetOutputFormatContext(), 1, soundData->GetSampleRate(), AV_SAMPLE_FMT_FLTP, muxer.GetAudioCodec()))
		return false;

	std::queue<AVPacket> encodedVideo;
	std::queue<AVPacket> encodedAudio;
	if (!muxer.AddStream(videoEncoder, encodedVideo) || !muxer.AddStream(audioEncoder, encodedAudio))
		return false;

	if (!muxer.WriteHeader())
		return false;

	// Encode the video
	double time(0.0);
	const double secondsPerPixel(soundData->GetDuration() / wholeSonogram.GetWidth());
	const auto lineColor(SonogramGenerator::ComputeContrastingMarkerColor(colorMap));
	while (time <= soundData->GetDuration())
	{
		const auto image(GetFrameImage(wholeSonogram, time, secondsPerPixel, lineColor));
		ImageToAVFrame(image, videoEncoder.rgbFrame);
		time += 1.0 / frameRate;

		if (!videoEncoder.ConvertFrame())
		{
			FreeQueuedPackets(encodedVideo);
			return false;
		}
			
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
	}

	// Encode the audio
	unsigned int startSample(0);
	while (startSample <= soundData->GetData().GetNumberOfPoints())
	{
		SoundToAVFrame(startSample, *soundData, audioEncoder.GetFrameSize(), audioEncoder.inputFrame);
		startSample += audioEncoder.GetFrameSize();

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

wxImage VideoMaker::GetFrameImage(const wxImage& wholeSonogram, const double& time, const double& secondsPerPixel, const wxColor& lineColor) const
{
	// Use the first "width" pixels of the sonogram for every frame until the time marker is at width / 2.
	// Then extract a new section of the sonogram to keep the time marker in the middle, until we get near the end.
	// Then use the last "width" pixels of the sonogram for every frame until we reach the end.

	const double cursorOnLeftHalfThreshold(0.5 * width * secondsPerPixel);// [sec]
	const double cursorOnRightHalfThreshold(wholeSonogram.GetWidth() * secondsPerPixel - cursorOnLeftHalfThreshold);// [sec]
	const int lineWidth(1);// [px]

	int leftmostPixel;
	int linePosition;
	if (time < cursorOnLeftHalfThreshold)
	{
		leftmostPixel = 0;
		linePosition = static_cast<int>(time / secondsPerPixel - 0.5 * lineWidth);
	}
	else if (time > cursorOnRightHalfThreshold)
	{
		leftmostPixel = wholeSonogram.GetWidth() - width;
		const double centerTime((wholeSonogram.GetWidth() - 0.5 * width) * secondsPerPixel);
		linePosition = static_cast<int>((time - centerTime)/ secondsPerPixel + 0.5 * width - 0.5 * lineWidth);///// wrong
	}
	else// cursor centered
	{
		leftmostPixel = time / secondsPerPixel - width * 0.5;
		linePosition = 0.5 * (width - lineWidth);
	}

	auto image(wholeSonogram.GetSubImage(wxRect(leftmostPixel, 0, width, height)));
	image.SetRGB(wxRect(linePosition, 0, lineWidth, height), lineColor.Red(), lineColor.Green(), lineColor.Blue());

	return image;
}
