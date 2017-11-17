// File:  colorMapDialog.cpp
// Date:  11/17/2017
// Auth:  K. Loux
// Desc:  Dialog box for defining color maps.

// Local headers
#include "colorMapDialog.h"

// Standard C++ headers
#include <cassert>

ColorMapDialog::ColorMapDialog(wxWindow *parent, const SonogramGenerator::ColorMap& colorMap)
	: wxDialog(parent, wxID_ANY, _T("Configure Color Map"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE), colorMap(colorMap)
{
	CreateControls();
	CenterOnParent();
	PopulateInitialMap();
}

BEGIN_EVENT_TABLE(ColorMapDialog, wxDialog)
	EVT_BUTTON(idAddButton,			ColorMapDialog::OnAddButtonClickEvent)
	EVT_BUTTON(idRemoveButton,		ColorMapDialog::OnRemoveButtonClickEvent)
	EVT_GRID_CELL_CHANGED(			ColorMapDialog::OnGridCellChangeEvent)
	EVT_GRID_CELL_LEFT_CLICK(		ColorMapDialog::OnGridCellDoubleClickEvent)
END_EVENT_TABLE();

void ColorMapDialog::CreateControls()
{
	wxSizer* topSizer(new wxBoxSizer(wxHORIZONTAL));
	wxSizer* mainSizer(new wxBoxSizer(wxVERTICAL));
	topSizer->Add(mainSizer, wxSizerFlags().Border(wxALL, 5));

	wxSizer* addRemoveButtonSizer(new wxBoxSizer(wxHORIZONTAL));
	mainSizer->Add(addRemoveButtonSizer, wxSizerFlags().Expand());

	addEntryButton = new wxButton(this, idAddButton, _T("Add Entry"));
	removeEntryButton = new wxButton(this, idRemoveButton, _T("Remove Entry"));
	addRemoveButtonSizer->Add(addEntryButton, wxSizerFlags().Border(wxALL, 5));
	addRemoveButtonSizer->Add(removeEntryButton, wxSizerFlags().Border(wxALL, 5));

	mapEntryGrid = new wxGrid(this, wxID_ANY);
	mainSizer->Add(mapEntryGrid, wxSizerFlags().Expand());

	mapEntryGrid->CreateGrid(0, 2);
	mapEntryGrid->SetColLabelValue(0, _T("Magnitude (%)"));
	mapEntryGrid->SetColLabelValue(0, _T("Color"));
	mapEntryGrid->SetColFormatFloat(0);
	mapEntryGrid->SetRowLabelSize(0);

	mainSizer->AddSpacer(15);
	wxSizer* buttonSizer(CreateButtonSizer(wxOK | wxCANCEL));
	if (buttonSizer)
		mainSizer->Add(buttonSizer);

	SetSizerAndFit(topSizer);// TODO:  Why aren't buttons showing up?
}

void ColorMapDialog::OnAddButtonClickEvent(wxCommandEvent& WXUNUSED(event))
{
	// TODO:  Make this smarter - better guess at color and magnitude, and insert based on selected item in box?
	// At the very least, don't allow a magnitude that's already been selected
	SonogramGenerator::MagnitudeColor entry(1.0, wxColor(0, 0, 0));
	AddEntryToGrid(entry);
	colorMap.insert(entry);
}

void ColorMapDialog::OnRemoveButtonClickEvent(wxCommandEvent& WXUNUSED(event))
{
	const auto selectedCells(mapEntryGrid->GetSelectedCells());
	if (selectedCells.Count() == 0)
		return;

	// TODO:  Implement
	// Remove from list and also colorMap
}

void ColorMapDialog::OnGridCellDoubleClickEvent(wxGridEvent& event)
{
	if (event.GetCol() != 1)
	{
		event.Skip();
		return;
	}

	// TODO:  Show color picker and update:
	// 1. cell bg color
	// 2. entry in color map
}

void ColorMapDialog::OnGridCellChangeEvent(wxGridEvent& event)
{
	assert(event.GetCol() == 0);

	double newValue;
	if (mapEntryGrid->GetCellValue(event.GetRow(), 0).ToDouble(&newValue))
	{
		wxMessageBox(_T("Failed to parse magnitude value."));
		event.Veto();// TODO:  Test
		return;
	}

	// TODO:  Change correct item in colorMap
}

void ColorMapDialog::PopulateInitialMap()
{
	mapEntryGrid->BeginBatch();
	for (const auto& entry : colorMap)
		AddEntryToGrid(entry);
	mapEntryGrid->EndBatch();
}

void ColorMapDialog::AddEntryToGrid(const SonogramGenerator::MagnitudeColor& entry)
{
	int row;
	for (row = 0; row < mapEntryGrid->GetNumberRows(); ++row)
	{
		double value;
		if (!mapEntryGrid->GetCellValue(row, 0).ToDouble(&value))
		{
			// TODO:  Handle error
		}
		else if (entry.magnitude < value)
			break;
	}

	mapEntryGrid->InsertRows(row);
	mapEntryGrid->SetCellValue(row, 0, wxString::Format(_T("%f"), entry.magnitude));
	mapEntryGrid->SetCellBackgroundColour(row, 1, entry.color);
	mapEntryGrid->SetReadOnly(row, 1);
}
