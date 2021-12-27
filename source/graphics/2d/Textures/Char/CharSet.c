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
void CharSet::constructor(CharSetSpec* charSetSpec, uint16 offset)
{
	Base::constructor();

	// save spec
	this->charSetSpec = charSetSpec;
	this->tilesDisplacement = 1;

	// set the offset
	this->offset = offset;
	this->usageCount = 1;
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
uint8 CharSet::getUsageCount()
{
	return this->usageCount;
}

/**
 * Retrieve allocation type
 *
 * @return				Allocation type
 */
uint32 CharSet::getAllocationType()
{
	return this->charSetSpec->allocationType;
}

/**
 * Retrieve the offset within CHAR memory
 *
 * @return				Offset within CHAR memory
 */
uint32 CharSet::getOffset()
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
uint32 CharSet::getNumberOfChars()
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
	uint32* source = this->charSetSpec->tiles + this->tilesDisplacement;

	uint32 uncompressedData = 0;
	uint32 uncompressedDataSize = 0;

	for(uint32 poxel = 0; poxel < totalPoxels; poxel++)
	{
		uint32 compressedData = source[poxel];

		uint32 cycles = 4;

		while(cycles--)
		{
			uint32 counter = ((0xF0000000 & compressedData) >> 28) + 1;
			uint32 data = (0x0F000000 & compressedData) >> 24;

			while(counter--)
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
	if(0 == this->charSetSpec->numberOfChars)
	{
		return;
	}
	
	switch(this->charSetSpec->tiles[0])
	{
		case __CHAR_SET_COMPRESSION_RLE:

			CharSet::writeRLE(this);
			break;

		default:

			Mem::copyWORD(
				(uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4)),
				this->charSetSpec->tiles + this->tilesDisplacement,
				__UINT32S_PER_CHARS(this->charSetSpec->numberOfChars)
			);

			break;
	}
}

/**
 * Rewrite the CHARs to DRAM
 */
void CharSet::rewrite()
{
	CharSet::write(this);

	// propagate event
	CharSet::fireEvent(this, kEventCharSetRewritten);
	NM_ASSERT(!isDeleted(this), "CharSet::rewrite: deleted this during kEventCharSetRewritten");
}

/**
 * Set displacement to add to the offset within the CHAR memory
 *
 * @param tilesDisplacement		Displacement
 */
void CharSet::setTilesDisplacement(uint32 tilesDisplacement)
{
	this->tilesDisplacement = tilesDisplacement;
}

/**
 * Write a single CHAR to DRAM
 *
 * @param charToReplace		Index of the CHAR to overwrite
 * @param newChar			CHAR data to write
 */
void CharSet::putChar(uint32 charToReplace, uint32* newChar)
{
	if(newChar && charToReplace < this->charSetSpec->numberOfChars)
	{
		Mem::copyWORD(
			(uint32*)(__CHAR_SPACE_BASE_ADDRESS + (((uint32)this->offset + charToReplace) << 4)),
			(uint32*)newChar,
			__UINT32S_PER_CHARS(1)
		);
	}
}

// TODO: if inline is allowed, the optization that GCC does makes this innefective in putPixel method
// It is not because of O3 optimization option, the same happens with O1
static void __attribute__ ((noinline)) CharSet::copyBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES)
{
	const BYTE* finalSource = source + numberOfBYTES;

	asm("				\n\t"      \
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
void CharSet::putPixel(uint32 charToReplace, Pixel* charSetPixel, BYTE newPixelColor)
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
	if(NULL != this->charSetSpec->frameOffsets)
	{
		CharSet::setTilesDisplacement(this, this->charSetSpec->frameOffsets[frame]);
	}
	else
	{
		CharSet::setTilesDisplacement(this, __UINT32S_PER_CHARS(this->charSetSpec->numberOfChars * frame) + 1);
	}
}