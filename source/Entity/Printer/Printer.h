/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PRINTER_H_
#define PRINTER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <CharSet.h>
#include <Entity.h>
#include <TimerManager.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class PrintingSprite;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define PRINT_TEXT(string, x, y)		Printer::text(string, x, y, NULL)
#define PRINT_INT(number, x, y)			Printer::int32(number, x, y, NULL)
#define PRINT_FLOAT(number, x, y)		Printer::float(number, x, y, 2, NULL)
#define PRINT_HEX(number, x, y)			Printer::hex(number, x, y, 8, NULL)
#define PRINT_HEX_EXT(number, x, y, d) Printer::hex(number, x, y, d, NULL)
#define PRINT_TIME(x, y)																					\
	Printer::int32																							\
	(																										\
		TimerManager::getTotalElapsedMilliseconds(),														\
		x,																									\
		y,																									\
		NULL																								\
	);

#define __TAB_SIZE				  4
#define __MAX_FONT_NAME_LENGTH	  16

#define __PRINTING_MODE_DEFAULT	  0
#define __PRINTING_MODE_DEBUG	  1

#define __CHAR_BATTERY			  "\x01\x02"

#define __CHAR_LINE_TOP_LEFT	  "\x03"
#define __CHAR_LINE_TOP_RIGHT	  "\x04"
#define __CHAR_LINE_BOTTOM_LEFT	  "\x05"
#define __CHAR_LINE_BOTTOM_RIGHT  "\x06"
#define __CHAR_LINE_VERTICAL	  "\x07"
#define __CHAR_LINE_HORIZONTAL	  "\x08"

#define __CHAR_SELECTOR			  "\x0B"
#define __CHAR_SELECTOR_LEFT	  "\x0C"

#define __CHAR_DARK_RED_BOX		  "\x0E"
#define __CHAR_MEDIUM_RED_BOX	  "\x0F"
#define __CHAR_BRIGHT_RED_BOX	  "\x10"

#define __CHAR_CHECKBOX_UNCHECKED "\x11"
#define __CHAR_CHECKBOX_CHECKED	  "\x12"

#define __CHAR_START_BUTTON		  "\x15"
#define __CHAR_SELECT_BUTTON	  "\x16"
#define __CHAR_A_BUTTON			  "\x13"
#define __CHAR_B_BUTTON			  "\x14"
#define __CHAR_L_TRIGGER		  "\x17"
#define __CHAR_R_TRIGGER		  "\x18"
#define __CHAR_D_PAD			  "\x19"
#define __CHAR_D_PAD_UP			  "\x1A"
#define __CHAR_D_PAD_DOWN		  "\x1B"
#define __CHAR_D_PAD_LEFT		  "\x1C"
#define __CHAR_D_PAD_RIGHT		  "\x1D"
#define __CHAR_L				  "\x1E"
#define __CHAR_R				  "\x1F"

#define __CHAR_L_D_PAD			  "\x1E\x19"
#define __CHAR_L_D_PAD_UP		  "\x1E\x1A"
#define __CHAR_L_D_PAD_DOWN		  "\x1E\x1B"
#define __CHAR_L_D_PAD_LEFT		  "\x1E\x1C"
#define __CHAR_L_D_PAD_RIGHT	  "\x1E\x1D"
#define __CHAR_R_D_PAD			  "\x1F\x19"
#define __CHAR_R_D_PAD_UP		  "\x1F\x1A"
#define __CHAR_R_D_PAD_DOWN		  "\x1F\x1B"
#define __CHAR_R_D_PAD_LEFT		  "\x1F\x1C"
#define __CHAR_R_D_PAD_RIGHT	  "\x1F\x1D"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// @memberof Printer
enum PrintingOrientation
{
	kPrintingOrientationHorizontal = 0,
	kPrintingOrientationVertical
};

/// @memberof Printer
enum PrintingDirection
{
	kPrintingDirectionLTR = 0,
	kPrintingDirectionRTL
};

/// @memberof Printer
typedef struct FontSize
{
	uint16 x;
	uint16 y;

} FontSize;

/// @memberof Printer
typedef struct FontSpec
{
	/// Pointer to the char spec that the font uses
	const CharSetSpec* charSetSpec;

	/// Offset at which character number the font starts
	int16 offset;

	/// Number of characters in this font
	uint16 characterCount;

	/// Number of characters per line in charset
	uint16 charactersPerLineInCharset;

	/// Size of a single character (in chars) ({width, height})
	FontSize fontSize;

	/// Font's name
	char name[__MAX_FONT_NAME_LENGTH];

} FontSpec;

/// @memberof Printer
typedef const FontSpec FontROMSpec;

/// @memberof Printer
typedef struct FontData
{
	/// Font spec
	const struct FontSpec* fontSpec;

	/// CharSet
	CharSet charSet;

} FontData;

