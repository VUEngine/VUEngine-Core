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

// spec of a CharSet for unanimated sprites
#define __NOT_ANIMATED						0x01

// spec of a CharSet for animated sprites
#define __ANIMATED_SINGLE					0x02

// spec of a CharSet for animated sprites
#define __ANIMATED_SINGLE_OPTIMIZED			0x03

// spec of a CharSet for animated sprites with one char set is shared by all
#define __ANIMATED_SHARED					0x04

// spec of a CharSet for animated sprites with a coordinator that syncs them
#define __ANIMATED_SHARED_COORDINATED		0x05

// spec of a charset for animated sprites whose all frames are written to memory and shared
#define __ANIMATED_MULTI					0x06

// char memory room to add
#define __CHAR_ROOM							1


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A CharSet spec
 *
 * @memberof CharSet
 */
typedef struct CharSetSpec
{
	/// number of chars, depending on allocation type:
	/// __ANIMATED_SINGLE: number of chars of a single animation frame (cols * rows)
	/// __ANIMATED_SHARED: number of chars of a single animation frame (cols * rows)
	/// __ANIMATED_SHARED_COORDINATED: number of chars of a single animation frame (cols * rows)
	/// __ANIMATED_MULTI: sum of chars of all animation frames
	/// __NOT_ANIMATED: number of chars of whole image
	u32 numberOfChars;

	/// the way its chars and bgtexture will be allocated in graphic memory
	u32 allocationType;

	/// pointer to the char spec in ROM
	BYTE* charSpec;

} CharSetSpec;

/**
 * A CharSet spec that is stored in ROM
 *
 * @memberof CharSet
 */
typedef const CharSetSpec CharSetROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-textures-char
class CharSet : Object
{
	// Charset spec
	CharSetSpec* charSetSpec;
	// Array spec of the charSet
	u32 charSpecDisplacement;
	// Memory displacement
	u16 offset;
	// How many textures are using me
	u8 usageCount;

	/// @publicsection
	void constructor(CharSetSpec* charSetSpec, u16 offset);
	void increaseUsageCount();
	bool decreaseUsageCount();
	u32 getAllocationType();
	u32 getOffset();
	void setOffset(u16 offset);
	void setCharSetSpec(CharSetSpec* charSetSpec);
	CharSetSpec* getCharSetSpec();
	u32 getNumberOfChars();
	void write();
	void rewrite();
	void setCharSpecDisplacement(u32 charSpecDisplacement);
	void putChar(u32 charToReplace, BYTE* newChar);
	void putPixel(u32 charToReplace, Pixel* charSetPixel, BYTE newPixelColor);
}


#endif
