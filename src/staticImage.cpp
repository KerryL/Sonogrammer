// File:  staticImage.cpp
// Date:  11/15/2017
// Auth:  K. Loux
// Desc:  Audio file object.

// Local headers
#include "staticImage.h"

StaticImage::StaticImage(wxWindow* parent, wxWindowID id, const unsigned int& width,
	const unsigned int& height) : wxPanel(parent, id), image(wxImage(width, height))
{
	SetMinSize(wxSize(width, height));
}

BEGIN_EVENT_TABLE(StaticImage, wxWindow)
	EVT_PAINT(StaticImage::OnPaint)
	EVT_SIZE(StaticImage::OnSize)
END_EVENT_TABLE();

void StaticImage::SetImage(wxImage&& bitmap)
{
	image = std::move(bitmap);
	width = -1;
	height = -1;
}

void StaticImage::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
	Render(dc);
}

void StaticImage::OnSize(wxSizeEvent& event)
{
	Refresh();
    event.Skip();
}

void StaticImage::Render(wxDC& dc)
{
	int newWidth, newHeight;
	dc.GetSize(&newWidth, &newHeight);
 
	if (newWidth != width || newHeight != height)
	{
		resizedImage = wxBitmap(image.Scale(newWidth, newHeight, wxIMAGE_QUALITY_HIGH));
		width = newWidth;
		height = newHeight;
	}
	
	dc.DrawBitmap(resizedImage, 0, 0, false);
}
