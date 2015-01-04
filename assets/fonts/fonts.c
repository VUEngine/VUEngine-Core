#ifndef FONT_H_
#define FONT_H_


extern BYTE VBJaEngineDefaultFontTiles[];
FontROMDef VBJAE_DEFAULT_FONT =
{
    // font chars definition pointer
	(BYTE*)VBJaEngineDefaultFontTiles,

	// number of characters in font
	256,

	// at which character number the font starts
	0,

	// size of a single character (in chars) ({width, height})
	{1, 1},

	// font's name
	"VBJaEngineFont",
};


#endif