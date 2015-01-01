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
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

FontSize Printing_getFontSize(u8 fontSizeDefine);

// global to change
static const FontDefinition* _fontDefinitions[8] = {NULL};

// number of registered fonts
u8 _fontsDefinitionCount = 0;

// default font
extern const u16 VBJAE_DEFAULT_FONT_CHARS[];
FontROMDef VBJAE_DEFAULT_FONT =
{
    VBJAE_DEFAULT_FONT_CHARS,
    256,
    0,
    kFont8x8,
    "VBJaEngineFont",
};


//---------------------------------------------------------------------------------------------------------
// 												IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

// setup the bgmap and char memory with printing data
void Printing_registerFont(const FontDefinition* fontDefinition)
{
	_fontDefinitions[_fontsDefinitionCount] = fontDefinition;
	_fontsDefinitionCount ++;
}

// load font data to char memory
void Printing_loadFonts()
{
    int lastFontDefEndPos = CharSeg3 + (512 << 4);
    FontSize fontSize;
    u16 numCharsToAdd = 0;
    u8 i = 0;

	// register vbjaengine default font if there's no custom font registered
	if (_fontDefinitions[0] == NULL)
	{
		Printing_registerFont(&VBJAE_DEFAULT_FONT);
	}

    // load registered fonts to (end of) char memory
    for (i = 0; i < _fontsDefinitionCount; i++)
    {
        fontSize = Printing_getFontSize(_fontDefinitions[i]->fontSize);

        numCharsToAdd = (_fontDefinitions[i]->characterCount * fontSize.x * fontSize.y) << 4;
        lastFontDefEndPos -= numCharsToAdd;

        // note: grit always adds an empty tile at the start of a tileset, so we need to factor this in here
	    Mem_copy(
	        (u8*)(lastFontDefEndPos - (1 << 4)),
	        (u8*)(_fontDefinitions[i]->fontCharDefinition),
	        numCharsToAdd + (1 << 4)
	    );
    }
}

// render general print output layer
void Printing_render(int textLayer)
{
	if (0 > textLayer || textLayer >= __TOTAL_LAYERS)
	{
		ASSERT(false, "Printing::render: invalid layer");
		return;
	}
	
	unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];

	// wait for screen to idle
	while (*xpstts & XPBSYR);

	WA[textLayer].head = WRLD_ON | WRLD_BGMAP | WRLD_OVR | (__PRINTING_BGMAP);
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
void Printing_clear()
{
	VPUManager_clearBgmap(VPUManager_getInstance(), __PRINTING_BGMAP, __PRINTABLE_BGMAP_AREA);
}

// direct printing out method
void Printing_out(u8 bgmap, u16 x, u16 y, const char* string, u16 bplt, const char* font)
{
	u16 i = 0, pos = 0, col = x, fontOffset = 2048;
	u8 j = 0, charOffsetX = 0, charOffsetY = 0;
	FontSize fontSize;

    // iterate over registered fonts to find offset for font to use
    for (j = 0; j < _fontsDefinitionCount; j++)
    {
        fontSize = Printing_getFontSize(_fontDefinitions[j]->fontSize);
        fontOffset -= (_fontDefinitions[j]->characterCount * fontSize.x * fontSize.y);
        if ((font == NULL) || (0 == strcmp(_fontDefinitions[j]->name, font)))
        {
            fontOffset -= (_fontDefinitions[j]->offset * fontSize.x);
            break;
        }
    }

    // print text
	while (string[i])
	{
		pos = (y << 6) + x;

		switch (string[i])
		{
			case 7: // Bell (!)
			case 13: // Line Feed

				break;

			case 9: // Horizontal Tab

				x = (x / TAB_SIZE + 1) * TAB_SIZE;
				break;

			case 10: // Carriage Return

				y += fontSize.y;
				x = col;
				break;

			default:

                for (charOffsetX = 0; charOffsetX < fontSize.x; charOffsetX++)
                {
                    for (charOffsetY = 0; charOffsetY < fontSize.y; charOffsetY++)
                    {
                        BGMM[(0x1000 * bgmap) + pos + (charOffsetY << 6) + charOffsetX] =
                            ((u8)string[i] + fontOffset - 1 + (charOffsetY << 5) + charOffsetX) | (bplt << 14);
                    }
                }
				x += fontSize.x;
				if (x >= 64)
				{
				    y += fontSize.y;
					x = col;
				}
				break;
		}
		i++;
	}
}

void Printing_int(int value, int x, int y, const char* font)
{
	if (value < 0)
	{
		value *= -1;

		Printing_out(__PRINTING_BGMAP, x++, y, "-", 0, font);

		Printing_out(__PRINTING_BGMAP, x, y, Utilities_itoa((int)(value), 10, Utilities_getDigitCount(value)), __PRINTING_PALETTE, font);
	}
	else
	{
		Printing_out(__PRINTING_BGMAP, x, y, Utilities_itoa((int)(value), 10, Utilities_getDigitCount(value)), __PRINTING_PALETTE, font);
	}
}

void Printing_hex(WORD value, int x, int y, const char* font)
{
	if (0 && value<0)
	{
		value *= -1;

		Printing_out(__PRINTING_BGMAP, x++,y,"-", 0, font);
		Printing_out(__PRINTING_BGMAP, x,y, Utilities_itoa((int)(value),16,8), __PRINTING_PALETTE, font);
	}
	else
	{
		Printing_out(__PRINTING_BGMAP, x,y, Utilities_itoa((int)(value),16,8), __PRINTING_PALETTE, font);
	}
}

void Printing_float(float value, int x, int y, const char* font)
{
	int sign = 1;
	int i = 0;
	int length;
	int size = 10;

	#define FIX19_13_FRAC(n)	((n)&0x1FFF)

	int decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);

	if (value < 0)
	{
		sign = -1;

		Printing_out(__PRINTING_BGMAP, x++,y,"-", 0, font);
	}

	decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);

	// print integral part
	length = Utilities_intLength((int)value * sign);

	Printing_out(__PRINTING_BGMAP, x, y, Utilities_itoa(F_FLOOR(value * sign), 10, length), __PRINTING_PALETTE, font);

	// print the dot
	Printing_out(__PRINTING_BGMAP, x + length, y, ".", __PRINTING_PALETTE, font);

	// print the decimal part
	for (i = 0; size; i++)
	{
		if (decimal < size)
		{
			Printing_out(__PRINTING_BGMAP, x + length + 1 + i,y, Utilities_itoa(0, 10, 1), __PRINTING_PALETTE, font);
		}
		else
		{
			i++;
			break;
		}
		size /= 10;
	}

	Printing_out(__PRINTING_BGMAP, x + length  + i ,y, Utilities_itoa(decimal, 10, 0), __PRINTING_PALETTE, font);
}

void Printing_text(char *string, int x, int y, const char* font)
{
	Printing_out(__PRINTING_BGMAP, x, y, string, __PRINTING_PALETTE, font);
}

FontSize Printing_getFontSize(u8 fontSizeDefine)
{
    FontSize fontSize;

    switch (fontSizeDefine)
    {
        default:
        case kFont8x8:
            fontSize.x = 1;
            fontSize.y = 1;
            break;

        case kFont8x16:
            fontSize.x = 1;
            fontSize.y = 2;
            break;
    }

    return fontSize;
}