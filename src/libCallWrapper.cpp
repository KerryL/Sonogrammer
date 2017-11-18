// File:  libCallWrapper.cpp
// Date:  2/28/2017
// Auth:  K. Loux
// Desc:  Top-level class for performing IP communication functions.

// Local headers
#include "libCallWrapper.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4244)
#endif// _WIN32

// FFmpeg headers
extern "C"
{
#include <libavcodec/avcodec.h>
}

#ifdef _WIN32
#pragma warning(pop)
#endif// _WIN32

// wxWidgets headers
#include <wx/wx.h>

// Standard C++ headers
#include <sstream>

namespace LibCallWrapper
{

bool FFmpegErrorCheck(int result, const std::string& message)
{
	if (result < 0)
	{
		const int errStrSize(100);
		char errStr[errStrSize];
		int errResult = av_strerror(result, errStr, errStrSize);
		if (errResult == 0)
			wxMessageBox(message + _T(":  ") + errStr);
		else
		{
			std::ostringstream ss;
			ss << message << ":  FFmpeg error " << result
				<< "; failed to obtain description (" << errResult << ')';
			wxMessageBox(ss.str());
		}

		return true;
	}

	return false;
}

bool AllocationFailed(void* ptr, const std::string& message)
{
	if (ptr)
		return false;

	wxMessageBox(message);
	return true;
}

}

