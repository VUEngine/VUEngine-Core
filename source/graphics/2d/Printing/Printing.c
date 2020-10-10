/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
#include <SpriteManager.h>
#include <Utilities.h>
#include <Mem.h>
#include <VirtualList.h>
#include <Printing.h>
#include <config.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern FontROMSpec* const __FONTS[];
extern FontROMSpec DEFAULT_FONT;


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// horizontal tab size in chars
#define __TAB_SIZE	4

#define VUENGINE_DEBUG_FONT_CHARSET_OFFSET		(__CHAR_MEMORY_TOTAL_CHARS - VUENGINE_DEBUG_FONT_SIZE)

// fontdata for debug output
#define VUENGINE_DEBUG_FONT_SIZE	160
FontROMData VUENGINE_DEBUG_FONT_DATA =
{
	// font spec
	(FontSpec*)&DEFAULT_FONT,

	// CharSet
	NULL,
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
	this->printingBgmapSegment = 0;
	this->orientation = kPrintingOrientationHorizontal;
	this->direction = kPrintingDirectionLTR;

	Printing::reset(this);
}

void Printing::destructor()
{
	delete this->fonts;

	// allow a new construct
	Base::destructor();
}

void Printing::reset()
{
	Printing::releaseFonts(this);

	VirtualList::clear(this->fonts);

	this->gx = 0;
	this->gy = 0;
	this->gp = 0;
	this->mx = __PRINTING_BGMAP_X_OFFSET;
	this->my = __PRINTING_BGMAP_Y_OFFSET;
	this->mp = 0;
	this->w = __SCREEN_WIDTH - 1;
	this->h = __SCREEN_HEIGHT - 1;
}

void Printing::setOrientation(u8 value)
{
	this->orientation = value;

	switch(this->orientation)
	{
		case kPrintingOrientationHorizontal:
		case kPrintingOrientationVertical:

			break;

		default:

			this->orientation = kPrintingOrientationHorizontal;
			break;
	}
}

void Printing::setDirection(u8 value)
{
	this->direction = value;

	switch(this->direction)
	{
		case kPrintingDirectionLTR:
		case kPrintingDirectionRTL:

			break;

		default:

			this->direction = kPrintingDirectionLTR;
			break;
	}
}

void Printing::onFontCharSetRewritten(Object eventFirer __attribute__((unused)))
{
	Printing::fireEvent(this, kEventFontRewritten);
	NM_ASSERT(!isDeleted(this), "Printing::onFontCharSetRewritten: deleted this during kEventFontRewritten");
}

void Printing::loadFonts(FontSpec** fontSpecs)
{
	this->printingBgmapSegment = BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());

	bool isDrawingAllowed = HardwareManager::isDrawingAllowed(HardwareManager::getInstance());

	// Prevent VIP's interrupt from calling render during this process
	HardwareManager::disableRendering(HardwareManager::getInstance());

	// Since fonts' charsets will be released, there is no reason to keep
	// anything in the printing area
	Printing::clear(Printing::getInstance());

	// empty list of registered fonts
	Printing::releaseFonts(this);

	// iterate over all defined fonts and add to internal list
	u32 i = 0, j = 0;
	for(i = 0; __FONTS[i]; i++)
	{
		// instance and initialize a new fontdata instance
		FontData* fontData = new FontData;
		fontData->fontSpec = __FONTS[i];
		fontData->charSet = NULL;

		// preload charset for font if in list of fonts to preload
		if(fontSpecs)
		{
			// find defined font in list of fonts to preload
			for(j = 0; fontSpecs[j]; j++)
			{
				// preload charset and save charset reference, if font was found
				if(__FONTS[i]->charSetSpec == fontSpecs[j]->charSetSpec)
				{
					fontData->charSet = CharSetManager::getCharSet(CharSetManager::getInstance(), fontSpecs[j]->charSetSpec);

					CharSet::removeEventListener(fontData->charSet, Object::safeCast(this), (EventListener)Printing::onFontCharSetRewritten, kEventCharSetRewritten);
					CharSet::addEventListener(fontData->charSet, Object::safeCast(this), (EventListener)Printing::onFontCharSetRewritten, kEventCharSetRewritten);
				}
			}
		}

		// add fontdata to internal list
		VirtualList::pushBack(this->fonts, fontData);
	}

	SpriteManager::writeTextures(SpriteManager::getInstance());

	if(isDrawingAllowed)
	{
		// Restore drawing
		HardwareManager::enableRendering(HardwareManager::getInstance());
		while(VIPManager::isRenderingPending(VIPManager::getInstance()));
	}
}

