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
#include <BgmapPrinting.h>
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
extern FontROMSpec DEFAULT_FONT;


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

void BgmapPrinting::constructor()
{
	Base::constructor();
}

void BgmapPrinting::destructor()
{
	// allow a new construct
	Base::destructor();
}

void BgmapPrinting::render(int textLayer)
{
	ASSERT(!(0 > textLayer || textLayer >= __TOTAL_LAYERS), "BgmapPrinting::render: invalid layer");

	_worldAttributesBaseAddress[textLayer].head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | (BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()));
	_worldAttributesBaseAddress[textLayer].mx = this->mx;
	_worldAttributesBaseAddress[textLayer].mp = this->mp;
	_worldAttributesBaseAddress[textLayer].my = this->my;
	_worldAttributesBaseAddress[textLayer].gx = this->gx;
	_worldAttributesBaseAddress[textLayer].gp = this->gp;
	_worldAttributesBaseAddress[textLayer].gy = this->gy;
	_worldAttributesBaseAddress[textLayer].w = this->w;
	_worldAttributesBaseAddress[textLayer].h = this->h;
}

void BgmapPrinting::out(u8 x, u8 y, const char* string, const char* font)
{
#ifdef __FORCE_FONT
	font = __FORCE_FONT;
#endif

	u32 i = 0;
	u32 position = 0;
	u32 startColumn = x;
	u32 charOffset = 0, charOffsetX = 0, charOffsetY = 0;
	u32 printingBgmap = __PRINTING_MODE_DEBUG == this->mode ? __EXCEPTIONS_BGMAP : BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());

	FontData* fontData = BgmapPrinting::getFontByName(this, font);

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

				x = (x / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.x;
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
							charOffset = charOffsetX + (charOffsetY * fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x);

							bgmapSpaceBaseAddress[(0x1000 * printingBgmap) + position + charOffsetX + (charOffsetY << 6)] =
								(
									// offset of charset in char memory
									fontData->offset +

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
