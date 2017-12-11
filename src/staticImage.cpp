// File:  staticImage.cpp
// Date:  11/15/2017
// Auth:  K. Loux
// Desc:  Audio file object.

// Local headers
#include "staticImage.h"
#include "mainFrame.h"

StaticImage::StaticImage(wxWindow* parent, MainFrame& mainFrame, wxWindowID id, const unsigned int& width,
	const unsigned int& height) : wxPanel(parent, id), mainFrame(mainFrame),
	image(wxImage(width, height))
{
	image.Replace(0, 0, 0, 255, 255, 255);
	SetMinSize(wxSize(width, height));
}

BEGIN_EVENT_TABLE(StaticImage, wxWindow)
	EVT_PAINT(StaticImage::OnPaint)
	EVT_SIZE(StaticImage::OnSize)
	EVT_MOTION(StaticImage::OnMouseMove)
	EVT_LEAVE_WINDOW(StaticImage::OnMouseLeaveWindow)
END_EVENT_TABLE();

void StaticImage::SetImage(wxImage&& newImage)
{
	image = std::move(newImage);
	width = -1;
	height = -1;
	Refresh();
	Update();
}

void StaticImage::Reset()
{
	wxImage empty(1,1);
	empty.Replace(0, 0, 0, 255, 255, 255);
	SetImage(std::move(empty));
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

	if (cursorVisible && cursorPosition > 0.0)
	{
		const int lineWidth(1);
		dc.SetPen(wxPen(markerColor, lineWidth));

		const int linePosition(newWidth * cursorPosition);
		wxPoint bottomPoint(linePosition, 0);
		wxPoint topPoint(linePosition, newHeight);
		dc.DrawLine(bottomPoint, topPoint);
	}
}

void StaticImage::ExportToFile(const wxString& fileName) const
{
	wxInitAllImageHandlers();
	if (!image.SaveFile(fileName))
		wxMessageBox(_T("Failed to save file to '") + fileName + _T("'."));
}

void StaticImage::OnMouseMove(wxMouseEvent& event)
{
	mainFrame.UpdateSonogramCursorInfo(static_cast<double>(event.GetX()) / GetSize().GetWidth(),
		1.0 - static_cast<double>(event.GetY()) / GetSize().GetHeight());
}

void StaticImage::OnMouseLeaveWindow(wxMouseEvent& WXUNUSED(event))
{
	mainFrame.UpdateSonogramCursorInfo(-1.0, -1.0);
}

void StaticImage::ShowTimeCursor()
{
	cursorVisible = true;
	cursorPosition = 0.0;
}

void StaticImage::HideTimeCursor()
{
	cursorVisible = false;
	Refresh();
}

void StaticImage::UpdateTimeCursor(const double& fraction)
{
	cursorPosition = fraction;
	Refresh();
}

void StaticImage::SetMarkerColor(const wxColor& c)
{
	markerColor = c;
}
