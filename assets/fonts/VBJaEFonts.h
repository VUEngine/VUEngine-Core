#ifndef __CUSTOM_FONTS
#ifndef VBJAE_FONTS_H_
#define VBJAE_FONTS_H_


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern BYTE VBJaEFontTiles[];


//---------------------------------------------------------------------------------------------------------
// 												DEFINITIONS
//---------------------------------------------------------------------------------------------------------

FontROMDef VBJAENGINE_DEFAULT_FONT =
{
    // font chars definition pointer
	VBJaEFontTiles,

	// number of characters in font
	256,

    // character number at which the font starts, allows you to skip the control characters for example
	0,

	// size of a single character (in chars) ({width, height})
	{1, 1},

	// font's name
    "VBJaEngineFont",
};

const FontROMDef* __FONTS[] =
{
    &VBJAENGINE_DEFAULT_FONT,
    NULL
};


#endif
#endif