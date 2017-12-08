// File:  audioRenderer.h
// Date:  11/16/2017
// Auth:  K. Loux
// Desc:  Class for rendering audio using SDL.

#ifndef AUDIO_RENDERER_H_
#define AUDIO_RENDERER_H_

// Local headers
#include "soundData.h"

// Standard C++ headers
#include <thread>
#include <mutex>
#include <condition_variable>

// wxWidgets headers
#include <wx/event.h>

DECLARE_LOCAL_EVENT_TYPE(RenderThreadInfoEvent, -1)

/// Class for applying an arbitrary digital filter (transfer function) to data.
class AudioRenderer
{
public:
	AudioRenderer(wxEvtHandler* appEventHandler);
	~AudioRenderer();

	void Play(const SoundData& soundData);
	void Resume();
	void Pause();
	void Stop();

	bool IsPaused() const { return state == State::Paused; }

	enum class InfoType
	{
		Error,
		Stopped,
		PositionUpdate
	};

private:
	wxEvtHandler* appEventHandler;

	std::thread renderThread;
	std::mutex mutex;
	std::condition_variable stateChangeCondition;
	std::unique_ptr<SoundData> data;

	enum class State
	{
		Idle,
		Playing,
		Paused
	};

	State state = State::Idle;

	void RenderLoop();
	void SendStoppedEvent();
	void SendErrorEvent(const std::string& errorString);
	void SendPositionUpdateEvent(const float& position);
};

#endif// AUDIO_RENDERER_H_
