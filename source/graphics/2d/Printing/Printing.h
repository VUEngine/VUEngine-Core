/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PRINTING_H_
#define	PRINTING_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>
#include <PrintingSprite.h>


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

// some handy macros
#define PRINT_TEXT(string, x, y)			Printing::text(Printing::getInstance(), string, x, y, NULL)
#define PRINT_INT(number, x, y)				PRINT_TEXT("        ", x, y); Printing::int32(Printing::getInstance(), number, x, y, NULL)
#define PRINT_FLOAT(number, x, y)			PRINT_TEXT("        ", x, y);Printing::float(Printing::getInstance(), number, x, y, 2, NULL)
#define PRINT_HEX(number, x, y)				PRINT_TEXT("        ", x, y);Printing::hex(Printing::getInstance(), number, x, y, 8, NULL)
#define PRINT_HEX_EXT(number, x, y, d)		PRINT_TEXT("        ", x, y);Printing::hex(Printing::getInstance(), number, x, y, d, NULL)

// horizontal tab size in chars
#define __TAB_SIZE					4

// max length of a font's name
#define __MAX_FONT_NAME_LENGTH		16

// printing modes
#define __PRINTING_MODE_DEFAULT		0
#define __PRINTING_MODE_DEBUG		1

// default font special chars
#define __CHAR_BATTERY				"\x01\x02"

#define __CHAR_LINE_TOP_LEFT		"\x03"
#define __CHAR_LINE_TOP_RIGHT		"\x04"
#define __CHAR_LINE_BOTTOM_LEFT		"\x05"
#define __CHAR_LINE_BOTTOM_RIGHT	"\x06"
#define __CHAR_LINE_VERTICAL		"\x07"
#define __CHAR_LINE_HORIZONTAL		"\x08"

#define __CHAR_SELECTOR				"\x0B"
#define __CHAR_SELECTOR_LEFT		"\x0C"

#define __CHAR_DARK_RED_BOX			"\x0E"
#define __CHAR_MEDIUM_RED_BOX		"\x0F"
#define __CHAR_BRIGHT_RED_BOX		"\x10"

#define __CHAR_CHECKBOX_UNCHECKED	"\x11"
#define __CHAR_CHECKBOX_CHECKED		"\x12"

#define __CHAR_START_BUTTON			"\x15"
#define __CHAR_SELECT_BUTTON		"\x16"
#define __CHAR_A_BUTTON				"\x13"
#define __CHAR_B_BUTTON				"\x14"
#define __CHAR_L_TRIGGER			"\x17"
#define __CHAR_R_TRIGGER			"\x18"
#define __CHAR_D_PAD				"\x19"
#define __CHAR_D_PAD_UP				"\x1A"
#define __CHAR_D_PAD_DOWN			"\x1B"
#define __CHAR_D_PAD_LEFT			"\x1C"
#define __CHAR_D_PAD_RIGHT			"\x1D"
#define __CHAR_L					"\x1E"
#define __CHAR_R					"\x1F"

#define __CHAR_L_D_PAD				"\x1E\x19"
#define __CHAR_L_D_PAD_UP			"\x1E\x1A"
#define __CHAR_L_D_PAD_DOWN			"\x1E\x1B"
#define __CHAR_L_D_PAD_LEFT			"\x1E\x1C"
#define __CHAR_L_D_PAD_RIGHT		"\x1E\x1D"
#define __CHAR_R_D_PAD				"\x1F\x19"
#define __CHAR_R_D_PAD_UP			"\x1F\x1A"
#define __CHAR_R_D_PAD_DOWN			"\x1F\x1B"
#define __CHAR_R_D_PAD_LEFT			"\x1F\x1C"
#define __CHAR_R_D_PAD_RIGHT		"\x1F\x1D"


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * Print oritentation
 */
enum PrintingOrientation
{
	kPrintingOrientationHorizontal = 0,
	kPrintingOrientationVertical
};

