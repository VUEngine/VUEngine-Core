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

// global to change
static const FontDefinition* _fontDefinitions[8] = {NULL};

// number of registered fonts
u8 _fontsDefinitionCount = 1;

// default font
extern const u16 VBJAE_DEFAULT_FONT_CHARS[];
FontROMDef VBJAE_DEFAULT_FONT =
{
    VBJAE_DEFAULT_FONT_CHARS,
    256,
    0,
    kFont8x8,
    "VBJaE Default",
};


//---------------------------------------------------------------------------------------------------------
// 												IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

// setup the bgmap and char memory with printing data
void Printing_registerFont(const FontDefinition* fontDefinition, bool makeDefault)
{
	_fontDefinitions[makeDefault ? 0 : _fontsDefinitionCount] = fontDefinition;
	_fontsDefinitionCount += (makeDefault && (_fontsDefinitionCount > 0)) ? 0 : 1;
}

// load font data to char memory
void Printing_loadFonts()
{
    int lastFontDefEndPos = CharSeg3 + (512 << 4);
    u16 numCharsToAdd = 0;
    u8 i = 0;

	// register vbjaengine default font if there's no default font yet
	if (_fontDefinitions[0] == NULL)
	{
		Printing_registerFont(&VBJAE_DEFAULT_FONT, true);
	}

    // load registered fonts to (end of) char memory
    for (i = 0; i < _fontsDefinitionCount; i++)
    {
        numCharsToAdd = (_fontDefinitions[i]->characterCount + 1) << 4;
        lastFontDefEndPos -= numCharsToAdd;
	    Mem_copy(
	        (u8*)lastFontDefEndPos,
	        (u8*)(_fontDefinitions[i]->fontCharDefinition),
	        numCharsToAdd
	    );

        // set char mem usage
        //CharSetManager_setChars(CharSetManager_getInstance(), 3, 200);
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
// TODO: make font name an argument
void Printing_out(u8 bgmap, u16 x, u16 y, const char* string, u16 bplt)
{
	u16 i = 0, pos = 0, col = x;

	u8 j = 0;
	u16 charOffset = 2048;

    // iterate over registered fonts to find offset for font to use
    // TODO: add fallback for when font could not be found
    // TODO: use font size of fontDefinition to adjust printing loop below
    for (j = 0; j < _fontsDefinitionCount; j++)
    {
        charOffset -= _fontDefinitions[j]->characterCount;
        if (0 == strcmp(_fontDefinitions[j]->name, "VBJaE Default"))
        {
            charOffset += _fontDefinitions[j]->offset;
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

				break;

			case 9: // Horizontal Tab

				x = (x / TAB_SIZE + 1) * TAB_SIZE;
				break;

			case 10: // Carriage Return

				y++;
				x = col;
				break;

			case 13: // Line Feed

				// x = col;
				break;

			default:

				BGMM[(0x1000 * bgmap) + pos] = ((u8)string[i] + charOffset) | (bplt << 14);
				if (x++ > 63)
				{
					y++;
					x = col;
				}
				break;
		}
		i++;
	}
}

void Printing_int(int value,int x,int y)
{
	if (value < 0)
	{
		value *= -1;

		Printing_out(__PRINTING_BGMAP, x++, y, "-", 0);

		Printing_out(__PRINTING_BGMAP, x, y, Utilities_itoa((int)(value), 10, Utilities_getDigitCount(value)), __PRINTING_PALETTE);
	}
	else
	{
		Printing_out(__PRINTING_BGMAP, x, y, Utilities_itoa((int)(value), 10, Utilities_getDigitCount(value)), __PRINTING_PALETTE);
	}
}

void Printing_hex(WORD value,int x,int y)
{
	if (0 && value<0)
	{
		value *= -1;

		Printing_out(__PRINTING_BGMAP, x++,y,"-", 0);
		Printing_out(__PRINTING_BGMAP, x,y, Utilities_itoa((int)(value),16,8), __PRINTING_PALETTE);
	}
	else
	{
		Printing_out(__PRINTING_BGMAP, x,y, Utilities_itoa((int)(value),16,8), __PRINTING_PALETTE);
	}
}

void Printing_float(float value,int x,int y)
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

		Printing_out(__PRINTING_BGMAP, x++,y,"-", 0);
	}

	decimal = (int)(((float)FIX19_13_FRAC(FTOFIX19_13(value)) / 8192.f) * 10000.f);

	// print integral part
	length = Utilities_intLength((int)value * sign);

	Printing_out(__PRINTING_BGMAP, x, y, Utilities_itoa(F_FLOOR(value * sign), 10, length), __PRINTING_PALETTE);

	// print the dot
	Printing_out(__PRINTING_BGMAP, x + length, y, ".", __PRINTING_PALETTE);

	// print the decimal part
	for (i = 0; size; i++)
	{
		if (decimal < size)
		{
			Printing_out(__PRINTING_BGMAP, x + length + 1 + i,y, Utilities_itoa(0, 10, 1), __PRINTING_PALETTE);
		}
		else
		{
			i++;
			break;
		}
		size /= 10;
	}

	Printing_out(__PRINTING_BGMAP, x + length  + i ,y, Utilities_itoa(decimal, 10, 0), __PRINTING_PALETTE);
}

void Printing_text(char *string, int x, int y)
{
	Printing_out(__PRINTING_BGMAP, x, y, string, __PRINTING_PALETTE);
}