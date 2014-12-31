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

#ifndef PRINTING_H_
#define	PRINTING_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CharSetManager.h>
#include <TextureManager.h>


//---------------------------------------------------------------------------------------------------------
// 												DEFINES
//---------------------------------------------------------------------------------------------------------

enum FontSizes
{
	kFont4x8,
	kFont4x16,
	kFont8x8,
	kFont8x16,
	kFont16x16,
	kFont16x32,
	kFont32x32,
	kFont32x64,
};

// max length of a font's name
#define __MAX_FONT_NAME_LENGTH	16


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct FontDefinition
{
    // font chars definition
	const u16* fontCharDefinition;

	// number of characters in font
	u16 characterCount;

	// at which character number the font starts
	s16 offset;

	// size of a single character (e.g. kFont8x8)
	u8 fontSize;

	// font's name
	char name[__MAX_FONT_NAME_LENGTH];

} FontDefinition;

typedef const FontDefinition FontROMDef;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void Printing_render(int textLayer);
void Printing_registerFont(const FontDefinition* fontDefinition, bool makeDefault);
void Printing_loadFonts();
void Printing_clear();
void Printing_int(int value,int x,int y);
void Printing_hex(WORD value,int x,int y);
int Utilities_intLength(int value);
void Printing_text(char *string,int x,int y);
void Printing_float(float value,int x,int y);


#endif