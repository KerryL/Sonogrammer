// File:  mainFrame.cpp
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Main frame for the application.

// Local headers
#include "mainFrame.h"
#include "sonogrammerApp.h"
#include "audioFile.h"
#include "sonogramGenerator.h"
#include "filter.h"
#include "soundData.h"

// wxWidgets headers
#include <wx/listctrl.h>

// SDL headers
#include <SDL_version.h>

// FFmpeg headers
extern "C"
{
#include <libavcodec/version.h>
#include <libavformat/version.h>
#include <libavutil/version.h>
}

// Standard C++ headers
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, wxEmptyString,
	wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE)
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
	EVT_BUTTON(idButtonLoadAudioFile,			MainFrame::LoadAudioButtonClickedEvent)
	EVT_BUTTON(idButtonLoadSonogramConfig,		MainFrame::LoadConfigButtonClickedEvent)
	EVT_BUTTON(idButtonSaveSonogramConfig,		MainFrame::SaveConfigButtonClickedEvent)
	EVT_TEXT(idPrimaryControl,					MainFrame::PrimaryTextCtrlChangedEvent)
	EVT_BUTTON(idExportSonogramImage,			MainFrame::ExportImageButtonClickedEvent)
	EVT_BUTTON(idAddFilter,						MainFrame::AddFilterButtonClickedEvent)
	EVT_BUTTON(idRemoveFilter,					MainFrame::RemoveFilterButtonClickedEvent)
	EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY,			MainFrame::FilterListRightClickEvent)
	EVT_LISTBOX_DCLICK(wxID_ANY,				MainFrame::FilterListDoubleClickEvent)
	EVT_BUTTON(idEditColorMap,					MainFrame::EditColorMapButtonClickedEvent)
	EVT_TEXT(idImageControl,					MainFrame::ImageTextCtrlChangedEvent)
	EVT_SLIDER(idFFT,							MainFrame::FFTSettingsChangedEvent)
	EVT_TEXT(idFFT,								MainFrame::FFTSettingsChangedEvent)
	EVT_COMBOBOX(idFFT,							MainFrame::FFTSettingsChangedEvent)
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

	sonogramImage = new wxStaticBitmap(panel, wxID_ANY, wxBitmap());
	sonogramImage->SetScaleMode(wxStaticBitmapBase::Scale_Fill);
	rightSizer->Add(sonogramImage, wxSizerFlags().Expand().Proportion(1));

	wxBoxSizer* rightBottomSizer(new wxBoxSizer(wxHORIZONTAL));
	rightSizer->Add(rightBottomSizer);

	rightBottomSizer->Add(CreateAudioControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	rightBottomSizer->Add(CreateFFTControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	rightBottomSizer->Add(CreateImageControls(panel), wxSizerFlags().Border(wxALL, 5).Expand());

	TransferDataToWindow();

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

	filterList = new wxListCtrl(sizer->GetStaticBox());
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
	includeFiltersInPlayback = new wxCheckBox(sizer->GetStaticBox(), idIncludeFilters, _T("Include Filters in Playback"));
	buttonSizer->Add(pauseButton, wxSizerFlags().Border(wxALL, 5));
	buttonSizer->Add(playButton, wxSizerFlags().Border(wxALL, 5));
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
	overlapTextBox = new wxTextCtrl(sizer->GetStaticBox(), idFFT, _T("0.0"));

	innerSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Resolution (Hz)")));
	innerSizer->Add(resolutionSlider);

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

	return sizer;
}

void MainFrame::SetProperties()
{
	SetTitle(SonogrammerApp::title);
	SetName(SonogrammerApp::internalName);
	Center();

#ifdef __WXMSW__
	//SetIcon(wxIcon(_T("ICON_ID_MAIN"), wxBITMAP_TYPE_ICO_RESOURCE));
#elif __WXGTK__
	SetIcon(wxIcon(plots16_xpm));
	SetIcon(wxIcon(plots24_xpm));
	SetIcon(wxIcon(plots32_xpm));
	SetIcon(wxIcon(plots48_xpm));
	SetIcon(wxIcon(plots64_xpm));
	SetIcon(wxIcon(plots128_xpm));
#endif
}

void MainFrame::LoadAudioButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
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

void MainFrame::PrimaryTextCtrlChangedEvent(wxCommandEvent& event)
{
	HandleNewAudioFile();
}

void MainFrame::ExportImageButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	// TODO:  Implement
}

void MainFrame::AddFilterButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	// TODO:  Implement
	// Show dialog, get resulting filter from dialog, add it to our list and also the filter control
	ApplyFilters();
}

void MainFrame::RemoveFilterButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	if (filterList->GetSelectedItemCount() == 0)
		return;

	long itemIndex(-1);
	do
	{
		itemIndex = filterList->GetNextItem(itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		assert(itemIndex >= 0);
		filters.erase(filters.begin() + itemIndex);
		filterList->DeleteItem(itemIndex);
	} while (itemIndex > 1);

	ApplyFilters();
}

