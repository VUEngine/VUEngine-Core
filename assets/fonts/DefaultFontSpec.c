///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                              THIS FILE WAS AUTO-GENERATED - DO NOT EDIT                               //
///////////////////////////////////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------------------------------------
//                                                INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
//                                              DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern uint32 VUEngineFontTiles[];


//---------------------------------------------------------------------------------------------------------
//                                               DEFINITIONS
//---------------------------------------------------------------------------------------------------------

CharSetROMSpec DefaultFontCharSet =
{
	// number of chars
	256,

	// allocation type
	__NOT_ANIMATED,

	// char spec
	VUEngineFontTiles,

	// pointer to the frames offsets
	NULL,
};

FontROMSpec DefaultFont =
{
	// font charset spec pointer
	(CharSetSpec*)&DefaultFontCharSet,

	// character number at which the font starts, allows you to skip the control characters for example
	0,

	// number of characters in this font
	256,

	// number of characters per line in charset
	32,

	// size of a single character (in chars) ({width, height})
	{1, 1},

	// font's name
	"Default",
};