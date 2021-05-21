// File:  audioEncoderInterface.cpp
// Date:  5/19/2021
// Auth:  K. Loux
// Desc:  Helper for audio encoding to automatically generate the proper FFmpeg objects to pass to the AudioEncoder object.

// Local headers
#include "audioEncoderInterface.h"
#include "soundData.h"
#include "muxer.h"

// FFmpeg
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

// Standard C++ headers
#include <iostream>

bool AudioEncoderInterface::Encode(const std::string& outputFileName, const std::unique_ptr<SoundData>& soundData, const int& bitRate)
{
	const auto lastDot(outputFileName.find_last_of('.'));
	if (lastDot == std::string::npos)
		return false;

	std::ostream& errorStream(std::cerr);
	Muxer muxer(errorStream);
	if (!muxer.Initialize(outputFileName.substr(lastDot + 1), outputFileName))
		return false;

	assert(!muxer.GetAudioCodecs().empty());

	AudioEncoder encoder(errorStream);
	const auto codecID([&muxer]() {
		if (muxer.GetAudioCodecs().size() > 1)
			return AV_CODEC_ID_PCM_F32LE;// Chosen so we don't need to resample
		return muxer.GetAudioCodecs().front();
	}());
	if (!encoder.Initialize(muxer.GetOutputFormatContext(), 1, static_cast<int>(soundData->GetSampleRate()), bitRate, AV_SAMPLE_FMT_FLTP, codecID))
		return false;

	std::queue<AVPacket> encodedAudio;
	if (!muxer.AddStream(encoder, encodedAudio))
		return false;

	if (!muxer.WriteHeader())
		return false;

	// Encode the audio
	unsigned int startSample(0);
	while (true)
	{
		if (startSample <= soundData->GetData().GetNumberOfPoints())
		{
			SoundToAVFrame(startSample, *soundData, encoder.GetFrameSize(), encoder.inputFrame);
			startSample += encoder.GetFrameSize();
		}
		else
			encoder.inputFrame = nullptr;// Begin flushing

		AVPacket packet;
		av_init_packet(&packet);
		const auto status(encoder.Encode(packet));
		if (status == Encoder::Status::Error)
		{
			av_packet_unref(&packet);
			FreeQueuedPackets(encodedAudio);
			return false;
		}
		if (status == Encoder::Status::HavePacket)
			encodedAudio.push(packet);
		else
			av_packet_unref(&packet);

		if (status == Encoder::Status::Done)
			break;
	}

	while (!encodedAudio.empty())
	{
		if (!muxer.WriteNextFrame())
		{
			FreeQueuedPackets(encodedAudio);
			return false;
		}
	}

	if (!muxer.WriteTrailer())
		return false;

	return true;
}

// TODO:  Clean up this (and VideoMaker) to have less repeated code
void AudioEncoderInterface::FreeQueuedPackets(std::queue<AVPacket>& q)
{
	while (!q.empty())
	{
		av_packet_unref(&q.front());
		q.pop();
	}
}

void AudioEncoderInterface::SoundToAVFrame(const unsigned int& startSample, const SoundData& soundData, const unsigned int& frameSize, AVFrame*& frame) const
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
