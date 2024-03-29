// File:  mainFrame.cpp
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Main frame for the application.

// Local headers
#include "mainFrame.h"
#include "sonogrammerApp.h"
#include "audioFile.h"
#include "filter.h"
#include "soundData.h"
#include "staticImage.h"
#include "colorMapDialog.h"
#include "dropTarget.h"
#include "videoMaker.h"
#include "waveFormGenerator.h"
#include "normalizer.h"
#include "audioEncoderInterface.h"
#include "radioDialog.h"

// wxWidgets headers
#include <wx/listbox.h>
#include <wx/valnum.h>
#include <wx/filename.h>
#include <wx/fileconf.h>

// SDL headers
#include <SDL_version.h>

// FFmpeg headers
extern "C"
{
#include <libavcodec/version.h>
#include <libavformat/version.h>
#include <libavutil/version.h>
#include <libswresample/version.h>
#include <libswscale/version.h>
#include <libavformat/avformat.h>
}

// Standard C++ headers
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <sstream>

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, wxEmptyString,
	wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE), audioRenderer(GetEventHandler())
{
	CreateControls();
	SetProperties();

	// Depending on linked FFmpeg versions, we may need to make these calls, or it may be depreciated and we should not call it
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
	av_register_all();
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 10, 100)
	avcodec_register_all();
#endif
}

//==========================================================================
// Class:			MainFrame
// Function:		Event Table
//
// Description:		Links GUI events with event handler functions.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_BUTTON(idButtonLoadAudioFile,				MainFrame::LoadAudioButtonClickedEvent)
	EVT_BUTTON(idButtonLoadSonogramConfig,			MainFrame::LoadConfigButtonClickedEvent)
	EVT_BUTTON(idButtonSaveSonogramConfig,			MainFrame::SaveConfigButtonClickedEvent)
	EVT_BUTTON(idLoadRecipe,						MainFrame::LoadRecipeButtonClickedEvent)
	EVT_BUTTON(idSaveRecipe,						MainFrame::SaveRecipeButtonClickedEvent)
	EVT_BUTTON(idBatchProcessRecipe,				MainFrame::BatchProcessRecipeButtonClickedEvent)
	EVT_TEXT(idPrimaryControl,						MainFrame::PrimaryTextCtrlChangedEvent)
	EVT_BUTTON(idExportSonogramImage,				MainFrame::ExportImageButtonClickedEvent)
	EVT_BUTTON(idAddFilter,							MainFrame::AddFilterButtonClickedEvent)
	EVT_BUTTON(idRemoveFilter,						MainFrame::RemoveFilterButtonClickedEvent)
	EVT_LISTBOX_DCLICK(wxID_ANY,					MainFrame::FilterListDoubleClickEvent)
	EVT_CHECKBOX(idNormalization,					MainFrame::NormalizationSettingsChangedEvent)
	EVT_TEXT(idNormalization,						MainFrame::NormalizationSettingsChangedEvent)
	EVT_COMBOBOX(idPlaybackDevice,					MainFrame::PlaybackDeviceChangedEvent)
	EVT_BUTTON(idPlayButton,						MainFrame::PlayButtonClickedEvent)
	EVT_BUTTON(idPauseButton,						MainFrame::PauseButtonClickedEvent)
	EVT_BUTTON(idStopButton,						MainFrame::StopButtonClickedEvent)
	EVT_BUTTON(idEditColorMap,						MainFrame::EditColorMapButtonClickedEvent)
	EVT_BUTTON(idExportVideo,						MainFrame::ExportVideoButtonClickedEvent)
	EVT_BUTTON(idExportAudio,						MainFrame::ExportAudioButtonClickedEvent)
	EVT_TEXT(idImageControl,						MainFrame::ImageTextCtrlChangedEvent)
	EVT_SLIDER(idFFT,								MainFrame::FFTSettingsChangedEvent)
	EVT_TEXT(idFFT,									MainFrame::FFTSettingsChangedEvent)
	EVT_COMBOBOX(idFFT,								MainFrame::FFTSettingsChangedEvent)
	EVT_COMMAND(wxID_ANY, RenderThreadInfoEvent,	MainFrame::OnRenderThreadInfoEvent)
	EVT_CLOSE(										MainFrame::OnClose)
END_EVENT_TABLE();

