///////////////////////////////////////////////////////////////////////////////////////////////////////////
//							  THIS FILE WAS AUTO-GENERATED - DO NOT EDIT							   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Printer.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const uint32 ProfilerFontTiles[] __attribute__((aligned(4))) =
{
	0x00000000,
	0x00000000,0x55000000,0x7D007D00,0x00005500,0x55500000,0x5D5075D0,0x75D05750,0x00005550,
	0x5FD41550,0x75747FF4,0x7FF47774,0x15505FD4,0x74005400,0x7FF47554,0x75D47FF4,0x555075D0,
	0x75D45550,0x777477F4,0x7F747774,0x55547D74,0x5DD41550,0x77747FF4,0x77747774,0x54547574,
	0x57500540,0x7FD07FD0,0x07F45754,0x055407F4,0x5D541500,0x77747F74,0x77F47774,0x555477F4,
	0x5D541500,0x77747F74,0x7FF47774,0x15505FD4,0x05F40154,0x577407F4,0x7D747F74,0x00545574,
	0x5DD41550,0x77747FF4,0x7FF47774,0x15505DD4,0x5FD41550,0x77747FF4,0x77F47774,0x015055D4,
	0x57407F40,0x57407F40,0x55407F40,0x00000000,0x00000000,0x57400540,0x7F407F40,0x55407540,
	0x07400100,0x7FF41FD0,0xFFFDFFFD,0xFFFDFFFD,0x00000000,0x55550000,0xFFFFFFFF,0xFFFFFFFF,
	0x7FF45554,0x7DF47774,0x7FF47774,0x00005554,0x7FF45554,0x75747F74,0x7FF477F4,0x00005554,
	0x7FF45554,0x77747774,0x7FF47574,0x00005554,0x7FD45550,0x57747FF4,0x7FF45774,0x55507FD4,
	0x5DD41550,0x77747FF4,0x7FF47774,0x55547FF4,0x5DD41550,0x75747DF4,0x7FF47574,0x15505FD4,
	0x5FD41550,0x75747FF4,0x7FF47574,0x55547FF4,0x75745454,0x77747774,0x7FF47774,0x55547FF4,
	0x05740054,0x07740774,0x7FF45774,0x55547FF4,0x7F745554,0x77747F74,0x7FF47574,0x15505FD4,
	0x7FF45554,0x57547FF4,0x7FF45754,0x55547FF4,0x54540000,0x7FF47574,0x75747FF4,0x00005454,
	0x00540000,0x5FF41574,0x75747FF4,0x54007454,0x7DF45554,0x57547FF4,0x7FF45754,0x55547FF4,
	0x74005400,0x74007400,0x7FF47554,0x55547FF4,0x7FF45554,0x075055D4,0x7FF455D4,0x55547FF4,
	0x7FF45554,0x57547FF4,0x7FF455D4,0x55547FF4,0x5FD41550,0x75747FF4,0x7FF47574,0x15505FD4,
	0x05D40150,0x077407F4,0x7FF45774,0x55547FF4,0x7FD45550,0x7D747FF4,0x7FF47574,0x15505FD4,
	0x7DD45550,0x57747FF4,0x7FF45774,0x55547FF4,0x5D741554,0x77747F74,0x77F47774,0x555075D4,
	0x00740054,0x7FF45574,0x55747FF4,0x00540074,0x5FF41554,0x75547FF4,0x7FF47554,0x15545FF4,
	0x17F40554,0x7D545FF4,0x5FF47D54,0x055417F4,0x5FF41554,0x5F407D54,0x5FF47D54,0x055417F4,
	0x7DF45554,0x57547FF4,0x7FF45754,0x55547DF4,0x05F40154,0x7F5457F4,0x57F47F54,0x015405F4,
	0x75745454,0x77F475F4,0x7D747F74,0x54547574,
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// DEFINITIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

CharSetROMSpec ProfilerFontCharSet =
{
	// Number of chars in function of the number of frames to load at the same time
	45,

	// Whether it is shared or not
	true,

	// Whether the tiles are optimized or not
	false,

	// Char data
	(uint32*)ProfilerFontTiles,

	// Frame offsets array
	NULL	
};

FontROMSpec ProfilerFontSpec =
{
	// Pointer to the char spec that the font uses
	(CharSetSpec*)&ProfilerFontCharSet,

	// Offset at which character number the font starts
	46,

	// Number of characters in this font
	45,

	// Number of characters per line in charset
	1,

	// Size of a single character (in chars) ({width, height})
	{1, 1},

	// Font's name
	"Profiler",
};
