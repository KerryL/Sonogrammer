// File:  audioRenderer.cpp
// Date:  11/16/2017
// Auth:  K. Loux
// Desc:  Class for rendering audio using SDL.

// Local headers
#include "audioRenderer.h"

// SDL headers
#include <SDL.h>

// Standard C++ headers
#include <cassert>
#include <sstream>

DEFINE_LOCAL_EVENT_TYPE(RenderThreadInfoEvent)

AudioRenderer::AudioRenderer(wxEvtHandler* appEventHandler) : appEventHandler(appEventHandler)
{
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0)
	{
		std::ostringstream ss;
		ss << "Failed to initialize SDL:  " << SDL_GetError();
		SendErrorEvent(ss.str());
	}
}

AudioRenderer::~AudioRenderer()
{
	Stop();
	if (renderThread.joinable())
		renderThread.join();
	SDL_Quit();
}

void AudioRenderer::Play(const SoundData& soundData)
{
	std::lock_guard<std::mutex> lock(mutex);
	state = State::Playing;

	data = std::make_unique<SoundData>(soundData);

	renderThread = std::thread(&AudioRenderer::RenderLoop, this);
}

void AudioRenderer::Resume()
{
	assert(state == State::Paused);
	assert(data->GetDuration() > 0.0);
	std::lock_guard<std::mutex> lock(mutex);
	state = State::Playing;
	stateChangeCondition.notify_one();
}

void AudioRenderer::Pause()
{
	std::lock_guard<std::mutex> lock(mutex);
	state = State::Paused;
	stateChangeCondition.notify_one();
}

void AudioRenderer::Stop()
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		state = State::Idle;
		stateChangeCondition.notify_one();
	}

	if (renderThread.joinable())
		renderThread.join();
}

void AudioRenderer::RenderLoop()
{
	const int outputDeviceId(0);// TODO:  Allow user to select different device?
	
	SDL_AudioSpec desiredSpec;
	desiredSpec.callback = nullptr;
	desiredSpec.userdata = static_cast<void*>(this);
	desiredSpec.freq = static_cast<int>(data->GetSampleRate());
	desiredSpec.channels = 1;
	desiredSpec.silence = 0;
	desiredSpec.samples = static_cast<uint16_t>(data->GetData().GetNumberOfPoints());
	desiredSpec.format = AUDIO_F32;

	SDL_AudioSpec obtainedSpec;
	SDL_AudioDeviceID outputDevice(SDL_OpenAudioDevice(SDL_GetAudioDeviceName(outputDeviceId, 0),
		0, &desiredSpec, &obtainedSpec, SDL_AUDIO_ALLOW_ANY_CHANGE));
	if (outputDevice == 0)
	{
		std::ostringstream ss;
		ss << "Failed to open audio output:  " << SDL_GetError();
		SendErrorEvent(ss.str());
		return;
	}

	assert(desiredSpec.format == obtainedSpec.format);

	SDL_ClearQueuedAudio(outputDevice);
	SDL_PauseAudioDevice(outputDevice, 0);

	if (SDL_QueueAudio(outputDevice, data->GetData().GetY().data(), data->GetData().GetNumberOfPoints() * sizeof(float)) != 0)
	{
		std::ostringstream ss;
		ss << "Failed to queue audio:  " << SDL_GetError();
		SendErrorEvent(ss.str());
		return;
	}

	const float timePerByte(1.0 / (sizeof(float) * desiredSpec.freq));// [sec/byte]
	const uint32_t initialQueueSize(SDL_GetQueuedAudioSize(outputDevice));

	while (state != State::Idle)
	{
		const std::chrono::milliseconds waitDuration(200);
		std::unique_lock<std::mutex> lock(mutex);
		stateChangeCondition.wait_for(lock, waitDuration);

		if (state == State::Paused)
			SDL_PauseAudioDevice(outputDevice, 1);
		else if (state == State::Playing)
		{
			SDL_PauseAudioDevice(outputDevice, 0);
			const uint32_t queueSize(SDL_GetQueuedAudioSize(outputDevice));
			if (queueSize == 0)
				break;
			SendPositionUpdateEvent((initialQueueSize - queueSize) * timePerByte);
		}
	}

	SDL_CloseAudioDevice(outputDevice);
	SendStoppedEvent();
}

void AudioRenderer::SendStoppedEvent()
{
	wxCommandEvent* event(new wxCommandEvent(RenderThreadInfoEvent, wxID_ANY));
	event->SetInt(static_cast<int>(InfoType::Stopped));
	appEventHandler->QueueEvent(event);
}

void AudioRenderer::SendErrorEvent(const std::string& errorString)
{
	wxCommandEvent* event(new wxCommandEvent(RenderThreadInfoEvent, wxID_ANY));
	event->SetString(errorString);
	event->SetInt(static_cast<int>(InfoType::Error));
	appEventHandler->QueueEvent(event);
}

void AudioRenderer::SendPositionUpdateEvent(const float& position)
{
	assert(sizeof(float) == sizeof(long));

	wxCommandEvent* event(new wxCommandEvent(RenderThreadInfoEvent, wxID_ANY));
	event->SetExtraLong(*reinterpret_cast<const long*>(&position));
	event->SetInt(static_cast<int>(InfoType::PositionUpdate));
	appEventHandler->QueueEvent(event);
}
