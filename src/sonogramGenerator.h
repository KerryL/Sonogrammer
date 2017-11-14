// File:  sonogramGenerator.h
// Date:  11/13/2017
// Auth:  K. Loux
// Desc:  Sound data object.

#ifndef SONOGRAM_GENERATOR_H_
#define SONOGRAM_GENERATOR_H_

// Local headers
#include "soundData.h"

// wxWidgets forward declarations
class wxBitmap;

class SonogramGenerator
{
public:
	SonogramGenerator(const SoundData& soundData);

	wxBitmap GetBitmap() const;

private:
	const SoundData& soundData;
};

#endif// SONOGRAM_GENERATOR_H_
