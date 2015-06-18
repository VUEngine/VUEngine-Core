/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <Printing.h>
#include <HardwareManager.h>
#include <Utilities.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern FontROMDef* __FONTS[];

typedef struct FontData
{
	const struct FontDefinition * fontDefinition;
    u16 memoryOffset;
} FontData;


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// horizontal tab size in chars
#define TAB_SIZE	4


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Printing_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\

// define the Printing
__CLASS_DEFINITION(Printing, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Printing_constructor(Printing this);
static void Printing_out(Printing this, u8 bgmap, u16 x, u16 y, const char* string, u16 bplt, const char* font);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(Printing);

// class's constructor
static void Printing_constructor(Printing this)
{
	__CONSTRUCT_BASE();
}

// class's destructor
void Printing_destructor(Printing this)
{
	// allow a new construct
	__SINGLETON_DESTROY;
}

// load font data to char memory
void Printing_loadFonts(Printing this)
{
    int lastFontDefEndPos = CharSeg3 + (512 << 4);
    u16 numCharsToAdd = 0;
    u8 i = 0;

    // load registered fonts to (end of) char memory
    for(; __FONTS[i]; i++)
    {
        numCharsToAdd = (__FONTS[i]->characterCount * __FONTS[i]->fontSize.x * __FONTS[i]->fontSize.y) << 4;
        lastFontDefEndPos -= numCharsToAdd;
	    Mem_copy((u8*)(lastFontDefEndPos), (u8*)(__FONTS[i]->fontCharDefinition), numCharsToAdd);
    }
}

// render general print output layer
void Printing_render(Printing this, int textLayer)
{
	if(0 > textLayer || textLayer >= __TOTAL_LAYERS)
	{
		ASSERT(false, "Printing::render: invalid layer");
		return;
	}
	
	WA[textLayer].head = WRLD_ON | WRLD_BGMAP | WRLD_OVR | (BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance()));
	WA[textLayer].mx = 0;
	WA[textLayer].mp = 0;
	WA[textLayer].my = 0;
	WA[textLayer].gx = __PRINTING_BGMAP_X_OFFSET;
	WA[textLayer].gp = __PRINTING_BGMAP_Z_OFFSET;
	WA[textLayer].gy = __PRINTING_BGMAP_Y_OFFSET;
	WA[textLayer].w = __SCREEN_WIDTH;
	WA[textLayer].h = __SCREEN_HEIGHT;
}

// clear printing area
void Printing_clear(Printing this)
{
	u8 printingBgmap = BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance());

	VPUManager_clearBgmap(VPUManager_getInstance(), printingBgmap, __PRINTABLE_BGMAP_AREA);
}

// get font definition and starting position in character memory
static FontData Printing_getFontByName(Printing this, const char* font)
{
    FontData fontData = {NULL, 2048};

	// iterate over registered fonts to find memory offset of font to use
	u8 j = 0;
    for(; __FONTS[j]; j++)
    {
        fontData.fontDefinition = __FONTS[j];
        fontData.memoryOffset -= (fontData.fontDefinition->characterCount * fontData.fontDefinition->fontSize.x * fontData.fontDefinition->fontSize.y);

        if((font == NULL) || (0 == strcmp(fontData.fontDefinition->name, font)))
        {
            break;
        }
    }

    return fontData;
}

// direct printing out method
static void Printing_out(Printing this, u8 bgmap, u16 x, u16 y, const char* string, u16 bplt, const char* font)
{
    FontData fontData;
	u16 i = 0, pos = 0, col = x;
	u8 charOffsetX = 0, charOffsetY = 0;

    fontData = Printing_getFontByName(this, font);
    
    // print text
	while (string[i] && x < __SCREEN_WIDTH / 8)
	{
		pos = (y << 6) + x;

		switch(string[i])
		{
			case 13: // Line Feed

				break;

			case 9: // Horizontal Tab

				x = (x / TAB_SIZE + 1) * TAB_SIZE * fontData.fontDefinition->fontSize.x;
				break;

			case 10: // Carriage Return

				y += fontData.fontDefinition->fontSize.y;
				x = col;
				break;

			default:

                for(charOffsetX = 0; charOffsetX < fontData.fontDefinition->fontSize.x; charOffsetX++)
                {
                    for(charOffsetY = 0; charOffsetY < fontData.fontDefinition->fontSize.y; charOffsetY++)
                    {
                        BGMM[(0x1000 * bgmap) + pos + charOffsetX + (charOffsetY << 6)] =
                            (
                                // start at correct font
                                fontData.memoryOffset +

                                // top left char of letter
                                ((u8)(string[i] - fontData.fontDefinition->offset) * fontData.fontDefinition->fontSize.x) +

                                // skip lower chars of multi-char fonts with y > 1
                                ((((u8)(string[i] - fontData.fontDefinition->offset) * fontData.fontDefinition->fontSize.x) >> 5) * ((fontData.fontDefinition->fontSize.y - 1)) << 5) +

                                // respective char of letter in multi-char fonts
                                charOffsetX + (charOffsetY << 5)
                            )
                            | (bplt << 14);
                    }
                }

                x += fontData.fontDefinition->fontSize.x;
				if(x >= 64)
				{
				    y += fontData.fontDefinition->fontSize.y;
					x = col;
				}

				break;
		}
		i++;
	}
}

