// File:  colorMapDialog.h
// Date:  11/17/2017
// Auth:  K. Loux
// Desc:  Dialog box for defining color maps.

#ifndef COLOR_MAP_DIALOG_H_
#define COLOR_MAP_DIALOG_H_

// Local headers
#include "sonogramGenerator.h"

// wxWidgets headers
#include <wx/wx.h>
#include <wx/grid.h>

class ColorMapDialog : public wxDialog
{
public:
	ColorMapDialog(wxWindow *parent, const SonogramGenerator::ColorMap& colorMap);

	SonogramGenerator::ColorMap GetColorMap() const { return colorMap; }

private:
	SonogramGenerator::ColorMap colorMap;

	void CreateControls();
	void PopulateInitialMap();
	void AddEntryToGrid(const SonogramGenerator::MagnitudeColor& entry);

	wxButton* addEntryButton;
	wxButton* removeEntryButton;
	wxGrid* mapEntryGrid;

	// Event handlers
	void OnAddButtonClickEvent(wxCommandEvent& event);
	void OnRemoveButtonClickEvent(wxCommandEvent& event);
	void OnGridCellDoubleClickEvent(wxGridEvent& event);
	void OnGridCellChangeEvent(wxGridEvent& event);

	enum EventIDs
	{
		idAddButton = wxID_HIGHEST + 200,
		idRemoveButton
	};

	// For the event table
	DECLARE_EVENT_TABLE();
};

#endif// COLOR_MAP_DIALOG_H_
