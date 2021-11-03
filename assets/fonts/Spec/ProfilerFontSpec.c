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

extern BYTE VUEngineProfilerFontTiles[];


//---------------------------------------------------------------------------------------------------------
//                                               DEFINITIONS
//---------------------------------------------------------------------------------------------------------

CharSetROMSpec ProfilerFontCharSet =
{
	// number of chars
	45,

	// allocation type
	__NOT_ANIMATED,

	// char spec
	VUEngineProfilerFontTiles,
};

FontROMSpec ProfilerFont =
{
	// font charset spec pointer
	(CharSetSpec*)&ProfilerFontCharSet,

	// character number at which the font starts, allows you to skip the control characters for example
	46,

	// number of characters in this font
	45,

	// number of characters per line in charset
	45,

	// size of a single character (in chars) ({width, height})
	{1, 1},

	// font's name
	"Profiler",
};