void Printing::setFontPage(const char* font, u16 page)
{
	FontData* fontData = Printing::getFontByName(this, font);

	if(!fontData)
	{
		return;
	}

	CharSet::setFrame(fontData->charSet, page);
}

void Printing::loadDebugFont()
{
	Mem::copyBYTE(
		(u8*)(__CHAR_SPACE_BASE_ADDRESS + (VUENGINE_DEBUG_FONT_CHARSET_OFFSET << 4)),
		(u8*)(VUENGINE_DEBUG_FONT_DATA.fontSpec->charSetSpec->charSpec),
		VUENGINE_DEBUG_FONT_SIZE << 4
	);
}

void Printing::setDebugMode()
{
	Printing::resetCoordinates(this);
	Printing::loadDebugFont(this);
	this->mode = __PRINTING_MODE_DEBUG;
	this->printingBgmapSegment = __EXCEPTIONS_BGMAP;
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
	Mem::clear((BYTE*)__BGMAP_SEGMENT(this->printingBgmapSegment + 1) - __PRINTABLE_BGMAP_AREA * 2, __PRINTABLE_BGMAP_AREA * 2);
}

void Printing::releaseFonts()
{
	Printing::removeAllEventListeners(this, kEventFontRewritten);

	VirtualNode node = VirtualList::begin(this->fonts);

	for(; node; node = VirtualNode::getNext(node))
	{
		FontData* fontData = VirtualNode::getData(node);

		if(!isDeleted(fontData) && !isDeleted(fontData->charSet))
		{
			CharSet::removeEventListener(fontData->charSet, Object::safeCast(this), (EventListener)Printing::onFontCharSetRewritten, kEventCharSetRewritten);

			while(!CharSetManager::releaseCharSet(CharSetManager::getInstance(), fontData->charSet));
		}

		delete fontData;
	}

	VirtualList::clear(this->fonts);
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
			if(NULL == result->charSet)
			{
				result->charSet = CharSetManager::getCharSet(CharSetManager::getInstance(), result->fontSpec->charSetSpec);

				CharSet::removeEventListener(result->charSet, Object::safeCast(this), (EventListener)Printing::onFontCharSetRewritten, kEventCharSetRewritten);
				CharSet::addEventListener(result->charSet, Object::safeCast(this), (EventListener)Printing::onFontCharSetRewritten, kEventCharSetRewritten);
			}
		}
	}

	return result;
}

void Printing::number(int value, u8 x, u8 y, const char* font)
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

void Printing::int(int value, u8 x, u8 y, const char* font)
{
	Printing::number(this, value, x, y, font);

	Printing::setOrientation(this, kPrintingOrientationHorizontal);
	Printing::setDirection(this, kPrintingDirectionLTR);
}

void Printing::hex(WORD value, u8 x, u8 y, u8 length, const char* font)
{
	Printing::out(this, x,y, Utilities::itoa((int)(value), 16, length), font);

	Printing::setOrientation(this, kPrintingOrientationHorizontal);
	Printing::setDirection(this, kPrintingDirectionLTR);
}

void Printing::float(float value, u8 x, u8 y, const char* font)
{
	char string[48];
	char* integer = Utilities::itoa((int)value, 10, Utilities::getDigitCount((int)value));

	int i = 0;

	if(0 > value)
	{
		string[i] = '-';
		i++;
	}

	for(int j = 0; integer[j]; i++, j++)
	{
		string[i] = integer[j];
	}

	int decimal = (int)(((float)__FIX19_13_FRAC(__F_TO_FIX19_13(value)) / 8192.f) * 100.f);
	//int length = Utilities::intLength(__ABS(integer)) + (0 > value ? 1 : 0);

	string[i++] = '.';

	if(decimal)
	{
		int auxDecimal = decimal;

		while(0 == (auxDecimal / 10))
		{
			auxDecimal *= 10;
			string[i++] = '0';
		}

		auxDecimal = decimal;

		while(0 == (auxDecimal % 10))
		{
			auxDecimal /= 10;
			string[i++] = Utilities::itoa((int)auxDecimal, 10, 1)[0];
		}
	}
	else
	{
		string[i++] = '0';
	}

	string[i++] = 0;

	Printing::text(this, string, x, y, font);

	Printing::setOrientation(this, kPrintingOrientationHorizontal);
	Printing::setDirection(this, kPrintingDirectionLTR);

}

