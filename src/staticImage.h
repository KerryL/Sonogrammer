// File:  staticImage.h
// Date:  11/15/2017
// Auth:  K. Loux
// Desc:  Audio file object.

#ifndef STATIC_IMAGE_H_
#define STATIC_IMAGE_H_

// wxWidgets headers
#include <wx/wx.h>

// Local forward declarations
class MainFrame;

class StaticImage : public wxPanel
{
public:
	StaticImage(wxWindow* parent, MainFrame& mainFrame, wxWindowID id, const unsigned int& width,
		const unsigned int& height);
	void SetImage(wxImage&& newImage);
	void Reset();
	void ExportToFile(const wxString& fileName) const;

	void ShowTimeCursor();
	void HideTimeCursor();
	void UpdateTimeCursor(const double& fraction);
	void SetMarkerColor(const wxColor& c);
	
private:
	MainFrame& mainFrame;
	wxImage image;
	wxBitmap resizedImage;
	int width = -1;
	int height = -1;

	wxColor markerColor = wxColor(255, 255, 255);
	bool cursorVisible = false;
	double cursorPosition;

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseLeaveWindow(wxMouseEvent& event);

	void Render(wxDC& dc);

	DECLARE_EVENT_TABLE();
};

#endif// STATIC_IMAGE_H_
