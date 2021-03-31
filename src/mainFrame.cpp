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
#include "filterDialog.h"
#include "colorMapDialog.h"
#include "dropTarget.h"
#include "videoMaker.h"

// wxWidgets headers
#include <wx/listbox.h>
#include <wx/valnum.h>

// SDL headers
#include <SDL_version.h>

// FFmpeg headers
extern "C"
{
#include <libavcodec/version.h>
#include <libavformat/version.h>
#include <libavutil/version.h>
#include <libswresample/version.h>
}

// Standard C++ headers
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, wxEmptyString,
	wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE), audioRenderer(GetEventHandler())
{
	CreateControls();
	SetProperties();
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
	EVT_TEXT(idPrimaryControl,						MainFrame::PrimaryTextCtrlChangedEvent)
	EVT_BUTTON(idExportSonogramImage,				MainFrame::ExportImageButtonClickedEvent)
	EVT_BUTTON(idAddFilter,							MainFrame::AddFilterButtonClickedEvent)
	EVT_BUTTON(idRemoveFilter,						MainFrame::RemoveFilterButtonClickedEvent)
	EVT_LISTBOX_DCLICK(wxID_ANY,					MainFrame::FilterListDoubleClickEvent)
	EVT_BUTTON(idPlayButton,						MainFrame::PlayButtonClickedEvent)
	EVT_BUTTON(idPauseButton,						MainFrame::PauseButtonClickedEvent)
	EVT_BUTTON(idStopButton,						MainFrame::StopButtonClickedEvent)
	EVT_BUTTON(idEditColorMap,						MainFrame::EditColorMapButtonClickedEvent)
	EVT_BUTTON(idMakeVideo,							MainFrame::MakeVideoButtonClickedEvent)
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

	sonogramImage = new StaticImage(panel, *this, wxID_ANY, 600, 200);
	rightSizer->Add(sonogramImage, wxSizerFlags().Expand().Proportion(1));

	wxBoxSizer* rightBottomSizer(new wxBoxSizer(wxHORIZONTAL));
	rightSizer->Add(rightBottomSizer);

	rightBottomSizer->Add(CreateAudioControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	rightBottomSizer->Add(CreateFFTControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	rightBottomSizer->Add(CreateImageControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	rightBottomSizer->Add(CreateVideoControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());

	TransferDataToWindow();
	DisableFileDependentControls();

	SetSizerAndFit(topSizer);
}

wxSizer* MainFrame::CreatePrimaryControls(wxWindow* parent)
{
	wxFlexGridSizer* sizer(new wxFlexGridSizer(4, wxSize(5, 5)));

	audioFileName = new wxTextCtrl(parent, idPrimaryControl);
	openAudioFileButton = new wxButton(parent, idButtonLoadAudioFile, _T("Open"));
	sizer->Add(new wxStaticText(parent, wxID_ANY, _T("Audio File Name")));
	sizer->Add(audioFileName, wxSizerFlags().Expand());
	sizer->Add(openAudioFileButton);
	sizer->AddStretchSpacer();

	sonogramConfigFileName = new wxTextCtrl(parent, wxID_ANY);
	openConfigFileButton = new wxButton(parent, idButtonLoadSonogramConfig, _T("Open"));
	saveConfigFileButton = new wxButton(parent, idButtonSaveSonogramConfig, _T("Save"));
	sizer->Add(new wxStaticText(parent, idPrimaryControl, _T("Config File Name")));
	sizer->Add(sonogramConfigFileName, wxSizerFlags().Expand());
	sizer->Add(openConfigFileButton);
	sizer->Add(saveConfigFileButton);

	exportSonogramImageButton = new wxButton(parent, idExportSonogramImage, _T("Export Sonogram"));
	sizer->Add(exportSonogramImageButton);

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
	buttonSizer->Add(removeFilterButton, wxSizerFlags().Border(wxBOTTOM, 5));

	filterList = new wxListBox(sizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize);
	sizer->Add(filterList, wxSizerFlags().Expand().Proportion(1));

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
	versionString << "SDL v" << static_cast<int>(compiledVersion.major) << "."
		<< static_cast<int>(compiledVersion.minor) << "."
		<< static_cast<int>(compiledVersion.patch) << " (compiled); "
		<< static_cast<int>(linkedVersion.major) << "."
		<< static_cast<int>(linkedVersion.minor) << "."
		<< static_cast<int>(linkedVersion.patch) << " (linked)";
	wxStaticText* versionText(new wxStaticText(parent, wxID_ANY, versionString));
	versionText->SetToolTip(_T("This software uses libraries from the FFmpeg project under the LGPLv2.1 license and libraries from the SDL project under the zlib license"));
	sizer->Add(versionText);
	return sizer;
}

wxSizer* MainFrame::CreateAudioControls(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("Audio")));
	wxBoxSizer* buttonSizer(new wxBoxSizer(wxHORIZONTAL));
	sizer->Add(buttonSizer);

	pauseButton = new wxButton(sizer->GetStaticBox(), idPauseButton, _T("Pause"));
	playButton = new wxButton(sizer->GetStaticBox(), idPlayButton, _T("Play"));
	stopButton = new wxButton(sizer->GetStaticBox(), idStopButton, _T("Stop"));
	currentTimeText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, wxString());
	includeFiltersInPlayback = new wxCheckBox(sizer->GetStaticBox(), wxID_ANY, _T("Include Filters in Playback"));

	buttonSizer->Add(pauseButton, wxSizerFlags().Border(wxALL, 5));
	buttonSizer->Add(playButton, wxSizerFlags().Border(wxALL, 5));
	buttonSizer->Add(stopButton, wxSizerFlags().Border(wxALL, 5));

	wxSizer* timeSizer(new wxBoxSizer(wxHORIZONTAL));
	sizer->Add(timeSizer, wxSizerFlags().Border(wxALL, 5));
	timeSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Position")));
	timeSizer->Add(currentTimeText, wxSizerFlags().Border(wxLEFT, 5));
	sizer->Add(includeFiltersInPlayback, wxSizerFlags().Border(wxALL, 5));

	sizer->AddSpacer(15);

	wxFlexGridSizer* audioInfoSizer(new wxFlexGridSizer(2, wxSize(5, 5)));
	sizer->Add(audioInfoSizer);

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

wxSizer* MainFrame::CreateVideoControls(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("Video")));
	wxFlexGridSizer* innerSizer(new wxFlexGridSizer(2, wxSize(5, 5)));
	sizer->Add(innerSizer);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Width")));
	innerSizer->Add(new wxTextCtrl(sizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0L, wxMakeIntegerValidator(&videoWidth)));

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Height")));
	innerSizer->Add(new wxTextCtrl(sizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0L, wxMakeIntegerValidator(&videoHeight)));

	makeVideoButton = new wxButton(sizer->GetStaticBox(), idMakeVideo, _T("Make Video"));
	makeVideoButton->Enable(false);
	innerSizer->Add(makeVideoButton);

	return sizer;
}

