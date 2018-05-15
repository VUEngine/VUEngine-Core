/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <Printing.h>
#include <CharSetManager.h>
#include <BgmapTextureManager.h>
#include <HardwareManager.h>
#include <Utilities.h>
#include <Mem.h>
#include <VirtualList.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern FontROMDef* const __FONTS[];
extern FontROMDef VUENGINE_FONT;


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// horizontal tab size in chars
#define TAB_SIZE	4

// fontdata for debug output
#define VUENGINE_DEBUG_FONT_SIZE	160
FontROMData VUENGINE_DEBUG_FONT_DATA =
{
	// font definition
	(FontDefinition*)&VUENGINE_FONT,

	// offset of font in char memory
	__CHAR_MEMORY_TOTAL_CHARS - VUENGINE_DEBUG_FONT_SIZE,
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	Printing
 * @extends Object
 * @ingroup graphics-2d
 * @brief 	Manages printing layer and offers various functions to write to it.
 */
implements Printing : Object;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Printing::constructor(Printing this);
static void Printing::out(Printing this, u8 x, u8 y, const char* string, const char* font);
void Printing::loadDebugFont(Printing this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Printing::getInstance()
 * @memberof	Printing
 * @public
 *
 * @return		Printing instance
 */
__SINGLETON(Printing);

/**
 * Class constructor
 *
 * @memberof	Printing
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) Printing::constructor(Printing this)
{
	ASSERT(this, "Printing::constructor: null this");

	Base::constructor();

	// initialize members
	this->fonts = __NEW(VirtualList);
	this->mode = __PRINTING_MODE_DEFAULT;
	this->palette = __PRINTING_PALETTE;
	this->gx = __PRINTING_BGMAP_X_OFFSET;
	this->gy = __PRINTING_BGMAP_Y_OFFSET;
}

/**
 * Class destructor
 *
 * @memberof	Printing
 * @public
 *
 * @param this	Function scope
 */
void Printing::destructor(Printing this)
{
	ASSERT(this, "Printing::destructor: null this");

	__DELETE(this->fonts);

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Render general print output layer
 *
 * @memberof		Printing
 * @public
 *
 * @param this		Function scope
 * @param textLayer	Number of layer (World) to set as printing layer
 */
void __attribute__ ((noinline)) Printing::render(Printing this __attribute__ ((unused)), int textLayer)
{
	ASSERT(this, "Printing::render: null this");

	ASSERT(!(0 > textLayer || textLayer >= __TOTAL_LAYERS), "Printing::render: invalid layer");

	_worldAttributesBaseAddress[textLayer].head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | (BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()));
	_worldAttributesBaseAddress[textLayer].mx = this->gx;
	_worldAttributesBaseAddress[textLayer].mp = 0;
	_worldAttributesBaseAddress[textLayer].my = this->gy;
	_worldAttributesBaseAddress[textLayer].gx = this->gx;
	_worldAttributesBaseAddress[textLayer].gp = __PRINTING_BGMAP_Z_OFFSET;
	_worldAttributesBaseAddress[textLayer].gy = this->gy;
	_worldAttributesBaseAddress[textLayer].w = __SCREEN_WIDTH - this->gx - 1;
	_worldAttributesBaseAddress[textLayer].h = __SCREEN_HEIGHT - this->gy - 1;
}

/**
 * Empties internal virtual list of registered fonts
 *
 * @memberof	Printing
 * @public
 *
 * @param this	Function scope
 */
void Printing::reset(Printing this)
{
	ASSERT(this, "Printing::destructor: null this");

	VirtualNode node = VirtualList::begin(this->fonts);

	for(; node; node = VirtualNode::getNext(node))
	{
		__DELETE_BASIC(VirtualNode::getData(node));
	}

	VirtualList::clear(this->fonts);

	this->gx = __PRINTING_BGMAP_X_OFFSET;
	this->gy = __PRINTING_BGMAP_Y_OFFSET;
}

/**
 * Add fonts to internal VirtualList and preload CharSets for specified fonts
 *
 * @memberof				Printing
 * @public
 *
 * @param this				Function scope
 * @param fontDefinitions	Array of font definitions whose charset should pre preloaded
 */
void __attribute__ ((noinline)) Printing::loadFonts(Printing this, FontDefinition** fontDefinitions)
{
	ASSERT(this, "Printing::loadFonts: null this");

	// empty list of registered fonts
	Printing::reset(this);

	// iterate over all defined fonts and add to internal list
	u32 i = 0, j = 0;
	for(i = 0; __FONTS[i]; i++)
	{
		// instance and initialize a new fontdata instance
		FontData* fontData = __NEW_BASIC(FontData);
		fontData->fontDefinition = __FONTS[i];
		fontData->offset = 0;

		// preload charset for font if in list of fonts to preload
		if(fontDefinitions)
		{
			// find defined font in list of fonts to preload
			for(j = 0; fontDefinitions[j]; j++)
			{
				// preload charset and save charset reference, if font was found
				if(__FONTS[i]->charSetDefinition == fontDefinitions[j]->charSetDefinition)
				{
					fontData->offset = CharSet::getOffset(CharSetManager::getCharSet(CharSetManager::getInstance(), fontDefinitions[j]->charSetDefinition));
				}
			}
		}

		// add fontdata to internal list
		VirtualList::pushBack(this->fonts, fontData);
	}
}

/**
 * Load engine's default font to end of char memory directly (for debug purposes)
 *
 * @memberof	Printing
 * @private
 *
 * @param this	Function scope
 */
void __attribute__ ((noinline)) Printing::loadDebugFont(Printing this __attribute__ ((unused)))
{
	ASSERT(this, "Printing::destructor: null this");

	Mem::copyBYTE(
		(u8*)(__CHAR_SPACE_BASE_ADDRESS + (VUENGINE_DEBUG_FONT_DATA.offset << 4)),
		(u8*)(VUENGINE_DEBUG_FONT_DATA.fontDefinition->charSetDefinition->charDefinition),
		VUENGINE_DEBUG_FONT_SIZE << 4
	);
}

/**
 * Set mode to debug to bypass loading fonts through CharSets
 *
 * @memberof	Printing
 * @public
 *
 * @param this	Function scope
 */
void Printing::setDebugMode(Printing this)
{
	ASSERT(this, "Printing::setDebugMode: null this");

	Printing::resetWorldCoordinates(this);
	Printing::loadDebugFont(this);
	this->mode = __PRINTING_MODE_DEBUG;
}

/**
 * Set palette
 *
 * @memberof	Printing
 * @public
 *
 * @param this	Function scope
 */
void Printing::setPalette(Printing this, u8 palette)
{
	ASSERT(this, "Printing::destructor: null this");

	if(palette < 4)
	{
		this->palette = palette;
	}
}

/**
 * Clear printing area
 *
 * @memberof	Printing
 * @public
 *
 * @param this	Function scope
 */
void __attribute__ ((noinline)) Printing::clear(Printing this __attribute__ ((unused)))
{
	ASSERT(this, "Printing::clear: null this");

	u32 printingBgmap = __PRINTING_MODE_DEBUG == this->mode? __EXCEPTIONS_BGMAP : BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());

	VIPManager::clearBgmapSegment(VIPManager::getInstance(), printingBgmap, __PRINTABLE_BGMAP_AREA);
}

/**
 * Get font definition and starting position in character memory
 *
 * @memberof	Printing
 * @private
 *
 * @param this	Function scope
 * @param font	Name of font to get definition for
 *
 * @return		FontData of desired font or default font if NULL or none could be found matching the name
 */
FontData* Printing::getFontByName(Printing this, const char* font)
{
	ASSERT(this, "Printing::getFontByName: null this");

	FontData* result = NULL;

	if(this->mode == __PRINTING_MODE_DEBUG)
	{
		result = (FontData*)&VUENGINE_DEBUG_FONT_DATA;
	}
	else if(this->fonts)
	{
		// set first defined font as default
		result = VirtualList::front(this->fonts);

		if(result)
		{
			if(font)
			{
				// iterate over registered fonts to find definition of font to use
				VirtualNode node = VirtualList::begin(this->fonts);
				for(; node; node = VirtualNode::getNext(node))
				{
					FontData* fontData = VirtualNode::getData(node);
					if(!strcmp(fontData->fontDefinition->name, font))
					{
						result = fontData;
						break;
					}
				}
			}

			// if font's charset has not been preloaded, load it now
			if(!result->offset)
			{
				result->offset = CharSet::getOffset(CharSetManager::getCharSet(CharSetManager::getInstance(), result->fontDefinition->charSetDefinition));
			}
		}
	}

	return result;
}

/**
 * Direct printing out method
 *
 * @memberof		Printing
 * @private
 *
 * @param this		Function scope
 * @param x			Column to start printing at
 * @param y			Row to start printing at
 * @param string	String to print
 * @param font		Name of font to use for printing
 */
static void __attribute__ ((noinline)) Printing::out(Printing this, u8 x, u8 y, const char* string, const char* font)
{
	ASSERT(this, "Printing::out: null this");

	u32 i = 0;
	u32 position = 0;
	u32 startColumn = x;
	u32 charOffsetX = 0;
	u32 charOffsetY = 0;
	u32 printingBgmap = __PRINTING_MODE_DEBUG == this->mode ? __EXCEPTIONS_BGMAP : BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());

	FontData* fontData = Printing::getFontByName(this, font);

	if(!fontData)
	{
		return;
	}

	u16* const bgmapSpaceBaseAddress = (u16*)__BGMAP_SPACE_BASE_ADDRESS;

	// print text
	while(string[i] && x < (__SCREEN_WIDTH_IN_CHARS))
	{
		// do not allow printing outside of the visible area, since that would corrupt the param table
		if(y >= 28)
		{
			break;
		}

		position = (y << 6) + x;

		switch(string[i])
		{
			// line feed
			case 13:

				break;

			// tab
			case 9:

				x = (x / TAB_SIZE + 1) * TAB_SIZE * fontData->fontDefinition->fontSize.x;
				break;

			// carriage return
			case 10:

				y += fontData->fontDefinition->fontSize.y;
				x = startColumn;
				break;

			default:
				{
					for(charOffsetX = 0; charOffsetX < fontData->fontDefinition->fontSize.x; charOffsetX++)
					{
						for(charOffsetY = 0; charOffsetY < fontData->fontDefinition->fontSize.y; charOffsetY++)
						{
							bgmapSpaceBaseAddress[(0x1000 * printingBgmap) + position + charOffsetX + (charOffsetY << 6)] =
								(
									// font offset
									fontData->offset +

									// top left char of letter
									((u8)(string[i] - fontData->fontDefinition->offset) * fontData->fontDefinition->fontSize.x) +

									// skip lower chars of multi-char fonts with y > 1
									((((u8)(string[i] - fontData->fontDefinition->offset) * fontData->fontDefinition->fontSize.x) >> 5) * ((fontData->fontDefinition->fontSize.y - 1)) << 5) +

									// respective char of letter in multi-char fonts
									charOffsetX + (charOffsetY << 5)
								)
								| (this->palette << 14);
						}
					}
				}

				x += fontData->fontDefinition->fontSize.x;
				if(x >= 48)
				{
					// wrap around when outside of the visible area
					y += fontData->fontDefinition->fontSize.y;
					x = startColumn;
				}

				break;
		}
		i++;
	}
}

