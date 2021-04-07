// File:  waveFormGenerator.h
// Date:  4/6/2021
// Auth:  K. Loux
// Desc:  Class for generating images of waveforms.

#ifndef WAVE_FORM_GENERATOR_H_
#define WAVE_FORM_GENERATOR_H_

// wxWidgets headers
#include <wx/colour.h>// Can't forward declare due to marcro (color vs. coulour)

// Local forward declarations
class SoundData;

// wxWidgets forward declarations
class wxImage;

class WaveFormGenerator
{
public:
	WaveFormGenerator(const SoundData& soundData);

	wxImage GetImage(const unsigned int& width, const unsigned int& height, const wxColor& backgroundColor, const wxColor& lineColor) const;

private:
	const SoundData& soundData;
};

#endif// WAVE_FORM_GENERATOR_H_
