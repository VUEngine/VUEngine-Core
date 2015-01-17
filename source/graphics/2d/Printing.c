/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <Printing.h>
#include <HardwareManager.h>
#include <Utilities.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// horizontal tab size in chars
#define TAB_SIZE 4


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Printing_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* array of registered fonts */												\
	const FontDefinition* fonts[8];												\
																				\
	/* total number of registered fonts */										\
	u8 fontsDefinitionCount;													\

// define the Printing
__CLASS_DEFINITION(Printing);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern FontDefinition VBJAE_DEFAULT_FONT;

static void Printing_constructor(Printing this);
FontSize Printing_getFontSize(Printing this, u8 fontSizeDefine);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(Printing);

// class's constructor
static void Printing_constructor(Printing this)
{
	__CONSTRUCT_BASE(Object);
}

// class's destructor
void Printing_destructor(Printing this)
{
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// setup the bgmap and char memory with printing data
void Printing_registerFont(Printing this, const FontDefinition* fontDefinition)
{
	this->fonts[this->fontsDefinitionCount++] = fontDefinition;
}

// load font data to char memory
void Printing_loadFonts(Printing this)
{
    int lastFontDefEndPos = CharSeg3 + (512 << 4);
    u16 numCharsToAdd = 0;
    u8 i = 0;

	// register vbjaengine default font if there's no custom font registered
	if (this->fonts[0] == NULL)
	{
		Printing_registerFont(Printing_getInstance(), &VBJAE_DEFAULT_FONT);
	}

    // load registered fonts to (end of) char memory
    for (i = 0; i < this->fontsDefinitionCount; i++)
    {
        numCharsToAdd = (this->fonts[i]->characterCount * this->fonts[i]->fontSize.x * this->fonts[i]->fontSize.y) << 4;
        lastFontDefEndPos -= numCharsToAdd;

	    Mem_copy((u8*)(lastFontDefEndPos), (u8*)(this->fonts[i]->fontCharDefinition), numCharsToAdd);
    }
}

// render general print output layer
void Printing_render(Printing this, int textLayer)
{
	if (0 > textLayer || textLayer >= __TOTAL_LAYERS)
	{
		ASSERT(false, "Printing::render: invalid layer");
		return;
	}
	
	WA[textLayer].head = WRLD_ON | WRLD_BGMAP | WRLD_OVR | (TextureManager_getPrintingBgmapSegment(TextureManager_getInstance()));
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
	int printingBgmap = TextureManager_getPrintingBgmapSegment(TextureManager_getInstance());

	VPUManager_clearBgmap(VPUManager_getInstance(), printingBgmap, __PRINTABLE_BGMAP_AREA);
}

// direct printing out method
void Printing_out(Printing this, u8 bgmap, u16 x, u16 y, const char* string, u16 bplt, const char* font)
{
	u16 i = 0, pos = 0, col = x, fontStart = 2048;
	u8 j = 0, charOffsetX = 0, charOffsetY = 0;

    // iterate over registered fonts to find memory offset of font to use
    for (j = 0; j < this->fontsDefinitionCount; j++)
    {
        fontStart -= (this->fonts[j]->characterCount * this->fonts[j]->fontSize.x * this->fonts[j]->fontSize.y);
        if ((font == NULL) || (0 == strcmp(this->fonts[j]->name, font)))
        {
            break;
        }
    }

    // print text
	while (string[i])
	{
		pos = (y << 6) + x;

		switch (string[i])
		{
			case 13: // Line Feed

				break;

			case 9: // Horizontal Tab

				x = (x / TAB_SIZE + 1) * TAB_SIZE * this->fonts[j]->fontSize.x;
				break;

			case 10: // Carriage Return

				y += this->fonts[j]->fontSize.y;
				x = col;
				break;

			default:

                for (charOffsetX = 0; charOffsetX < this->fonts[j]->fontSize.x; charOffsetX++)
                {
                    for (charOffsetY = 0; charOffsetY < this->fonts[j]->fontSize.y; charOffsetY++)
                    {
                        BGMM[(0x1000 * bgmap) + pos + charOffsetX + (charOffsetY << 6)] =
                            (
                                // start at correct font
                                fontStart +

                                // top left char of letter
                                ((u8)(string[i] - this->fonts[j]->offset) * this->fonts[j]->fontSize.x) +

                                // skip lower chars of multi-char fonts with y > 1
                                ((((u8)(string[i] - this->fonts[j]->offset) * this->fonts[j]->fontSize.x) >> 5) * ((this->fonts[j]->fontSize.y - 1)) << 5) +

                                // respective char of letter in multi-char fonts
                                charOffsetX + (charOffsetY << 5)
                            )
                            | (bplt << 14);
                    }
                }

                x += this->fonts[j]->fontSize.x;
				if (x >= 64)
				{
				    y += this->fonts[j]->fontSize.y;
					x = col;
				}

				break;
		}
		i++;
	}
}

void Printing_int(Printing this, int value, int x, int y, const char* font)
{
	int printingBgmap = TextureManager_getPrintingBgmapSegment(TextureManager_getInstance());
	
	if (value < 0)
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
	int printingBgmap = TextureManager_getPrintingBgmapSegment(TextureManager_getInstance());

	if (0 && value<0)
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
	int printingBgmap = TextureManager_getPrintingBgmapSegment(TextureManager_getInstance());

	int sign = 1;
	int i = 0;
	int length;
	int size = 10;

	#define FIX19_13_FRAC(n)	((n)&0x1FFF)

	int decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);

	if (value < 0)
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
	for (i = 0; size; i++)
	{
		if (decimal < size)
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

void Printing_text(Printing this, char *string, int x, int y, const char* font)
{
	int printingBgmap = TextureManager_getPrintingBgmapSegment(TextureManager_getInstance());

	Printing_out(this, printingBgmap, x, y, string, __PRINTING_PALETTE, font);
}