// File:  mainFrame.h
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Main frame for the application.

#ifndef MAIN_FRAME_H_
#define MAIN_FRAME_H_

// Local headers
#include "sonogramGenerator.h"
#include "audioRenderer.h"

// wxWidgets headers
#include <wx/wx.h>

// Standard C++ headers
#include <memory>

// Local forward declarations
class AudioFile;
class SoundData;
class Filter;
struct FilterParameters;
class StaticImage;

// wxWidgets forward declarations
class wxListBox;
class wxListEvent;

// The main frame class
class MainFrame : public wxFrame
{
public:
	MainFrame();

	void LoadFile(const wxString& fileName);

	void UpdateSonogramCursorInfo(const double& timePercent, const double& frequencyPercent);

private:
	void CreateControls();
	void SetProperties();

	std::unique_ptr<SoundData> originalSoundData;
	std::unique_ptr<SoundData> filteredSoundData;

	wxSizer* CreatePrimaryControls(wxWindow* parent);
	wxSizer* CreateFilterControls(wxWindow* parent);
	wxSizer* CreateVersionText(wxWindow* parent);
	wxSizer* CreateAudioControls(wxWindow* parent);
	wxSizer* CreateFFTControls(wxWindow* parent);
	wxSizer* CreateImageControls(wxWindow* parent);
	wxSizer* CreateVideoControls(wxWindow* parent);

	// Controls
	StaticImage* sonogramImage;

	wxTextCtrl* audioFileName;
	wxButton* openAudioFileButton;
	wxButton* openConfigFileButton;
	wxButton* saveConfigFileButton;
	wxButton* exportSonogramImageButton;

	wxButton* addFilterButton;
	wxButton* removeFilterButton;
	wxListBox* filterList;

	wxStaticText* audioDurationText;
	wxStaticText* audioSampleRateText;
	wxStaticText* audioChannelFormatText;
	wxStaticText* audioBitRateText;
	wxStaticText* audioSampleFormatText;

	wxButton* pauseButton;
	wxButton* playButton;
	wxButton* stopButton;
	wxStaticText* currentTimeText;
	wxCheckBox* includeFiltersInPlayback;

	wxSlider* resolutionSlider;
	wxStaticText* resolutionText;
	wxStaticText* timeSliceText;
	wxComboBox* windowComboBox;
	wxStaticText* rangeText;
	wxStaticText* windowSizeText;
	wxTextCtrl* overlapTextBox;
	wxCheckBox* autoUpdateWindow;

	wxTextCtrl* timeMaxText;
	wxTextCtrl* timeMinText;
	wxTextCtrl* frequencyMinText;
	wxTextCtrl* frequencyMaxText;
	wxCheckBox* logarithmicFrequencyCheckBox;
	wxButton* editColorMapButton;
	wxStaticText* cursorTimeText;
	wxStaticText* cursorFrequencyText;

	wxButton* makeVideoButton;
	wxStaticText* pixelsPerSecond;

	// The event IDs
	enum MainFrameEventID
	{
		idButtonLoadAudioFile = wxID_HIGHEST + 100,
		idButtonLoadSonogramConfig,
		idButtonSaveSonogramConfig,
		idPrimaryControl,
		idExportSonogramImage,

		idAddFilter,
		idRemoveFilter,

		idEditColorMap,

		idPauseButton,
		idPlayButton,
		idStopButton,

		idImageControl,

		idFFT,

		idMakeVideo
	};

	// Events
	void LoadAudioButtonClickedEvent(wxCommandEvent& event);
	void LoadConfigButtonClickedEvent(wxCommandEvent& event);
	void SaveConfigButtonClickedEvent(wxCommandEvent& event);
	void PrimaryTextCtrlChangedEvent(wxCommandEvent& event);
	void ExportImageButtonClickedEvent(wxCommandEvent& event);

	void AddFilterButtonClickedEvent(wxCommandEvent& event);
	void RemoveFilterButtonClickedEvent(wxCommandEvent& event);
	void FilterListDoubleClickEvent(wxCommandEvent& event);

	void ImageTextCtrlChangedEvent(wxCommandEvent& event);
	void EditColorMapButtonClickedEvent(wxCommandEvent& event);

	void FFTSettingsChangedEvent(wxCommandEvent& event);

	void PlayButtonClickedEvent(wxCommandEvent& event);
	void PauseButtonClickedEvent(wxCommandEvent& event);
	void StopButtonClickedEvent(wxCommandEvent& event);

	void OnRenderThreadInfoEvent(wxCommandEvent& event);

	void OnClose(wxCloseEvent& event);

	void MakeVideoButtonClickedEvent(wxCommandEvent& event);

	void HandleNewAudioFile();
	void UpdateAudioInformation();
	void UpdateFFTInformation();
	void UpdateFFTCalculatedInformation();
	void UpdateSonogramInformation();
	void UpdateSonogram();
	void ApplyFilters();
	void UpdateFFTResolutionLimits();

	bool ImageInformationComplete() const;
	bool GetTimeValues(double& minTime, double& maxTime) const;
	bool GetFrequencyValues(double& minFrequency, double& maxFrequency) const;

	std::unique_ptr<AudioFile> audioFile;
	std::vector<Filter> filters;
	std::vector<FilterParameters> filterParameters;

	SonogramGenerator::ColorMap colorMap;
	static wxString SerializeColorMap(const SonogramGenerator::ColorMap& colorMap);
	static SonogramGenerator::ColorMap DeserializeColorMap(const wxString& s);

	bool GetFFTParameters(SonogramGenerator::FFTParameters& parameters);

	unsigned int GetNumberOfResolutions() const;
	double GetResolution() const;
	unsigned int GetWindowSize() const;
	double GetTimeSlice() const;
	double currentTimeSlice = 0.0;

	static Filter GetFilter(const FilterParameters &parameters,
		const double &sampleRate);
	void UpdateFilterSampleRates();

	void EnableFileDependentControls();
	void DisableFileDependentControls();

	AudioRenderer audioRenderer;
	void StopPlayingAudio();

	void SetControlEnablesOnPlay();
	void SetControlEnablesOnStop();

	void UpdateAudioPosition(const float& position);

	unsigned int videoWidth = 256;
	unsigned int videoHeight = 256;

	DECLARE_EVENT_TABLE();
};

#endif// MAIN_FRAME_H_
