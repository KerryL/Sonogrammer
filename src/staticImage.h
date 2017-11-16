// File:  staticImage.h
// Date:  11/15/2017
// Auth:  K. Loux
// Desc:  Audio file object.

#ifndef STATIC_IMAGE_H_
#define STATIC_IMAGE_H_

// wxWidgets headers
#include <wx/wx.h>

class StaticImage : public wxPanel
{
public:
	StaticImage(wxWindow* parent, wxWindowID id, const unsigned int& width,
		const unsigned int& height);
	void SetImage(wxImage&& newImage);
	void Reset();
	void ExportToFile(const wxString& fileName) const;
	
private:
	wxImage image;
	wxBitmap resizedImage;
	int width = -1;
	int height = -1;

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void Render(wxDC& dc);

	DECLARE_EVENT_TABLE();
};

#endif// STATIC_IMAGE_H_
