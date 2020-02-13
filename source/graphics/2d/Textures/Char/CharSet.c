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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CharSet.h>
#include <HardwareManager.h>
#include <CharSetManager.h>
#include <Mem.h>
#include <VIPManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 * @param charSetSpec				CharSet spec
 * @param offset						Displacement within the CHAR segment
 */
void CharSet::constructor(CharSetSpec* charSetSpec, u16 offset)
{
	Base::constructor();

	// save spec
	this->charSetSpec = charSetSpec;
	this->charSpecDisplacement = 0;

	// set the offset
	this->offset = offset;
	this->usageCount = 1;
}

/**
 * Class destructor
 */
void CharSet::destructor()
{
	Object::fireEvent(this, kEventCharSetDeleted);

	// make sure that I'm not destroyed again
	this->usageCount = 00;

	// free processor memory
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Increase CharSet usage
 */
void CharSet::increaseUsageCount()
{
	this->usageCount++;
}

/**
 * Decrease CharSet usage
 *
 * @return								True if usage count is zero
 */
bool CharSet::decreaseUsageCount()
{
	if(this->usageCount)
	{
		this->usageCount--;
	}

	return 0 == this->usageCount;
}

/**
 * Get usage count
 *
 * @return								Usage count
 */
u8 CharSet::getUsageCount()
{
	return this->usageCount;
}

/**
 * Retrieve allocation type
 *
 * @return				Allocation type
 */
u32 CharSet::getAllocationType()
{
	return this->charSetSpec->allocationType;
}

/**
 * Retrieve the offset within CHAR memory
 *
 * @return				Offset within CHAR memory
 */
u32 CharSet::getOffset()
{
	return this->offset;
}

/**
 * Set the offset within CHAR memory
 *
 * @param offset		Offset within CHAR memory
 */
void CharSet::setOffset(u16 offset)
{
	ASSERT(offset < 2048, "CharSet::setOffset: offset out of bounds");

	this->offset = offset;
}

/**
 * Retrieve the spec
 *
 * @return				CharSetSpec
 */
CharSetSpec* CharSet::getCharSetSpec()
{
	return this->charSetSpec;
}

/**
 * Set the spec
 *
 * @param charSetSpec			CharSetSpec
 */
void CharSet::setCharSetSpec(CharSetSpec* charSetSpec)
{
	this->charSetSpec = charSetSpec;
}

/**
 * Retrieve the number of CHARS in the spec
 *
 * @return 			Number of CHARS in the spec
 */
u32 CharSet::getNumberOfChars()
{
	return this->charSetSpec->numberOfChars;
}

/**
 * Write the CHARs to DRAM
 */
void CharSet::write()
{
	s16 numberOfWORDS = __BYTES_PER_CHARS(this->charSetSpec->numberOfChars) / sizeof(WORD);

	bool disableCache = false;

	if(numberOfWORDS >= DRM_WRINTING_PASSES_TO_ENABLE_CACHE)
	{
		disableCache = true;
		CACHE_DISABLE;
		CACHE_CLEAR;
		CACHE_ENABLE;
	}

	Mem::copyWORD(
		(WORD*)(__CHAR_SPACE_BASE_ADDRESS + (((u32)this->offset) << 4)),
		(WORD*)(this->charSetSpec->charSpec + __BYTES_PER_CHARS(this->charSpecDisplacement)),
		numberOfWORDS
	);

	if(disableCache)
	{
		CACHE_DISABLE;
		CACHE_CLEAR;
		CACHE_ENABLE;
	}
}

/**
 * Rewrite the CHARs to DRAM
 */
void CharSet::rewrite()
{
	switch(this->charSetSpec->allocationType)
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
			break;

		default:
			// write again
			CharSet::write(this);
			break;
	}

	// propagate event
	Object::fireEvent(this, kEventCharSetRewritten);
}

/**
 * Set displacement to add to the offset within the CHAR memory
 *
 * @param charSpecDisplacement		Displacement
 */
void CharSet::setCharSpecDisplacement(u32 charSpecDisplacement)
{
	this->charSpecDisplacement = charSpecDisplacement;
}

/**
 * Write a single CHAR to DRAM
 *
 * @param charToReplace		Index of the CHAR to overwrite
 * @param newChar			CHAR data to write
 */
void CharSet::putChar(u32 charToReplace, BYTE* newChar)
{
	if(newChar && charToReplace < this->charSetSpec->numberOfChars)
	{
		Mem::copyBYTE((BYTE*)__CHAR_SPACE_BASE_ADDRESS + (((u32)this->offset) << 4) + (charToReplace << 4), newChar, (int)(sizeof(BYTE) << 4));
	}
}

/**
 * Write a single pixel to DRAM
 *
 * @param charToReplace		Index of the CHAR to overwrite
 * @param charSetPixel		Pixel data
 * @param newPixelColor		Color value of pixel
 */
void CharSet::putPixel(u32 charToReplace, Pixel* charSetPixel, BYTE newPixelColor)
{
	if(charSetPixel && charToReplace < this->charSetSpec->numberOfChars && charSetPixel->x < 8 && charSetPixel->y < 8)
	{
		static BYTE auxChar[] =
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};

		Mem::copyBYTE(auxChar, (u8*)__CHAR_SPACE_BASE_ADDRESS + (((u32)this->offset) << 4) + (charToReplace << 4), (int)(1 << 4));

		u16 displacement = (charSetPixel->y << 1) + (charSetPixel->x >> 2);
		u16 pixelToReplaceDisplacement = (charSetPixel->x % 4) << 1;
		auxChar[displacement] &= (~(0x03 << pixelToReplaceDisplacement) | ((u16)newPixelColor << pixelToReplaceDisplacement));
		Mem::copyBYTE((u8*)__CHAR_SPACE_BASE_ADDRESS + (((u32)this->offset) << 4) + (charToReplace << 4), auxChar, (int)(1 << 4));
	}
}

/**
 * Write CharSet with displacement = frame * numberOfFrames
 *
 * @param frame		ROM memory displacement multiplier
 */
void CharSet::setFrame(u16 frame)
{
	CharSet::setCharSpecDisplacement(this, this->charSetSpec->numberOfChars * frame);

	CharSet::write(this);
}