void Printing::text(const char* string, int x, int y, const char* font)
{
#ifdef __FORCE_UPPERCASE
	Printing::out(this, x, y, Utilities::toUppercase(string), font);
#else
	Printing::out(this, x, y, string, font);
#endif
	Printing::setOrientation(this, kPrintingOrientationHorizontal);
	Printing::setDirection(this, kPrintingDirectionLTR);
}

#ifdef __FORCE_PRINTING_LAYER
void Printing::setCoordinates(s16 x __attribute__ ((unused)), s16 y __attribute__ ((unused)), s8 p __attribute__ ((unused)))
{
	Printing::setWorldCoordinates(this, 0, 0, 0);
	Printing::setBgmapCoordinates(this, 0, 0, 0);
	Printing::setWorldSize(this, __SCREEN_WIDTH, __SCREEN_HEIGHT);
}

void Printing::setWorldCoordinates(s16 gx __attribute__ ((unused)), s16 gy __attribute__ ((unused)), s8 gp __attribute__ ((unused)))
{
	this->gx = 0;
	this->gy = 0;
	this->gp = 0;
}

void Printing::setBgmapCoordinates(s16 mx __attribute__ ((unused)), s16 my __attribute__ ((unused)), s8 mp __attribute__ ((unused)))
{
	this->mx = __PRINTING_BGMAP_X_OFFSET;
	this->my = __PRINTING_BGMAP_Y_OFFSET;
	this->mp = 0;
}

void Printing::setWorldSize(u16 w __attribute__ ((unused)), u16 h __attribute__ ((unused)))
{
	this->w = __SCREEN_WIDTH - 1;
	this->h = __SCREEN_HEIGHT - 1;
}

#else
void Printing::setCoordinates(s16 x, s16 y, s8 p)
{
	Printing::setWorldCoordinates(this, x, y, p);
	Printing::setBgmapCoordinates(this, x, y, 0);
}

void Printing::setWorldCoordinates(s16 gx, s16 gy, s8 gp)
{
	this->gx = gx <= __SCREEN_WIDTH ? gx : 0;
	this->gy = gy <= __SCREEN_HEIGHT ? gy : 0;
	this->gp = gp;
}

void Printing::setBgmapCoordinates(s16 mx __attribute__ ((unused)), s16 my __attribute__ ((unused)), s8 mp __attribute__ ((unused)))
{
	this->mx = mx <= 64 * 8 ? mx : 0;
	this->my = my + __PRINTING_BGMAP_Y_OFFSET <= 64 * 8 ? my + __PRINTING_BGMAP_Y_OFFSET : __PRINTING_BGMAP_Y_OFFSET;
	this->mp = mp;
}

void Printing::setWorldSize(u16 w __attribute__ ((unused)), u16 h __attribute__ ((unused)))
{
	this->w = w < __SCREEN_WIDTH ? w : __SCREEN_WIDTH;
	this->h = h < __SCREEN_HEIGHT ? h : __SCREEN_HEIGHT;
}
#endif

s16 Printing::getWorldCoordinatesX()
{
	return this->gx;
}

s16 Printing::getWorldCoordinatesY()
{
	return this->gy;
}

s16 Printing::getWorldCoordinatesP()
{
	return this->gp;
}

