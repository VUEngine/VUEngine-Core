/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CharSetManager.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <VIPManager.h>

#include "CharSet.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S DEFINITIONS
//---------------------------------------------------------------------------------------------------------



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
void CharSet::constructor(CharSetSpec* charSetSpec, uint16 offset)
{
	Base::constructor();

	// save spec
	this->charSetSpec = charSetSpec;
	this->tilesDisplacement = 0;

	// set the offset
	this->offset = offset;
	this->usageCount = 1;
	this->written = false;

	CharSet::write(this);
}

/**
 * Class destructor
 */
void CharSet::destructor()
{
	CharSet::fireEvent(this, kEventCharSetDeleted);
	NM_ASSERT(!isDeleted(this), "CharSet::destructor: deleted this during kEventCharSetDeleted");

	// make sure that I'm not destroyed again
	this->usageCount = 0;

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
	if(0 < this->usageCount)
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
uint8 CharSet::getUsageCount()
{
	return this->usageCount;
}

/**
 * Return if shared or not
 *
 * @return				Boolean
 */
bool CharSet::isShared()
{
	return this->charSetSpec->shared;
}

/**
 * Return if optimized or not
 *
 * @return				Boolean
 */
bool CharSet::isOptimized()
{
	return this->charSetSpec->optimized;
}

/**
 * Retrieve the offset within CHAR memory
 *
 * @return				Offset within CHAR memory
 */
uint16 CharSet::getOffset()
{
	return this->offset;
}

/**
 * Set the offset within CHAR memory
 *
 * @param offset		Offset within CHAR memory
 */
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

/**
 * Retrieve the spec
 *
 * @return				CharSetSpec
 */
CharSetSpec* CharSet::getSpec()
{
	return this->charSetSpec;
}

/**
 * Retrieve the number of CHARS in the spec
 *
 * @return 			Number of CHARS in the spec
 */
uint16 CharSet::getNumberOfChars()
{
	return this->charSetSpec->numberOfChars;
}

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

/**
 * Write the CHARs to DRAM
 */
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

			CharSet::writeRLE(this);
			break;

		default:

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
				"loop%=:				\n\t"      \
				"ld.w	0[%1], r11		\n\t"      \
				"ld.w	4[%1], r12		\n\t"      \
				"ld.w	8[%1], r13		\n\t"      \
				"st.w	r11, 0[%0]		\n\t"      \
				"addi	16, %0, %0		\n\t"      \
				"ld.w	12[%1], r11		\n\t"      \
				"st.w	r12, -12[%0]	\n\t"      \
				"addi	16, %1, %1		\n\t"      \
				"st.w	r13, -8[%0]		\n\t"      \
				"add 	-1, %2			\n\t"      \
				"st.w	r11, -4[%0]		\n\t"      \
				"bne   loop%=			\n\t"
				: // No Output
				: "r" ((uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4))), "r" (&this->charSetSpec->tiles[1] + this->tilesDisplacement), "r" (__UINT32S_PER_CHARS(tilesToWrite) >> 2)
				: "r11", "r12", "r13" // regs used
			);
#endif
			break;
	}

	this->written = true;
}

/**
 * Write a single CHAR to DRAM
 *
 * @param charToReplace		Index of the CHAR to overwrite
 * @param newChar			CHAR data to write
 */
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

/**
 * Add a single CHAR to DRAM
 *
 * @param charToAddTo		Index of the CHAR to overwrite
 * @param newChar			CHAR data to write
 */
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

// TODO: if inline is allowed, the optimization that GCC does makes this ineffective in putPixel method
// It is not because of O3 optimization option, the same happens with O1
static void __attribute__ ((noinline)) CharSet::copyBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES)
{
	const BYTE* finalSource = source + numberOfBYTES;

	asm
	(
		"jr end%=		\n\t"      \
		"loop%=:		\n\t"      \
		"ld.b 0[%1],r10	\n\t"      \
		"st.b r10,0[%0] \n\t"      \
		"add 1,%0		\n\t"      \
		"add 1,%1		\n\t"      \
		"end%=:			\n\t"      \
		"cmp %1,%2		\n\t"      \
		"bgt loop%=		\n\t"
		: // No Output
		: "r" (destination), "r" (source), "r" (finalSource)
		: "r10" // regs used
	);
}

/**
 * Write a single pixel to DRAM
 *
 * @param charToReplace		Index of the CHAR to overwrite
 * @param charSetPixel		Pixel data
 * @param newPixelColor		Color value of pixel
 */
void CharSet::putPixel(const uint32 charToReplace, const Pixel* charSetPixel, BYTE newPixelColor)
{
	if(charSetPixel && charToReplace < this->charSetSpec->numberOfChars && charSetPixel->x < 8 && charSetPixel->y < 8)
	{
		static BYTE auxChar[] =
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};

		CharSet::copyBYTE(auxChar, (uint8*)__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4) + (charToReplace << 4), (int32)(1 << 4));

		uint16 displacement = (charSetPixel->y << 1) + (charSetPixel->x >> 2);
		uint16 pixelToReplaceDisplacement = (charSetPixel->x % 4) << 1;

		// TODO: review this, only works with non transparent pixels
		auxChar[displacement] &= (~(0x03 << pixelToReplaceDisplacement) | ((uint16)newPixelColor << pixelToReplaceDisplacement));
//		auxChar[displacement] |= (uint16)newPixelColor << pixelToReplaceDisplacement;

		CharSet::copyBYTE((uint8*)__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4) + (charToReplace << 4), auxChar, (int32)(sizeof(BYTE) << 4));
	}
}

/**
 * Write CharSet with displacement = frame * numberOfFrames
 *
 * @param frame		ROM memory displacement multiplier
 */
void CharSet::setFrame(uint16 frame)
{
	uint32 tilesDisplacement = 0;

	if(NULL != this->charSetSpec->frameOffsets)
	{
		tilesDisplacement = this->charSetSpec->frameOffsets[frame] - 1;
	}
	else
	{
		tilesDisplacement = __UINT32S_PER_CHARS(this->charSetSpec->numberOfChars * frame);
	}

	if(!this->written || this->tilesDisplacement != tilesDisplacement)
	{
		this->tilesDisplacement = tilesDisplacement;

		CharSet::write(this);
	}
}