/**
 * Print an Integer value
 *
 * @memberof	Printing
 * @public
 *
 * @param this	Function scope
 * @param value	Integer to print
 * @param x		Column to start printing at
 * @param y		Row to start printing at
 * @param font	Name of font to use for printing
 */
void __attribute__ ((noinline)) Printing::int(Printing this, int value, u8 x, u8 y, const char* font)
{
	ASSERT(this, "Printing::int: null this");

	if(value < 0)
	{
		value *= -1;

		Printing::out(this, x++, y, "-", font);
		Printing::out(this, x, y, Utilities::itoa((int)(value), 10, Utilities::getDigitCount(value)), font);
	}
	else
	{
		Printing::out(this, x, y, Utilities::itoa((int)(value), 10, Utilities::getDigitCount(value)), font);
	}
}

/**
 * Print a hex value
 *
 * @memberof		Printing
 * @public
 *
 * @param this		Function scope
 * @param value		Hex value to print
 * @param x			Column to start printing at
 * @param y			Row to start printing at
 * @param length	digits to print
 * @param font		Name of font to use for printing
 */
void __attribute__ ((noinline)) Printing::hex(Printing this, WORD value, u8 x, u8 y, u8 length, const char* font)
{
	ASSERT(this, "Printing::destructor: null this");

	Printing::out(this, x,y, Utilities::itoa((int)(value), 16, length), font);
}

