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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CharSet.h>
#include <CharSetManager.h>
#include <Mem.h>
#include <VIPManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	CharSet
 * @extends Object
 * @ingroup graphics-2d-textures-char
 */
__CLASS_DEFINITION(CharSet, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals

static void CharSet_constructor(CharSet this, CharSetDefinition* charSetDefinition, u16 offset);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(CharSet, CharSetDefinition* charSetDefinition, u16 offset)
__CLASS_NEW_END(CharSet, charSetDefinition, offset);

/**
 * Class constructor
 *
 * @memberof							CharSet
 * @private
 *
 * @param this							Function scope
 * @param charSetDefinition				CharSet definition
 * @param offset						Displacement within the CHAR segment
 */
static void CharSet_constructor(CharSet this, CharSetDefinition* charSetDefinition, u16 offset)
{
	ASSERT(this, "CharSet::constructor: null this");

	__CONSTRUCT_BASE(Object);

	// save definition
	this->charSetDefinition = charSetDefinition;
	this->charDefinitionDisplacement = 0;

	// set the offset
	this->offset = offset;
	this->usageCount = 1;
}

/**
 * Class destructor
 *
 * @memberof							CharSet
 * @public
 *
 * @param this							Function scope
 */
void CharSet_destructor(CharSet this)
{
	ASSERT(this, "CharSet::destructor: null this");

	Object_fireEvent(__SAFE_CAST(Object, this), kEventCharSetDeleted);

	// make sure that I'm not destroyed again
	this->usageCount = 00;

	// free processor memory
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Increase CharSet usage
 *
 * @memberof							CharSet
 * @public
 *
 * @param this							Function scope
 */
void CharSet_increaseUsageCount(CharSet this)
{
	ASSERT(this, "CharSet::increaseUsageCoung: null this");

	this->usageCount++;
}

/**
 * Decrease CharSet usage
 *
 * @memberof							CharSet
 * @public
 *
 * @param this							Function scope
 *
 * @return								True if usage count is zero
 */
bool CharSet_decreaseUsageCount(CharSet this)
{
	ASSERT(this, "CharSet::getAllocationType: null this");

	if(this->usageCount)
	{
		this->usageCount--;
	}

	return 0 == this->usageCount;
}

/**
 * Get usage count
 *
 * @memberof							CharSet
 * @public
 *
 * @param this							Function scope
 *
 * @return								Usage count
 */
u8 CharSet_getUsageCount(CharSet this)
{
	ASSERT(this, "CharSet::getUsageCount: null this");

	return this->usageCount;
}

/**
 * Retrieve allocation type
 *
 * @memberof			CharSet
 * @public
 *
 * @param this			Function scope
 *
 * @return				Allocation type
 */
u32 CharSet_getAllocationType(CharSet this)
{
	ASSERT(this, "CharSet::getAllocationType: null this");

	return this->charSetDefinition->allocationType;
}

/**
 * Retrieve the offset within CHAR memory
 *
 * @memberof			CharSet
 * @public
 *
 * @param this			Function scope
 *
 * @return				Offset within CHAR memory
 */
u32 CharSet_getOffset(CharSet this)
{
	ASSERT(this, "CharSet::getOffset: null this");

	return this->offset;
}

/**
 * Set the offset within CHAR memory
 *
 * @memberof			CharSet
 * @public
 *
 * @param this			Function scope
 * @param offset		Offset within CHAR memory
 */
void CharSet_setOffset(CharSet this, u16 offset)
{
	ASSERT(this, "CharSet::setOffset: null this");
	ASSERT(offset < 2048, "CharSet::setOffset: offset out of bounds");

	this->offset = offset;
}

/**
 * Retrieve the definition
 *
 * @memberof			CharSet
 * @public
 *
 * @param this			Function scope
 *
 * @return				CharSetDefinition
 */
CharSetDefinition* CharSet_getCharSetDefinition(CharSet this)
{
	ASSERT(this, "CharSet::getCharDefinition: null this");

	return this->charSetDefinition;
}

/**
 * Set the definition
 *
 * @memberof						CharSet
 * @public
 *
 * @param this						Function scope
 * @param charSetDefinition			CharSetDefinition
 */
void CharSet_setCharSetDefinition(CharSet this, CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "CharSet::setCharDefinition: null this");

	this->charSetDefinition = charSetDefinition;
}

/**
 * Retrieve the number of CHARS in the definition
 *
 * @memberof		CharSet
 * @public
 *
 * @param this		Function scope
 *
 * @return 			Number of CHARS in the definition
 */
u32 CharSet_getNumberOfChars(CharSet this)
{
	ASSERT(this, "CharSet::getNumberOfChars: null this");

	return this->charSetDefinition->numberOfChars;
}

/**
 * Write the CHARs to DRAM
 *
 * @memberof		CharSet
 * @public
 *
 * @param this		Function scope
 */
void CharSet_write(CharSet this)
{
	ASSERT(this, "CharSet::write: null this");

	Mem_copyWORD(
		(WORD*)(__CHAR_SPACE_BASE_ADDRESS + (((u32)this->offset) << 4)),
		(WORD*)(this->charSetDefinition->charDefinition + this->charDefinitionDisplacement),
		(u32)(this->charSetDefinition->numberOfChars + __CHAR_ROOM) << 2
	);
}

/**
 * Rewrite the CHARs to DRAM
 *
 * @memberof		CharSet
 * @public
 *
 * @param this		Function scope
 */
void CharSet_rewrite(CharSet this)
{
	ASSERT(this, "CharSet::rewrite: null this");

	// write again
	CharSet_write(this);

	// propagate event
	Object_fireEvent(__SAFE_CAST(Object, this), kEventCharSetRewritten);
}

/**
 * Set displacement to add to the offset within the CHAR memory
 *
 * @memberof								CharSet
 * @public
 *
 * @param this								Function scope
 * @param charDefinitionDisplacement		Displacement
 */
void CharSet_setCharDefinitionDisplacement(CharSet this, u32 charDefinitionDisplacement)
{
	ASSERT(this, "CharSet::setCharDefinitionDisplacement: null this");

	this->charDefinitionDisplacement = charDefinitionDisplacement;
}

/**
 * Write a single CHAR to DRAM
 *
 * @memberof				CharSet
 * @public
 *
 * @param this				Function scope
 * @param charToReplace		Index of the CHAR to overwrite
 * @param newChar			CHAR data to write
 */
void CharSet_putChar(CharSet this, u32 charToReplace, BYTE* newChar)
{
	ASSERT(this, "CharSet::putChar: null this");

	if(newChar && charToReplace < this->charSetDefinition->numberOfChars + __CHAR_ROOM)
	{
		Mem_copyBYTE((BYTE*)__CHAR_SPACE_BASE_ADDRESS + (((u32)this->offset) << 4) + (charToReplace << 4), newChar, (int)(sizeof(BYTE) << 3));
	}
}

/**
 * Write a single pixel to DRAM
 *
 * @memberof				CharSet
 * @public
 *
 * @param this				Function scope
 * @param charToReplace		Index of the CHAR to overwrite
 * @param charSetPixel		Pixel data
 * @param newPixelColor		Color value of pixel
 */
void CharSet_putPixel(CharSet this, u32 charToReplace, Pixel* charSetPixel, BYTE newPixelColor)
{
	ASSERT(this, "CharSet::putPixel: null this");

	if(charSetPixel && charToReplace < this->charSetDefinition->numberOfChars + __CHAR_ROOM && charSetPixel->x < 8 && charSetPixel->y < 8)
	{
		static BYTE auxChar[] =
		{
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,
		};

		Mem_copyBYTE(auxChar, (u8*)__CHAR_SPACE_BASE_ADDRESS + (((u32)this->offset) << 4) + (charToReplace << 4), (int)(1 << 4));

		u16 displacement = (charSetPixel->y << 1) + (charSetPixel->x >> 2);
		u16 pixelToReplaceDisplacement = (charSetPixel->x % 4) << 1;
		auxChar[displacement] &= (~(0x03 << pixelToReplaceDisplacement) | ((u16)newPixelColor << pixelToReplaceDisplacement));
		Mem_copyBYTE((u8*)__CHAR_SPACE_BASE_ADDRESS + (((u32)this->offset) << 4) + (charToReplace << 4), auxChar, (int)(1 << 4));
	}
}
