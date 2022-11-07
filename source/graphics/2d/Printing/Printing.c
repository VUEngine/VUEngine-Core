/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

extern FontROMSpec* const _fonts[];
extern FontROMSpec DefaultFont;


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
	(FontSpec*)&DefaultFont,

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
	this->orientation = kPrintingOrientationHorizontal;
	this->direction = kPrintingDirectionLTR;
	this->lastUsedFontData = NULL;
	this->printingSprite = NULL;

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
	if(!isDeleted(this->printingSprite))
	{
		SpriteManager::disposeSprite(SpriteManager::getInstance(), Sprite::safeCast(this->printingSprite));
	}

	this->printingSprite = NULL;

	Printing::releaseFonts(this);

	VirtualList::clear(this->fonts);

	Printing::setOrientation(this, kPrintingOrientationHorizontal);
	Printing::setDirection(this, kPrintingDirectionLTR);
}

void Printing::setupSprite()
{
	if(!isDeleted(this->printingSprite))
	{
		return;
	}
	
	PrintingSpriteSpec PRINTING_SP =
	{
		{
			{
				// sprite's type
				__TYPE(PrintingSprite),

				// texture spec
				NULL,

				// transparency (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
				__TRANSPARENCY_NONE,

				// displacement
				{
					0, // x
					0, // y
					0, // z
					0, // parallax
				},
			},

			// bgmap mode (__WORLD_BGMAP, __WORLD_AFFINE, __WORLD_OBJECT or __WORLD_HBIAS)
			// make sure to use the proper corresponding sprite type throughout the spec (BgmapSprite or ObjectSprite)
			__WORLD_BGMAP,

			// pointer to affine/hbias manipulation function
			NULL,

			// display mode (__WORLD_ON, __WORLD_LON or __WORLD_RON)
			__WORLD_ON,
		}
	};

	this->printingSprite = PrintingSprite::safeCast(SpriteManager::createSprite(SpriteManager::getInstance(), (SpriteSpec*)&PRINTING_SP, NULL));

	PixelVector position = 
	{
		0, 0, 0, 0
	};

	PrintingSprite::setPosition(this->printingSprite, &position);
}

void Printing::setOrientation(uint8 value)
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

void Printing::setDirection(uint8 value)
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

void Printing::onFontCharSetRewritten(ListenerObject eventFirer __attribute__((unused)))
{
	Printing::fireEvent(this, kEventFontRewritten);
	NM_ASSERT(!isDeleted(this), "Printing::onFontCharSetRewritten: deleted this during kEventFontRewritten");
}

void Printing::loadFonts(FontSpec** fontSpecs)
{
	// Since fonts' charsets will be released, there is no reason to keep
	// anything in the printing area
	Printing::clear(this);

	// empty list of registered fonts
	Printing::releaseFonts(this);

	// Prevent VIP's interrupt from calling render during this process
	HardwareManager::suspendInterrupts();

	// Make sure all sprites are ready
	SpriteManager::prepareAll(SpriteManager::getInstance());

	// iterate over all defined fonts and add to internal list
	uint32 i = 0, j = 0;
	for(i = 0; _fonts[i]; i++)
	{
		// instance and initialize a new fontdata instance
		FontData* fontData = new FontData;
		fontData->fontSpec = _fonts[i];
		fontData->charSet = NULL;

		// preload charset for font if in list of fonts to preload
		if(fontSpecs)
		{
			// find defined font in list of fonts to preload
			for(j = 0; fontSpecs[j]; j++)
			{
				// preload charset and save charset reference, if font was found
				if(_fonts[i]->charSetSpec == fontSpecs[j]->charSetSpec)
				{
					fontData->charSet = CharSetManager::getCharSet(CharSetManager::getInstance(), fontSpecs[j]->charSetSpec);

					CharSet::addEventListener(fontData->charSet, ListenerObject::safeCast(this), (EventListener)Printing::onFontCharSetRewritten, kEventCharSetRewritten);
				}
			}
		}

		// add fontdata to internal list
		VirtualList::pushBack(this->fonts, fontData);
	}

	HardwareManager::resumeInterrupts();
}

void Printing::setFontPage(const char* font, uint16 page)
{
	FontData* fontData = Printing::getFontByName(this, font);

	if(NULL == fontData || isDeleted(fontData->charSet))
	{
		return;
	}

	CharSet::setFrame(fontData->charSet, page);
	CharSet::write(fontData->charSet);
}

void Printing::loadDebugFont()
{
	Mem::copyWORD(
		(uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)VUENGINE_DEBUG_FONT_CHARSET_OFFSET) << 4)),
		VUENGINE_DEBUG_FONT_DATA.fontSpec->charSetSpec->tiles + 1,
		__UINT32S_PER_CHARS(VUENGINE_DEBUG_FONT_SIZE)
	);
}

void Printing::setDebugMode()
{
	Printing::loadDebugFont(this);
	this->mode = __PRINTING_MODE_DEBUG;
}

