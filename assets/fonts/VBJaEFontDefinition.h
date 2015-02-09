#ifndef VBJAENGINE_DEFAULT_FONT_DEFINITION_H_
#define VBJAENGINE_DEFAULT_FONT_DEFINITION_H_


BYTE VBJaEFontTiles[]= {};
FontROMDef VBJAENGINE_DEFAULT_FONT =
{
    // font chars definition pointer
	(BYTE*)VBJaEFontTiles,

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