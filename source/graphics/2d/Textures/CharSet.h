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

#ifndef CHARSET_H_
#define CHARSET_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// definition of a charset of an animated character or background
#define __ANIMATED			0x01

// definition of a charset of an unanimated character or background
#define __NO_ANIMATED		0x02

// definition of a charset of an animated character of which all frames are written to memory and shared
#define __ANIMATED_SHARED	0x03

// future expansion
#define __ANIMATED_SHARED_2	0x04


// event
#define __EVENT_CHARSET_REWRITTEN				"charSetRewritten"

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */

#define CharSet_METHODS														\
		Object_METHODS															\

#define CharSet_SET_VTABLE(ClassName)											\
		Object_SET_VTABLE(ClassName)											\

#define CharSet_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* memory displacement */													\
	u16 offset;																	\
																				\
	/* memory segment */														\
	u8 segment: 2;																\
																				\
	/* allocation type */														\
	u8 allocationType: 3;														\
																				\
	/* number of chars */														\
	u16 numberOfChars: 10;														\
																				\
	/* array definition of the charSet */										\
	BYTE* charDefinition;														\
																				\
	/* array definition of the charSet */										\
	u16 charDefinitionDisplacement;												\

__CLASS(CharSet);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct CharSetDefinition
{
	// number of chars, depending on allocationType:
	// __ANIMATED: number of chars of a single animation frame
	// __ANIMATED_SHARED: sum of chars of all animation frames
	// __NO_ANIMATED: number of chars of whole image
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

__CLASS_NEW_DECLARE(CharSet, CharSetDefinition* charSetDefinition);

void CharSet_destructor(CharSet this);
int CharSet_getAllocationType(CharSet this);
u16 CharSet_getOffset(CharSet this);
void CharSet_setOffset(CharSet this, u16 offset);
BYTE* CharSet_getCharDefinition(CharSet this);
void CharSet_setCharDefinition(CharSet this, BYTE* charDefinition);
void CharSet_setNumberOfChars(CharSet this, int numberOfChars);
int CharSet_getNumberOfChars(CharSet this);
int CharSet_getSegment(CharSet this);
void CharSet_setSegment(CharSet this, int segment);
void CharSet_copy(CharSet this, CharSet source);
void CharSet_write(CharSet this);
void CharSet_rewrite(CharSet this);
void CharSet_setCharDefinitionDisplacement(CharSet this, u16 charDefinitionDisplacement);
void CharSet_putChar(CharSet this, u16 charToReplace, BYTE* newChar);
void CharSet_putPixel(CharSet this, u16 charToReplace, Point* charSetPixel, BYTE newPixelColor);


#endif