wxSizer* MainFrame::CreateFFTControls(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("FFT")));
	wxFlexGridSizer* innerSizer(new wxFlexGridSizer(2, wxSize(5,5)));
	sizer->Add(innerSizer);

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

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Resolution (Hz)")));
	innerSizer->Add(resolutionSlider, wxSizerFlags().Expand());

	innerSizer->AddStretchSpacer();
	innerSizer->Add(resolutionText);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Window Function")));
	innerSizer->Add(windowComboBox);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Overlap")));
	innerSizer->Add(overlapTextBox);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Range")));
	innerSizer->Add(rangeText);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Time Slize")));
	innerSizer->Add(timeSliceText);

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Window Size")));
	innerSizer->Add(windowSizeText);

	return sizer;
}

wxSizer* MainFrame::CreateImageControls(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("Sonogram")));
	wxFlexGridSizer* upperSizer(new wxFlexGridSizer(4, wxSize(5,5)));
	sizer->Add(upperSizer);

	upperSizer->AddStretchSpacer();
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Min")));
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Max")));
	upperSizer->AddStretchSpacer();

	timeMinText = new wxTextCtrl(sizer->GetStaticBox(), idImageControl);
	timeMaxText = new wxTextCtrl(sizer->GetStaticBox(), idImageControl);
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Time Range")));
	upperSizer->Add(timeMinText);
	upperSizer->Add(timeMaxText);
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("sec")));

	frequencyMinText = new wxTextCtrl(sizer->GetStaticBox(), idImageControl);
	frequencyMaxText = new wxTextCtrl(sizer->GetStaticBox(), idImageControl);
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Frequency Range")));
	upperSizer->Add(frequencyMinText);
	upperSizer->Add(frequencyMaxText);
	upperSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Hz")));

	logarithmicFrequencyCheckBox = new wxCheckBox(sizer->GetStaticBox(), idImageControl, _T("Logarithmic Frequency Scale"));
	sizer->Add(logarithmicFrequencyCheckBox, wxSizerFlags().Border(wxALL, 5));

	editColorMapButton = new wxButton(sizer->GetStaticBox(), idEditColorMap, _T("Edit Color Map"));
	sizer->Add(editColorMapButton, wxSizerFlags().Border(wxALL, 5));

	wxGridSizer* cursoInfoSizer(new wxGridSizer(3, wxSize(5, 5)));
	sizer->Add(cursoInfoSizer);
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
	colorMap.insert(SonogramGenerator::MagnitudeColor(0.0, wxColor(255, 255, 255)));
	colorMap.insert(SonogramGenerator::MagnitudeColor(1.0, wxColor(0, 0, 0)));
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
	// TODO:  Implement
}

void MainFrame::SaveConfigButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	// TODO:  Implement
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
}

void MainFrame::ImageTextCtrlChangedEvent(wxCommandEvent& WXUNUSED(event))
{
	UpdateFFTInformation();
	ApplyFilters();
	UpdateSonogram();
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
}

