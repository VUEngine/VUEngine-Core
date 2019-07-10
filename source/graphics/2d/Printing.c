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

extern FontROMSpec* const __FONTS[];
extern FontROMSpec VUENGINE_FONT;


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// horizontal tab size in chars
#define TAB_SIZE	4

// fontdata for debug output
#define VUENGINE_DEBUG_FONT_SIZE	160
FontROMData VUENGINE_DEBUG_FONT_DATA =
{
	// font spec
	(FontSpec*)&VUENGINE_FONT,

	// offset of font in char memory
	__CHAR_MEMORY_TOTAL_CHARS - VUENGINE_DEBUG_FONT_SIZE,
};


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

void Printing::constructor()
{
	Base::constructor();

	// initialize members
	this->fonts = new VirtualList();
	this->mode = __PRINTING_MODE_DEFAULT;
	this->palette = __PRINTING_PALETTE;
	this->gx = __PRINTING_BGMAP_X_OFFSET;
	this->gy = __PRINTING_BGMAP_Y_OFFSET;
	this->gp = __PRINTING_BGMAP_PARALLAX_OFFSET;
}

void Printing::destructor()
{
	delete this->fonts;

	// allow a new construct
	Base::destructor();
}

void Printing::render(int textLayer)
{
	ASSERT(!(0 > textLayer || textLayer >= __TOTAL_LAYERS), "Printing::render: invalid layer");

	_worldAttributesBaseAddress[textLayer].head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | (BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()));
	_worldAttributesBaseAddress[textLayer].mx = this->gx;
	_worldAttributesBaseAddress[textLayer].mp = 0;
	_worldAttributesBaseAddress[textLayer].my = this->gy;
	_worldAttributesBaseAddress[textLayer].gx = this->gx;
	_worldAttributesBaseAddress[textLayer].gp = this->gp;
	_worldAttributesBaseAddress[textLayer].gy = this->gy;
	_worldAttributesBaseAddress[textLayer].w = __SCREEN_WIDTH - this->gx - 1;
	_worldAttributesBaseAddress[textLayer].h = __SCREEN_HEIGHT - this->gy - 1;
}

void Printing::reset()
{
	VirtualNode node = VirtualList::begin(this->fonts);

	for(; node; node = VirtualNode::getNext(node))
	{
		delete VirtualNode::getData(node);
	}

	VirtualList::clear(this->fonts);

	this->gx = __PRINTING_BGMAP_X_OFFSET;
	this->gy = __PRINTING_BGMAP_Y_OFFSET;
	this->gp = __PRINTING_BGMAP_PARALLAX_OFFSET;
}

void Printing::loadFonts(FontSpec** fontSpecs)
{
	// empty list of registered fonts
	Printing::reset(this);

	// iterate over all defined fonts and add to internal list
	u32 i = 0, j = 0;
	for(i = 0; __FONTS[i]; i++)
	{
		// instance and initialize a new fontdata instance
		FontData* fontData = new FontData;
		fontData->fontSpec = __FONTS[i];
		fontData->offset = 0;

		// preload charset for font if in list of fonts to preload
		if(fontSpecs)
		{
			// find defined font in list of fonts to preload
			for(j = 0; fontSpecs[j]; j++)
			{
				// preload charset and save charset reference, if font was found
				if(__FONTS[i]->charSetSpec == fontSpecs[j]->charSetSpec)
				{
					fontData->offset = CharSet::getOffset(CharSetManager::getCharSet(CharSetManager::getInstance(), fontSpecs[j]->charSetSpec));
				}
			}
		}

		// add fontdata to internal list
		VirtualList::pushBack(this->fonts, fontData);
	}
}

void Printing::loadDebugFont()
{
	Mem::copyBYTE(
		(u8*)(__CHAR_SPACE_BASE_ADDRESS + (VUENGINE_DEBUG_FONT_DATA.offset << 4)),
		(u8*)(VUENGINE_DEBUG_FONT_DATA.fontSpec->charSetSpec->charSpec),
		VUENGINE_DEBUG_FONT_SIZE << 4
	);
}

void Printing::setDebugMode()
{
	Printing::resetWorldCoordinates(this);
	Printing::loadDebugFont(this);
	this->mode = __PRINTING_MODE_DEBUG;
}

void Printing::setPalette(u8 palette)
{
	if(palette < 4)
	{
		this->palette = palette;
	}
}

void Printing::clear()
{
	u32 printingBgmap = __PRINTING_MODE_DEBUG == this->mode ? __EXCEPTIONS_BGMAP : BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());

	VIPManager::clearBgmapSegment(VIPManager::getInstance(), printingBgmap, __PRINTABLE_BGMAP_AREA);
}

FontData* Printing::getFontByName(const char* font)
{
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
				// iterate over registered fonts to find spec of font to use
				VirtualNode node = VirtualList::begin(this->fonts);
				for(; node; node = VirtualNode::getNext(node))
				{
					FontData* fontData = VirtualNode::getData(node);
					if(!strcmp(fontData->fontSpec->name, font))
					{
						result = fontData;
						break;
					}
				}
			}

			// if font's charset has not been preloaded, load it now
			if(!result->offset)
			{
				result->offset = CharSet::getOffset(CharSetManager::getCharSet(CharSetManager::getInstance(), result->fontSpec->charSetSpec));
			}
		}
	}

	return result;
}