/**
 * Print direction
 */
enum PrintingDirection
{
	kPrintingDirectionLTR = 0,
	kPrintingDirectionRTL
};

/**
 * Size of a font's characters (in chars)
 *
 * @memberof 	Printing
 */
typedef struct FontSize
{
	uint16 x;
	uint16 y;

} FontSize;

/**
 * A font
 *
 * @memberof 	Printing
 */
typedef struct FontSpec
{
	/// font charset spec pointer
	CharSetSpec* charSetSpec;

	/// at which character number the font starts
	int16 offset;

	/// number of characters in this font
	uint16 characterCount;

	/// number of characters per line in charset
	uint16 charactersPerLineInCharset;

	/// size of a single character (in chars) ({width, height})
	FontSize fontSize;

	/// font's name
	char name[__MAX_FONT_NAME_LENGTH];

} FontSpec;

/**
 * A FontSpec that is stored in ROM
 *
 * @memberof 	Printing
 */
typedef const FontSpec FontROMSpec;

/**
 * A FontSpec plus the offset of its charset in memory
 *
 * @memberof 	Printing
 */
typedef struct FontData
{
	/// Font spec
	const struct FontSpec* fontSpec;

	/// CharSet
	CharSet charSet;

} FontData;

/**
 * A FontData that is stored in ROM
 *
 * @memberof 	Printing
 */
typedef const FontData FontROMData;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * Manages printing layer and offers various functions to write to it.
 *
 * @ingroup graphics-2d
 */
