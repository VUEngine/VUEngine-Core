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

#ifndef CHARSET_H_
#define CHARSET_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// definition of a charset of an unanimated character or background
#define __NOT_ANIMATED						0x01

// definition of a charset of an animated character or background
#define __ANIMATED_SINGLE					0x02

// one char set is shared by all sprites
#define __ANIMATED_SHARED					0x03

// a coordinator syncs all sprites
#define __ANIMATED_SHARED_COORDINATED		0x04

// definition of a charset of an animated character of which all frames are written to memory and shared
#define __ANIMATED_MULTI					0x05

// char memory room to add
#define __CHAR_ROOM							1

// event
#define __EVENT_CHARSET_REWRITTEN			"charSetRewritten"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// Defines as a pointer to a structure that's not defined here and so is not accessible to the outside world

#define CharSet_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\

#define CharSet_SET_VTABLE(ClassName)																	\
		Object_SET_VTABLE(ClassName)																	\

#define CharSet_ATTRIBUTES																				\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* char set definition */																		\
        CharSetDefinition* charSetDefinition;															\
        /* array definition of the charSet */															\
        u16 charDefinitionDisplacement;																	\
        /* memory displacement */																		\
        u16 offset;																						\
        /* how many textures are using me */															\
        u8 usageCount;																					\
        /* memory segment */																			\
        u8 segment;																						\

__CLASS(CharSet);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct CharSetDefinition
{
    // number of chars, depending on allocation type:
    // __ANIMATED_SINGLE: number of chars of a single animation frame (cols * rows)
    // __ANIMATED_SHARED: number of chars of a single animation frame (cols * rows)
    // __ANIMATED_SHARED_COORDINATED: number of chars of a single animation frame (cols * rows)
    // __ANIMATED_MULTI: sum of chars of all animation frames
    // __NOT_ANIMATED: number of chars of whole image
	u16 numberOfChars;

	// the way its chars and bgtexture will be allocated in graphic memory
	u16 allocationType;

	// pointer to the char definition in ROM
	BYTE* charDefinition;

} CharSetDefinition;

typedef const CharSetDefinition CharSetROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(CharSet, CharSetDefinition* charSetDefinition, u8 segment, u16 offset);

void CharSet_destructor(CharSet this);
void CharSet_increaseUsageCount(CharSet this);
bool CharSet_decreaseUsageCount(CharSet this);
int CharSet_getAllocationType(CharSet this);
u16 CharSet_getOffset(CharSet this);
void CharSet_setOffset(CharSet this, u16 offset);
void CharSet_setCharSetDefinition(CharSet this, CharSetDefinition* charSetDefinition);
CharSetDefinition* CharSet_getCharSetDefinition(CharSet this);
u16 CharSet_getNumberOfChars(CharSet this);
u8 CharSet_getSegment(CharSet this);
void CharSet_write(CharSet this);
void CharSet_rewrite(CharSet this);
void CharSet_setCharDefinitionDisplacement(CharSet this, u16 charDefinitionDisplacement);
void CharSet_putChar(CharSet this, u16 charToReplace, BYTE* newChar);
void CharSet_putPixel(CharSet this, u16 charToReplace, Point* charSetPixel, BYTE newPixelColor);


#endif
