/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern FontROMDef* const __FONTS[];


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// horizontal tab size in chars
#define TAB_SIZE	4


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	Printing
 * @extends Object
 * @ingroup graphics-2d
 * @brief 	Manages printing layer and offers various functions to write to it.
 */
__CLASS_DEFINITION(Printing, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Printing_constructor(Printing this);
static void Printing_out(Printing this, u8 x, u8 y, const char* string, const char* font);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Printing_getInstance()
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
static void __attribute__ ((noinline)) Printing_constructor(Printing this)
{
	this->fonts = __NEW(VirtualList);

	__CONSTRUCT_BASE(Object);
}

/**
 * Class destructor
 *
 * @memberof	Printing
 * @public
 *
 * @param this	Function scope
 */
void Printing_destructor(Printing this)
{
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
void __attribute__ ((noinline)) Printing_render(Printing this __attribute__ ((unused)), int textLayer)
{
	ASSERT(!(0 > textLayer || textLayer >= __TOTAL_LAYERS), "Printing::render: invalid layer");

	_worldAttributesBaseAddress[textLayer].head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | (BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()));
	_worldAttributesBaseAddress[textLayer].mx = 0;
	_worldAttributesBaseAddress[textLayer].mp = 0;
	_worldAttributesBaseAddress[textLayer].my = 0;
	_worldAttributesBaseAddress[textLayer].gx = __PRINTING_BGMAP_X_OFFSET;
	_worldAttributesBaseAddress[textLayer].gp = __PRINTING_BGMAP_Z_OFFSET;
	_worldAttributesBaseAddress[textLayer].gy = __PRINTING_BGMAP_Y_OFFSET;
	_worldAttributesBaseAddress[textLayer].w = __SCREEN_WIDTH;
	_worldAttributesBaseAddress[textLayer].h = __SCREEN_HEIGHT;
}

/**
 * Empties internal virtual list of registered fonts
 *
 * @memberof	Printing
 * @public
 *
 * @param this	Function scope
 */
void Printing_reset(Printing this)
{
	VirtualNode node = VirtualList_begin(this->fonts);

	for(; node; node = VirtualNode_getNext(node))
	{
		__DELETE_BASIC(VirtualNode_getData(node));
	}

	VirtualList_clear(this->fonts);
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
void __attribute__ ((noinline)) Printing_loadFonts(Printing this, FontDefinition** fontDefinitions)
{
	// empty list of registered fonts
	Printing_reset(this);

	// iterate over all defined fonts and add to internal list
	u32 i = 0, j = 0;
	for(i = 0; __FONTS[i]; i++)
	{
		// instance and initialize a new fontdata instance
		FontData* fontData = __NEW_BASIC(FontData);
		fontData->fontDefinition = __FONTS[i];
		fontData->charSet = NULL;

		// preload charset for font if in list of fonts to preload
		if(fontDefinitions)
		{
			// find defined font in list of fonts to preload
			for(j = 0; fontDefinitions[j]; j++)
			{
				// preload charset and save charset reference, if font was found
				if(__FONTS[i]->charSetDefinition == fontDefinitions[j]->charSetDefinition)
				{
					CharSet charSet = CharSetManager_getCharSet(CharSetManager_getInstance(), fontDefinitions[j]->charSetDefinition);
					fontData->charSet = charSet;
				}
			}
		}

		// add fontdata to internal list
		VirtualList_pushBack(this->fonts, fontData);
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
void __attribute__ ((noinline)) Printing_clear(Printing this __attribute__ ((unused)))
{
	u32 printingBgmap = BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance());

	VIPManager_clearBgmapSegment(VIPManager_getInstance(), printingBgmap, __PRINTABLE_BGMAP_AREA);
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
FontData* Printing_getFontByName(Printing this, const char* font)
{
	// set first defined font as default
	FontData* result = VirtualList_front(this->fonts);

	// iterate over registered fonts to find definition of font to use
	VirtualNode node = VirtualList_begin(this->fonts);
	for(; node; node = VirtualNode_getNext(node))
	{
		FontData* fontData = VirtualNode_getData(node);
		if(!strcmp(fontData->fontDefinition->name, font))
		{
			result = fontData;
			break;
		}
	}

	// if font's charset has not been preloaded, load it now
	if(result && !result->charSet)
	{
		result->charSet = CharSetManager_getCharSet(CharSetManager_getInstance(), result->fontDefinition->charSetDefinition);
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
static void __attribute__ ((noinline)) Printing_out(Printing this, u8 x, u8 y, const char* string, const char* font)
{
	u32 i = 0;
	u32 position = 0;
	u32 startColumn = x;
	u32 charOffsetX = 0;
	u32 charOffsetY = 0;
	u32 printingBgmap = BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance());

	FontData* fontData = Printing_getFontByName(this, font);

	if(!fontData)
	{
		return;
	}

	u16* const bgmapSpaceBaseAddress = (u16*)__BGMAP_SPACE_BASE_ADDRESS;

	// print text
	while(string[i] && x < (__SCREEN_WIDTH >> 3))
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
									CharSet_getOffset(fontData->charSet) +

									// top left char of letter
									((u8)(string[i] - fontData->fontDefinition->offset) * fontData->fontDefinition->fontSize.x) +

									// skip lower chars of multi-char fonts with y > 1
									((((u8)(string[i] - fontData->fontDefinition->offset) * fontData->fontDefinition->fontSize.x) >> 5) * ((fontData->fontDefinition->fontSize.y - 1)) << 5) +

									// respective char of letter in multi-char fonts
									charOffsetX + (charOffsetY << 5)
								)
								| (__PRINTING_PALETTE << 14);
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
void __attribute__ ((noinline)) Printing_int(Printing this, int value, u8 x, u8 y, const char* font)
{
	if(value < 0)
	{
		value *= -1;

		Printing_out(this, x++, y, "-", font);
		Printing_out(this, x, y, Utilities_itoa((int)(value), 10, Utilities_getDigitCount(value)), font);
	}
	else
	{
		Printing_out(this, x, y, Utilities_itoa((int)(value), 10, Utilities_getDigitCount(value)), font);
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
void __attribute__ ((noinline)) Printing_hex(Printing this, WORD value, u8 x, u8 y, u8 length, const char* font)
{
	Printing_out(this, x,y, Utilities_itoa((int)(value), 16, length), font);
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
void __attribute__ ((noinline)) Printing_float(Printing this, float value, u8 x, u8 y, const char* font)
{
	int sign = 1;
	int i = 0;
	int length;
	int size = 1000;

	#define FIX19_13_FRAC(n)	((n)&0x1FFF)

	int decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);

	if(value < 0)
	{
		sign = -1;
		Printing_out(this, x++, y, "-", font);

		decimal = (int)(((__1I_FIX19_13 - (float)FIX19_13_FRAC(FTOFIX19_13(value))) / 8192.f) * 10000.f);
	}
	else
	{
		decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);
	}

	// print integral part
	length = Utilities_intLength((int)value * sign);

	Printing_out(this, x, y, Utilities_itoa(F_FLOOR(value * sign), 10, length), font);

	// print the dot
	Printing_out(this, x + length, y, ".", font);

	// print the decimal part
	for(i = 0; size; i++)
	{
		if(decimal < size)
		{
			Printing_out(this, x + length + 1 + i,y, Utilities_itoa(0, 10, 1), font);
		}
		else
		{
			i++;
			break;
		}

		size /= 10;
	}

	Printing_out(this, x + length + i, y, Utilities_itoa(decimal, 10, 0), font);
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
void __attribute__ ((noinline)) Printing_text(Printing this, const char* string, int x, int y, const char* font)
{
	Printing_out(this, x, y, string, font);
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
Size __attribute__ ((noinline)) Printing_getTextSize(Printing this, const char* string, const char* font)
{
	Size size = {0, 0, 0};
	u16 i = 0, currentLineLength = 0;

	FontData* fontData = Printing_getFontByName(this, font);

	if(!fontData)
	{
		// just to make sure that no client code does a 0 division with these results
		size = (Size){8, 8, 8};
		return size;
	}

	size.y = fontData->fontDefinition->fontSize.y;

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

				size.y += fontData->fontDefinition->fontSize.y;
				currentLineLength = 0;
				break;

			default:

				currentLineLength += fontData->fontDefinition->fontSize.x;
				if(currentLineLength >= 64)
				{
					size.y += fontData->fontDefinition->fontSize.y;
					currentLineLength = 0;
				}

				break;
		}

		if(currentLineLength > size.x)
		{
			size.x = currentLineLength;
		}

		i++;
	}

	return size;
}
