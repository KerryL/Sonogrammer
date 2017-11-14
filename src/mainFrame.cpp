// File:  mainFrame.cpp
// Created:  11/13/2017
// Author:  K. Loux
// Description:  Main frame for the application.

// Local headers
#include "mainFrame.h"
#include "sonogrammerApp.h"

// wxWidgets headers
#include <wx/listctrl.h>

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
	mainSizer->Add(leftSizer);

	leftSizer->Add(CreatePrimaryControls(panel), wxSizerFlags().Border(wxALL, 5));
	leftSizer->Add(CreateFilterControls(panel), wxSizerFlags().Expand().Border(wxALL, 5).Proportion(1));
	leftSizer->Add(CreateVersionText(panel));

	wxBoxSizer* rightSizer(new wxBoxSizer(wxVERTICAL));
	mainSizer->Add(rightSizer, wxSizerFlags().Expand().Proportion(1));

	sonogramImage = new wxStaticBitmap(panel, wxID_ANY, wxBitmap());
	rightSizer->Add(sonogramImage);

	wxBoxSizer* rightBottomSizer(new wxBoxSizer(wxHORIZONTAL));
	rightSizer->Add(rightBottomSizer);

	/*rightBottomSizer->Add(CreateAudioControls(panel), wxSizerFlags().Expand().Border(wxALL, 5));
	rightBottomSizer->Add(CreateImageControls(panel), wxSizerFlags().Expand().Border(wxALL, 5));
	rightBottomSizer->Add(CreateFFTControls(panel), wxSizerFlags().Expand().Border(wxALL, 5));*/

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
	wxBoxSizer* sizer(new wxBoxSizer(wxVERTICAL));
	wxBoxSizer* buttonSizer(new wxBoxSizer(wxHORIZONTAL));
	sizer->Add(buttonSizer, wxSizerFlags().Expand());
	addFilterButton = new wxButton(parent, idAddFilter, _T("Add"));
	removeFilterButton = new wxButton(parent, idRemoveFilter, _T("Remove"));

	buttonSizer->Add(new wxStaticText(parent, wxID_ANY, _T("Filters")), wxSizerFlags().Align(wxBOTTOM));
	buttonSizer->AddStretchSpacer();
	buttonSizer->Add(addFilterButton, wxSizerFlags().Border(wxRIGHT | wxLEFT, 5));
	buttonSizer->Add(removeFilterButton, wxSizerFlags().Border(wxBOTTOM, 5));

	filterList = new wxListCtrl(parent);
	sizer->Add(filterList, wxSizerFlags().Expand());

	return sizer;
}

wxSizer* MainFrame::CreateVersionText(wxWindow* parent)
{
	wxBoxSizer* sizer(new wxBoxSizer(wxVERTICAL));
	wxString versionString;
	versionString << SonogrammerApp::versionString << " (" << SonogrammerApp::gitHash << ")";
	sizer->Add(new wxStaticText(parent, wxID_ANY, versionString));
	return sizer;
}

wxSizer* MainFrame::CreateAudioControls(wxWindow* parent)
{
	// TODO:  Complete
	return nullptr;
}

wxSizer* MainFrame::CreateFFTControls(wxWindow* parent)
{
	// TODO:  Complete
	return nullptr;
}

wxSizer* MainFrame::CreateImageControls(wxWindow* parent)
{
	// TODO:  Complete
	return nullptr;
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
}

void MainFrame::LoadConfigButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
}

void MainFrame::SaveConfigButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
}

void MainFrame::PrimaryTextCtrlChangedEvent(wxCommandEvent& event)
{
}

void MainFrame::ExportImageButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
}

void MainFrame::AddFilterButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
}

void MainFrame::RemoveFilterButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
}

void MainFrame::FilterListRightClickEvent(wxListEvent& event)
{
}

void MainFrame::FilterListDoubleClickEvent(wxCommandEvent& event)
{
}

void MainFrame::EditColorMapButtonClickedEvent(wxCommandEvent& WXUNUSED(event))
{
}
