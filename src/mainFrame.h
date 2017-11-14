// File:  mainFrame.h
// Created:  11/13/2017
// Author:  K. Loux
// Description:  Main frame for the application.

#ifndef MAIN_FRAME_H_
#define MAIN_FRAME_H_

// Local headers


// wxWidgets headers
#include <wx/wx.h>

// Standard C++ headers
#include <vector>

// wxWidgets forward declarations
class wxListCtrl;

// The main frame class
class MainFrame : public wxFrame
{
public:
	MainFrame();

private:
	void CreateControls();
	void SetProperties();

	wxSizer* CreatePrimaryControls(wxWindow* parent);
	wxSizer* CreateFilterControls(wxWindow* parent);
	wxSizer* CreateVersionText(wxWindow* parent);
	wxSizer* CreateAudioControls(wxWindow* parent);
	wxSizer* CreateFFTControls(wxWindow* parent);
	wxSizer* CreateImageControls(wxWindow* parent);

	// Controls
	wxStaticBitmap* sonogramImage;

	wxTextCtrl* audioFileName;
	wxTextCtrl* sonogramConfigFileName;
	wxButton* openAudioFileButton;
	wxButton* openConfigFileButton;
	wxButton* saveConfigFileButton;
	wxButton* exportSonogramImageButton;

	wxButton* addFilterButton;
	wxButton* removeFilterButton;
	wxListCtrl* filterList;

	wxSlider* resolutionSlider;
	wxComboBox* windowComboBox;
	wxStaticText* rangeText;
	wxStaticText* windowSizeText;
	wxTextCtrl* overlapTextBox;
	wxStaticText* numberOfAveragesText;

	wxButton* pauseButton;
	wxButton* playButton;
	wxCheckBox* includeFiltersInPlayback;

	wxButton* editColorMapButton;

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
		idEditColorMap
	};

	// Events
	void LoadAudioButtonClickedEvent(wxCommandEvent& event);
	void LoadConfigButtonClickedEvent(wxCommandEvent& event);
	void SaveConfigButtonClickedEvent(wxCommandEvent& event);
	void PrimaryTextCtrlChangedEvent(wxCommandEvent& event);
	void ExportImageButtonClickedEvent(wxCommandEvent& event);

	void AddFilterButtonClickedEvent(wxCommandEvent& event);
	void RemoveFilterButtonClickedEvent(wxCommandEvent& event);
	void FilterListRightClickEvent(wxListEvent& event);
	void FilterListDoubleClickEvent(wxCommandEvent& event);

	void EditColorMapButtonClickedEvent(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
};

#endif// MAIN_FRAME_H_