/**
 * Print a float value
 *
 * @memberof	Printing
 * @public
 *
 * @param this	Function scope
 * @param value	Float value to print
 * @param x		Column to start printing at
 * @param y		Row to start printing at
 * @param font	Name of font to use for printing
 */
void __attribute__ ((noinline)) Printing::float(Printing this, float value, u8 x, u8 y, const char* font)
{
	ASSERT(this, "Printing::float: null this");

	if(0 > value)
	{
		value = -value;
		Printing::text(this, "-", x++, y, font);
	}

	int integer = (int)__FIX19_13_TO_I(__F_TO_FIX19_13(value));
	int decimal = (int)(((float)__FIX19_13_FRAC(__F_TO_FIX19_13(value)) / 8192.f) * 100.f);
	int length = Utilities::intLength(__ABS(integer)) + (0 > value ? 1 : 0);

	Printing::int(this, integer, x, y, font);

	Printing::text(this, ".", x + length, y, font);

	if(decimal)
	{
		int auxDecimal = decimal;
		int displacement = 0;
		while(!(auxDecimal / 10))
		{
			auxDecimal *= 10;
			Printing::int(this, 0, x + length + 1 + displacement++, y, font);
		}

		Printing::int(this, decimal, x + length + 1 + displacement, y, font);
	}
	else
	{
		Printing::int(this, 0, x + length + 1, y, font);
	}
}

