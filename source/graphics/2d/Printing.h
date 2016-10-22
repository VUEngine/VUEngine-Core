/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef PRINTING_H_
#define	PRINTING_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>


//---------------------------------------------------------------------------------------------------------
// 												DEFINES
//---------------------------------------------------------------------------------------------------------

typedef struct FontSize
{
	u8 x;
	u8 y;
} FontSize;

// max length of a font's name
#define __MAX_FONT_NAME_LENGTH	16


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Printing_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define Printing_SET_VTABLE(ClassName)																	\
		Object_SET_VTABLE(ClassName)																	\

// declare class
__CLASS(Printing);

// declare class attributes
#define Printing_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\
        VirtualList fonts;																				\


typedef struct FontDefinition
{
    // font charset definition pointer
	CharSetDefinition* charSetDefinition;

	// at which character number the font starts
	s16 offset;

	// size of a single character (in chars) ({width, height})
	FontSize fontSize;

	// font's name
	char name[__MAX_FONT_NAME_LENGTH];

} FontDefinition;

typedef const FontDefinition FontROMDef;


typedef struct FontData
{
	const struct FontDefinition* fontDefinition;
    CharSet charSet;

} FontData;


//---------------------------------------------------------------------------------------------------------
// 												PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

Printing Printing_getInstance();

void Printing_destructor(Printing this);
void Printing_render(Printing this, int textLayer);
void Printing_reset(Printing this);
void Printing_loadFonts(Printing this, FontDefinition** fontDefinitions);
void Printing_clear(Printing this);
void Printing_int(Printing this, int value, u8 x, u8 y, const char* font);
void Printing_hex(Printing this, WORD value, u8 x, u8 y, u8 length, const char* font);
void Printing_float(Printing this, float value, u8 x, u8 y, const char* font);
void Printing_text(Printing this, const char *string, int x, int y, const char* font);
Size Printing_getTextSize(Printing this, const char* string, const char* font);


#endif
