// File:  mainFrame.h
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Main frame for the application.

#ifndef MAIN_FRAME_H_
#define MAIN_FRAME_H_

// Local headers
#include "sonogramGenerator.h"
#include "audioRenderer.h"
#include "filterDialog.h"

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

	void UpdateSonogramCursorInfo(const double& timePercent, const double& frequencyPercent, const bool& hasFreqencyAxis);

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
	wxSizer* CreateExportControls(wxWindow* parent);

	// Controls
	StaticImage* sonogramImage;
	StaticImage* waveFormImage;

	wxTextCtrl* audioFileName;
	wxButton* openAudioFileButton;
	wxButton* openConfigFileButton;
	wxButton* saveConfigFileButton;
	wxButton* exportSonogramImageButton;

	wxButton* openRecipeButton;
	wxButton* saveRecipeButton;
	wxButton* batchProcessRecipeButton;

	wxButton* addFilterButton;
	wxButton* removeFilterButton;
	wxListBox* filterList;

	wxStaticText* audioDurationText;
	wxStaticText* audioSampleRateText;
	wxStaticText* audioChannelFormatText;
	wxStaticText* audioBitRateText;
	wxStaticText* audioSampleFormatText;

	wxComboBox* playbackDeviceComboBox;
	wxButton* pauseButton;
	wxButton* playButton;
	wxButton* stopButton;
	wxStaticText* currentTimeText;
	wxCheckBox* includeFiltersInPlayback;

	wxCheckBox* applyNormalization;
	wxTextCtrl* normalizationReferenceTimeMin;
	wxTextCtrl* normalizationReferenceTimeMax;
	wxTextCtrl* normalizationLevel;
	wxStaticText* addedGain;

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

	wxButton* exportVideoButton;
	wxButton* exportAudioButton;
	wxStaticText* pixelsPerSecond;

	// The event IDs
	enum MainFrameEventID
	{
		idButtonLoadAudioFile = wxID_HIGHEST + 100,
		idButtonLoadSonogramConfig,
		idButtonSaveSonogramConfig,
		idPrimaryControl,
		idExportSonogramImage,

		idLoadRecipe,
		idSaveRecipe,
		idBatchProcessRecipe,

		idAddFilter,
		idRemoveFilter,

		idEditColorMap,

		idPlaybackDevice,
		idPauseButton,
		idPlayButton,
		idStopButton,

		idImageControl,
		idNormalization,

		idFFT,

		idExportVideo,
		idExportAudio
	};

	// Events
	void LoadAudioButtonClickedEvent(wxCommandEvent& event);
	void LoadConfigButtonClickedEvent(wxCommandEvent& event);
	void SaveConfigButtonClickedEvent(wxCommandEvent& event);
	void PrimaryTextCtrlChangedEvent(wxCommandEvent& event);
	void ExportImageButtonClickedEvent(wxCommandEvent& event);

	void LoadRecipeButtonClickedEvent(wxCommandEvent& event);
	void SaveRecipeButtonClickedEvent(wxCommandEvent& event);
	void BatchProcessRecipeButtonClickedEvent(wxCommandEvent& event);

	void AddFilterButtonClickedEvent(wxCommandEvent& event);
	void RemoveFilterButtonClickedEvent(wxCommandEvent& event);
	void FilterListDoubleClickEvent(wxCommandEvent& event);

	void NormalizationSettingsChangedEvent(wxCommandEvent& event);

	void ImageTextCtrlChangedEvent(wxCommandEvent& event);
	void EditColorMapButtonClickedEvent(wxCommandEvent& event);

	void FFTSettingsChangedEvent(wxCommandEvent& event);

	void PlaybackDeviceChangedEvent(wxCommandEvent& event);
	void PlayButtonClickedEvent(wxCommandEvent& event);
	void PauseButtonClickedEvent(wxCommandEvent& event);
	void StopButtonClickedEvent(wxCommandEvent& event);

	void OnRenderThreadInfoEvent(wxCommandEvent& event);

	void OnClose(wxCloseEvent& event);

	void ExportVideoButtonClickedEvent(wxCommandEvent& event);
	void ExportAudioButtonClickedEvent(wxCommandEvent& event);

	bool ExportVideo(const wxString& fileName);
	bool ExportAudio(const wxString& fileName);

	void HandleNewAudioFile();
	void UpdateAudioInformation();
	void UpdateFFTInformation();
	void UpdateFFTCalculatedInformation();
	void UpdateSonogramInformation();
	void UpdateSonogram();
	void ApplyFilters();
	void ApplyNormalization(const double& targetPower);
	void UpdateFFTResolutionLimits();
	void UpdateWaveForm();

	void PopulatePlaybackDeviceList();

	bool ImageInformationComplete() const;
	bool GetTimeValues(double& minTime, double& maxTime);
	bool GetFrequencyValues(double& minFrequency, double& maxFrequency);
	bool GetNormalizationTimeValues(double& minTime, double& maxTime);
	bool GetMinMaxValues(double& minValue, double& maxValue, wxTextCtrl* minTextCtrl, wxTextCtrl* maxTextCtrl);

	std::unique_ptr<AudioFile> audioFile;
	std::vector<Filter> filters;
	std::vector<FilterParameters> filterParameters;

	SonogramGenerator::ColorMap colorMap;
	static wxString SerializeColorMap(const SonogramGenerator::ColorMap& colorMap);
	static SonogramGenerator::ColorMap DeserializeColorMap(const wxString& s);

	bool GetFFTParameters(SonogramGenerator::FFTParameters& parameters);

	unsigned int GetNumberOfResolutions();
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

	unsigned int videoWidth = 256;// [px]
	unsigned int videoHeight = 256;// [px]
	unsigned int audioBitRate = 64;// [kb/s]
	unsigned int videoBitRate = 128;// [kb/s]

	bool ValidateInputs();
	void SetTextCtrlBackground(wxTextCtrl* textCtrl, const bool& highlight);

	bool LoadRecipe(const wxString& fileName, wxString& errorString);
	bool SaveRecipe(const wxString& fileName, wxString& errorString);

	static wxString SerializeFilterParameters(const std::vector<FilterParameters>& fp);
	static std::vector<FilterParameters> DeserializeFilterParameters(const wxString& s);
	static FilterParameters DeserializeSingleFilterParameters(const wxString& s);

	static wxString GetFilterTypeString(const FilterParameters::Type& t);
	static FilterParameters::Type GetFilterTypeFromString(const wxString& s);

	DECLARE_EVENT_TABLE();
};

#endif// MAIN_FRAME_H_