void Printing::resetCoordinates()
{
	this->gx = 0;
	this->gy = 0;
	this->gp = 0;

	this->mx = __PRINTING_BGMAP_X_OFFSET;
	this->my = __PRINTING_BGMAP_Y_OFFSET;
	this->mp = __PRINTING_BGMAP_PARALLAX_OFFSET;

	this->w = __SCREEN_WIDTH;
	this->h = __SCREEN_HEIGHT;
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

				currentLineLength += (currentLineLength / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.x;
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

void Printing::render(u8 textLayer)
{
	ASSERT(!(0 > textLayer || textLayer >= __TOTAL_LAYERS), "Printing::render: invalid layer");

	_worldAttributesBaseAddress[textLayer].mx = this->mx;
	_worldAttributesBaseAddress[textLayer].mp = this->mp;
	_worldAttributesBaseAddress[textLayer].my = this->my;
	_worldAttributesBaseAddress[textLayer].gx = this->gx;
	_worldAttributesBaseAddress[textLayer].gp = this->gp;
	_worldAttributesBaseAddress[textLayer].gy = this->gy;
	_worldAttributesBaseAddress[textLayer].w = this->w;
	_worldAttributesBaseAddress[textLayer].h = this->h;
	_worldAttributesBaseAddress[textLayer].head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | this->printingBgmapSegment;
}

void Printing::out(u8 x, u8 y, const char* string, const char* font)
{
#ifdef __DEFAULT_FONT
	if(NULL == font)
	{
		font = __DEFAULT_FONT;
	}
#endif

#ifdef __FORCE_FONT
	font = __FORCE_FONT;
#endif

	u32 i = 0, position = 0, startColumn = x, temp = 0;
	u32 charOffset = 0, charOffsetX = 0, charOffsetY = 0;
	u32 printingBgmap = this->printingBgmapSegment;

	FontData* fontData = Printing::getFontByName(this, font);

	if(!fontData)
	{
		return;
	}

	u16* const bgmapSpaceBaseAddress = (u16*)__BGMAP_SPACE_BASE_ADDRESS;
	u32 offset = __PRINTING_MODE_DEBUG == this->mode ? VUENGINE_DEBUG_FONT_CHARSET_OFFSET : CharSet::getOffset(fontData->charSet);

	// print text
	while(string[i] && x < (__SCREEN_WIDTH_IN_CHARS))
	{
		// do not allow printing outside of the visible area, since that would corrupt the param table
		if(y > 27/* || y < 0*/)
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

				if(kPrintingOrientationHorizontal == this->orientation)
				{
					x = (x / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.x;
				}
				else
				{
					y = (y / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.y;
				}
				break;

			// carriage return
			case 10:

				temp = fontData->fontSpec->fontSize.y;
				y = (this->direction == kPrintingDirectionLTR)
					? y + temp
					: y - temp;
				x = startColumn;
				break;

			default:
				{
					for(charOffsetX = 0; charOffsetX < fontData->fontSpec->fontSize.x; charOffsetX++)
					{
						for(charOffsetY = 0; charOffsetY < fontData->fontSpec->fontSize.y; charOffsetY++)
						{
							charOffset = charOffsetX + (charOffsetY * fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x);

							bgmapSpaceBaseAddress[(0x1000 * (printingBgmap + 1) - __PRINTABLE_BGMAP_AREA) + position + charOffsetX + (charOffsetY << 6)] =
								(
									// offset of charset in char memory
									offset +

									// offset of character in charset
									((u8)(string[i] - fontData->fontSpec->offset) * fontData->fontSpec->fontSize.x) +

									// additional y offset in charset
									(((u8)(string[i] - fontData->fontSpec->offset)
										/ fontData->fontSpec->charactersPerLineInCharset
										* fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x)
											* (fontData->fontSpec->fontSize.y - 1)) +

									// respective char of character
									charOffset
								)
								| (this->palette << 14);
						}
					}
				}

				if(kPrintingOrientationHorizontal == this->orientation)
				{
					temp = fontData->fontSpec->fontSize.x;
					x = (this->direction == kPrintingDirectionLTR)
						? x + temp
						: x - temp;
				}
				else
				{
					temp = fontData->fontSpec->fontSize.y;
					y = (this->direction == kPrintingDirectionLTR)
						? y + temp
						: y - temp;
				}

				if(x >= 48/* || x < 0*/)
				{
					// wrap around when outside of the visible area
					temp = fontData->fontSpec->fontSize.y;
					y = (this->direction == kPrintingDirectionLTR)
						? y + temp
						: y - temp;
					x = startColumn;
				}

				break;
		}
		i++;
	}
}
