// File:  colorMapDialog.cpp
// Date:  11/17/2017
// Auth:  K. Loux
// Desc:  Dialog box for defining color maps.

// Local headers
#include "colorMapDialog.h"

// wxWidgets
#include <wx/colordlg.h>

// Standard C++ headers
#include <cassert>

ColorMapDialog::ColorMapDialog(wxWindow *parent, const SonogramGenerator::ColorMap& colorMap)
	: wxDialog(parent, wxID_ANY, _T("Configure Color Map"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE), colorMap(colorMap)
{
	CreateControls();
	CenterOnParent();
}

BEGIN_EVENT_TABLE(ColorMapDialog, wxDialog)
	EVT_BUTTON(idAddButton,			ColorMapDialog::OnAddButtonClickEvent)
	EVT_BUTTON(idRemoveButton,		ColorMapDialog::OnRemoveButtonClickEvent)
	EVT_GRID_CELL_CHANGING(			ColorMapDialog::OnGridCellChangingEvent)
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

	mapEntryGrid->BeginBatch();

	mapEntryGrid->CreateGrid(0, 2, wxGrid::wxGridSelectRows);
	mapEntryGrid->SetColLabelValue(0, _T("Magnitude (0..1)"));
	mapEntryGrid->SetColLabelValue(1, _T("Color"));
	mapEntryGrid->SetColFormatFloat(0);
	mapEntryGrid->SetRowLabelSize(0);
	mapEntryGrid->AutoSizeColLabelSize(0);
	mapEntryGrid->AutoSizeColLabelSize(1);

	PopulateInitialMap();

	const int minRowCount(6);
	if (mapEntryGrid->GetNumberRows() < minRowCount)
		mapEntryGrid->InsertRows(mapEntryGrid->GetNumberRows(), minRowCount - mapEntryGrid->GetNumberRows());

	mapEntryGrid->EndBatch();

	mainSizer->AddSpacer(15);
	wxSizer* buttonSizer(CreateButtonSizer(wxOK | wxCANCEL));
	if (buttonSizer)
		mainSizer->Add(buttonSizer);

	SetSizerAndFit(topSizer);
	mapEntryGrid->SetMinSize(mapEntryGrid->GetSize());

	if (static_cast<unsigned int>(mapEntryGrid->GetNumberRows()) > colorMap.size())
		mapEntryGrid->DeleteRows(colorMap.size(), minRowCount - colorMap.size());
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
	auto selectedRows(mapEntryGrid->GetSelectedRows());
	if (selectedRows.GetCount() == 0)
		return;

	if (mapEntryGrid->GetNumberRows() - selectedRows.GetCount() < 2)
	{
		wxMessageBox(_T("Must have at least two entries"));
		return;
	}

	selectedRows.Sort([](int* a, int* b)
	{
		if (*a < *b)
			return 1;
		else if (*a > *b)
			return -1;
		return 0;
	});

	for (auto& row : selectedRows)
	{
		double value;
		if (!mapEntryGrid->GetCellValue(row, 0).ToDouble(&value))
		{
			assert(false);
		}
		colorMap.erase(GetBestMapEntry(value));
		mapEntryGrid->DeleteRows(row);
	}
}

void ColorMapDialog::OnGridCellDoubleClickEvent(wxGridEvent& event)
{
	if (event.GetCol() != 1)
	{
		event.Skip();
		return;
	}

	wxColourData colorData;
	colorData.SetColour(mapEntryGrid->GetCellBackgroundColour(event.GetRow(), 1));

	wxColourDialog dialog(this, &colorData);
	if (dialog.ShowModal() != wxID_OK)
		return;

	double magnitude;
	if (!mapEntryGrid->GetCellValue(event.GetRow(), 0).ToDouble(&magnitude))
	{
		wxMessageBox(_T("Failed to parse magnitude value"));
		return;
	}

	colorMap.erase(GetBestMapEntry(magnitude));
	colorMap.insert(SonogramGenerator::MagnitudeColor(magnitude, dialog.GetColourData().GetColour()));
	mapEntryGrid->SetCellBackgroundColour(event.GetRow(), 1, dialog.GetColourData().GetColour());
}

SonogramGenerator::ColorMap::iterator ColorMapDialog::GetBestMapEntry(const double& value)
{
	double magnitudeError(2.0);
	SonogramGenerator::ColorMap::iterator bestMatch(colorMap.end());
	auto it = colorMap.begin();
	for (; it != colorMap.end(); ++it)
	{
		if (fabs(it->magnitude - value) < magnitudeError)
		{
			magnitudeError = fabs(it->magnitude - value);
			bestMatch = it;
		}
	}

	return bestMatch;
}

void ColorMapDialog::OnGridCellChangingEvent(wxGridEvent& event)
{
	assert(event.GetCol() == 0);

	double oldValue, newValue;
	if (!mapEntryGrid->GetCellValue(event.GetRow(), 0).ToDouble(&oldValue))
	{
		wxMessageBox(_T("Failed to parse old magnitude value."));
		event.Veto();
		return;
	}
	else if (!event.GetString().ToDouble(&newValue))
	{
		wxMessageBox(_T("Failed to parse new magnitude value."));
		event.Veto();
		return;
	}

	colorMap.erase(GetBestMapEntry(oldValue));
	colorMap.insert(SonogramGenerator::MagnitudeColor(newValue, mapEntryGrid->GetCellBackgroundColour(event.GetRow(), 1)));
}

void ColorMapDialog::PopulateInitialMap()
{
	for (const auto& entry : colorMap)
		AddEntryToGrid(entry);
}

void ColorMapDialog::AddEntryToGrid(const SonogramGenerator::MagnitudeColor& entry)
{
	int row;
	for (row = 0; row < mapEntryGrid->GetNumberRows(); ++row)
	{
		double value;
		if (!mapEntryGrid->GetCellValue(row, 0).ToDouble(&value))
		{
			wxMessageBox(_T("Failed to add color map entry to grid."));
			return;
		}
		else if (entry.magnitude < value)
			break;
	}

	mapEntryGrid->InsertRows(row);
	mapEntryGrid->SetCellValue(row, 0, wxString::Format(_T("%f"), entry.magnitude));
	mapEntryGrid->SetCellBackgroundColour(row, 1, entry.color);
	mapEntryGrid->SetReadOnly(row, 1);
}