void MainFrame::CreateControls()
{
	wxBoxSizer *topSizer(new wxBoxSizer(wxHORIZONTAL));
	wxPanel *panel(new wxPanel(this));
	topSizer->Add(panel, wxSizerFlags().Expand().Proportion(1));

	wxBoxSizer *panelSizer(new wxBoxSizer(wxHORIZONTAL));
	panel->SetSizer(panelSizer);

	wxBoxSizer *mainSizer(new wxBoxSizer(wxHORIZONTAL));
	panelSizer->Add(mainSizer, wxSizerFlags().Expand().Border(wxALL, 5).Proportion(1));

	wxBoxSizer* leftSizer(new wxBoxSizer(wxVERTICAL));
	mainSizer->Add(leftSizer, wxSizerFlags().Expand());

	leftSizer->Add(CreatePrimaryControls(panel), wxSizerFlags().Border(wxALL, 5));
	leftSizer->Add(CreateFilterControls(panel), wxSizerFlags().Expand().Border(wxALL, 5).Proportion(1));
	leftSizer->Add(CreateVersionText(panel));

	wxBoxSizer* rightSizer(new wxBoxSizer(wxVERTICAL));
	mainSizer->Add(rightSizer, wxSizerFlags().Expand().Proportion(1));

	sonogramImage = new StaticImage(panel, *this, wxID_ANY, 600, 200, true);
	rightSizer->Add(sonogramImage, wxSizerFlags().Expand().Proportion(1));

	rightSizer->AddSpacer(5);

	const double heightRatio(0.2);
	waveFormImage = new StaticImage(panel, *this, wxID_ANY, sonogramImage->GetMinWidth(), heightRatio * sonogramImage->GetMinHeight(), false);
	rightSizer->Add(waveFormImage, wxSizerFlags().Expand().Proportion(heightRatio));

	wxBoxSizer* rightBottomSizer(new wxBoxSizer(wxHORIZONTAL));
	rightSizer->Add(rightBottomSizer);

	rightBottomSizer->Add(CreateAudioControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	rightBottomSizer->Add(CreateFFTControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	rightBottomSizer->Add(CreateImageControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	rightBottomSizer->Add(CreateExportControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());

	TransferDataToWindow();
	DisableFileDependentControls();

	SetSizerAndFit(topSizer);
}

wxSizer* MainFrame::CreatePrimaryControls(wxWindow* parent)
{
	wxSizer* sizer(new wxBoxSizer(wxVERTICAL));

	const int padding(3);
	wxSizer* fileNameSizer(new wxBoxSizer(wxHORIZONTAL));
	wxFlexGridSizer* configSizer(new wxFlexGridSizer(3, padding, padding));
	sizer->Add(fileNameSizer, wxSizerFlags().Expand());
	sizer->Add(configSizer, wxSizerFlags().Expand());

	configSizer->SetFlexibleDirection(wxHORIZONTAL);
	configSizer->AddGrowableCol(0);
	configSizer->AddGrowableCol(1);
	configSizer->AddGrowableCol(2);

	audioFileName = new wxTextCtrl(parent, idPrimaryControl);
	openAudioFileButton = new wxButton(parent, idButtonLoadAudioFile, _T("Open"));
	fileNameSizer->Add(new wxStaticText(parent, wxID_ANY, _T("Audio File Name")), wxSizerFlags().Border(wxALL, 5));
	fileNameSizer->Add(audioFileName, wxSizerFlags().Expand().Border(wxALL, padding));
	fileNameSizer->Add(openAudioFileButton, wxSizerFlags().Border(wxALL, padding));

	openConfigFileButton = new wxButton(parent, idButtonLoadSonogramConfig, _T("Load Config"));
	saveConfigFileButton = new wxButton(parent, idButtonSaveSonogramConfig, _T("Save Config"));
	exportSonogramImageButton = new wxButton(parent, idExportSonogramImage, _T("Export Sonogram"));
	configSizer->Add(openConfigFileButton, wxSizerFlags().Expand());
	configSizer->Add(saveConfigFileButton, wxSizerFlags().Expand());
	configSizer->Add(exportSonogramImageButton, wxSizerFlags().Expand());

	openRecipeButton = new wxButton(parent, idLoadRecipe, _T("Load Recipe"));
	saveRecipeButton = new wxButton(parent, idSaveRecipe, _T("Save Recipe"));
	batchProcessRecipeButton = new wxButton(parent, idBatchProcessRecipe, _T("Batch Recipe"));
	configSizer->Add(openRecipeButton, wxSizerFlags().Expand());
	configSizer->Add(saveRecipeButton, wxSizerFlags().Expand());
	configSizer->Add(batchProcessRecipeButton, wxSizerFlags().Expand());

	return sizer;
}

wxSizer* MainFrame::CreateFilterControls(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("Filters")));
	wxBoxSizer* buttonSizer(new wxBoxSizer(wxHORIZONTAL));
	sizer->Add(buttonSizer, wxSizerFlags().Expand());
	addFilterButton = new wxButton(sizer->GetStaticBox(), idAddFilter, _T("Add"));
	removeFilterButton = new wxButton(sizer->GetStaticBox(), idRemoveFilter, _T("Remove"));

	buttonSizer->AddStretchSpacer();
	buttonSizer->Add(addFilterButton, wxSizerFlags().Border(wxRIGHT | wxLEFT, 5));
	buttonSizer->Add(removeFilterButton, wxSizerFlags().Border(wxRIGHT, 5));

	filterList = new wxListBox(sizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize);
	sizer->Add(filterList, wxSizerFlags().Expand().Proportion(1).Border(wxALL, 5));

	return sizer;
}

wxSizer* MainFrame::CreateVersionText(wxWindow* parent)
{
	SDL_version compiledVersion;
	SDL_version linkedVersion;
	SDL_VERSION(&compiledVersion);
	SDL_GetVersion(&linkedVersion);

	wxBoxSizer* sizer(new wxBoxSizer(wxVERTICAL));
	wxString versionString;
	versionString << SonogrammerApp::versionString << " (" << SonogrammerApp::gitHash << ")\n";
	versionString << LIBAVCODEC_IDENT << '\n';
	versionString << LIBAVFORMAT_IDENT << '\n';
	versionString << LIBAVUTIL_IDENT << '\n';
	versionString << LIBSWRESAMPLE_IDENT << '\n';
	versionString << LIBSWSCALE_IDENT << '\n';
	versionString << "SDL v" << static_cast<int>(compiledVersion.major) << "."
		<< static_cast<int>(compiledVersion.minor) << "."
		<< static_cast<int>(compiledVersion.patch) << " (compiled); "
		<< static_cast<int>(linkedVersion.major) << "."
		<< static_cast<int>(linkedVersion.minor) << "."
		<< static_cast<int>(linkedVersion.patch) << " (linked)";
	wxStaticText* versionText(new wxStaticText(parent, wxID_ANY, versionString));
	versionText->SetToolTip(_T("This software uses libraries from the FFmpeg project under the LGPLv2.1 license and libraries from the SDL project under the zlib license"));
	sizer->Add(versionText, wxSizerFlags().Border(wxALL, 3));
	return sizer;
}

wxSizer* MainFrame::CreateAudioControls(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("Audio")));

	auto playbackSizer(new wxBoxSizer(wxHORIZONTAL));
	sizer->Add(playbackSizer);
	playbackDeviceComboBox = new wxComboBox(sizer->GetStaticBox(), idPlaybackDevice);
	playbackSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Playback Device")), wxSizerFlags().Border(wxALL, 5));
	playbackSizer->Add(playbackDeviceComboBox, wxSizerFlags().Border(wxALL, 5));
	PopulatePlaybackDeviceList();

	wxBoxSizer* buttonSizer(new wxBoxSizer(wxHORIZONTAL));
	sizer->Add(buttonSizer);

	pauseButton = new wxButton(sizer->GetStaticBox(), idPauseButton, _T("Pause"));
	playButton = new wxButton(sizer->GetStaticBox(), idPlayButton, _T("Play"));
	stopButton = new wxButton(sizer->GetStaticBox(), idStopButton, _T("Stop"));
	currentTimeText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxString());
	includeFiltersInPlayback = new wxCheckBox(sizer->GetStaticBox(), wxID_ANY, _T("Include Filters in Playback"));
	includeFiltersInPlayback->SetValue(true);
	applyNormalization = new wxCheckBox(sizer->GetStaticBox(), idNormalization, _T("Normalize Audio"));
	applyNormalization->SetValue(true);

	normalizationReferenceTimeMin = new wxTextCtrl(sizer->GetStaticBox(), idNormalization);
	normalizationReferenceTimeMax = new wxTextCtrl(sizer->GetStaticBox(), idNormalization);
	normalizationLevel = new wxTextCtrl(sizer->GetStaticBox(), idNormalization, _T("-3"));
	addedGain = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxEmptyString);

	buttonSizer->Add(pauseButton, wxSizerFlags().Border(wxALL, 5));
	buttonSizer->Add(playButton, wxSizerFlags().Border(wxALL, 5));
	buttonSizer->Add(stopButton, wxSizerFlags().Border(wxALL, 5));

	wxSizer* timeSizer(new wxBoxSizer(wxHORIZONTAL));
	sizer->Add(timeSizer, wxSizerFlags().Border(wxALL, 5));
	timeSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Position")));
	timeSizer->Add(currentTimeText, wxSizerFlags().Border(wxLEFT, 5));

	sizer->Add(includeFiltersInPlayback, wxSizerFlags().Border(wxALL, 5));
	sizer->Add(applyNormalization, wxSizerFlags().Border(wxALL, 5));

	wxFlexGridSizer* normalizationSizer(new wxFlexGridSizer(3, 5, 5));
	sizer->Add(normalizationSizer, wxSizerFlags().Border(wxALL, 5));
	normalizationSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Normalize to")));
	normalizationSizer->Add(normalizationLevel);
	normalizationSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("dB")));
	normalizationSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Added Gain")));
	normalizationSizer->Add(addedGain);
	normalizationSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("dB")));
	normalizationSizer->AddStretchSpacer();
	normalizationSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Min")));
	normalizationSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Max")));
	normalizationSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Reference Time (sec)")));
	normalizationSizer->Add(normalizationReferenceTimeMin);
	normalizationSizer->Add(normalizationReferenceTimeMax);

	wxFlexGridSizer* audioInfoSizer(new wxFlexGridSizer(2, wxSize(5, 5)));
	sizer->Add(audioInfoSizer, wxSizerFlags().Border(wxALL, 5));

	audioDurationText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T(""));
	audioSampleRateText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T(""));
	audioChannelFormatText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T(""));
	audioSampleFormatText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T(""));
	audioBitRateText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T(""));

	audioInfoSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Duration")));
	audioInfoSizer->Add(audioDurationText);
	audioInfoSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Sample Rate")));
	audioInfoSizer->Add(audioSampleRateText);
	audioInfoSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Channel Format")));
	audioInfoSizer->Add(audioChannelFormatText);
	audioInfoSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Sample Format")));
	audioInfoSizer->Add(audioSampleFormatText);
	audioInfoSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Bit Rate")));
	audioInfoSizer->Add(audioBitRateText);

	return sizer;
}

wxSizer* MainFrame::CreateExportControls(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("Export")));
	wxFlexGridSizer* innerSizer(new wxFlexGridSizer(2, wxSize(5, 5)));
	sizer->Add(innerSizer, wxSizerFlags().Border(wxALL, 5));

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Video Width (px)")));
	innerSizer->Add(new wxTextCtrl(sizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0L, wxMakeIntegerValidator(&videoWidth)));

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Video Height (px)")));
	innerSizer->Add(new wxTextCtrl(sizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0L, wxMakeIntegerValidator(&videoHeight)));
	
	auto pixPerSecLabel(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Video X Scale")));
	pixPerSecLabel->SetToolTip(_T("Based on FFT window size"));
	innerSizer->Add(pixPerSecLabel);
	pixelsPerSecond = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxEmptyString);
	innerSizer->Add(pixelsPerSecond);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Audio Bit Rate (kb/s)")));
	innerSizer->Add(new wxTextCtrl(sizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0L, wxMakeIntegerValidator(&audioBitRate)));

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Video Bit Rate (kb/s)")));
	innerSizer->Add(new wxTextCtrl(sizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0L, wxMakeIntegerValidator(&videoBitRate)));
	
	exportVideoButton = new wxButton(sizer->GetStaticBox(), idExportVideo, _T("Export Video"));
	exportVideoButton->Enable(false);
	innerSizer->Add(exportVideoButton, wxSizerFlags().Expand());
	innerSizer->AddStretchSpacer();

	exportAudioButton = new wxButton(sizer->GetStaticBox(), idExportAudio, _T("Export Audio"));
	exportAudioButton->Enable(false);
	innerSizer->Add(exportAudioButton, wxSizerFlags().Expand());

	return sizer;
}

