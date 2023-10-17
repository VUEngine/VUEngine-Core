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

extern uint32 VUEngineDebugFontTiles[];


//---------------------------------------------------------------------------------------------------------
//                                               DEFINITIONS
//---------------------------------------------------------------------------------------------------------

CharSetROMSpec DebugFontCharSet =
{
	// number of chars
	43,

	// allocation type
	__NOT_ANIMATED,

	// char spec
	VUEngineDebugFontTiles,

	// pointer to the frames offsets
	NULL,
};

FontROMSpec DebugFont =
{
	// font charset spec pointer
	(CharSetSpec*)&DebugFontCharSet,

	// character number at which the font starts, allows you to skip the control characters for example
	48,

	// number of characters in this font
	43,

	// number of characters per line in charset
	16,

	// size of a single character (in chars) ({width, height})
	{1, 1},

	// font's name
	"Debug",
};