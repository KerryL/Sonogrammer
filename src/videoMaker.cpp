// File:  videoMaker.cpp
// Date:  3/1/2021
// Auth:  K. Loux
// Desc:  Tool for turning sonograms into videos.

// Local headers
#include "videoMaker.h"
#include "videoEncoder.h"

// wxWidgets headers
#include <wx/image.h>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
//#include <libavcodec/avcodec.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// Standard C++ headers
#include <sstream>

const double VideoMaker::frameRate(30.0);// [Hz]

bool VideoMaker::MakeVideo(const std::unique_ptr<SoundData>& soundData, const SonogramGenerator::FFTParameters& parameters,
	const std::set<SonogramGenerator::MagnitudeColor>& colorMap, const std::string& fileName)
{
	wxInitAllImageHandlers();

	// Create the sonogram (one pixel for every FFT slice)
	SonogramGenerator generator(*soundData, parameters);
	auto wholeSonogram(generator.GetImage(colorMap));
	wholeSonogram.Rescale(std::max(static_cast<unsigned int>(wholeSonogram.GetWidth()), width), height, wxIMAGE_QUALITY_HIGH);

	std::ostringstream errorStream;
	/*VideoEncoder encoder(errorStream);
	if (!encoder.InitializeEncoder(width, height, frameRate, 0, AV_PIX_FMT_YUV420P, "H264"))
		return false;*/

	AVFormatContext* outputContext;
	avformat_alloc_output_context2(&outputContext, nullptr, nullptr, fileName.c_str());
	if (!outputContext)
	{
		errorString = "Failed to allocate output context";
		return false;
	}

	AVStream* audioStream(nullptr);
	AVStream* videoStream(nullptr);
	if (outputContext->oformat->video_codec != AV_CODEC_ID_NONE)
		videoStream = AddVideoStream(outputContext, outputContext->oformat->video_codec);
	if (outputContext->oformat->audio_codec != AV_CODEC_ID_NONE)
		audioStream = AddAudioStream(outputContext, outputContext->oformat->audio_codec);

	// Let's try encoding the video first
	double time(0.0);
	const double secondsPerPixel(soundData->GetDuration() / wholeSonogram.GetWidth());
	const auto lineColor(SonogramGenerator::ComputeContrastingMarkerColor(colorMap));
	std::vector<AVPacket*> encodedPackets;
	int i(0);
	while (time <= soundData->GetDuration())
	{
		const auto frame(GetFrameImage(wholeSonogram, time, secondsPerPixel, lineColor));
		time += 1.0 / frameRate;

		frame.SaveFile(wxString::Format("D:\\lib\\ffmpeg-3.4.2-win64-shared\\bin\\bird\\img%06d.jpg", i++));

		// TODO:  Convert to AVFrame
		/*AVFrame inputFrame;

		encodedPackets.push_back(encoder.EncodeVideo(inputFrame));*/
	}

	/*AVFormatContext &outFmtCtx;

	// write to file for debugging
	avio_open(&outFmtCtx->pb, "test.h264", AVIO_FLAG_WRITE);
	avformat_write_header(outFmtCtx, nullptr);*/

	// TODO:  At some point, would be nice to include labeled scales for x and y axes

	return false;
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

AVStream* VideoMaker::AddAudioStream(AVFormatContext* context, AVCodecID id)
{
	AVCodecContext* codecContext;
	AVStream* s(avformat_new_stream(context, 1));

	return s;
}

AVStream* VideoMaker::AddVideoStream(AVFormatContext* context, AVCodecID id)
{
}