wxSizer* MainFrame::CreateFFTControls(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("FFT")));
	wxFlexGridSizer* innerSizer(new wxFlexGridSizer(2, wxSize(5,5)));
	sizer->Add(innerSizer, wxSizerFlags().Border(wxALL, 5));

	resolutionSlider = new wxSlider(sizer->GetStaticBox(), idFFT, 1, 0, 1, wxDefaultPosition, wxDefaultSize);
	resolutionText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxString());
	timeSliceText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxString());

	wxArrayString windowChoices;
	windowChoices.resize(static_cast<unsigned int>(FastFourierTransform::WindowType::Count));
	unsigned int i;
	for (i = 0; i < windowChoices.size(); ++i)
		windowChoices[i] = FastFourierTransform::GetWindowName(static_cast<FastFourierTransform::WindowType>(i));

	windowComboBox = new wxComboBox(sizer->GetStaticBox(), idFFT,
		FastFourierTransform::GetWindowName(FastFourierTransform::WindowType::Hann), wxDefaultPosition, wxDefaultSize, windowChoices, wxCB_READONLY);

	rangeText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxString());
	windowSizeText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxString());
	overlapTextBox = new wxTextCtrl(sizer->GetStaticBox(), idFFT, _T("0.7"));
	autoUpdateWindow = new wxCheckBox(sizer->GetStaticBox(), wxID_ANY, _T("Auto-update Time Slice"));
	autoUpdateWindow->SetValue(true);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Resolution (Hz)")));
	innerSizer->Add(resolutionSlider, wxSizerFlags().Expand());

	innerSizer->AddStretchSpacer();
	innerSizer->Add(resolutionText);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Window Function")));
	innerSizer->Add(windowComboBox, wxSizerFlags().Expand());

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Overlap Factor")));
	innerSizer->Add(overlapTextBox, wxSizerFlags().Expand());

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Range")));
	innerSizer->Add(rangeText);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Time Slice")));
	innerSizer->Add(timeSliceText);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Window Size")));
	innerSizer->Add(windowSizeText);
	
	innerSizer->Add(autoUpdateWindow);

	return sizer;
}

wxSizer* MainFrame::CreateImageControls(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("Sonogram")));
	wxFlexGridSizer* upperSizer(new wxFlexGridSizer(3, wxSize(5,5)));
	sizer->Add(upperSizer, wxSizerFlags().Border(wxALL, 5));

	upperSizer->AddStretchSpacer();
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Min")));
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Max")));

	timeMinText = new wxTextCtrl(sizer->GetStaticBox(), idImageControl);
	timeMaxText = new wxTextCtrl(sizer->GetStaticBox(), idImageControl);
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Time Range (sec)")));
	upperSizer->Add(timeMinText);
	upperSizer->Add(timeMaxText);

	frequencyMinText = new wxTextCtrl(sizer->GetStaticBox(), idImageControl);
	frequencyMaxText = new wxTextCtrl(sizer->GetStaticBox(), idImageControl);
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Frequency Range (Hz)")));
	upperSizer->Add(frequencyMinText);
	upperSizer->Add(frequencyMaxText);

	logarithmicFrequencyCheckBox = new wxCheckBox(sizer->GetStaticBox(), idImageControl, _T("Logarithmic Frequency Scale"));
	sizer->Add(logarithmicFrequencyCheckBox, wxSizerFlags().Border(wxALL, 5));

	editColorMapButton = new wxButton(sizer->GetStaticBox(), idEditColorMap, _T("Edit Color Map"));
	sizer->Add(editColorMapButton, wxSizerFlags().Border(wxALL, 5));

	wxGridSizer* cursoInfoSizer(new wxGridSizer(3, wxSize(5, 5)));
	sizer->Add(cursoInfoSizer, wxSizerFlags().Border(wxALL, 5));
	cursorTimeText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxString());
	cursorFrequencyText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxString());

	cursoInfoSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Cursor Position")));
	cursoInfoSizer->Add(cursorTimeText);
	cursoInfoSizer->Add(cursorFrequencyText);

	return sizer;
}

void MainFrame::SetProperties()
{
	SetTitle(SonogrammerApp::title);
	SetName(SonogrammerApp::internalName);
	Center();
	SetDropTarget(new DropTarget(*this));

	// Default color map
	colorMap.push_back(SonogramGenerator::MagnitudeColor(0.0, wxColor(255, 255, 255)));
	colorMap.push_back(SonogramGenerator::MagnitudeColor(0.75, wxColor(255, 255, 255)));
	colorMap.push_back(SonogramGenerator::MagnitudeColor(1.0, wxColor(0, 0, 0)));
	sonogramImage->SetMarkerColor(SonogramGenerator::ComputeContrastingMarkerColor(colorMap));

#ifdef __WXMSW__
	//SetIcon(wxIcon(_T("ICON_ID_MAIN"), wxBITMAP_TYPE_ICO_RESOURCE));
#elif __WXGTK__
	/*SetIcon(wxIcon(plots16_xpm));
	SetIcon(wxIcon(plots24_xpm));
	SetIcon(wxIcon(plots32_xpm));
	SetIcon(wxIcon(plots48_xpm));
	SetIcon(wxIcon(plots64_xpm));
	SetIcon(wxIcon(plots128_xpm));*/
#endif
}

void MainFrame::LoadAudioButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dialog(this, _T("Load Audio File"), wxString(), wxString(), _T(""), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dialog.ShowModal() != wxID_OK)
		return;

	LoadFile(dialog.GetPath());
}

void MainFrame::LoadFile(const wxString& fileName)
{
	audioFileName->ChangeValue(fileName);
	HandleNewAudioFile();
}

void MainFrame::LoadConfigButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dialog(this, _T("Load Configuration"), wxString(), wxString(),
		_T("Sonogram files (*.sgram)|*.sgram"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dialog.ShowModal() != wxID_OK)
		return;
	
	wxFileConfig config(wxEmptyString, wxEmptyString, dialog.GetPath());
	
	bool tempBool;
	if (config.Read(_T("audio/includeFilters"), &tempBool))
		includeFiltersInPlayback->SetValue(tempBool);
	if (config.Read(_T("audio/applyNormalization"), &tempBool))
		applyNormalization->SetValue(tempBool);
	
	wxString tempString;	
	if (config.Read(_T("audio/normalizationLevel"), &tempString))
		normalizationLevel->SetValue(tempString);
	else
		normalizationLevel->SetValue(_T("-3"));
		
	if (config.Read(_T("fft/windowFunction"), &tempString))
		windowComboBox->SetValue(tempString);
	if (config.Read(_T("fft/overlap"), &tempString))
		overlapTextBox->SetValue(tempString);
	if (config.Read(_T("fft/autoUpdateTimeSlice"), &tempBool))
		autoUpdateWindow->SetValue(tempBool);
	config.Read(_T("fft/timeSlice"), &currentTimeSlice, 0.0);
		
	if (config.Read(_T("sonogram/logarithmicFrequencyRange"), &tempBool))
		logarithmicFrequencyCheckBox->SetValue(tempBool);
	if (config.Read(_T("sonogram/colorMap"), &tempString))
		colorMap = DeserializeColorMap(tempString);
	
	long tempLong;
	config.Read(_T("video/width"), &tempLong, videoWidth);
	videoWidth = tempLong;
	config.Read(_T("video/height"), &tempLong, videoHeight);
	videoHeight = tempLong;
	config.Read(_T("video/audioBitRate"), &tempLong, audioBitRate);
	audioBitRate = tempLong;
	config.Read(_T("video/videoBitRate"), &tempLong, videoBitRate);
	videoBitRate = tempLong;
	
	TransferDataToWindow();
}