singleton class Printing : Object
{
	/// @protectedsection

	// sprite
	PrintingSprite printingSprite;

	// A list of loaded fonts and their respective CharSets
	VirtualList fonts;
	// Cache the last used font to speed up searchs
	FontData* lastUsedFontData;
	// Printing mode (Default or Debug)
	uint8 mode;
	// Palette to use for printing
	uint8 palette;
	// printing orientation
	uint8 orientation;
	// printing direction
	uint8 direction;

	/// @publicsection

	/** Get instance
	 *
	 * @return		Printing instance
	 */
	static Printing getInstance();

	/**
	 * Clear printing area
	 */
	void clear();

	/**
	 * Release fonts, free VRAM
	 */
	void releaseFonts();

	/**
	 * Print a float value
	 *
	 * @param value	Float value to print
	 * @param x				Column to start printing at
	 * @param y				Row to start printing at
	 * @param precision		How many decimals to print
	 * @param font	Name of font to use for printing
	 */
	void float(float value, uint8 x, uint8 y, int32 precision, const char* font);

	/**
     * Get font spec and starting position in character memory
     *
     * @param font	Name of font to get spec for
     * @return		FontData of desired font or default font if NULL or none could be found matching the name
     */
	FontData* getFontByName(const char* font);

	/**
	 * Get the size of a (block of) text so you can for example center it on screen
	 *
	 * @param string	String to compute size for
	 * @param font		Name of font to use for size computation
	 */
	FontSize getTextSize(const char* string, const char* font);

	/**
	 * Print a hex value
	 *
	 * @param value		Hex value to print
	 * @param x			Column to start printing at
	 * @param y			Row to start printing at
	 * @param length	digits to print
	 * @param font		Name of font to use for printing
	 */
	void hex(WORD value, uint8 x, uint8 y, uint8 length, const char* font);

	/**
	 * Print an Integer value
	 *
	 * @param value	Integer to print
	 * @param x		Column to start printing at
	 * @param y		Row to start printing at
	 * @param font	Name of font to use for printing
	 */
	void int32(int32 value, uint8 x, uint8 y, const char* font);

	/**
     * Load engine's default font to end of char memory directly (for debug purposes)
     */
	void loadDebugFont();

	/**
     * Add fonts to internal VirtualList and preload CharSets for specified fonts
     *
     * @param fontSpecs	Array of font specs whose charset should pre preloaded
     */
	void loadFonts(FontSpec** fontSpecs);

	/**
     * Writes the font's char set to CHAR memory with displacing the source pointer
	 * by numberOfChars * page
     *
	 * @param font	Name of font to use for printing
	 * @param page	ROM's displacement multiplier
     */
	void setFontPage(const char* font, uint16 page);

	/**
     * Empties internal virtual list of registered fonts
     */
	void reset();

	/**
     * Setup printing sprite
     */
	void setupSprite();

	/**
     * Sets the orientation for the following call to print.
	 * Resets its self automatically to horizonal.
     */
	void setOrientation(uint8 value);

	/**
     * Sets the direction for the following call to print.
	 * Resets its self automatically to LTR (Left to Right).
     */
	void setDirection(uint8 value);

	/**
	 * Reset the coordinates of the WORLD used for printing
	 */
	void resetCoordinates();

	/**
	 * Set mode to debug to bypass loading fonts through CharSets
	 */
	void setDebugMode();

	/**
	 * Set palette
	 */
	void setPalette(uint8 palette);

	/**
	 * Set the coordinates used for printing
	 *
	 * @param x				WORLD x coordinate
	 * @param y				WORLD y coordinate
	 * @param z				WORLD parallax value
	 * @param parallax		WORLD parallax value
	 */
	void setCoordinates(int16 x, int16 y, int16 z, int8 parallax);

	/**
	 * Set the coordinates of the WORLD used for printing
	 *
	 * @param x				WORLD x coordinate
	 * @param y				WORLD y coordinate
	 * @param z				WORLD parallax value
	 * @param parallax		WORLD parallax value
	 */
	void setWorldCoordinates(int16 x, int16 y, int16 z, int8 parallax);

	/**
	 * Set the coordinates of the BGMAP used for printing
	 *
	 * @param mx		BGMAP x coordinate
	 * @param my		BGMAP y coordinate
	 * @param mp		BGMAP parallax value
	 */
	void setBgmapCoordinates(int16 mx, int16 my, int8 mp);

	/**
	 * Set WORLD's size
	 *
	 * @param w			WORLD's width
	 * @param h			WORLD's height
	 */
	void setWorldSize(uint16 w, uint16 h);

	/**
	 * Retrieve WORLD's gx
	 *
	 * @return			WORLD's gx
	 */
	int16 getWorldCoordinatesX();

	/**
	 * Retrieve WORLD's gy
	 *
	 * @return			WORLD's gy
	 */
	int16 getWorldCoordinatesY();

	/**
	 * Retrieve WORLD's gp
	 *
	 * @return			WORLD's gp
	 */
	int16 getWorldCoordinatesP();

	/**
	 * Print a string
	 *
	 * @param string	String to print
	 * @param x			Column to start printing at
	 * @param y			Row to start printing at
	 * @param font		Name of font to use for printing
	 */
	void text(const char *string, int32 x, int32 y, const char* font);


	/// @privatesection

	/**
	 * Class constructor
	 *
	 * @fn 	void Printing::constructor()
     * @memberof  Printing
	 */
	void constructor();

	/**
	 * Class destructor
	 *
	 * @fn	void Printing::destructor()
     * @memberof  Printing
	 */
	void destructor();

 	/**
 	 * Direct printing out method
 	 *
     * @fn	void Printing::out(uint8 x, uint8 y, const char* string, const char* font)
     * @memberof  Printing
     *
     * @param x			Column to start printing at
     * @param y			Row to start printing at
     * @param string	String to print
     * @param font		Name of font to use for printing
     */
	void out(uint8 x, uint8 y, const char* string, const char* font);

	/**
     * Render general print output layer
     *
     * @param textLayer	Number of layer (World) to set as printing layer
     */
	void render(uint8 textLayer);

	/**
     * Force printing layer to show up
     *
     */
	void show();

	/**
     * Force printing layer to hide
     *
     */
	void hide();
}


#endif