/// A FontData spec that is stored in ROM
/// @memberof Printer
typedef const FontData FontROMData;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Printer
///
/// Inherits from ListenerObject
///
/// Manages printing layer and offers various functions to write to it.
singleton class Printer : Entity
{
	/// @protectedsection

	/// Active sprite that is manipulated by this class' interface
	PrintingSprite activePrintingSprite;

	/// A list of loaded fonts and their respective CharSets
	VirtualList fonts;

	/// Cache the last used font to speed up searches
	const char* lastUsedFont;

	/// Cache the last used font data to speed up searches
	FontData* lastUsedFontData;

	/// Printer mode (Default or Debug)
	uint8 mode;

	/// Palette to use for printing
	uint8 palette;

	/// Printer orientation
	uint8 orientation;

	/// Printer direction
	uint8 direction;

	/// Printer segment (BGMAP memory)
	int8 printingBgmapSegment;

	/// @publicsection

	/// Set mode to debug to bypass loading fonts through CharSets
	static void setDebugMode();

	/// Add fonts to internal VirtualList and preload CharSets for specified fonts.
	/// @param fontSpecs: Array of font specs whose charset should pre preloaded
	static void loadFonts(FontSpec** fontSpecs);

	/// Release fonts, free VRAM
	static void releaseFonts();

	/// Clear printing area in BGMAP memory
	static void clear();

	/// Clear a given row in the printable area of BGMAP memory.
	/// @param row: Row to clean (0-27)
	static void clearRow(uint16 row);

	/// Print a string.
	/// @param string: String to print
	/// @param x: Column to start printing at
	/// @param y: Row to start printing at
	/// @param font: Name of font to use for printing
	static void text(const char* string, int32 x, int32 y, const char* font);

	/// Print an integer value.
	/// @param value: Integer to print
	/// @param x: Column to start printing at
	/// @param y: Row to start printing at
	/// @param font: Name of font to use for printing
	static void int32(int32 value, uint8 x, uint8 y, const char* font);

	/// Print a hex value.
	/// @param value: Hex value to print
	/// @param x: Column to start printing at
	/// @param y: Row to start printing at
	/// @param length: Digits to print
	/// @param font: Name of font to use for printing
	static void hex(WORD value, uint8 x, uint8 y, uint8 length, const char* font);

	/// Print a float value.
	/// @param value: Float value to print
	/// @param x: Column to start printing at
	/// @param y: Row to start printing at
	/// @param precision: How many decimals to print
	/// @param font: Name of font to use for printing
	static void float(float value, uint8 x, uint8 y, int32 precision, const char* font);

	/// Writes the font's char set to CHAR memory with displacing the source pointer
	/// by numberOfChars * page.
	/// @param font: Name of font to use for printing
	/// @param page: ROM's displacement multiplier
	static void setFontPage(const char* font, uint16 page);

	/// Sets the orientation for the following call to print.
	/// Resets its self automatically to horizonal.
	/// @param value: PrintingOrientation
	static void setOrientation(uint8 value);

	/// Sets the direction for the following call to print.
	/// Resets its self automatically to LTR (Left to Right).
	/// @param value: PrintingDirection
	static void setTextDirection(uint8 value);

	/// Cache the printing bgmap segment.
	/// @param printingBgmapSegment: Index of the bgmap segment to print to
	static void setPrintingBgmapSegment(int8 printingBgmapSegment);

	/// Get the printing bgmap segment.
	/// @return Index of the bgmap segment to print to
	static int8 getPrintingBgmapSegment();

	/// Create a printing sprite.
	static void addSprite();

	/// Set the active printing sprite.
	/// @param  printingSpriteIndex: Index of the sprite to activate
	static bool setActiveSprite(uint16 printingSpriteIndex);

	/// Print active printing sprite's info.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void printSprite(int16 x, int16 y);

	/// Set the coordinates used for printing.
	/// @param x: WORLD x coordinate
	/// @param y: WORLD y coordinate
	/// @param z: WORLD parallax value
	/// @param parallax: WORLD parallax value
	static void setCoordinates(int16 x, int16 y, int16 z, int8 parallax);

	/// Set the coordinates of the WORLD used for printing.
	/// @param x: WORLD x coordinate
	/// @param y: WORLD y coordinate
	/// @param z: WORLD parallax value
	/// @param parallax: WORLD parallax value
	static void setWorldCoordinates(int16 x, int16 y, int16 z, int8 parallax);

	/// Set the coordinates of the BGMAP used for printing.
	/// @param mx: BGMAP x coordinate
	/// @param my: BGMAP y coordinate
	/// @param mp: BGMAP parallax value
	static void setBgmapCoordinates(int16 mx, int16 my, int8 mp);

	/// Set WORLD's size.
	/// @param w: WORLD's width
	/// @param h: WORLD's height
	static void setWorldSize(uint16 w, uint16 h);

	/// Set sprite's transparency
	/// @param transparency: Transparent value (__TRANSPARENCY_NONE, _EVEN or _ODD)
	static void setTransparency(uint8 transparency);

	/// Set palette for the printing area.
	/// @param palette: Palette for the printing area
	static void setPalette(uint8 palette);

	/// Reset the coordinates of the WORLD used for printing.
	static void resetCoordinates();

	/// Retrieve WORLD's gx.
	/// @return WORLD's gx
	static int16 getWorldCoordinatesX();

	/// Retrieve WORLD's gy.
	/// @return: WORLD's gy
	static int16 getWorldCoordinatesY();

	/// Retrieve WORLD's gp.
	/// @return: WORLD's gp
	static int16 getWorldCoordinatesP();

	/// Retrieve the active sprite's position.
	/// @return Sprite's position
	static PixelVector getActiveSpritePosition();

	/// Get font spec and starting position in character memory.
	/// @param font: Name of font to get spec for
	/// @return	FontData of desired font or default font if NULL or none could be found matching the name
	static FontData* getFontByName(const char* font);

	/// Get the size of a (block of) text so you can for example center it on screen.
	/// @param string: String to compute size for
	/// @param font: Name of font to use for size computation
	static FontSize getTextSize(const char* string, const char* font);

	/// Process an event that the instance is listen for.
	/// @param eventFirer: ListenerObject that signals the event
	/// @param eventCode: Code of the firing event
	/// @return False if the listener has to be removed; true to keep it
	override bool onEvent(ListenerObject eventFirer, uint16 eventCode);

	/// A component has been removed from this entity. 
	/// @param component: Removed component
	override void removedComponent(Component component);

	/// Empties internal virtual list of registered fonts
	void reset();
}

#endif