/**
 * Print a string
 *
 * @memberof		Printing
 * @public
 *
 * @param this		Function scope
 * @param string	String to print
 * @param x			Column to start printing at
 * @param y			Row to start printing at
 * @param font		Name of font to use for printing
 */
void __attribute__ ((noinline)) Printing::text(Printing this, const char* string, int x, int y, const char* font)
{
	ASSERT(this, "Printing::text: null this");

#ifdef __FORCE_UPPERCASE
	Printing::out(this, x, y, Utilities::toUppercase(string), font);
#else
	Printing::out(this, x, y, string, font);
#endif
}

/**
 * Set the coordinates of the WORLD used for printing
 *
 * @memberof		Printing
 * @public
 *
 * @param this		Function scope
 * @param gx		WORLD x coordinate
 * @param gy		WORLD y coordinate
 */
#ifdef __FORCE_PRINTING_LAYER
void Printing::setWorldCoordinates(Printing this, u16 gx __attribute__ ((unused)), u16 gy __attribute__ ((unused)))
{
	ASSERT(this, "Printing::setWorldCoordinates: null this");

	this->gx = 0;
	this->gy = 0;
}
#else
void Printing::setWorldCoordinates(Printing this, u16 gx, u16 gy)
{
	ASSERT(this, "Printing::setWorldCoordinates: null this");

	this->gx = gx <= __SCREEN_WIDTH ? gx : 0;
	this->gy = gy <= __SCREEN_HEIGHT ? gy : 0;
}
#endif

/**
 * Reset the coordinates of the WORLD used for printing
 *
 * @memberof		Printing
 * @public
 *
 * @param this		Function scope
 */
void Printing::resetWorldCoordinates(Printing this)
{
	ASSERT(this, "Printing::destructor: null this");

	this->gx = __PRINTING_BGMAP_X_OFFSET;
	this->gy = __PRINTING_BGMAP_Y_OFFSET;
}


/**
 * Retrieve the pixels used by the WORLD for printing
 *
 * @memberof		Printing
 * @public
 *
 * @param this		Function scope
 *
 * @return			number of pixels
 */
int Printing::getPixelCount(Printing this)
{
	ASSERT(this, "Printing::getPixelCount: null this");

	return (__SCREEN_WIDTH - this->gx) * (__SCREEN_HEIGHT - this->gy);
}


/**
 * Get the size of a (block of) text so you can for example center it on screen
 *
 * @memberof		Printing
 * @public
 *
 * @param this		Function scope
 * @param string	String to compute size for
 * @param font		Name of font to use for size computation
 */
FontSize __attribute__ ((noinline)) Printing::getTextSize(Printing this, const char* string, const char* font)
{
	ASSERT(this, "Printing::getTextSize: null this");

	FontSize fontSize = {0, 0};
	u16 i = 0, currentLineLength = 0;

	FontData* fontData = Printing::getFontByName(this, font);

	if(!fontData)
	{
		// just to make sure that no client code does a 0 division with these results
		fontSize = (FontSize){8, 8};
		return fontSize;
	}

	fontSize.y = fontData->fontDefinition->fontSize.y;

	while(string[i])
	{
		switch(string[i])
		{
			// line feed
			case 13:

				break;

			// tab
			case 9:

				currentLineLength += (currentLineLength / TAB_SIZE + 1) * TAB_SIZE * fontData->fontDefinition->fontSize.x;
				break;

			// carriage return
			case 10:

				fontSize.y += fontData->fontDefinition->fontSize.y;
				currentLineLength = 0;
				break;

			default:

				currentLineLength += fontData->fontDefinition->fontSize.x;
				if(currentLineLength >= 64)
				{
					fontSize.y += fontData->fontDefinition->fontSize.y;
					currentLineLength = 0;
				}

				break;
		}

		if(currentLineLength > fontSize.x)
		{
			fontSize.x = currentLineLength;
		}

		i++;
	}

	return fontSize;
}
