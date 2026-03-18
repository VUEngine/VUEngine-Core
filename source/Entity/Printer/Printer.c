/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <TileSet.h>
#include <TileSetManager.h>
#include <ComponentManager.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <ParamTableManager.h>
#include <PrintingSprite.h>
#include <Singleton.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <DisplayUnit.h>

#include "Printer.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

extern FontData _fontData[];
extern FontROMSpec DefaultFontSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define VUENGINE_DEBUG_FONT_TILESET_OFFSET		(__TOTAL_TILES - VUENGINE_DEBUG_FONT_SIZE)
#define VUENGINE_DEBUG_FONT_SIZE				160

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

FontData VUENGINE_DEBUG_FONT_DATA =
{
	// Font spec
	(FontSpec*)&DefaultFontSpec,

	// TileSet
	NULL,
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setDebugMode()
{
	Printer printer = Printer::getInstance();

	if(__PRINTING_MODE_DEBUG == printer->mode)
	{
		return;
	}

	printer->mode = __PRINTING_MODE_DEBUG;
	printer->lastUsedFontData = NULL;
	printer->lastUsedFont = NULL;
	Printer::loadDebugFont();
	Printer::clear();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::loadFonts(FontSpec** fontSpecs)
{
	Printer printer = Printer::getInstance();

	// Since fonts' tileSets will be released, there is no reason to keep
	// Anything in the printer area
	Printer::clear();

	// Empty list of registered fonts
	Printer::releaseFonts();

	// Prevent VIP's interrupt from calling render during printer process
	Hardware::suspendInterrupts();

	/// Must force TILE defragmentation
	TileSetManager::writeTileSets(TileSetManager::getInstance());

	// Iterate over all defined fonts and add to internal list
	// Preload tileSet for font if in list of fonts to preload
	if(NULL != fontSpecs)
	{
		for(int16 i = 0; NULL != fontSpecs[i]; i++)
		{
			// Find defined font in list of fonts to preload
			for(int16 j = 0; NULL != _fontData[j].fontSpec; j++)
			{
				// Preload tileSet and save tileSet reference, if font was found
				if(fontSpecs[i] == _fontData[j].fontSpec)
				{
					_fontData[j].tileSet = TileSet::get(_fontData[j].fontSpec->tileSetSpec);

					if(NULL != _fontData[j].tileSet)
					{
						TileSet::addEventListener(_fontData[j].tileSet, ListenerObject::safeCast(printer), kEventTileSetChangedOffset);
					}
				}
			}
		}
	}

	if(NULL == printer->activePrintingSprite)
	{
		Printer::addSprite(printer);
	}

	Hardware::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::releaseFonts()
{
	Printer printer = Printer::getInstance();

	Printer::removeEventListeners(printer, NULL, kEventFontRewritten);

	for(int16 i = 0; NULL != _fontData[i].fontSpec; i++)
	{
		// Preload tileSet and save tileSet reference, if font was found
		if(!isDeleted(_fontData[i].tileSet))
		{
			TileSet::removeEventListener(_fontData[i].tileSet, ListenerObject::safeCast(printer), kEventTileSetChangedOffset);

			while(!TileSet::release(_fontData[i].tileSet));
		}

		_fontData[i].tileSet = NULL;
	}

	printer->lastUsedFont = NULL;
	printer->lastUsedFontData = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::clear()
{
	Printer printer = Printer::getInstance();

	if(!isDeleted(printer->activePrintingSprite))
	{
		PrintingSprite::clear(printer->activePrintingSprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::clearRow(uint16 row)
{
	// TODO: implement something more elegant and performant
	Printer::text("                                                ", 0, row, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::text(const char* string, int32 x, int32 y, const char* font)
{
#ifdef __FORCE_UPPERCASE
	Printer::out(x, y, Utilities::toUppercase(string), font);
#else
	Printer::out(x, y, string, font);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::int32(int32 value, uint8 x, uint8 y, const char* font)
{
	Printer::number(value, x, y, font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::hex(uint32 value, uint8 x, uint8 y, uint8 length, const char* font)
{
	Printer::out(x, y, Utilities::itoa((int32)(value), 16, length), font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::float(float value, uint8 x, uint8 y, int32 precision, const char* font)
{
	if(1 > precision)
	{
		precision = 1;
	}
	else if(10 < precision)
	{
		precision = 10;
	}

	int32 decMultiplier = 1;

	int32 decimals = 0;

	for(; decimals < precision; decimals++)
	{
		decMultiplier *= 10;
	}

	// Round last precision digit
	value += (0.5f / (decMultiplier));

	char string[48] = "\0";

	int32 i = 0;

	// Handle negatives
	if(0 > value)
	{
		string[i] = '-';
		i++;

		value *= -1;
	}

	// Get integral part
	int32 floorValue = ((int32)(value * 10)) / 10;
	char* integer = Utilities::itoa(floorValue, 10, Math::getDigitsCount(floorValue));

	// Save it right away
	for(int32 j = 0; integer[j];)
	{
		string[i++] = integer[j++];
	}

	// Start decimal part
	string[i++] = '.';

	// Get decimal part
	float decimalValue = value - floorValue;

	// Promote to integral all the decimals up to precision
	decimalValue *= decMultiplier; 

	int32 zeros = 0;
	int32 flooredDecimalValue = (int32)Math::floor(decimalValue);

	while(10 <= decMultiplier)
	{
		decMultiplier /= 10;

		if(0 != (flooredDecimalValue / decMultiplier))
		{
			break;
		}

		string[i++] = '0';
		zeros++;
	}

	if(decimals <= precision && zeros < precision)
	{
		long roundedDecimalValue = (int32)(decimalValue * 10) / 10;

		if(0 == roundedDecimalValue)
		{
			string[i] = 0;
		}
		else
		{
			int32 totalDecimalDigits = Math::getDigitsCount(roundedDecimalValue);

			char* decimalString = Utilities::itoa((int32)(decimalValue * 10) / 10, 10, totalDecimalDigits);

			int32 j = 0;

			for(; j < totalDecimalDigits; j++)
			{
				string[i + j] = decimalString[j];
			}

			string[i + j] = 0;
		}
	}
	else
	{
		string[i] = 0;
	}

	Printer::text(string, x, y, font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setFontPage(const char* font, uint16 page)
{
	FontData* fontData = Printer::getFontByName(font);

	if(NULL == fontData || isDeleted(fontData->tileSet))
	{
		return;
	}

	TileSet::setFrame(fontData->tileSet, page);
	TileSet::write(fontData->tileSet);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setOrientation(uint8 value)
{
	Printer printer = Printer::getInstance();

	printer->orientation = value;

	switch(printer->orientation)
	{
		case kPrintingOrientationHorizontal:
		case kPrintingOrientationVertical:
		{
			break;
		}

		default:
		{
			printer->orientation = kPrintingOrientationHorizontal;
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setTextDirection(uint8 value)
{
	Printer printer = Printer::getInstance();

	printer->direction = value;

	switch(printer->direction)
	{
		case kPrintingDirectionLTR:
		case kPrintingDirectionRTL:
		{
			break;
		}

		default:
		{
			printer->direction = kPrintingDirectionLTR;
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::addSprite()
{
	extern PrintingSpriteSpec DefaultPrintingSpriteSpec;
	
	Printer printer = Printer::getInstance();
	printer->activePrintingSprite = 
		PrintingSprite::safeCast(Printer::addComponent(printer, (ComponentSpec*)&DefaultPrintingSpriteSpec));

	PixelVector position = 
	{
		0, 0, 0, 0
	};

	PrintingSprite::setPosition(printer->activePrintingSprite, &position);

	Printer::clearComponentLists(printer, kSpriteComponent);

	Printer::fireEvent(printer, kEventFontRewritten);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool Printer::setActiveSprite(uint16 printingSpriteIndex)
{
	Printer printer = Printer::getInstance();

	printer->activePrintingSprite = 
		PrintingSprite::safeCast(Printer::getComponentAtIndex(printer, kSpriteComponent, printingSpriteIndex));

	bool result = NULL != printer->activePrintingSprite;

	if(NULL == printer->activePrintingSprite)
	{
		printer->activePrintingSprite = PrintingSprite::safeCast(Printer::getComponentAtIndex(printer, kSpriteComponent, 0));
	}

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static PrintingSprite Printer::getActiveSprite()
{
	Printer printer = Printer::getInstance();

	return printer->activePrintingSprite;	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::printSprite(int16 x, int16 y)
{
	Printer printer = Printer::getInstance();

	if(!isDeleted(printer->activePrintingSprite))
	{
		PrintingSprite::print(printer->activePrintingSprite, x, y);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setScreenPosition(int16 x, int16 y, int16 z, int8 parallax)
{
#ifdef __FORCE_PRINTING_LAYER
	return;
#endif

	Printer printer = Printer::getInstance();

	if(!isDeleted(printer->activePrintingSprite))
	{
		PixelVector position = 
		{
			x <= __SCREEN_WIDTH ? x : 0, 
			y <= __SCREEN_HEIGHT ? y : 0, 
			z, 
			parallax
		};

		PrintingSprite::setPosition(printer->activePrintingSprite, &position);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setTextureSource(int16 mx, int16 my, int8 mp)
{
#ifdef __FORCE_PRINTING_LAYER
	return;
#endif

	Printer printer = Printer::getInstance();

	if(!isDeleted(printer->activePrintingSprite))
	{
		PrintingSprite::setMValues
		(
			printer->activePrintingSprite, 
			mx <= 64 * 8 ? mx : 0, 
			my <= 64 * 8 ? my : 0,
			mp
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setSize(uint16 w, uint16 h)
{
#ifdef __FORCE_PRINTING_LAYER
	return;
#endif

	Printer printer = Printer::getInstance();

	if(!isDeleted(printer->activePrintingSprite))
	{
		PrintingSprite::setSize
		(
			printer->activePrintingSprite, w < __SCREEN_WIDTH ? w : __SCREEN_WIDTH, h < __SCREEN_HEIGHT ? h : __SCREEN_HEIGHT
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setTransparency(uint8 transparency)
{
	Printer printer = Printer::getInstance();

	if(!isDeleted(printer->activePrintingSprite))
	{
		Sprite::setTransparency(printer->activePrintingSprite, transparency);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::setMask(uint16 mask)
{
	Printer printer = Printer::getInstance();

	if(4 > mask)
	{
		printer->mask = mask;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::resetScreenPosition()
{
	Printer printer = Printer::getInstance();

	if(!isDeleted(printer->activePrintingSprite))
	{
		PrintingSprite::reset(printer->activePrintingSprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static PixelVector Printer::getActiveSpritePosition()
{
	Printer printer = Printer::getInstance();

	return 
		!isDeleted(printer->activePrintingSprite) ? 
			PrintingSprite::getDisplacedPosition(printer->activePrintingSprite) 
			: 
			(PixelVector){0, 0, 0, 0};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static FontData* Printer::getFontByName(const char* font)
{
	Printer printer = Printer::getInstance();

	FontData* result = NULL;

	if(printer->mode == __PRINTING_MODE_DEBUG)
	{
		result = (FontData*)&VUENGINE_DEBUG_FONT_DATA;
	}
	else
	{
		// Set first defined font as default
		result = &_fontData[0];

		if(NULL != font)
		{
			for(int16 i = 0; NULL != _fontData[i].fontSpec; i++)
			{
				if(0 == strcmp(_fontData[i].fontSpec->name, font))
				{
					result = &_fontData[i];
					break;
				}
			}
		}

		if(NULL == result->tileSet && NULL != result->fontSpec->tileSetSpec)
		{
			result->tileSet = TileSet::get(result->fontSpec->tileSetSpec);

			if(NULL != result->tileSet)
			{
				TileSet::addEventListener(result->tileSet, ListenerObject::safeCast(printer), kEventTileSetChangedOffset);
			}
			else
			{
				result = NULL;
			}
		}
	}
	
	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static FontSize Printer::getTextSize(const char* string, const char* font)
{
	FontSize fontSize = {0, 0};
	uint16 i = 0, currentLineLength = 0;

	FontData* fontData = Printer::getFontByName(font);

	if(NULL == fontData)
	{
		// Just to make sure that no client code does a 0 division with these results
		fontSize = (FontSize){8, 8};
		return fontSize;
	}

	fontSize.y = fontData->fontSpec->fontSize.y;

	while('\0' != string[i])
	{
		switch(string[i])
		{
			// Line feed
			case 13:
			{
				break;
			}

			// Tab
			case 9:
			{
				currentLineLength += (currentLineLength / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.x;
				break;
			}

			// Carriage return
			case 10:
			{
				fontSize.y += fontData->fontSpec->fontSize.y;
				currentLineLength = 0;
				break;
			}

			default:
			{
				currentLineLength += fontData->fontSpec->fontSize.x;
				if(currentLineLength >= 64)
				{
					fontSize.y += fontData->fontSpec->fontSize.y;
					currentLineLength = 0;
				}

				break;
			}
		}

		if(currentLineLength > fontSize.x)
		{
			fontSize.x = currentLineLength;
		}

		i++;
	}

	return fontSize;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::loadDebugFont()
{
	Mem::copyWORD
	(
		(uint32*)(__TILE_SPACE_BASE_ADDRESS + (((uint32)VUENGINE_DEBUG_FONT_TILESET_OFFSET) << 4)),
		VUENGINE_DEBUG_FONT_DATA.fontSpec->tileSetSpec->tiles + 1,
		__UINT32S_PER_TILES(VUENGINE_DEBUG_FONT_SIZE)
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::number(int32 value, uint8 x, uint8 y, const char* font)
{
	if(value < 0)
	{
		value = -value;
		Printer::out(x++, y, "-", font);
	}

	Printer::out(x, y, Utilities::itoa((int32)(value), 10, 0), font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printer::out(uint8 x, uint8 y, const char* string, const char* font)
{
	Printer printer = Printer::getInstance();

#ifdef __DEFAULT_FONT
	if(NULL == font)
	{
		font = __DEFAULT_FONT;
	}
#endif

#ifdef __FORCE_FONT
	font = __FORCE_FONT;
#endif

	uint32 i = 0, position = 0, startColumn = x, xDisplacement = 0, yDisplacement = 0;
	FontData* fontData = printer->lastUsedFontData;

	if(NULL == fontData)
	{
		fontData = Printer::getFontByName(font);
		printer->lastUsedFontData = fontData;
		printer->lastUsedFont = font;
	}
	else if(printer->lastUsedFont != font && strcmp(fontData->fontSpec->name, font))
	{
		fontData = Printer::getFontByName(font);
		printer->lastUsedFontData = fontData;
		printer->lastUsedFont = font;
	}
	else
	{
		printer->lastUsedFontData = fontData;
		printer->lastUsedFont = font;
	}

	if(NULL == fontData || (__PRINTING_MODE_DEBUG != printer->mode && isDeleted(fontData->tileSet)))
	{
		return;
	}

	uint32 offset = __PRINTING_MODE_DEBUG == printer->mode ? VUENGINE_DEBUG_FONT_TILESET_OFFSET : TileSet::getOffset(fontData->tileSet);

	uint16 fontSizeX = fontData->fontSpec->fontSize.x;
	uint16 fontSizeY = fontData->fontSpec->fontSize.y;

	if(kPrintingOrientationHorizontal == printer->orientation)
	{
		if(printer->direction == kPrintingDirectionLTR)
		{
			xDisplacement = fontSizeX;
		}
		else
		{
			xDisplacement = -fontSizeX;
		}
	}
	else
	{
		if(printer->direction == kPrintingDirectionLTR)
		{
			yDisplacement = fontSizeY;
		}
		else
		{
			yDisplacement = -fontSizeY;
		}
	}
	
	uint16* offsetDisplacementStart = isDeleted(printer->activePrintingSprite) ? 
		(uint16*)__TEXTURE_SPACE_BASE_ADDRESS 
		: 
		(uint16*)PrintingSprite::getPrintingAddress(printer->activePrintingSprite, true);

	int16 tileLineSize = fontData->fontSpec->charactersPerLineInTileSet * fontSizeX;
	int16 tileLineSizeYModifier = tileLineSize * (fontSizeY - 1);
	uint16 fontOffsetCache = (uint8)fontData->fontSpec->offset;

	// Print text
	while('\0' != string[i] && x < (__SCREEN_WIDTH_IN_TILES))
	{
		// Do not allow printer outside of the visible area, since that would corrupt the param table
		if(y > 27/* || y < 0*/)
		{
			break;
		}

		position = (y << 6) + x;

		switch(string[i])
		{
			// Line feed
			case 13:
			{
				break;
			}

			// Tab
			case 9:
			{
				if(kPrintingOrientationHorizontal == printer->orientation)
				{
					x = (x / __TAB_SIZE + 1) * __TAB_SIZE * fontSizeX;
				}
				else
				{
					y = (y / __TAB_SIZE + 1) * __TAB_SIZE * fontSizeY;
				}
				break;
			}

			// Carriage return
			case 10:
			{
				y += (printer->direction == kPrintingDirectionLTR)
					? fontSizeY
					: -fontSizeY;
				x = startColumn;
				break;
			}

			default:
			{
				if(1 < fontSizeX || 1 < fontSizeY)
				{					
					uint16 stringEntryOffset = (uint8)(string[i] - fontOffsetCache);
					uint16 stringEntryOffsetBySizeX = stringEntryOffset * fontSizeX;
					uint16 stringEntryOffsetBySizeY = 
						(stringEntryOffset / fontData->fontSpec->charactersPerLineInTileSet) * tileLineSizeYModifier;

					for(uint32 charOffsetX = 0; charOffsetX < fontSizeX; charOffsetX++)
					{
						uint16* offsetDisplacement = offsetDisplacementStart + position + charOffsetX;

						for(uint32 charOffsetY = 0; charOffsetY < fontSizeY; charOffsetY++)
						{
							uint32 charOffset = charOffsetX + charOffsetY * tileLineSize;

							*(offsetDisplacement + (charOffsetY << 6)) =
								(
									// Offset of tileSet in char memory + respective char of character
									offset + stringEntryOffsetBySizeX + stringEntryOffsetBySizeY + charOffset								
								)
								| (printer->mask << 14);
						}
					}
				}
				else
				{
					uint16* offsetDisplacement = offsetDisplacementStart + position;

					*(offsetDisplacement + (0 << 6)) =
						(
							// Offset of tileSet in char memory + respective char of character
							offset + (string[i] - fontOffsetCache)								
						)
						| (printer->mask << 14);					
				}

				x += xDisplacement;
				y += yDisplacement;
				
				if(x >= 48/* || x < 0*/)
				{
					// Wrap around when outside of the visible area
					y += (printer->direction == kPrintingDirectionLTR)
						? fontSizeY
						: -fontSizeY;
					x = startColumn;
				}

				break;
			}
		}

		i++;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Printer::onEvent(ListenerObject eventFirer, uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventTileSetChangedOffset:
		{
			TileSet tileSet = TileSet::safeCast(eventFirer);

			if(!isDeleted(tileSet))
			{
				TileSet::write(tileSet);
				Printer::fireEvent(this, kEventFontRewritten);
				NM_ASSERT(!isDeleted(this), "Printer::onEvent: deleted printer during kEventFontRewritten");
			}

			return true;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Printer::removedComponent(Component component __attribute__((unused)))
{
	this->activePrintingSprite = NULL;

	Printer::setActiveSprite(0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void Printer::reset()
{
	this->activePrintingSprite = NULL;

	Printer::releaseFonts(this);
	Printer::resetScreenPosition();
	Printer::setOrientation(kPrintingOrientationHorizontal);
	Printer::setTextDirection(kPrintingDirectionLTR);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Printer::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Initialize members
	this->mode = __PRINTING_MODE_DEFAULT;
	this->mask = __PRINTING_PALETTE;
	this->orientation = kPrintingOrientationHorizontal;
	this->direction = kPrintingDirectionLTR;
	this->lastUsedFont = NULL;
	this->lastUsedFontData = NULL;
	this->activePrintingSprite = NULL;

	for(int16 i = 0; NULL != _fontData[i].fontSpec; i++)
	{
		_fontData[i].tileSet = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Printer::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