void Printing::out(u8 x, u8 y, const char* string, const char* font)
{
#ifdef __FORCE_FONT
	font = __FORCE_FONT;
#endif

	u32 i = 0;
	u32 position = 0;
	u32 startColumn = x;
	u32 charOffset = 0, charOffsetX = 0, charOffsetY = 0;
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

				x = (x / TAB_SIZE + 1) * TAB_SIZE * fontData->fontSpec->fontSize.x;
				break;

			// carriage return
			case 10:

				y += fontData->fontSpec->fontSize.y;
				x = startColumn;
				break;

			default:
				{
					for(charOffsetX = 0; charOffsetX < fontData->fontSpec->fontSize.x; charOffsetX++)
					{
						for(charOffsetY = 0; charOffsetY < fontData->fontSpec->fontSize.y; charOffsetY++)
						{
							// allow fonts with less than 32 letters
							charOffset = (fontData->fontSpec->characterCount < 32)
								? charOffsetX + (charOffsetY * fontData->fontSpec->characterCount * fontData->fontSpec->fontSize.x)
								: charOffsetX + (charOffsetY << 5);

							bgmapSpaceBaseAddress[(0x1000 * printingBgmap) + position + charOffsetX + (charOffsetY << 6)] =
								(
									// font offset in char memory
									fontData->offset +

									// top left char of letter
									((u8)(string[i] - fontData->fontSpec->offset) * fontData->fontSpec->fontSize.x) +

									// skip lower chars of multi-char fonts with y > 1
									((((u8)(string[i] - fontData->fontSpec->offset) * fontData->fontSpec->fontSize.x) >> 5) * ((fontData->fontSpec->fontSize.y - 1)) << 5) +

									// respective char of letter in multi-char fonts
									charOffset
								)
								| (this->palette << 14);
						}
					}
				}

				x += fontData->fontSpec->fontSize.x;
				if(x >= 48)
				{
					// wrap around when outside of the visible area
					y += fontData->fontSpec->fontSize.y;
					x = startColumn;
				}

				break;
		}
		i++;
	}
}

void Printing::int(int value, u8 x, u8 y, const char* font)
{
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

void Printing::hex(WORD value, u8 x, u8 y, u8 length, const char* font)
{
	Printing::out(this, x,y, Utilities::itoa((int)(value), 16, length), font);
}

void Printing::float(float value, u8 x, u8 y, const char* font)
{
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

void Printing::text(const char* string, int x, int y, const char* font)
{
#ifdef __FORCE_UPPERCASE
	Printing::out(this, x, y, Utilities::toUppercase(string), font);
#else
	Printing::out(this, x, y, string, font);
#endif
}

#ifdef __FORCE_PRINTING_LAYER
void Printing::setWorldCoordinates(s16 gx __attribute__ ((unused)), s16 gy __attribute__ ((unused)), s8 gp __attribute__ ((unused)))
{
	this->gx = 0;
	this->gy = 0;
	this->gp = 0;
}
#else
void Printing::setWorldCoordinates(s16 gx, s16 gy, s8 gp)
{
	this->gx = gx <= __SCREEN_WIDTH ? gx : 0;
	this->gy = gy <= __SCREEN_HEIGHT ? gy : 0;
	this->gp = gp;
}
#endif

void Printing::resetWorldCoordinates()
{
	this->gx = __PRINTING_BGMAP_X_OFFSET;
	this->gy = __PRINTING_BGMAP_Y_OFFSET;
	this->gp = __PRINTING_BGMAP_PARALLAX_OFFSET;
}

int Printing::getPixelCount()
{
	return (__SCREEN_WIDTH - this->gx) * (__SCREEN_HEIGHT - this->gy);
}

FontSize Printing::getTextSize(const char* string, const char* font)
{
	FontSize fontSize = {0, 0};
	u16 i = 0, currentLineLength = 0;

	FontData* fontData = Printing::getFontByName(this, font);

	if(!fontData)
	{
		// just to make sure that no client code does a 0 division with these results
		fontSize = (FontSize){8, 8};
		return fontSize;
	}

	fontSize.y = fontData->fontSpec->fontSize.y;

	while(string[i])
	{
		switch(string[i])
		{
			// line feed
			case 13:

				break;

			// tab
			case 9:

				currentLineLength += (currentLineLength / TAB_SIZE + 1) * TAB_SIZE * fontData->fontSpec->fontSize.x;
				break;

			// carriage return
			case 10:

				fontSize.y += fontData->fontSpec->fontSize.y;
				currentLineLength = 0;
				break;

			default:

				currentLineLength += fontData->fontSpec->fontSize.x;
				if(currentLineLength >= 64)
				{
					fontSize.y += fontData->fontSpec->fontSize.y;
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