void MainFrame::SaveConfigButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dialog(this, _T("Save Configuration"), wxString(), wxString(),
		_T("Sonogram files (*.sgram)|*.sgram"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() != wxID_OK)
		return;
	
	TransferDataFromWindow();
	wxFileConfig config(wxEmptyString, wxEmptyString, dialog.GetPath());
	
	config.Write(_T("audio/includeFilters"), includeFiltersInPlayback->GetValue());
	config.Write(_T("audio/applyNormalization"), applyNormalization->GetValue());
	config.Write(_T("audio/normalizationLevel"), normalizationLevel->GetValue());
	
	config.Write(_T("fft/windowFunction"), windowComboBox->GetStringSelection());
	config.Write(_T("fft/overlap"), overlapTextBox->GetValue());
	config.Write(_T("fft/autoUpdateTimeSlice"), autoUpdateWindow->GetValue());
	if (!autoUpdateWindow->GetValue())
		config.Write(_T("fft/timeSlice"), currentTimeSlice);
	else
		config.Write(_T("fft/timeSlice"), 0.0);
		
	config.Write(_T("sonogram/logarithmicFrequencyRange"), logarithmicFrequencyCheckBox->GetValue());
	config.Write(_T("sonogram/colorMap"), SerializeColorMap(colorMap));
	
	config.Write(_T("video/width"), videoWidth);
	config.Write(_T("video/height"), videoHeight);
	config.Write(_T("video/audioBitRate"), audioBitRate);
	config.Write(_T("video/videoBitRate"), videoBitRate);
}

void MainFrame::LoadRecipeButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dialog(this, _T("Load Recipe"), wxString(), wxString(),
		_T("Sonogram recipes (*.sgRecipe)|*.sgRecipe"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dialog.ShowModal() != wxID_OK)
		return;

	wxString errorString;
	if (!LoadRecipe(dialog.GetPath(), errorString))
		wxMessageBox("Error:  " + errorString, _T("Error"));
}

void MainFrame::SaveRecipeButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dialog(this, _T("Save Recipe"), wxString(), audioFileName->GetValue().BeforeLast(wxUniChar('.')) + _T(".sgRecipe"),
		_T("Sonogram recipes (*.sgRecipe)|*.sgRecipe"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() != wxID_OK)
		return;

	wxString errorString;
	if (!SaveRecipe(dialog.GetPath(), errorString))
		wxMessageBox("Error:  " + errorString, _T("Error"));
}

void MainFrame::BatchProcessRecipeButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	enum class OutputType
	{
		Audio,
		Video
	};

	class OutputChoiceFactory : public RadioDialogItemFactory<OutputType>
	{
	public:
		size_t GetCount() const override { return 2; }
		wxString GetItemString(const unsigned int& i) const override
		{
			if (i == 0)
				return _T("Audio");
			return _T("Video");
		}

		OutputType GetItem(const unsigned int& i) const override
		{
			if (i == 0)
				return OutputType::Audio;
			return OutputType::Video;
		}
	};

	OutputChoiceFactory factory;
	RadioDialog<OutputType> avDialog(this, _T("Select Output Type"), factory);
	if (avDialog.ShowModal() != wxID_OK)
		return;

	wxFileDialog dialog(this, _T("Load Recipes"), wxString(), wxString(),
		_T("Sonogram recipes (*.sgRecipe)|*.sgRecipe"), wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	if (dialog.ShowModal() != wxID_OK)
		return;

	wxArrayString fileNames;
	dialog.GetPaths(fileNames);
	for (const auto& fn : fileNames)
	{
		wxString errorString;
		if (!LoadRecipe(fn, errorString))
		{
			if (wxMessageBox("Error:  " + errorString, _T("Error"), wxYES_NO) == wxNO)
				break;
		}

		wxString baseFileName(audioFileName->GetValue().BeforeLast(wxUniChar('.')));
		if (avDialog.GetSelection() == OutputType::Audio)
			ExportAudio(baseFileName + _T(".wav"));
		else
			ExportVideo(baseFileName + _T(".mp4"));
	}
}