void MainFrame::FilterListRightClickEvent(wxListEvent& event)
{
	// TODO:  Implement
}

void MainFrame::FilterListDoubleClickEvent(wxCommandEvent& event)
{
	// TODO:  Implement
	// Edit the selected filter
}

void MainFrame::ImageTextCtrlChangedEvent(wxCommandEvent& event)
{
	// TODO:  Implement
}

void MainFrame::EditColorMapButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
	// TODO:  Implement
}

void MainFrame::ApplyFilters()
{
	filteredSoundData = std::make_unique<SoundData>(*originalSoundData);
	for (auto& f : filters)
		filteredSoundData = filteredSoundData->ApplyFilter(f);
}

void MainFrame::FFTSettingsChangedEvent(wxCommandEvent& WXUNUSED(event))
{
	UpdateFFTCalculatedInformation();
}

void MainFrame::UpdateFFTCalculatedInformation()
{
	resolutionText->SetLabel(wxString::Format(_T("%f Hz"), GetResolution()));
	windowSizeText->SetLabel(wxString::Format(_T("%d"), GetWindowSize()));
	timeSliceText->SetLabel(wxString::Format(_T("%0.3f sec"), static_cast<double>(GetWindowSize()) / audioFile->GetSampleRate()));
}

void MainFrame::HandleNewAudioFile()
{
	const wxString fileName(audioFileName->GetValue());
	if (fileName.IsEmpty())
		return;

	if (!wxFileExists(fileName))
	{
		wxMessageBox(_T("File '") + fileName + _T("' does not exist."));
		return;
	}

	audioFile = std::make_unique<AudioFile>(std::string(fileName.c_str()));
	UpdateAudioInformation();
	UpdateFFTInformation();
	UpdateSonogramInformation();
	UpdateSonogram();
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
	resolutionSlider->SetMin(0);
	resolutionSlider->SetMax(GetNumberOfResolutions());
	resolutionSlider->SetValue(resolutionSlider->GetMax() / 2);// TODO:  Needs to be improved (balance of resolution and time slice) - could be based on available number of pixels for both directions?
	rangeText->SetLabel(wxString::Format(_T("%0.0f Hz"), audioFile->GetSampleRate() * 0.5));

	UpdateFFTCalculatedInformation();
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
	double startTime, endTime;
	if (!timeMinText->GetValue().ToDouble(&startTime))
	{
		wxMessageBox(_T("Failed to parse minimum time."));
		return;
	}

	if (!timeMaxText->GetValue().ToDouble(&endTime))
	{
		wxMessageBox(_T("Failed to parse maximum time."));
		return;
	}

	SonogramGenerator::FFTParameters parameters;
	parameters.windowFunction = static_cast<FastFourierTransform::WindowType>(windowComboBox->GetSelection());
	parameters.windowSize = GetWindowSize();

	if (overlapTextBox->GetValue().ToDouble(&parameters.overlap))
	{
		wxMessageBox(_T("Failed to parse overlap."));
		return;
	}
	else if (parameters.overlap < 0.0 || parameters.overlap > 1.0)
	{
		wxMessageBox(_T("Overlap must be between 0 and 1."));
		return;
	}

	SonogramGenerator::ColorMap colorMap;// TODO:  Don't hardcode
	colorMap[0.0] = wxColor(0, 0, 0);
	colorMap[1.0] = wxColor(255, 255, 255);

	SonogramGenerator generator(*filteredSoundData->ExtractSegment(startTime, endTime), parameters);
	sonogramImage->SetBitmap(generator.GetBitmap(400, 150, colorMap));// TODO:  Don't hardcode size
}

unsigned int MainFrame::GetNumberOfResolutions() const
{
	double startTime, endTime;
	if (!timeMinText->GetValue().ToDouble(&startTime))
	{
		wxMessageBox(_T("Failed to parse minimum time."));
		return 0;
	}

	if (!timeMaxText->GetValue().ToDouble(&endTime))
	{
		wxMessageBox(_T("Failed to parse minimum time."));
		return 0;
	}

	const double currentDuration(endTime - startTime);
	if (currentDuration <= 0.0)
	{
		wxMessageBox(_T("Invalid segment duration."));
		return 0;
	}

	const unsigned int numberOfPoints(currentDuration * audioFile->GetSampleRate());
	return FastFourierTransform::GetMaxPowerOfTwo(numberOfPoints) - 1;
}

double MainFrame::GetResolution() const
{
	return static_cast<double>(audioFile->GetSampleRate()) / GetWindowSize();
}

unsigned int MainFrame::GetWindowSize() const
{
	return pow(2, resolutionSlider->GetValue() + 1);
}
