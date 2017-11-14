// File:  sonogrammerApp.h
// Auth:  K. Loux
// Date:  11/13/2017
// Desc:  Entry point for sonogram tool.

#ifndef SONOGRAMMER_APP_H_
#define SONOGRAMMER_APP_H_

// wxWidgets headers
#include <wx/wx.h>

// Local forward declarations
class MainFrame;

class SonogrammerApp : public wxApp
{
public:
	// Initialization function
	bool OnInit();

	// The name of the application
	static const wxString title;// As displayed
	static const wxString internalName;// Internal
	static const wxString creator;
	static const wxString versionString;
	static const wxString gitHash;

private:
	// The main class for the application - this object is the parent for all other objects
	MainFrame *mainFrame = nullptr;
};

// Declare the application object (have wxWidgets create the wxGetApp() function)
DECLARE_APP(SonogrammerApp);

#endif// SONOGRAMMER_APP_H_
