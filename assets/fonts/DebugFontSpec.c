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
	46,

	// whether it is shared or not
	true,

	// whether the tiles are optimized or not
	false,	

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
	45,

	// number of characters in this font
	46,

	// number of characters per line in charset
	46,

	// size of a single character (in chars) ({width, height})
	{1, 1},

	// font's name
	"Debug",
};