void Printing_int(Printing this, int value, int x, int y, const char* font)
{
	u8 printingBgmap = BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance());
	
	if(value < 0)
	{
		value *= -1;

		Printing_out(this, printingBgmap, x++, y, "-", 0, font);
		Printing_out(this, printingBgmap, x, y, Utilities_itoa((int)(value), 10, Utilities_getDigitCount(value)), __PRINTING_PALETTE, font);
	}
	else
	{
		Printing_out(this, printingBgmap, x, y, Utilities_itoa((int)(value), 10, Utilities_getDigitCount(value)), __PRINTING_PALETTE, font);
	}
}

void Printing_hex(Printing this, WORD value, int x, int y, const char* font)
{
	u8 printingBgmap = BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance());

	if(0 && value<0)
	{
		value *= -1;

		Printing_out(this, printingBgmap, x++,y,"-", 0, font);
		Printing_out(this, printingBgmap, x,y, Utilities_itoa((int)(value),16,8), __PRINTING_PALETTE, font);
	}
	else
	{
		Printing_out(this, printingBgmap, x,y, Utilities_itoa((int)(value),16,8), __PRINTING_PALETTE, font);
	}
}

void Printing_float(Printing this, float value, int x, int y, const char* font)
{
	u8 printingBgmap = BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance());

	int sign = 1;
	int i = 0;
	int length;
	int size = 10;

	#define FIX19_13_FRAC(n)	((n)&0x1FFF)

	int decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);

	if(value < 0)
	{
		sign = -1;
		Printing_out(this, printingBgmap, x++,y,"-", 0, font);
	}

	decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);

	// print integral part
	length = Utilities_intLength((int)value * sign);

	Printing_out(this, printingBgmap, x, y, Utilities_itoa(F_FLOOR(value * sign), 10, length), __PRINTING_PALETTE, font);

	// print the dot
	Printing_out(this, printingBgmap, x + length, y, ".", __PRINTING_PALETTE, font);

	// print the decimal part
	for(i = 0; size; i++)
	{
		if(decimal < size)
		{
			Printing_out(this, printingBgmap, x + length + 1 + i,y, Utilities_itoa(0, 10, 1), __PRINTING_PALETTE, font);
		}
		else
		{
			i++;
			break;
		}
		size /= 10;
	}

	Printing_out(this, printingBgmap, x + length  + i ,y, Utilities_itoa(decimal, 10, 0), __PRINTING_PALETTE, font);
}

void Printing_text(Printing this, char* string, int x, int y, const char* font)
{
	u8 printingBgmap = BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager_getInstance());
	Printing_out(this, printingBgmap, x, y, string, __PRINTING_PALETTE, font);
}

Size Printing_getTextSize(Printing this, const char* string, const char* font)
{
    Size size = {0, 0};
    FontData fontData;
	u16 i = 0, currentLineLength = 0;

    fontData = Printing_getFontByName(this, font);
    size.y =  fontData.fontDefinition->fontSize.y;

	while (string[i])
	{
		switch(string[i])
		{
			case 13: // Line Feed

				break;

			case 9: // Horizontal Tab

				currentLineLength += (currentLineLength / TAB_SIZE + 1) * TAB_SIZE * fontData.fontDefinition->fontSize.x;
				break;

			case 10: // Carriage Return

				size.y += fontData.fontDefinition->fontSize.y;
				currentLineLength = 0;
				break;

			default:

                currentLineLength += fontData.fontDefinition->fontSize.x;
				if(currentLineLength >= 64)
				{
                    size.y += fontData.fontDefinition->fontSize.y;
                    currentLineLength = 0;
				}

				break;
		}

        if(currentLineLength > size.x) {
            size.x = currentLineLength;
        }

		i++;
	}

	return size;
}