void Printing::setPalette(uint8 palette)
{
	if(palette < 4)
	{
		this->palette = palette;
	}
}

void Printing::clear()
{
	if(!isDeleted(this->printingSprite))
	{
		Mem::clear((BYTE*)__BGMAP_SEGMENT(BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()) + 1) - __PRINTABLE_BGMAP_AREA * 2, __PRINTABLE_BGMAP_AREA * 2);
	}
}

void Printing::releaseFonts()
{
	Printing::removeEventListeners(this, NULL, kEventFontRewritten);

	VirtualNode node = VirtualList::begin(this->fonts);

	for(; NULL != node; node = VirtualNode::getNext(node))
	{
		FontData* fontData = VirtualNode::getData(node);

		if(!isDeleted(fontData) && !isDeleted(fontData->charSet))
		{
			CharSet::removeEventListener(fontData->charSet, ListenerObject::safeCast(this), (EventListener)Printing::onFontCharSetRewritten, kEventCharSetRewritten);

			while(!CharSetManager::releaseCharSet(CharSetManager::getInstance(), fontData->charSet));
		}

		delete fontData;
	}

	VirtualList::clear(this->fonts);

	this->lastUsedFontData = NULL;
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
		if(NULL != this->lastUsedFontData && NULL != font)
		{
			if(!strcmp(this->lastUsedFontData->fontSpec->name, font))
			{
				return this->lastUsedFontData;
			}
		}

		// set first defined font as default
		result = VirtualList::front(this->fonts);

		if(result)
		{
			if(font)
			{
				// iterate over registered fonts to find spec of font to use
				VirtualNode node = VirtualList::begin(this->fonts);
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

			// if font's charset has not been preloaded, load it now
			if(NULL == result->charSet)
			{
				result->charSet = CharSetManager::getCharSet(CharSetManager::getInstance(), result->fontSpec->charSetSpec);

				CharSet::addEventListener(result->charSet, ListenerObject::safeCast(this), (EventListener)Printing::onFontCharSetRewritten, kEventCharSetRewritten);
			}
		}
	}

	this->lastUsedFontData = result;
	
	return result;
}

void Printing::number(int32 value, uint8 x, uint8 y, const char* font)
{
	if(value < 0)
	{
		value = -value;

		Printing::out(this, x++, y, "-", font);
		Printing::out(this, x, y, Utilities::itoa((int32)(value), 10, Utilities::getDigitCount(value)), font);
	}
	else
	{
		Printing::out(this, x, y, Utilities::itoa((int32)(value), 10, Utilities::getDigitCount(value)), font);
	}
}

void Printing::int32(int32 value, uint8 x, uint8 y, const char* font)
{
	Printing::number(this, value, x, y, font);
}

void Printing::hex(WORD value, uint8 x, uint8 y, uint8 length, const char* font)
{
	Printing::out(this, x,y, Utilities::itoa((int32)(value), 16, length), font);
}

void Printing::float(float value, uint8 x, uint8 y, int32 precision, const char* font)
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
	char* integer = Utilities::itoa(floorValue, 10, Utilities::getDigitCount(floorValue));

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
	int32 flooredDecimalValue = (int32)Utilities::floor(decimalValue);

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
			int32 totalDecimalDigits = Utilities::getDigitCount(roundedDecimalValue);

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

	Printing::text(this, string, x, y, font);
}

void Printing::text(const char* string, int32 x, int32 y, const char* font)
{
#ifdef __FORCE_UPPERCASE
	Printing::out(this, x, y, Utilities::toUppercase(string), font);
#else
	Printing::out(this, x, y, string, font);
#endif
}

#ifdef __FORCE_PRINTING_LAYER
void Printing::setCoordinates(int16 x __attribute__ ((unused)), int16 y __attribute__ ((unused)), int16 z __attribute__ ((unused)), int8 parallax __attribute__ ((unused)))
{
	Printing::setWorldCoordinates(this, 0, 0, 0, 0);
	Printing::setBgmapCoordinates(this, 0, 0, 0);
	Printing::setWorldSize(this, __SCREEN_WIDTH, __SCREEN_HEIGHT);
}

void Printing::setWorldCoordinates(int16 x __attribute__ ((unused)), int16 y __attribute__ ((unused)), int16 z __attribute__ ((unused)), int8 parallax __attribute__ ((unused)))
{
	if(!isDeleted(this->printingSprite))
	{
		PixelVector position = 
		{
			0, 0, -64, -4
		};

		PrintingSprite::setPosition(this->printingSprite, &position);
	}
}

void Printing::setBgmapCoordinates(int16 mx __attribute__ ((unused)), int16 my __attribute__ ((unused)), int8 mp __attribute__ ((unused)))
{
	if(!isDeleted(this->printingSprite))
	{
		PrintingSprite::setMValues(this->printingSprite, __PRINTING_BGMAP_X_OFFSET, __PRINTING_BGMAP_Y_OFFSET, 0);
	}
}

