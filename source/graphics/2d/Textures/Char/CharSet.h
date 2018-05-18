/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


class CharSet : Object
{
	/**
	* @var CharSetDefinition*  charSetDefinition
	* @brief					Charset definition
	* @memberof				CharSet
	*/
	CharSetDefinition* charSetDefinition;
	/**
	* @var u32 				charDefinitionDisplacement
	* @brief					Array definition of the charSet
	* @memberof				CharSet
	*/
	u32 charDefinitionDisplacement;
	/**
	* @var u16 				offset
	* @brief					Memory displacement
	* @memberof				CharSet
	*/
	u16 offset;
	/**
	* @var u8 					usageCount
	* @brief					How many textures are using me
	* @memberof				CharSet
	*/
	u8 usageCount;

	void constructor(CharSetDefinition* charSetDefinition, u16 offset);
	void increaseUsageCount();
	bool decreaseUsageCount();
	u32 getAllocationType();
	u32 getOffset();
	void setOffset(u16 offset);
	void setCharSetDefinition(CharSetDefinition* charSetDefinition);
	CharSetDefinition* getCharSetDefinition();
	u32 getNumberOfChars();
	void write();
	void rewrite();
	void setCharDefinitionDisplacement(u32 charDefinitionDisplacement);
	void putChar(u32 charToReplace, BYTE* newChar);
	void putPixel(u32 charToReplace, Pixel* charSetPixel, BYTE newPixelColor);
}


#endif
