// File:  libCallWrapper.h
// Date:  2/28/2017
// Auth:  K. Loux
// Desc:  Top-level class for performing IP communication functions.

#ifndef LIB_CALL_WRAPPER_H_
#define LIB_CALL_WRAPPER_H_

// Standard C++ headers
#include <string>
#include <iostream>

namespace LibCallWrapper
{

bool FFmpegErrorCheck(int result, const std::string& message);
bool AllocationFailed(const void* const ptr, const std::string& message);

}

#endif// LIB_CALL_WRAPPER_H_