void Printing::setWorldSize(uint16 w __attribute__ ((unused)), uint16 h __attribute__ ((unused)))
{
	if(!isDeleted(this->printingSprite))
	{
		PrintingSprite::setSize(this->printingSprite, __SCREEN_WIDTH - 1, __SCREEN_HEIGHT - 1);
	}
}

#else
void Printing::setCoordinates(int16 x, int16 y, int16 z, int8 parallax)
{
	Printing::setWorldCoordinates(this, x, y, z, parallax);
	Printing::setBgmapCoordinates(this, x, y, 0);
}

void Printing::setWorldCoordinates(int16 x, int16 y, int16 z, int8 parallax)
{
	if(!isDeleted(this->printingSprite))
	{
		PixelVector position = 
		{
			x <= __SCREEN_WIDTH ? x : 0, 
			y <= __SCREEN_HEIGHT ? y : 0, 
			z, 
			parallax
		};

		PrintingSprite::setPosition(this->printingSprite, &position);
	}
}

void Printing::setBgmapCoordinates(int16 mx __attribute__ ((unused)), int16 my __attribute__ ((unused)), int8 mp __attribute__ ((unused)))
{
	if(!isDeleted(this->printingSprite))
	{
		PrintingSprite::setMValues(this->printingSprite, mx <= 64 * 8 ? mx : 0, __PRINTING_BGMAP_Y_OFFSET + my <= 64 * 8 ? __PRINTING_BGMAP_Y_OFFSET + my : __PRINTING_BGMAP_Y_OFFSET, mp);
	}
}

void Printing::setWorldSize(uint16 w __attribute__ ((unused)), uint16 h __attribute__ ((unused)))
{
	if(!isDeleted(this->printingSprite))
	{
		PrintingSprite::setSize(this->printingSprite, w < __SCREEN_WIDTH ? w : __SCREEN_WIDTH, h < __SCREEN_HEIGHT ? h : __SCREEN_HEIGHT);
	}
}
#endif

int16 Printing::getWorldCoordinatesX()
{
	return !isDeleted(this->printingSprite) ? PrintingSprite::getGX(this->printingSprite) : 0;
}

int16 Printing::getWorldCoordinatesY()
{
	return !isDeleted(this->printingSprite) ? PrintingSprite::getGY(this->printingSprite) : 0;
}

int16 Printing::getWorldCoordinatesP()
{
	return !isDeleted(this->printingSprite) ? PrintingSprite::getGP(this->printingSprite) : 0;
}

PixelVector Printing::getSpritePosition()
{
	return !isDeleted(this->printingSprite) ? PrintingSprite::getDisplacedPosition(this->printingSprite) : (PixelVector){0, 0, 0, 0};
}

void Printing::setTransparent(uint8 value)
{
	if(!isDeleted(this->printingSprite))
	{
		Sprite::setTransparent(this->printingSprite, value);
	}
}

void Printing::resetCoordinates()
{
	if(!isDeleted(this->printingSprite))
	{
		PrintingSprite::reset(this->printingSprite);
	}
}

FontSize Printing::getTextSize(const char* string, const char* font)
{
	FontSize fontSize = {0, 0};
	uint16 i = 0, currentLineLength = 0;

	FontData* fontData = Printing::getFontByName(this, font);

	if(NULL == fontData)
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

void Printing::show()
{
	if(!isDeleted(this->printingSprite))
	{
		PrintingSprite::show(this->printingSprite);
		PrintingSprite::setPosition(this->printingSprite, PrintingSprite::getPosition(this->printingSprite));
	}
}

void Printing::hide()
{
	if(!isDeleted(this->printingSprite))
	{
		PrintingSprite::hide(this->printingSprite);
	}
}

void Printing::render(uint8 textLayer)
{
	if(!isDeleted(this->printingSprite))
	{
		PrintingSprite::doRender(this->printingSprite, textLayer, false);
	}
}

void Printing::out(uint8 x, uint8 y, const char* string, const char* font)
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

	uint32 i = 0, position = 0, startColumn = x, temp = 0;
	uint32 charOffset = 0, charOffsetX = 0, charOffsetY = 0;
	uint32 printingBgmap = BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance());

	FontData* fontData = Printing::getFontByName(this, font);

	if(NULL == fontData || (__PRINTING_MODE_DEBUG != this->mode && isDeleted(fontData->charSet)))
	{
		return;
	}

	uint16* const bgmapSpaceBaseAddress = (uint16*)__BGMAP_SPACE_BASE_ADDRESS;
	uint32 offset = __PRINTING_MODE_DEBUG == this->mode ? VUENGINE_DEBUG_FONT_CHARSET_OFFSET : CharSet::getOffset(fontData->charSet);

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
									((uint8)(string[i] - fontData->fontSpec->offset) * fontData->fontSpec->fontSize.x) +

									// additional y offset in charset
									(((uint8)(string[i] - fontData->fontSpec->offset)
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
