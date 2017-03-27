/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CHARSET_H_
#define CHARSET_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// definition of a CharSet for unanimated sprites
#define __NOT_ANIMATED						0x01

// definition of a CharSet for animated sprites
#define __ANIMATED_SINGLE					0x02

// definition of a CharSet for animated sprites
#define __ANIMATED_SINGLE_OPTIMIZED			0x03

// definition of a CharSet for animated sprites with one char set is shared by all
#define __ANIMATED_SHARED					0x04

// definition of a CharSet for animated sprites with a coordinator that syncs them
#define __ANIMATED_SHARED_COORDINATED		0x05

// definition of a charset for animated sprites whose all frames are written to memory and shared
#define __ANIMATED_MULTI					0x06

// char memory room to add
#define __CHAR_ROOM							1


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
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
		u32 charDefinitionDisplacement;																	\
		/* memory displacement */																		\
		u16 offset;																						\
		/* how many textures are using me */															\
		u16 usageCount;																					\

__CLASS(CharSet);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct CharSetDefinition
{
	// number of chars, depending on allocation type:
	// __ANIMATED_SINGLE: number of chars of a single animation frame (cols * rows)
	// __ANIMATED_SHARED: number of chars of a single animation frame (cols * rows)
	// __ANIMATED_SHARED_COORDINATED: number of chars of a single animation frame (cols * rows)
	// __ANIMATED_MULTI: sum of chars of all animation frames
	// __NOT_ANIMATED: number of chars of whole image
	u32 numberOfChars;

	// the way its chars and bgtexture will be allocated in graphic memory
	u32 allocationType;

	// pointer to the char definition in ROM
	BYTE* charDefinition;

} CharSetDefinition;

typedef const CharSetDefinition CharSetROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(CharSet, CharSetDefinition* charSetDefinition, u16 offset);

void CharSet_destructor(CharSet this);
void CharSet_increaseUsageCount(CharSet this);
bool CharSet_decreaseUsageCount(CharSet this);
u32 CharSet_getAllocationType(CharSet this);
u32 CharSet_getOffset(CharSet this);
void CharSet_setOffset(CharSet this, u16 offset);
void CharSet_setCharSetDefinition(CharSet this, CharSetDefinition* charSetDefinition);
CharSetDefinition* CharSet_getCharSetDefinition(CharSet this);
u32 CharSet_getNumberOfChars(CharSet this);
void CharSet_write(CharSet this);
void CharSet_rewrite(CharSet this);
void CharSet_setCharDefinitionDisplacement(CharSet this, u32 charDefinitionDisplacement);
void CharSet_putChar(CharSet this, u32 charToReplace, BYTE* newChar);
void CharSet_putPixel(CharSet this, u32 charToReplace, Point* charSetPixel, BYTE newPixelColor);


#endif