bool MainFrame::LoadRecipe(const wxString& fileName, wxString& errorString)
{
	audioFile.reset();

	wxFileConfig config(wxEmptyString, wxEmptyString, fileName);

	wxString tempString;

	if (config.Read(_T("audioFileName"), &tempString))
		audioFileName->ChangeValue(tempString);
	else
	{
		errorString = _T("Failed to read 'audioFileName' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("filters"), &tempString))
	{
		filterParameters = DeserializeFilterParameters(tempString);

		filters.clear();
		filterList->Clear();
		for (const auto& fp : filterParameters)
		{
			filters.push_back(GetFilter(fp, 1.0));
			filterList->Append(FilterDialog::GetFilterNamePrefix(fp));
		}
	}
	else
	{
		errorString = _T("Failed to read 'filters' from '") + fileName + _T("'.");
		return false;
	}

	bool tempBool;
	if (config.Read(_T("audio/includeFilters"), &tempBool))
		includeFiltersInPlayback->SetValue(tempBool);
	else
	{
		errorString = _T("Failed to read 'audio/includeFilters' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("audio/applyNormalization"), &tempBool))
		applyNormalization->SetValue(tempBool);
	else
	{
		errorString = _T("Failed to read 'audio/applyNormalization' from '") + fileName + _T("'.");
		return false;
	}
	
	if (config.Read(_T("audio/normalizationLevel"), &tempString))
		normalizationLevel->SetValue(tempString);
	else
	{
		// Could be an older file - don't generate an error
		//errorString = _T("Failed to read 'audio/normalizationLevel' from '") + fileName + _T("'.");
		//return false;
		normalizationLevel->SetValue(_T("-3"));
	}

	if (config.Read(_T("fft/windowFunction"), &tempString))
		windowComboBox->SetValue(tempString);
	else
	{
		errorString = _T("Failed to read 'fft/windowFunction' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("fft/overlap"), &tempString))
		overlapTextBox->ChangeValue(tempString);
	else
	{
		errorString = _T("Failed to read 'fft/overlap' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("fft/autoUpdateTimeSlice"), &tempBool))
		autoUpdateWindow->SetValue(tempBool);
	else
	{
		errorString = _T("Failed to read 'fft/autoUpdateTimeSlice' from '") + fileName + _T("'.");
		return false;
	}

	if (!config.Read(_T("fft/timeSlice"), &currentTimeSlice, 0.0))
	{
		errorString = _T("Failed to read 'fft/timeSlice' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("sonogram/logarithmicFrequencyRange"), &tempBool))
		logarithmicFrequencyCheckBox->SetValue(tempBool);
	else
	{
		errorString = _T("Failed to read 'sonogram/logarithmicFrequencyRange' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("sonogram/colorMap"), &tempString))
		colorMap = DeserializeColorMap(tempString);
	else
	{
		errorString = _T("Failed to read 'sonogram/colorMap' from '") + fileName + _T("'.");
		return false;
	}

	long tempLong;
	if (config.Read(_T("video/width"), &tempLong, videoWidth))
		videoWidth = tempLong;
	else
	{
		errorString = _T("Failed to read 'video/width' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("video/height"), &tempLong, videoHeight))
		videoHeight = tempLong;
	else
	{
		errorString = _T("Failed to read 'video/height' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("video/audioBitRate"), &tempLong, audioBitRate))
		audioBitRate = tempLong;
	else
	{
		errorString = _T("Failed to read 'video/audioBitRate' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("video/videoBitRate"), &tempLong, videoBitRate))
		videoBitRate = tempLong;
	else
	{
		errorString = _T("Failed to read 'video/videoBitRate' from '") + fileName + _T("'.");
		return false;
	}

	if (!TransferDataToWindow())
	{
		errorString = _T("Failed to transfer data to window");
		return false;
	}
	
	LoadFile(audioFileName->GetValue());
	
	// The following must be set AFTER the file is loaded to prevent overwritting
	if (config.Read(_T("sonogram/minTime"), &tempString))
		timeMinText->ChangeValue(tempString);
	else
	{
		errorString = _T("Failed to read 'sonogram/minTime' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("sonogram/maxTime"), &tempString))
		timeMaxText->ChangeValue(tempString);
	else
	{
		errorString = _T("Failed to read 'sonogram/maxTime' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("sonogram/minFrequency"), &tempString))
		frequencyMinText->ChangeValue(tempString);
	else
	{
		errorString = _T("Failed to read 'sonogram/minFrequency' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("sonogram/maxFrequency"), &tempString))
		frequencyMaxText->ChangeValue(tempString);
	else
	{
		errorString = _T("Failed to read 'sonogram/maxFrequency' from '") + fileName + _T("'.");
		return false;
	}
	
	if (config.Read(_T("audio/minRefTime"), &tempString))
		normalizationReferenceTimeMin->ChangeValue(tempString);
	else
	{
		errorString = _T("Failed to read 'audio/minRefTime' from '") + fileName + _T("'.");
		return false;
	}

	if (config.Read(_T("audio/maxRefTime"), &tempString))
		normalizationReferenceTimeMax->ChangeValue(tempString);
	else
	{
		errorString = _T("Failed to read 'audio/maxRefTime' from '") + fileName + _T("'.");
		return false;
	}
	
	UpdateFFTInformation();
	ApplyFilters();// This happens when the file is loaded, but we need to do it again to ensure normalization is up-to-date
	UpdateSonogram();
	UpdateWaveForm();

	return true;
}

bool MainFrame::SaveRecipe(const wxString& fileName, wxString& errorString)
{
	// Like a configuration, but includes filters, audio file name, and time/frequency ranges

	if (!TransferDataFromWindow())
	{
		errorString = _T("Failed to transfer data from window.");
		return false;
	}

	wxFileConfig config(wxEmptyString, wxEmptyString, fileName);

	config.Write(_T("audioFileName"), audioFileName->GetValue());

	config.Write(_T("filters"), SerializeFilterParameters(filterParameters));

	config.Write(_T("audio/includeFilters"), includeFiltersInPlayback->GetValue());
	config.Write(_T("audio/applyNormalization"), applyNormalization->GetValue());
	config.Write(_T("audio/normalizationLevel"), normalizationLevel->GetValue());
	config.Write(_T("audio/minRefTime"), normalizationReferenceTimeMin->GetValue());
	config.Write(_T("audio/maxRefTime"), normalizationReferenceTimeMax->GetValue());

	config.Write(_T("fft/windowFunction"), windowComboBox->GetStringSelection());
	config.Write(_T("fft/overlap"), overlapTextBox->GetValue());
	config.Write(_T("fft/autoUpdateTimeSlice"), autoUpdateWindow->GetValue());
	if (!autoUpdateWindow->GetValue())
		config.Write(_T("fft/timeSlice"), currentTimeSlice);
	else
		config.Write(_T("fft/timeSlice"), 0.0);

	config.Write(_T("sonogram/logarithmicFrequencyRange"), logarithmicFrequencyCheckBox->GetValue());
	config.Write(_T("sonogram/colorMap"), SerializeColorMap(colorMap));
	config.Write(_T("sonogram/minTime"), timeMinText->GetValue());
	config.Write(_T("sonogram/maxTime"), timeMaxText->GetValue());
	config.Write(_T("sonogram/minFrequency"), frequencyMinText->GetValue());
	config.Write(_T("sonogram/maxFrequency"), frequencyMaxText->GetValue());

	config.Write(_T("video/width"), videoWidth);
	config.Write(_T("video/height"), videoHeight);
	config.Write(_T("video/audioBitRate"), audioBitRate);
	config.Write(_T("video/videoBitRate"), videoBitRate);

	return true;
}

wxString MainFrame::SerializeColorMap(const SonogramGenerator::ColorMap& colorMap)
{
	std::ostringstream ss;
	auto it(colorMap.begin());
	for (; it != colorMap.end(); ++it)
		ss << it->magnitude << ',' << it->color.GetRGB() << ';';
	return ss.str();
}

SonogramGenerator::ColorMap MainFrame::DeserializeColorMap(const wxString& s)
{
	SonogramGenerator::ColorMap map;
	
	std::string segment;
	std::istringstream ss(s.ToStdString());
	while (std::getline(ss, segment, ';'))
	{
		std::istringstream ssSegment(segment);
		std::string token;
		SonogramGenerator::MagnitudeColor entry;
		if ((ssSegment >> entry.magnitude).fail())
			break;
			
		if (ssSegment.peek() != ',')
			break;
		ssSegment.ignore();
		wxUint32 colorValue;
		if ((ssSegment >> colorValue).fail())
			break;
		entry.color.SetRGB(colorValue);
		map.push_back(entry);
	}

	return map;
}

wxString MainFrame::SerializeFilterParameters(const std::vector<FilterParameters>& fp)
{
	std::ostringstream ss;
	for (const auto& f : fp)
	{
		ss << GetFilterTypeString(f.type) << ',' << static_cast<int>(f.butterworth) << ',' << f.order << ','
			<< f.cutoffFrequency << ',' << f.dampingRatio << ',' << f.width << ',' << f.numerator << ',' << f.denominator << ';';
	}

	return ss.str();
}

std::vector<FilterParameters> MainFrame::DeserializeFilterParameters(const wxString& s)
{
	std::vector<FilterParameters> fp;
	std::istringstream ss(s.ToStdString());
	std::string singleFilterParameters;
	while (std::getline(ss, singleFilterParameters, ';'))
		fp.push_back(DeserializeSingleFilterParameters(singleFilterParameters));

	return fp;
}

FilterParameters MainFrame::DeserializeSingleFilterParameters(const wxString& s)
{
	FilterParameters fp;

	// TODO:  Return false on error
	std::istringstream ss(s.ToStdString());
	std::string token;
	if (std::getline(ss, token, ','))
		fp.type = GetFilterTypeFromString(token);

	ss >> fp.butterworth;
	ss.ignore();

	ss >> fp.order;
	ss.ignore();

	ss >> fp.cutoffFrequency;
	ss.ignore();

	ss >> fp.dampingRatio;
	ss.ignore();

	ss >> fp.width;
	ss.ignore();

	if (std::getline(ss, token, ','))
		fp.numerator = token;

	if (std::getline(ss, token, ';'))
		fp.denominator = token;
	
	return fp;
}

wxString MainFrame::GetFilterTypeString(const FilterParameters::Type& t)
{
	if (t == FilterParameters::Type::LowPass)
		return _T("LowPass");
	else if (t == FilterParameters::Type::HighPass)
		return _T("HighPass");
	else if (t == FilterParameters::Type::BandPass)
		return _T("BandPass");
	else if (t == FilterParameters::Type::BandStop)
		return _T("BandStop");
	else if (t == FilterParameters::Type::Notch)
		return _T("Notch");
	return _T("Custom");
}

FilterParameters::Type MainFrame::GetFilterTypeFromString(const wxString& s)
{
	if (s == _T("LowPass"))
		return FilterParameters::Type::LowPass;
	else if (s == _T("HighPass"))
		return FilterParameters::Type::HighPass;
	else if (s == _T("BandPass"))
		return FilterParameters::Type::BandPass;
	else if (s == _T("BandStop"))
		return FilterParameters::Type::BandStop;
	else if (s == _T("Notch"))
		return FilterParameters::Type::Notch;
	return FilterParameters::Type::Custom;
}

void MainFrame::PrimaryTextCtrlChangedEvent(wxCommandEvent& WXUNUSED(event))
{
	HandleNewAudioFile();
}

void MainFrame::ExportImageButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	assert(audioFile);

	wxFileDialog dialog(this, _T("Export Sonogram"), wxString(), wxString(),
		_T("PNG files (*.png)|*.png|JPG files (*.jpg)|*.jpg"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_CANCEL)
		return;

	sonogramImage->ExportToFile(dialog.GetPath());
}

void MainFrame::AddFilterButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	FilterDialog dialog(this);
	if (dialog.ShowModal() != wxID_OK)
		return;

	filterParameters.push_back(dialog.GetFilterParameters());
	if (audioFile)
		filters.push_back(GetFilter(filterParameters.back(), audioFile->GetSampleRate()));
	else
		filters.push_back(GetFilter(filterParameters.back(), 1.0));
	filterList->Append(dialog.GetFilterNamePrefix(filterParameters.back()));

	ApplyFilters();
	UpdateSonogram();
	UpdateWaveForm();
}

Filter MainFrame::GetFilter(const FilterParameters &parameters, const double &sampleRate)
{
	return Filter(sampleRate,
		Filter::CoefficientsFromString(std::string(parameters.numerator.mb_str())),
		Filter::CoefficientsFromString(std::string(parameters.denominator.mb_str())));
}

void MainFrame::RemoveFilterButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	wxArrayInt selections;
	filterList->GetSelections(selections);

	if (selections.size() == 0)
		return;

	selections.Sort([](int* a, int* b)
	{
		if (*a < *b)
			return 1;
		else if (*a > *b)
			return -1;
		return 0;
	});

	for (const auto& itemIndex : selections)
	{
		assert(itemIndex >= 0);
		filters.erase(filters.begin() + itemIndex);
		filterParameters.erase(filterParameters.begin() + itemIndex);
		filterList->Delete(itemIndex);
	}

	ApplyFilters();
	UpdateSonogram();
	UpdateWaveForm();
}

void MainFrame::FilterListDoubleClickEvent(wxCommandEvent& WXUNUSED(event))
{
	const int selectedIndex(filterList->GetSelection());
	if (selectedIndex == wxNOT_FOUND)
		return;

	FilterDialog dialog(this, &filterParameters[selectedIndex]);
	if (dialog.ShowModal() != wxID_OK)
		return;

	filterParameters[selectedIndex] = dialog.GetFilterParameters();
	if (audioFile)
		filters[selectedIndex] = GetFilter(filterParameters[selectedIndex], audioFile->GetSampleRate());
	else
		filters[selectedIndex] = GetFilter(filterParameters[selectedIndex], 1.0);
	filterList->Delete(selectedIndex);
	filterList->Insert(dialog.GetFilterNamePrefix(filterParameters[selectedIndex]), selectedIndex);

	ApplyFilters();
	UpdateSonogram();
	UpdateWaveForm();
}

void MainFrame::ImageTextCtrlChangedEvent(wxCommandEvent& WXUNUSED(event))
{
	if (!ValidateInputs())
		return;

	UpdateFFTInformation();
	ApplyFilters();
	UpdateSonogram();
	UpdateWaveForm();
}

void MainFrame::EditColorMapButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	ColorMapDialog dialog(this, colorMap);
	if (dialog.ShowModal() != wxID_OK)
		return;

	colorMap = dialog.GetColorMap();
	sonogramImage->SetMarkerColor(SonogramGenerator::ComputeContrastingMarkerColor(colorMap));
	assert(colorMap.size() > 1);
	UpdateSonogram();
	UpdateWaveForm();
}

void MainFrame::ApplyFilters()
{
	if (!originalSoundData)
		return;

	filteredSoundData = std::make_unique<SoundData>(*originalSoundData);
	for (auto& f : filters)
		filteredSoundData = filteredSoundData->ApplyFilter(f);

	addedGain->SetLabel(_T("0"));
	if (applyNormalization->GetValue())
	{
		double normLeveldB;
		if (normalizationLevel->GetValue().ToDouble(&normLeveldB))
			ApplyNormalization(normLeveldB);
	}
}

void MainFrame::ApplyNormalization(const double& targetPower)
{
	double startTime, endTime;
	if (!GetTimeValues(startTime, endTime))
		return;

	double normStartTime, normEndTime;
	if (!GetNormalizationTimeValues(normStartTime, normEndTime))
		return;
		
	startTime = std::max(startTime, normStartTime);
	endTime = std::min(endTime, normEndTime);
		
	if (endTime <= startTime)
		return;// Could be in the middle of typing a number

	auto segmentData(filteredSoundData->ExtractSegment(startTime, endTime));

	Normalizer normalizer;
	const auto gainFactor(normalizer.ComputeGainFactor(*segmentData, targetPower, Normalizer::Method::Peak/*AWeighted*/));
	normalizer.Normalize(*filteredSoundData, gainFactor);
	addedGain->SetLabel(wxString::Format(_T("%0.1f"), 20.0 * log10(gainFactor)));
}

void MainFrame::NormalizationSettingsChangedEvent(wxCommandEvent& WXUNUSED(event))
{
	const bool enableNormalizationControls(applyNormalization->GetValue() && !audioFileName->GetValue().IsEmpty());
	normalizationReferenceTimeMin->Enable(enableNormalizationControls);
	normalizationReferenceTimeMax->Enable(enableNormalizationControls);
	normalizationLevel->Enable(enableNormalizationControls);
	
	if (applyNormalization->GetValue())
	{
		double normLeveldB;
		if (!normalizationLevel->GetValue().ToDouble(&normLeveldB))
		{
			SetTextCtrlBackground(normalizationLevel, true);
			return;
		}
		else
			SetTextCtrlBackground(normalizationLevel, false);
	}

	ApplyFilters();
	UpdateWaveForm();
}

void MainFrame::FFTSettingsChangedEvent(wxCommandEvent& WXUNUSED(event))
{
	if (!audioFile)
		return;

	UpdateFFTCalculatedInformation();
	ApplyFilters();
	UpdateSonogram();
}

void MainFrame::UpdateFFTCalculatedInformation()
{
	resolutionText->SetLabel(wxString::Format(_T("%f Hz"), GetResolution()));
	windowSizeText->SetLabel(wxString::Format(_T("%d"), GetWindowSize()));

	double overlap;
	if (!overlapTextBox->GetValue().ToDouble(&overlap))
	{
		wxMessageBox(_T("Failed to parse overlap."));
		return;
	}
	else if (overlap < 0.0 || overlap > 1.0)
	{
		wxMessageBox(_T("Overlap must be between 0.0 and 1.0."));
		return;
	}

	currentTimeSlice = GetTimeSlice();
	timeSliceText->SetLabel(wxString::Format(_T("%0.3f sec"), currentTimeSlice));
	pixelsPerSecond->SetLabel(wxString::Format(_T("%0.0f px/sec"), 1.0 / currentTimeSlice));
}

double MainFrame::GetTimeSlice() const
{
	if (!audioFile)
		return 0.0;
		
	double overlap;
	if (!overlapTextBox->GetValue().ToDouble(&overlap))
		return 0.0;

	return static_cast<double>(GetWindowSize()) / audioFile->GetSampleRate() * overlap;
}

void MainFrame::PlayButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	SetControlEnablesOnPlay();
	if (audioRenderer.IsPaused())
	{
		audioRenderer.Resume();
		return;
	}

	double startTime, endTime;
	if (!GetTimeValues(startTime, endTime))
		return;

	if (endTime <= startTime)
	{
		wxMessageBox(_T("End time must be greater than start time."));
		return;
	}

	if (includeFiltersInPlayback->GetValue())
		audioRenderer.Play(*filteredSoundData->ExtractSegment(startTime, endTime));
	else
		audioRenderer.Play(*originalSoundData->ExtractSegment(startTime, endTime));
}

void MainFrame::PauseButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	audioRenderer.Pause();
	playButton->Enable();
}

void MainFrame::StopButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	StopPlayingAudio();
}

void MainFrame::PlaybackDeviceChangedEvent(wxCommandEvent& WXUNUSED(event))
{
	// TODO:  This is kind of a work-around to not having an implementation to detect device changes.
	// We check the list of devices every time the list changes, so if the selected device is no
	// longer available, we re-build the list in the control.
	const auto deviceList(AudioRenderer::GetPlaybackDevices());
	const auto selectedDevice(playbackDeviceComboBox->GetString(playbackDeviceComboBox->GetSelection()));
	const auto matchingItem(deviceList.find(selectedDevice.ToStdString()));
	if (matchingItem == deviceList.end())
		PopulatePlaybackDeviceList();
	else
		audioRenderer.SetPlaybackDevice(matchingItem->second);
}

void MainFrame::PopulatePlaybackDeviceList()
{
	playbackDeviceComboBox->Clear();
	const auto deviceList(AudioRenderer::GetPlaybackDevices());
	for (auto it = deviceList.begin(); it != deviceList.end(); ++it)
		playbackDeviceComboBox->Append(it->first);
	playbackDeviceComboBox->SetSelection(deviceList.begin()->second);
}

void MainFrame::StopPlayingAudio()
{
	audioRenderer.Stop();
	SetControlEnablesOnStop();
	currentTimeText->SetLabel(wxString());
}

void MainFrame::SetControlEnablesOnPlay()
{
	includeFiltersInPlayback->Enable(false);
	playButton->Enable(false);
	pauseButton->Enable();
	stopButton->Enable();

	sonogramImage->ShowTimeCursor();
}

void MainFrame::SetControlEnablesOnStop()
{
	includeFiltersInPlayback->Enable();
	playButton->Enable();
	pauseButton->Enable(false);
	stopButton->Enable(false);

	sonogramImage->HideTimeCursor();
}

void MainFrame::ExportVideoButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	if (!filteredSoundData || !ImageInformationComplete())
		return;

	SonogramGenerator::FFTParameters parameters;
	if (!GetFFTParameters(parameters))
		return;

	wxFileDialog dialog(this, _T("Export Sonogram Video"), wxString(), wxFileName::StripExtension(wxFileName::FileName(audioFileName->GetValue()).GetFullName()) + _T(".mp4"),
		_T("MP4 files (*.mp4)|*.mp4"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_CANCEL)
		return;

	ExportVideo(dialog.GetPath());
}

bool MainFrame::ExportVideo(const wxString& fileName)
{
	if (!filteredSoundData || !ImageInformationComplete())
		return false;

	SonogramGenerator::FFTParameters parameters;
	if (!GetFFTParameters(parameters))
		return false;

	TransferDataFromWindow();

	double startTime, endTime;
	if (!GetTimeValues(startTime, endTime))
		return false;

	auto segmentData(filteredSoundData->ExtractSegment(startTime, endTime));
	VideoMaker videoMaker(videoWidth, videoHeight, audioBitRate * 1000, videoBitRate * 1000);
	videoMaker.MakeVideo(segmentData, parameters, colorMap, fileName.ToStdString());

	return true;
}

void MainFrame::ExportAudioButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	if (!filteredSoundData || !ImageInformationComplete())
		return;

	SonogramGenerator::FFTParameters parameters;
	if (!GetFFTParameters(parameters))
		return;

	wxFileDialog dialog(this, _T("Export Filtered Audio"), wxString(), wxFileName::StripExtension(wxFileName::FileName(audioFileName->GetValue()).GetFullName()) + _T(".wav"),
		_T("WAV files (*.wav)|*.wav|MP3 Files (*.mp3)|*.mp3"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_CANCEL)
		return;

	ExportAudio(dialog.GetPath());
}

bool MainFrame::ExportAudio(const wxString& fileName)
{
	if (!filteredSoundData || !ImageInformationComplete())
		return false;

	SonogramGenerator::FFTParameters parameters;
	if (!GetFFTParameters(parameters))
		return false;

	TransferDataFromWindow();

	double startTime, endTime;
	if (!GetTimeValues(startTime, endTime))
		return false;

	auto segmentData(filteredSoundData->ExtractSegment(startTime, endTime));
	AudioEncoderInterface encoderInterface;
	encoderInterface.Encode(fileName.ToStdString(), segmentData, audioBitRate * 1000);

	return true;
}

void MainFrame::HandleNewAudioFile()
{
	const wxString fileName(audioFileName->GetValue());
	if (fileName.IsEmpty())
	{
		audioFile.reset();
		sonogramImage->Reset();
		DisableFileDependentControls();
		StopPlayingAudio();
		return;
	}

	if (!wxFileExists(fileName))
	{
		wxMessageBox(_T("File '") + fileName + _T("' does not exist."));
		audioFile.reset();
		sonogramImage->Reset();
		DisableFileDependentControls();
		StopPlayingAudio();
		return;
	}

	audioFile = std::make_unique<AudioFile>(std::string(fileName.c_str()));
	EnableFileDependentControls();

	UpdateAudioInformation();
	UpdateSonogramInformation();

	originalSoundData = std::make_unique<SoundData>(audioFile->GetSoundData());
	UpdateFFTInformation();
	UpdateFilterSampleRates();
	ApplyFilters();
	UpdateSonogram();
	UpdateWaveForm();
}

void MainFrame::UpdateFilterSampleRates()
{
	unsigned int i;
	for (i = 0; i < filters.size(); ++i)
		filters[i] = GetFilter(filterParameters[i], audioFile->GetSampleRate());
}

void MainFrame::UpdateAudioInformation()
{
	const unsigned int minutes(static_cast<unsigned int>(audioFile->GetDuration()) / 60);
	const double seconds(audioFile->GetDuration() - minutes * 60);
	if (minutes > 0)
		audioDurationText->SetLabel(wxString::Format(_T("%u:%2.2f"), minutes, seconds));
	else if (seconds > 0.0)
		audioDurationText->SetLabel(wxString::Format(_T("%0.2f s"), seconds));
	else
		audioDurationText->SetLabel(wxString());

	if (audioFile->GetSampleRate() > 0)
		audioSampleRateText->SetLabel(wxString::Format(_T("%u Hz"), audioFile->GetSampleRate()));
	else
		audioSampleRateText->SetLabel(wxString());

	audioChannelFormatText->SetLabel(audioFile->GetChannelFormat());
	audioSampleFormatText->SetLabel(audioFile->GetSampleFormat());

	if (audioFile->GetBitRate() > 0)
		audioBitRateText->SetLabel(wxString::Format(_T("%" PRId64 " kb/s"), audioFile->GetBitRate() / 1000));
	else
		audioBitRateText->SetLabel(wxString());
}

void MainFrame::UpdateFFTInformation()
{
	UpdateFFTResolutionLimits();
	rangeText->SetLabel(wxString::Format(_T("%0.0f Hz"), audioFile->GetSampleRate() * 0.5));

	UpdateFFTCalculatedInformation();
}

void MainFrame::UpdateFFTResolutionLimits()
{
	resolutionSlider->Enable(false);// In case we fail to set the limits, don't allow the user to move it
	if (!ImageInformationComplete() || !originalSoundData)
		return;

	double dummy, maxImageFrequency;
	if (!GetFrequencyValues(dummy, maxImageFrequency))
		return;

	const double maxAllowedResolution(std::min(originalSoundData->GetSampleRate() / 2, static_cast<float>(maxImageFrequency)));
	const unsigned int minSliderValue(static_cast<unsigned int>(ceil(log2(originalSoundData->GetSampleRate() / maxAllowedResolution) - 1.0)));

	resolutionSlider->Enable(true);
	resolutionSlider->SetMin(0);// Min, then max, then min again to prevent GTK warning
	resolutionSlider->SetMax(GetNumberOfResolutions());
	resolutionSlider->SetMin(minSliderValue);
	
	// Automatic resolution determination maintains a nice-looking image, but when making videos it can be desirable to use
	// different values (because we don't need to see the entire sonogram at once and because we might want to achieve a
	// specific scroll rate).
	if (autoUpdateWindow->GetValue() || currentTimeSlice == 0.0)
		resolutionSlider->SetValue(resolutionSlider->GetMin() + (resolutionSlider->GetMax() - resolutionSlider->GetMin()) / 2);
	else
	{
		double overlap;
		if (!overlapTextBox->GetValue().ToDouble(&overlap))
			return;

		auto idealSliderPosition(log(currentTimeSlice * audioFile->GetSampleRate() / overlap) / log(2) - 1);
		if (static_cast<int>(idealSliderPosition + 0.5) > resolutionSlider->GetMax())
		{
			wxMessageBox(_T("Warning:  Could not maintain desired time slice."), _T("Warning"));
			resolutionSlider->SetValue(resolutionSlider->GetMax());
		}
		else
			resolutionSlider->SetValue(idealSliderPosition);
	}
	currentTimeSlice = GetTimeSlice();
}

void MainFrame::UpdateSonogramInformation()
{
	timeMinText->ChangeValue(_T("0.0"));
	timeMaxText->ChangeValue(wxString::Format(_T("%f"), audioFile->GetDuration()));

	frequencyMinText->ChangeValue(_T("0.0"));
	frequencyMaxText->ChangeValue(wxString::Format(_T("%0.0f"), std::min(12000.0, audioFile->GetSampleRate() * 0.5)));

	normalizationReferenceTimeMin->ChangeValue(_T("0.0"));
	normalizationReferenceTimeMax->ChangeValue(wxString::Format(_T("%f"), audioFile->GetDuration()));
}

void MainFrame::UpdateSonogram()
{
	if (!filteredSoundData || !ImageInformationComplete())
		return;

	double startTime, endTime;
	if (!GetTimeValues(startTime, endTime))
		return;

	if (endTime <= startTime)
		return;// Could be in the middle of typing a number

	SonogramGenerator::FFTParameters parameters;
	if (!GetFFTParameters(parameters))
		return;

	auto segmentData(filteredSoundData->ExtractSegment(startTime, endTime));
	SonogramGenerator generator(*segmentData, parameters);
	sonogramImage->SetImage(generator.GetImage(colorMap));
}

void MainFrame::UpdateWaveForm()
{
	if (!filteredSoundData || !ImageInformationComplete())
		return;

	double startTime, endTime;
	if (!GetTimeValues(startTime, endTime))
		return;

	if (endTime <= startTime)
		return;// Could be in the middle of typing a number

	auto segmentData(filteredSoundData->ExtractSegment(startTime, endTime));
	WaveFormGenerator generator(*segmentData);
	waveFormImage->SetImage(generator.GetImage(waveFormImage->GetSize().GetWidth(), waveFormImage->GetSize().GetHeight(),
		SonogramGenerator::GetScaledColorFromMap(0.0, colorMap), SonogramGenerator::GetScaledColorFromMap(1.0, colorMap)));
}

bool MainFrame::GetFFTParameters(SonogramGenerator::FFTParameters& parameters)
{
	parameters.windowFunction = static_cast<FastFourierTransform::WindowType>(windowComboBox->GetSelection());
	parameters.windowSize = GetWindowSize();

	if (!overlapTextBox->GetValue().ToDouble(&parameters.overlap))
	{
		wxMessageBox(_T("Failed to parse overlap."));
		return false;
	}
	else if (parameters.overlap < 0.0 || parameters.overlap > 1.0)
	{
		//wxMessageBox(_T("Overlap must be between 0 and 1."));
		// Message handled in UpdateFFTCalcualtedInformation()
		return false;
	}

	if (!GetFrequencyValues(parameters.minFrequency, parameters.maxFrequency))
		return false;

	if (parameters.maxFrequency <= parameters.minFrequency)
		return false;// Could be in the middle of typing a number -> TODO:  Best approach may be to start a timer upon encountering an invalid number of conflicting inputs.  After 2 or 3 seconds, if not corrected (or if user starts typing in another box?), then disply the error message.

	return true;
}

bool MainFrame::ValidateInputs()
{
	bool ok(true);
	double startTime, endTime;
	if (!GetTimeValues(startTime, endTime))
		ok = false;

	double minFreq, maxFreq;
	if (!GetFrequencyValues(minFreq, maxFreq))
		ok = false;

	return ok;
}

unsigned int MainFrame::GetNumberOfResolutions()
{
	double startTime, endTime;
	if (!GetTimeValues(startTime, endTime))
		return 0;

	const double currentDuration(endTime - startTime);
	if (currentDuration <= 0.0)
		return 0;

	const unsigned int numberOfPoints(currentDuration * audioFile->GetSampleRate());
	return FastFourierTransform::GetMaxPowerOfTwo(numberOfPoints) - 1;
}

bool MainFrame::ImageInformationComplete() const
{
	return !timeMinText->GetValue().IsEmpty() &&
		!timeMaxText->GetValue().IsEmpty() &&
		!frequencyMinText->GetValue().IsEmpty() &&
		!frequencyMaxText->GetValue().IsEmpty();
}

bool MainFrame::GetTimeValues(double& minTime, double& maxTime)
{
	return GetMinMaxValues(minTime, maxTime, timeMinText, timeMaxText);
}

bool MainFrame::GetFrequencyValues(double& minFrequency, double& maxFrequency)
{
	return GetMinMaxValues(minFrequency, maxFrequency, frequencyMinText, frequencyMaxText);
}

bool MainFrame::GetNormalizationTimeValues(double& minTime, double& maxTime)
{
	return GetMinMaxValues(minTime, maxTime, normalizationReferenceTimeMin, normalizationReferenceTimeMax);
}

bool MainFrame::GetMinMaxValues(double& minValue, double& maxValue, wxTextCtrl* minTextCtrl, wxTextCtrl* maxTextCtrl)
{
	bool minOK(true), maxOK(true);
	if (!minTextCtrl->GetValue().ToDouble(&minValue))
		minOK = false;

	if (!maxTextCtrl->GetValue().ToDouble(&maxValue))
		maxOK = false;

	if (minOK && maxOK)
	{
		if (minValue >= maxValue)
		{
			minOK = false;
			maxOK = false;
		}
	}

	SetTextCtrlBackground(minTextCtrl, !minOK);
	SetTextCtrlBackground(maxTextCtrl, !maxOK);
	Refresh();

	return minOK && maxOK;
}

void MainFrame::SetTextCtrlBackground(wxTextCtrl* textCtrl, const bool& highlight)
{
	if (highlight)
		textCtrl->SetBackgroundColour(*wxYELLOW);
	else
		textCtrl->SetBackgroundColour(audioFileName->GetBackgroundColour());// Kind of a hacky way to do it, but it works
}

double MainFrame::GetResolution() const
{
	return static_cast<double>(audioFile->GetSampleRate()) / GetWindowSize();
}

unsigned int MainFrame::GetWindowSize() const
{
	return pow(2, resolutionSlider->GetValue() + 1);
}

void MainFrame::EnableFileDependentControls()
{
	exportSonogramImageButton->Enable();
	exportVideoButton->Enable();
	exportAudioButton->Enable();

	playButton->Enable();
	//pauseButton->Enable();// Gets enabled after we begin playing
	//stopButton->Enable();// Gets enabled after we begin playing

	timeMinText->Enable();
	timeMaxText->Enable();
	frequencyMinText->Enable();
	frequencyMaxText->Enable();
	normalizationReferenceTimeMin->Enable();
	normalizationReferenceTimeMax->Enable();
}

void MainFrame::DisableFileDependentControls()
{
	exportSonogramImageButton->Enable(false);
	exportVideoButton->Enable(false);
	exportAudioButton->Enable(false);

	playButton->Enable(false);
	pauseButton->Enable(false);
	stopButton->Enable(false);

	timeMinText->Enable(false);
	timeMaxText->Enable(false);
	frequencyMinText->Enable(false);
	frequencyMaxText->Enable(false);
	normalizationReferenceTimeMin->Enable(false);
	normalizationReferenceTimeMax->Enable(false);
}

void MainFrame::OnRenderThreadInfoEvent(wxCommandEvent& event)
{
	long positionAsLong(event.GetExtraLong());
	const float* positionAsFloat(reinterpret_cast<const float*>(&positionAsLong));
	switch (static_cast<AudioRenderer::InfoType>(event.GetInt()))
	{
	case AudioRenderer::InfoType::Error:
		wxMessageBox(event.GetString());
		break;

	case AudioRenderer::InfoType::Stopped:
		StopPlayingAudio();
		break;

	case AudioRenderer::InfoType::PositionUpdate:
		UpdateAudioPosition(*positionAsFloat);
		break;

	default:
		assert(false);
	}
}

void MainFrame::OnClose(wxCloseEvent& event)
{
	if (!IsActive())
		wxQueueEvent(this, new wxActivateEvent());// fix for application not closing if closed from taskbar when not focused; see https://forums.wxwidgets.org/viewtopic.php?t=43498

	StopPlayingAudio();
	event.Skip();
}

void MainFrame::UpdateAudioPosition(const float& position)
{
	double minTime, maxTime;
	if (!GetTimeValues(minTime, maxTime))
		return;

	const double adjPosition(position + minTime);

	const int minutes(static_cast<int>(adjPosition / 60.0));
	currentTimeText->SetLabel(wxString::Format(_T("%02d:%04.1f"), minutes, adjPosition - minutes * 60.0));

	if (adjPosition < minTime || adjPosition > maxTime)
		sonogramImage->UpdateTimeCursor(0.0);
	else
		sonogramImage->UpdateTimeCursor(position / (maxTime - minTime));
}

void MainFrame::UpdateSonogramCursorInfo(const double& timePercent, const double& frequencyPercent, const bool& hasFreqencyAxis)
{
	if (!audioFile || timePercent < 0.0 || frequencyPercent < 0.0)
	{
		cursorTimeText->SetLabel(wxString());
		cursorFrequencyText->SetLabel(wxString());
		return;
	}

	double minTime(0.0), maxTime(0.0);
	double minFrequency(0.0), maxFrequency(0.0);

	if (!timeMinText->GetValue().ToDouble(&minTime) ||
		!timeMaxText->GetValue().ToDouble(&maxTime) ||
		!frequencyMinText->GetValue().ToDouble(&minFrequency) ||
		!frequencyMaxText->GetValue().ToDouble(&maxFrequency))
	{
		cursorTimeText->SetLabel(wxString());
		cursorFrequencyText->SetLabel(wxString());
		return;
	}

	cursorTimeText->SetLabel(wxString::Format(_T("%f sec"), minTime + (maxTime - minTime) * timePercent));

	if (hasFreqencyAxis)
		cursorFrequencyText->SetLabel(wxString::Format(_T("%f Hz"), minFrequency + (maxFrequency - minFrequency) * frequencyPercent));
}
