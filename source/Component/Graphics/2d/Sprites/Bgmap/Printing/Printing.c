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

#include <BgmapTextureManager.h>
#include <CharSetManager.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <PrintingSprite.h>
#include <SpriteManager.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VIPManager.h>

#include "Printing.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

extern FontROMSpec* const _fonts[];
extern FontROMSpec DefaultFontSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define VUENGINE_DEBUG_FONT_CHARSET_OFFSET		(__CHAR_MEMORY_TOTAL_CHARS - VUENGINE_DEBUG_FONT_SIZE)
#define VUENGINE_DEBUG_FONT_SIZE				160

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

FontROMData VUENGINE_DEBUG_FONT_DATA =
{
	// Font spec
	(FontSpec*)&DefaultFontSpec,

	// CharSet
	NULL,
};

const PrintingSpriteSpec DefaultPrintingSpriteSpec =
{
	{
		{
			// Component
			{
				// Allocator
				__TYPE(PrintingSprite),

				// Component type
				kSpriteComponent
			},

			// Spec for the texture to display
			NULL,

			// Transparency mode (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
			__TRANSPARENCY_NONE,

			// Displacement added to the sprite's position
			{
				0, // x
				0, // y
				0, // z
				0, // parallax
			},
		},

		// The display mode (__WORLD_BGMAP, __WORLD_AFFINE, __WORLD_OBJECT or __WORLD_HBIAS)
		// Make sure to use the proper corresponding sprite type throughout the spec (BgmapSprite or ObjectSprite)
		__WORLD_BGMAP,

		// Pointer to affine/hbias manipulation function
		NULL,

		// Flag to indicate in which display to show the texture (__WORLD_ON, __WORLD_LON or __WORLD_RON)
		__WORLD_ON,
	}
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::registerEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	Printing printing = Printing::getInstance();

	Printing::addEventListener(printing, listener, callback, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::unregisterEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	Printing printing = Printing::getInstance();

	Printing::removeEventListener(printing, listener, callback, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::setDebugMode()
{
	Printing printing = Printing::getInstance();

	Printing::loadDebugFont();
	printing->mode = __PRINTING_MODE_DEBUG;
	printing->lastUsedFontData = NULL;
	printing->lastUsedFont = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::reset()
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->printingSprites))
	{
		for(VirtualNode node = VirtualList::begin(printing->printingSprites); NULL != node; node = VirtualNode::getNext(node))
		{
			ComponentManager::destroyComponent(NULL, VirtualNode::getData(node));
		}

		VirtualList::clear(printing->printingSprites);
	}

	printing->activePrintingSprite = NULL;
	printing->printingBgmapSegment = -1;

	Printing::releaseFonts(printing);

	VirtualList::clear(printing->fonts);

	Printing::setOrientation(kPrintingOrientationHorizontal);
	Printing::setDirection(kPrintingDirectionLTR);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::show()
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::show(printing->activePrintingSprite);
		PrintingSprite::setPosition(printing->activePrintingSprite, PrintingSprite::getPosition(printing->activePrintingSprite));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::hide()
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::hide(printing->activePrintingSprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::loadFonts(FontSpec** fontSpecs)
{
	Printing printing = Printing::getInstance();

	// Since fonts' charsets will be released, there is no reason to keep
	// Anything in the printing area
	Printing::clear(printing);

	// Empty list of registered fonts
	Printing::releaseFonts(printing);

	// Prevent VIP's interrupt from calling render during printing process
	HardwareManager::suspendInterrupts();

	// Make sure all sprites are ready
	SpriteManager::prepareAll();

	// Iterate over all defined fonts and add to internal list
	uint32 i = 0, j = 0;
	for(i = 0; _fonts[i]; i++)
	{
		// Instance and initialize a new fontdata instance
		FontData* fontData = new FontData;
		fontData->fontSpec = _fonts[i];
		fontData->charSet = NULL;

		// Preload charset for font if in list of fonts to preload
		if(fontSpecs)
		{
			// Find defined font in list of fonts to preload
			for(j = 0; fontSpecs[j]; j++)
			{
				// Preload charset and save charset reference, if font was found
				if(_fonts[i]->charSetSpec == fontSpecs[j]->charSetSpec)
				{
					fontData->charSet = CharSetManager::getCharSet(fontSpecs[j]->charSetSpec);

					CharSet::addEventListener(fontData->charSet, ListenerObject::safeCast(printing), (EventListener)Printing::onFontCharChangedOffset, kEventCharSetChangedOffset);
				}
			}
		}

		// Add fontdata to internal list
		VirtualList::pushBack(printing->fonts, fontData);
	}

	if(NULL == printing->activePrintingSprite)
	{
		Printing::addSprite(printing);
	}

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::releaseFonts()
{
	Printing printing = Printing::getInstance();

	Printing::removeEventListeners(printing, NULL, kEventFontRewritten);

	VirtualNode node = VirtualList::begin(printing->fonts);

	for(; NULL != node; node = VirtualNode::getNext(node))
	{
		FontData* fontData = VirtualNode::getData(node);

		if(!isDeleted(fontData))
		{
			if(!isDeleted(fontData->charSet))
			{
				CharSet::removeEventListener
				(
					fontData->charSet, ListenerObject::safeCast(printing), (EventListener)Printing::onFontCharChangedOffset,
					kEventCharSetChangedOffset
				);

				while(!CharSetManager::releaseCharSet(fontData->charSet));
			}

			delete fontData;
		}
	}

	VirtualList::clear(printing->fonts);

	printing->lastUsedFont = NULL;
	printing->lastUsedFontData = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::clear()
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		Mem::clear
		(
			(BYTE*)__BGMAP_SEGMENT(BgmapTextureManager::getPrintingBgmapSegment() + 1) - 
			__PRINTABLE_BGMAP_AREA * 2, __PRINTABLE_BGMAP_AREA * 2
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::clearRow(uint16 row)
{
	// TODO: implement something more elegant and performant
	Printing::text("                                                ", 0, row, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::text(const char* string, int32 x, int32 y, const char* font)
{
#ifdef __FORCE_UPPERCASE
	Printing::out(x, y, Utilities::toUppercase(string), font);
#else
	Printing::out(x, y, string, font);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::int32(int32 value, uint8 x, uint8 y, const char* font)
{
	Printing::number(value, x, y, font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::hex(WORD value, uint8 x, uint8 y, uint8 length, const char* font)
{
	Printing::out(x, y, Utilities::itoa((int32)(value), 16, length), font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::float(float value, uint8 x, uint8 y, int32 precision, const char* font)
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

	Printing::text(string, x, y, font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::setFontPage(const char* font, uint16 page)
{
	FontData* fontData = Printing::getFontByName(font);

	if(NULL == fontData || isDeleted(fontData->charSet))
	{
		return;
	}

	CharSet::setFrame(fontData->charSet, page);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::setOrientation(uint8 value)
{
	Printing printing = Printing::getInstance();

	printing->orientation = value;

	switch(printing->orientation)
	{
		case kPrintingOrientationHorizontal:
		case kPrintingOrientationVertical:

			break;

		default:

			printing->orientation = kPrintingOrientationHorizontal;
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::setDirection(uint8 value)
{
	Printing printing = Printing::getInstance();

	printing->direction = value;

	switch(printing->direction)
	{
		case kPrintingDirectionLTR:
		case kPrintingDirectionRTL:

			break;

		default:

			printing->direction = kPrintingDirectionLTR;
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::setPrintingBgmapSegment(int8 printingBgmapSegment)
{
	Printing printing = Printing::getInstance();

	if((unsigned)printingBgmapSegment < __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		printing->printingBgmapSegment = printingBgmapSegment;

		for(VirtualNode node = VirtualList::begin(printing->printingSprites); NULL != node; node = VirtualNode::getNext(node))
		{
			PrintingSprite::setPrintingBgmapSegment(PrintingSprite::safeCast(VirtualNode::getData(node)), printingBgmapSegment);
		}
	}	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::addSprite()
{
	Printing printing = Printing::getInstance();
	printing->printingBgmapSegment = BgmapTextureManager::getPrintingBgmapSegment();
	printing->activePrintingSprite = 
		PrintingSprite::safeCast(ComponentManager::createComponent(NULL, (ComponentSpec*)&DefaultPrintingSpriteSpec));

	PrintingSprite::setPrintingBgmapSegment(printing->activePrintingSprite, printing->printingBgmapSegment);

	PixelVector position = 
	{
		0, 0, 0, 0
	};

	PrintingSprite::setPosition(printing->activePrintingSprite, &position);

	VirtualList::pushBack(printing->printingSprites, printing->activePrintingSprite);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool Printing::setActiveSprite(uint16 printingSpriteIndex)
{
	Printing printing = Printing::getInstance();

	printing->activePrintingSprite = PrintingSprite::safeCast(VirtualList::getDataAtIndex(printing->printingSprites, printingSpriteIndex));

	bool result = NULL != printing->activePrintingSprite;

	if(NULL == printing->activePrintingSprite)
	{
		printing->activePrintingSprite = PrintingSprite::safeCast(VirtualList::getDataAtIndex(printing->printingSprites, 0));
	}

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::printSprite(int16 x, int16 y)
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::print(printing->activePrintingSprite, x, y);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __FORCE_PRINTING_LAYER
static void Printing::setCoordinates(int16 x __attribute__ ((unused)), int16 y __attribute__ ((unused)), int16 z __attribute__ ((unused)), int8 parallax __attribute__ ((unused)))
{
	Printing::setWorldCoordinates(0, 0, 0, 0);
	Printing::setBgmapCoordinates(0, 0, 0);
	Printing::setWorldSize(__SCREEN_WIDTH, __SCREEN_HEIGHT);
}
#else
static void Printing::setCoordinates(int16 x, int16 y, int16 z, int8 parallax)
{
	Printing::setWorldCoordinates(x, y, z, parallax);
	Printing::setBgmapCoordinates(x, y, 0);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __FORCE_PRINTING_LAYER
static void Printing::setWorldCoordinates(int16 x __attribute__ ((unused)), int16 y __attribute__ ((unused)), int16 z __attribute__ ((unused)), int8 parallax __attribute__ ((unused)))
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PixelVector position = 
		{
			0, 0, -64, -4
		};

		PrintingSprite::setPosition(printing->activePrintingSprite, &position);
	}
}
#else
static void Printing::setWorldCoordinates(int16 x, int16 y, int16 z, int8 parallax)
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PixelVector position = 
		{
			x <= __SCREEN_WIDTH ? x : 0, 
			y <= __SCREEN_HEIGHT ? y : 0, 
			z, 
			parallax
		};

		PrintingSprite::setPosition(printing->activePrintingSprite, &position);
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __FORCE_PRINTING_LAYER
static void Printing::setBgmapCoordinates(int16 mx __attribute__ ((unused)), int16 my __attribute__ ((unused)), int8 mp __attribute__ ((unused)))
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::setMValues(printing->activePrintingSprite, __PRINTING_BGMAP_X_OFFSET, __PRINTING_BGMAP_Y_OFFSET, 0);
	}
}
#else
static void Printing::setBgmapCoordinates(int16 mx __attribute__ ((unused)), int16 my __attribute__ ((unused)), int8 mp __attribute__ ((unused)))
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::setMValues
		(
			printing->activePrintingSprite, mx <= 64 * 8 ? 
				mx : 
				0, 
				__PRINTING_BGMAP_Y_OFFSET + my <= 64 * 8 ? 
					__PRINTING_BGMAP_Y_OFFSET + my 
					: 
					__PRINTING_BGMAP_Y_OFFSET, mp
		);
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __FORCE_PRINTING_LAYER
static void Printing::setWorldSize(uint16 w __attribute__ ((unused)), uint16 h __attribute__ ((unused)))
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::setSize(printing->activePrintingSprite, __SCREEN_WIDTH - 1, __SCREEN_HEIGHT - 1);
	}
}
#else
static void Printing::setWorldSize(uint16 w __attribute__ ((unused)), uint16 h __attribute__ ((unused)))
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::setSize
		(
			printing->activePrintingSprite, w < __SCREEN_WIDTH ? w : __SCREEN_WIDTH, h < __SCREEN_HEIGHT ? h : __SCREEN_HEIGHT
		);
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::setTransparency(uint8 transparency)
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		Sprite::setTransparency(printing->activePrintingSprite, transparency);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::setPalette(uint8 palette)
{
	Printing printing = Printing::getInstance();

	if(4 > palette)
	{
		printing->palette = palette;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::resetCoordinates()
{
	Printing printing = Printing::getInstance();

	if(!isDeleted(printing->activePrintingSprite))
	{
		PrintingSprite::reset(printing->activePrintingSprite);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 Printing::getWorldCoordinatesX()
{
	Printing printing = Printing::getInstance();

	return !isDeleted(printing->activePrintingSprite) ? PrintingSprite::getEffectiveX(printing->activePrintingSprite) : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 Printing::getWorldCoordinatesY()
{
	Printing printing = Printing::getInstance();

	return !isDeleted(printing->activePrintingSprite) ? PrintingSprite::getEffectiveY(printing->activePrintingSprite) : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 Printing::getWorldCoordinatesP()
{
	Printing printing = Printing::getInstance();

	return !isDeleted(printing->activePrintingSprite) ? PrintingSprite::getEffectiveP(printing->activePrintingSprite) : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static PixelVector Printing::getSpriteIndex()
{
	Printing printing = Printing::getInstance();

	return 
		!isDeleted(printing->activePrintingSprite) ? 
			PrintingSprite::getDisplacedPosition(printing->activePrintingSprite) 
			: 
			(PixelVector){0, 0, 0, 0};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static FontData* Printing::getFontByName(const char* font)
{
	Printing printing = Printing::getInstance();

	FontData* result = NULL;

	if(printing->mode == __PRINTING_MODE_DEBUG)
	{
		result = (FontData*)&VUENGINE_DEBUG_FONT_DATA;
	}
	else if(NULL != printing->fonts)
	{
		// Set first defined font as default
		result = VirtualList::front(printing->fonts);

		if(NULL != result)
		{
			if(NULL != font)
			{
				// Iterate over registered fonts to find spec of font to use
				VirtualNode node = VirtualList::begin(printing->fonts);
				
				for(; NULL != node; node = VirtualNode::getNext(node))
				{
					FontData* fontData = VirtualNode::getData(node);
					
					if(!strcmp(fontData->fontSpec->name, font))
					{
						result = fontData;
						break;
					}
				}
			}

			// If font's charset has not been preloaded, load it now
			if(NULL == result->charSet)
			{
				result->charSet = CharSetManager::getCharSet(result->fontSpec->charSetSpec);

				CharSet::addEventListener
				(
					result->charSet, ListenerObject::safeCast(printing), (EventListener)Printing::onFontCharChangedOffset, 
					kEventCharSetChangedOffset
				);
			}
		}
	}
	
	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static FontSize Printing::getTextSize(const char* string, const char* font)
{
	FontSize fontSize = {0, 0};
	uint16 i = 0, currentLineLength = 0;

	FontData* fontData = Printing::getFontByName(font);

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

				break;

			// Tab
			case 9:

				currentLineLength += (currentLineLength / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.x;
				break;

			// Carriage return
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::loadDebugFont()
{
	Mem::copyWORD(
		(uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)VUENGINE_DEBUG_FONT_CHARSET_OFFSET) << 4)),
		VUENGINE_DEBUG_FONT_DATA.fontSpec->charSetSpec->tiles + 1,
		__UINT32S_PER_CHARS(VUENGINE_DEBUG_FONT_SIZE)
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::number(int32 value, uint8 x, uint8 y, const char* font)
{
	if(value < 0)
	{
		value = -value;
		Printing::out(x++, y, "-", font);
	}

	Printing::out(x, y, Utilities::itoa((int32)(value), 10, 0), font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Printing::out(uint8 x, uint8 y, const char* string, const char* font)
{
	Printing printing = Printing::getInstance();

#ifdef __DEFAULT_FONT
	if(NULL == font)
	{
		font = __DEFAULT_FONT;
	}
#endif

#ifdef __FORCE_FONT
	font = __FORCE_FONT;
#endif

	if(-1 == printing->printingBgmapSegment)
	{
		printing->printingBgmapSegment = BgmapTextureManager::getPrintingBgmapSegment();

		if(-1 == printing->printingBgmapSegment)
		{
			return;
		}
	}

	uint32 i = 0, position = 0, startColumn = x, temp = 0;
	uint32 charOffset = 0, charOffsetX = 0, charOffsetY = 0;
	FontData* fontData = printing->lastUsedFontData;

	if(NULL == fontData)
	{
		fontData = Printing::getFontByName(font);
		printing->lastUsedFontData = fontData;
		printing->lastUsedFont = font;
	}
	else if(printing->lastUsedFont != font && strcmp(fontData->fontSpec->name, font))
	{
		fontData = Printing::getFontByName(font);
		printing->lastUsedFontData = fontData;
		printing->lastUsedFont = font;
	}
	else
	{
		printing->lastUsedFontData = fontData;
		printing->lastUsedFont = font;
	}

	if(NULL == fontData || (__PRINTING_MODE_DEBUG != printing->mode && isDeleted(fontData->charSet)))
	{
		return;
	}

	uint16* const bgmapSpaceBaseAddress = (uint16*)__BGMAP_SPACE_BASE_ADDRESS;
	uint32 offset = __PRINTING_MODE_DEBUG == printing->mode ? VUENGINE_DEBUG_FONT_CHARSET_OFFSET : CharSet::getOffset(fontData->charSet);

	// Print text
	while('\0' != string[i] && x < (__SCREEN_WIDTH_IN_CHARS))
	{
		// Do not allow printing outside of the visible area, since that would corrupt the param table
		if(y > 27/* || y < 0*/)
		{
			break;
		}

		position = (y << 6) + x;

		switch(string[i])
		{
			// Line feed
			case 13:

				break;

			// Tab
			case 9:

				if(kPrintingOrientationHorizontal == printing->orientation)
				{
					x = (x / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.x;
				}
				else
				{
					y = (y / __TAB_SIZE + 1) * __TAB_SIZE * fontData->fontSpec->fontSize.y;
				}
				break;

			// Carriage return
			case 10:

				temp = fontData->fontSpec->fontSize.y;
				y = (printing->direction == kPrintingDirectionLTR)
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
							charOffset = 
								charOffsetX + 
								(charOffsetY * fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x);

							bgmapSpaceBaseAddress[(0x1000 * (printing->printingBgmapSegment + 1) - 
							__PRINTABLE_BGMAP_AREA) + position + charOffsetX + (charOffsetY << 6)] =
								(
									// Offset of charset in char memory
									offset +
									((uint8)(string[i] - fontData->fontSpec->offset) * fontData->fontSpec->fontSize.x) +

									(((uint8)(string[i] - fontData->fontSpec->offset)
										/ fontData->fontSpec->charactersPerLineInCharset
										* fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x)
											* (fontData->fontSpec->fontSize.y - 1)) +

									// Respective char of character
									charOffset
								)
								| (printing->palette << 14);
						}
					}
				}

				if(kPrintingOrientationHorizontal == printing->orientation)
				{
					temp = fontData->fontSpec->fontSize.x;
					x = (printing->direction == kPrintingDirectionLTR)
						? x + temp
						: x - temp;
				}
				else
				{
					temp = fontData->fontSpec->fontSize.y;
					y = (printing->direction == kPrintingDirectionLTR)
						? y + temp
						: y - temp;
				}

				if(x >= 48/* || x < 0*/)
				{
					// Wrap around when outside of the visible area
					temp = fontData->fontSpec->fontSize.y;
					y = (printing->direction == kPrintingDirectionLTR)
						? y + temp
						: y - temp;
					x = startColumn;
				}

				break;
		}
		i++;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Printing::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Initialize members
	this->fonts = new VirtualList();
	this->printingSprites = new VirtualList();
	this->mode = __PRINTING_MODE_DEFAULT;
	this->palette = __PRINTING_PALETTE;
	this->orientation = kPrintingOrientationHorizontal;
	this->direction = kPrintingDirectionLTR;
	this->lastUsedFont = NULL;
	this->lastUsedFontData = NULL;
	this->activePrintingSprite = NULL;
	this->printingBgmapSegment = -1;

	Printing::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Printing::destructor()
{
	if(!isDeleted(this->fonts))
	{
		delete this->fonts;
	}

	this->fonts = NULL;

	if(!isDeleted(this->printingSprites))
	{
		delete this->printingSprites;
	}

	this->printingSprites = NULL;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Printing::onFontCharChangedOffset(ListenerObject eventFirer __attribute__((unused)))
{
	Printing printing = Printing::getInstance();

	CharSet charSet = CharSet::safeCast(eventFirer);

	if(!isDeleted(charSet))
	{
		CharSet::write(charSet);
		Printing::fireEvent(printing, kEventFontRewritten);
		NM_ASSERT(!isDeleted(printing), "Printing::onFontCharChangedOffset: deleted printing during kEventFontRewritten");
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
