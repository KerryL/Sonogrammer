/*=============================================================================
                                   LibPlot2D
                       Copyright Kerry R. Loux 2011-2016

                  This code is licensed under the GPLv2 License
                    (http://opensource.org/licenses/GPL-2.0).
=============================================================================*/

// File:  dropTarget.cpp
// Date:  5/2/2011
// Auth:  K. Loux
// Desc:  Derives from wxFileDropTarget and overrides OnDropFiles to load files
//        when the user drags-and-drops them onto the main window.

// Local headers
#include "dropTarget.h"
#include "mainFrame.h"

//=============================================================================
// Class:			DropTarget
// Function:		DropTarget
//
// Description:		Constructor for DropTarget class.
//
// Input Arguments:
//		mainFrame	= &MainFrame, reference to main application window
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//=============================================================================
DropTarget::DropTarget(MainFrame &mainFrame)
	: mMainFrame(mainFrame)
{
	wxDataObjectComposite *dataObject = new wxDataObjectComposite;

	dataObject->Add(new wxFileDataObject);

	SetDataObject(dataObject);
}

//=============================================================================
// Class:			DropTarget
// Function:		OnDropFiles
//
// Description:		Handles dragging and dropping of files.
//
// Input Arguments:
//		filenames	= const &wxArrayString containing the list of filenames
//					  being dropped
//
// Output Arguments:
//		None
//
// Return Value:
//		true to accept the data, false to veto
//
//=============================================================================
bool DropTarget::OnDropFiles(const wxArrayString &filenames)
{
	mMainFrame.LoadFile(filenames.front());
	return true;// TODO:  Should I ever return false?
}

//=============================================================================
// Class:			DropTarget
// Function:		OnData
//
// Description:		Overloaded virtual method from wxTextDropTarget.
//
// Input Arguments:
//		x		= wxCoord (unused)
//		y		= wxCoord (unused)
//		def		= wxDragResult
//
// Output Arguments:
//		None
//
// Return Value:
//		wxDragResult
//
//=============================================================================
wxDragResult DropTarget::OnData(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
	wxDragResult def)
{
	if (!GetData())
		return wxDragNone;

	wxDataObjectComposite *dataObject(
		static_cast<wxDataObjectComposite*>(m_dataObject));
	const size_t bufferSize(
		dataObject->GetDataSize(dataObject->GetReceivedFormat()));

	std::vector<char> buffer(bufferSize);
	if (!dataObject->GetDataHere(
		dataObject->GetReceivedFormat(), buffer.data()))
		return wxDragNone;

	if (dataObject->GetReceivedFormat().GetType() == wxDF_FILENAME)
	{
		wxFileDataObject fileData;
		fileData.SetData(bufferSize, buffer.data());
		return OnDropFiles(fileData.GetFilenames()) ? def : wxDragNone;
	}

	assert(false);
	return wxDragNone;
}
