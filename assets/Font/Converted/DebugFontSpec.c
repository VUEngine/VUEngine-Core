///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                              THIS FILE WAS AUTO-GENERATED - DO NOT EDIT                               //
///////////////////////////////////////////////////////////////////////////////////////////////////////////


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


#include <Printing.h>



//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


const uint32 DebugFontTiles[] __attribute__((aligned(4))) =
{
    0x00000000,
    0x00000000,0x0FFC0000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x000000F0,
    0x0F000C00,0x00F003C0,0x000F003C,0x00000003,0x3C0F3FFF,0x3CCF3F0F,0x3C0F3C3F,0x00003FFF,
    0x03FC03F0,0x03F003FC,0x03F003F0,0x00003FFF,0x3C0F3FFF,0x3FFF3C00,0x000F000F,0x00003FFF,
    0x3C0F3FFF,0x3FF03C00,0x3C0F3C00,0x00003FFF,0x3C0F3C0F,0x3FFF3C0F,0x3C003C00,0x00003C00,
    0x000F3FFF,0x3FFF000F,0x3C0F3C00,0x00003FFF,0x3C0F3FFF,0x3FFF000F,0x3C0F3C0F,0x00003FFF,
    0x3C0F3FFF,0x3FF03C00,0x3C003C00,0x00003C00,0x3C0F3FFF,0x3FFF3C0F,0x3C0F3C0F,0x00003FFF,
    0x3C0F3FFF,0x3FFF3C0F,0x3C0F3C00,0x00003FFF,0x00000000,0x00F00000,0x00000000,0x000000F0,
    0x00000000,0x00F00000,0x00000000,0x003C00F0,0x03C00F00,0x003C00F0,0x03C000F0,0x00000F00,
    0x00000000,0x00000FFC,0x00000FFC,0x00000000,0x00F0003C,0x0F0003C0,0x00F003C0,0x0000003C,
    0x3C0F3FFF,0x3FC03C00,0x000003C0,0x000003C0,0x3C0F3FFF,0x3CCF3FCF,0x000F3FCF,0x00003FFF,
    0x3C0F3FFF,0x3FFF3C0F,0x3C0F3C0F,0x00003C0F,0x3C0F0FFF,0x0FFF3C0F,0x3C0F3C0F,0x00000FFF,
    0x3C0F3FFF,0x000F000F,0x3C0F000F,0x00003FFF,0x3C0F0FFF,0x3C0F3C0F,0x3C0F3C0F,0x00000FFF,
    0x000F3FFF,0x0FFF000F,0x000F000F,0x00003FFF,0x000F3FFF,0x0FFF000F,0x000F000F,0x0000000F,
    0x3C0F3FFF,0x3F0F000F,0x3C0F3C0F,0x00003FFF,0x3C0F3C0F,0x3FFF3C0F,0x3C0F3C0F,0x00003C0F,
    0x03F03FFF,0x03F003F0,0x03F003F0,0x00003FFF,0x3C003FFF,0x3C003C00,0x3C0F3C00,0x00003FFF,
    0x0F0F3C0F,0x00FF03CF,0x0F0F03CF,0x00003C0F,0x000F000F,0x000F000F,0x000F000F,0x00003FFF,
    0x3F3F3C0F,0x3CCF3FFF,0x3C0F3C0F,0x00003C0F,0x3C3F3C0F,0x3FCF3CFF,0x3C0F3F0F,0x00003C0F,
    0x3C0F3FFF,0x3C0F3C0F,0x3C0F3C0F,0x00003FFF,0x3C0F3FFF,0x3FFF3C0F,0x000F000F,0x0000000F,
    0x3C0F3FFF,0x3C0F3C0F,0x030F3CCF,0x00003CFF,0x3C0F3FFF,0x3FFF3C0F,0x0F0F03CF,0x00003C0F,
    0x000F3FFF,0x3FFF000F,0x3C003C00,0x00003FFF,0x03F03FFF,0x03F003F0,0x03F003F0,0x000003F0,
    0x3C0F3C0F,0x3C0F3C0F,0x3C0F3C0F,0x00003FFF,0x3C0F3C0F,0x0F3C3C0F,0x03F00FFC,0x000003F0,
    0x3C0F3C0F,0x3CCF3C0F,0x3F3F3FFF,0x00003C0F,0x3C0F3C0F,0x03F00F3C,0x3C0F0F3C,0x00003C0F,
    0x3C0F3C0F,0x0FFC0F3C,0x03F003F0,0x000003F0,0x3C0F3FFF,0x03F00F00,0x3C0F003C,0x00003FFF,
};



//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DEFINITIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


CharSetROMSpec DebugFontCharSet =
{
	// Number of chars in function of the number of frames to load at the same time
	46,

	// Whether it is shared or not
	true,

	// Whether the tiles are optimized or not
	false,

	// char data
	(uint32*)DebugFontTiles,

	// Frame offsets array
	NULL    
};

FontROMSpec DebugFontSpec =
{
	// Pointer to the char spec that the font uses
	(CharSetSpec*)&DebugFontCharSet,

	// Offset at which character number the font starts
	45,

	// Number of characters in this font
	46,

	// Number of characters per line in charset
	1,

	// Size of a single character (in chars) ({width, height})
	{1, 1},

	// Font's name
	"Debug",
};