void MainFrame::ApplyFilters()
{
	if (!originalSoundData)
		return;

	filteredSoundData = std::make_unique<SoundData>(*originalSoundData);
	for (auto& f : filters)
		filteredSoundData = filteredSoundData->ApplyFilter(f);
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

	timeSliceText->SetLabel(wxString::Format(_T("%0.3f sec"), static_cast<double>(GetWindowSize()) / audioFile->GetSampleRate()));
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

void MainFrame::MakeVideoButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	if (!filteredSoundData || !ImageInformationComplete())
		return;

	SonogramGenerator::FFTParameters parameters;
	if (!GetFFTParameters(parameters))
		return;

	wxFileDialog dialog(this, _T("Export Sonogram Video"), wxString(), wxString(),
		_T("MP4 files (*.mp4)|*.mp4"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_CANCEL)
		return;

	VideoMaker videoMaker(videoWidth, videoHeight);
	if (!videoMaker.MakeVideo(filteredSoundData, parameters, colorMap, dialog.GetPath().ToStdString()))
		wxMessageBox(_T("Failed to generate sonogram:  ") + videoMaker.GetErrorString(), _T("Error"));
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
		audioDurationText->SetLabel(wxString::Format(_T("%u:%0.2f"), minutes, seconds));
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
	resolutionSlider->SetValue(resolutionSlider->GetMin() + (resolutionSlider->GetMax() - resolutionSlider->GetMin()) / 2);
}

void MainFrame::UpdateSonogramInformation()
{
	timeMinText->SetValue(_T("0.0"));
	timeMaxText->SetValue(wxString::Format(_T("%f"), audioFile->GetDuration()));

	frequencyMinText->SetValue(_T("0.0"));
	frequencyMaxText->SetValue(wxString::Format(_T("%0.0f"), std::min(8000.0, audioFile->GetSampleRate() * 0.5)));
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

	SonogramGenerator generator(*filteredSoundData->ExtractSegment(startTime, endTime), parameters);
	sonogramImage->SetImage(generator.GetImage(colorMap));
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

unsigned int MainFrame::GetNumberOfResolutions() const
{
	double startTime, endTime;
	if (!GetTimeValues(startTime, endTime))
		return 0;

	const double currentDuration(endTime - startTime);
	if (currentDuration <= 0.0)
	{
		wxMessageBox(_T("Invalid segment duration."));
		return 0;
	}

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

bool MainFrame::GetTimeValues(double& minTime, double& maxTime) const
{
	if (!timeMinText->GetValue().ToDouble(&minTime))
	{
		wxMessageBox(_T("Failed to parse minimum time."));
		return false;
	}
	else if (!timeMaxText->GetValue().ToDouble(&maxTime))
	{
		wxMessageBox(_T("Failed to parse maximum time."));
		return false;
	}

	return true;
}

bool MainFrame::GetFrequencyValues(double& minFrequency, double& maxFrequency) const
{
	if (!frequencyMinText->GetValue().ToDouble(&minFrequency))
	{
		wxMessageBox(_T("Failed to parse minimum frequency."));
		return false;
	}
	else if (!frequencyMaxText->GetValue().ToDouble(&maxFrequency))
	{
		wxMessageBox(_T("Failed to parse maximum frequency."));
		return false;
	}

	return true;
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
	makeVideoButton->Enable();

	playButton->Enable();
	//pauseButton->Enable();// Gets enabled after we begin playing
	//stopButton->Enable();// Gets enabled after we begin playing

	timeMinText->Enable();
	timeMaxText->Enable();
	frequencyMinText->Enable();
	frequencyMaxText->Enable();
}

void MainFrame::DisableFileDependentControls()
{
	exportSonogramImageButton->Enable(false);
	makeVideoButton->Enable(false);

	playButton->Enable(false);
	pauseButton->Enable(false);
	stopButton->Enable(false);

	timeMinText->Enable(false);
	timeMaxText->Enable(false);
	frequencyMinText->Enable(false);
	frequencyMaxText->Enable(false);
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
	StopPlayingAudio();
	event.Skip();
}

void MainFrame::UpdateAudioPosition(const float& position)
{
	const int minutes(static_cast<int>(position / 60.0));
	currentTimeText->SetLabel(wxString::Format(_T("%02d:%04.1f"), minutes, position - minutes * 60.0));

	double minTime, maxTime;
	if (!GetTimeValues(minTime, maxTime))
		return;

	if (position < minTime || position > maxTime)
		sonogramImage->UpdateTimeCursor(0.0);
	else
		sonogramImage->UpdateTimeCursor((position - minTime) / (maxTime - minTime));
}

void MainFrame::UpdateSonogramCursorInfo(const double& timePercent, const double& frequencyPercent)
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
	cursorFrequencyText->SetLabel(wxString::Format(_T("%f Hz"), minFrequency + (maxFrequency - minFrequency) * frequencyPercent));
}
