/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <CharSetManager.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <VIPManager.h>

#include "CharSet.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static CharSet CharSet::get(const CharSetSpec* charSetSpec)
{
	NM_ASSERT(NULL != charSetSpec, "CharSet::get: NULL charSetSpec");

	if(NULL == charSetSpec)
	{
		return NULL;
	}

	return CharSetManager::getCharSet(CharSetManager::getInstance(), charSetSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CharSet::release(CharSet charSet)
{
	return CharSetManager::releaseCharSet(CharSetManager::getInstance(), charSet);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::constructor(const CharSetSpec* charSetSpec, uint16 offset)
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Save spec
	this->charSetSpec = charSetSpec;
	this->tilesDisplacement = 0;

	this->frame = 0;
	this->offset = offset;
	this->usageCount = 1;
	this->written = false;

	CharSet::write(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::destructor()
{
	CharSet::fireEvent(this, kEventCharSetDeleted);
	NM_ASSERT(!isDeleted(this), "CharSet::destructor: deleted this during kEventCharSetDeleted");

	// Make sure that I'm not destroyed again
	this->usageCount = 0;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::increaseUsageCount()
{
	this->usageCount++;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CharSet::decreaseUsageCount()
{
	if(0 > --this->usageCount)
	{
		this->usageCount = 0;
	}

	return 0 == this->usageCount;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int8 CharSet::getUsageCount()
{
	return this->usageCount;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CharSet::hasMultipleFrames()
{
	return NULL != this->charSetSpec->frameOffsets;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CharSet::isShared()
{
	return this->charSetSpec->shared;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CharSet::isOptimized()
{
	return this->charSetSpec->optimized;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::setOffset(uint16 offset)
{
	ASSERT(offset < 2048, "CharSet::setOffset: offset out of bounds");

	this->written = this->written && this->offset == offset;

	this->offset = offset;

	if(!this->written)
	{
		CharSet::fireEvent(this, kEventCharSetChangedOffset);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 CharSet::getOffset()
{
	return this->offset;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const CharSetSpec* CharSet::getSpec()
{
	return this->charSetSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 CharSet::getNumberOfChars()
{
	return this->charSetSpec->numberOfChars;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::addChar(uint32 charToAddTo, const uint32* newChar)
{
	if(NULL != newChar && charToAddTo < this->charSetSpec->numberOfChars)
	{
		Mem::combineWORDs
		(
			(uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset + charToAddTo) << 4)),
			(uint32*)&this->charSetSpec->tiles[__UINT32S_PER_CHARS(charToAddTo) + 1] + this->tilesDisplacement,
			(uint32*)newChar,
			__UINT32S_PER_CHARS(1)
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::putChar(uint32 charToReplace, const uint32* newChar)
{
	if(NULL != newChar && charToReplace < this->charSetSpec->numberOfChars)
	{
		Mem::copyWORD
		(
			(uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset + charToReplace) << 4)),
			(uint32*)newChar,
			__UINT32S_PER_CHARS(1)
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::putPixel(const uint32 charToReplace, const Pixel* charSetPixel, BYTE newPixelColor)
{
	if(charSetPixel && charToReplace < this->charSetSpec->numberOfChars && charSetPixel->x < 8 && charSetPixel->y < 8)
	{
		static BYTE auxChar[] =
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};

		Mem::copyBYTE(auxChar, (uint8*)__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4) + (charToReplace << 4), (int32)(1 << 4));

		uint16 displacement = (charSetPixel->y << 1) + (charSetPixel->x >> 2);
		uint16 pixelToReplaceDisplacement = (charSetPixel->x % 4) << 1;

		// TODO: review this, only works with non transparency pixels
		auxChar[displacement] &= (~(0x03 << pixelToReplaceDisplacement) | ((uint16)newPixelColor << pixelToReplaceDisplacement));
//		auxChar[displacement] |= (uint16)newPixelColor << pixelToReplaceDisplacement;

		Mem::copyBYTE
		(
			(uint8*)__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4) + (charToReplace << 4), auxChar, (int32)(sizeof(BYTE) << 4)
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::setFrame(uint16 frame)
{	
	if(!this->written || this->frame != frame)
	{
		this->frame = frame;

		if(NULL != this->charSetSpec->frameOffsets)
		{
			this->tilesDisplacement = this->charSetSpec->frameOffsets[frame] - 1;
		}
		else
		{
			this->tilesDisplacement = __UINT32S_PER_CHARS(this->charSetSpec->numberOfChars * this->frame);
		}

		CharSet::write(this);

		if(CharSet::isShared(this))
		{
			CharSet::fireEvent(this, kEventCharSetChangedFrame);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 CharSet::getFrame()
{
	return this->frame;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::write()
{
	NM_ASSERT(0 < this->charSetSpec->numberOfChars, "CharSet::write: 0 chars");

	uint16 tilesToWrite = this->charSetSpec->numberOfChars;

#ifdef __SHOW_SPRITES_PROFILING
	extern int32 _writtenTiles;
	_writtenTiles += tilesToWrite;
#endif

	switch(this->charSetSpec->tiles[0])
	{
		case __CHAR_SET_COMPRESSION_RLE:
		{
			CharSet::writeRLE(this);
			break;
		}

		default:
		{
			if(100 < tilesToWrite)
			{
				CACHE_RESET;
			}

#ifndef __RELEASE
			Mem::copyWORD
			(
				(uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4)),
				&this->charSetSpec->tiles[1] + this->tilesDisplacement,
				__UINT32S_PER_CHARS(tilesToWrite)
			);
#else
			// Based on dasi's memcpy
			asm
			(
				"loop%=:				\n\t"	  \
				"ld.w	0[%1], r11		\n\t"	  \
				"ld.w	4[%1], r12		\n\t"	  \
				"ld.w	8[%1], r13		\n\t"	  \
				"st.w	r11, 0[%0]		\n\t"	  \
				"addi	16, %0, %0		\n\t"	  \
				"ld.w	12[%1], r11		\n\t"	  \
				"st.w	r12, -12[%0]	\n\t"	  \
				"addi	16, %1, %1		\n\t"	  \
				"st.w	r13, -8[%0]		\n\t"	  \
				"add 	-1, %2			\n\t"	  \
				"st.w	r11, -4[%0]		\n\t"	  \
				"bne   loop%=			\n\t"
				: // No Output
				: "r" ((uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4))), 
					"r" (&this->charSetSpec->tiles[1] + this->tilesDisplacement), "r" (__UINT32S_PER_CHARS(tilesToWrite) >> 2)
				: "r11", "r12", "r13" // regs used
			);
#endif
			break;
		}
	}

	this->written = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSet::writeRLE()
{
	// 1 poxel = 2 pixels = 4 bits = 1 hex digit
	// So, each char has 32 poxels
	uint32 totalPoxels = this->charSetSpec->numberOfChars << 5;

	uint32* destination = (uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4));
	uint32* limit = destination + __UINT32S_PER_CHARS(this->charSetSpec->numberOfChars);
	uint32* source = &this->charSetSpec->tiles[1] + this->tilesDisplacement;

	uint32 uncompressedData = 0;
	uint32 uncompressedDataSize = 0;

	CACHE_RESET;

	for(uint32 poxel = 0; poxel < totalPoxels; poxel++)
	{
		uint32 compressedData = source[poxel];

		uint32 cycles = 4;

		while(cycles--)
		{
			uint8 pack = ((BYTE*)&compressedData)[3];

			uint8 counter = (pack >> 4) + 1;
			uint8 data = 0x0F & (pack);

			while(0 < counter--)
			{
				uncompressedData = (uncompressedData << 4) | data;
				uncompressedDataSize++;

				if(8 <= uncompressedDataSize)
				{
					if(limit <= destination)
					{
						return;
					}

					*destination = uncompressedData;
					destination++;
					
					uncompressedData = 0;
					uncompressedDataSize = 0;
				}
			}

			compressedData <<= 8;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
