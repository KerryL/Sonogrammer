// File:  sonogrammerApp.cpp
// Auth:  K. Loux
// Date:  11/13/2017
// Desc:  Entry point for sonogram tool.

// Local headers
#include "sonogrammerApp.h"
#include "mainFrame.h"

// Implement the application (have wxWidgets set up the appropriate entry points, etc.)
IMPLEMENT_APP(SonogrammerApp);

//==========================================================================
// Class:			SonogrammerApp
// Function:		Constant Declarations
//
// Description:		Constant declarations for the FugeDynamicsApp class.
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
const wxString SonogrammerApp::title = _T("Sonogrammer");
const wxString SonogrammerApp::internalName = _T("Sonogrammer");
const wxString SonogrammerApp::creator = _T("K. Loux");
// gitHash and versionString are defined in gitHash.cpp, which is automatically generated during the build

//==========================================================================
// Class:			SonogrammerApp
// Function:		OnInit
//
// Description:		Initializes the application window.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool = true for successful window initialization, false for error
//
//==========================================================================
bool SonogrammerApp::OnInit()
{
	// Set the application's name and the vendor's name
	SetAppName(internalName);
	SetVendorName(creator);

	// Create the MainFrame object - this is the parent for all other objects
	mainFrame = new MainFrame();

	// Make sure the MainFrame was successfully created
	if (!mainFrame)
		return false;

	// Make the window visible
	mainFrame->Show(true);

	// Bring the window to the top
	//SetTopWindow(mainFrame);

	return true;
}
