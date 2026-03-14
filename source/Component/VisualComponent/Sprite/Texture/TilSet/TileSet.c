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

#include <TileSetManager.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <DisplayUnit.h>

#include "TileSet.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static TileSet TileSet::get(const TileSetSpec* tileSetSpec)
{
	NM_ASSERT(NULL != tileSetSpec, "TileSet::get: NULL tileSetSpec");

	if(NULL == tileSetSpec)
	{
		return NULL;
	}

	return TileSetManager::getTileSet(TileSetManager::getInstance(), tileSetSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool TileSet::release(TileSet tileSet)
{
	return TileSetManager::releaseTileSet(TileSetManager::getInstance(), tileSet);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSet::constructor(const TileSetSpec* tileSetSpec, uint16 offset)
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Save spec
	this->tileSetSpec = tileSetSpec;
	this->generation = 0;
	this->tilesDisplacement = 0;

	this->frame = 0;
	this->offset = offset;
	this->usageCount = 1;
	this->written = false;

	TileSet::write(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSet::destructor()
{
	// Make sure that I'm not destroyed again
	this->usageCount = 0;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSet::increaseUsageCount()
{
	this->usageCount++;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool TileSet::decreaseUsageCount()
{
	if(0 > --this->usageCount)
	{
		this->usageCount = 0;
	}

	return 0 == this->usageCount;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int8 TileSet::getUsageCount()
{
	return this->usageCount;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool TileSet::hasMultipleFrames()
{
	return NULL != this->tileSetSpec->frameOffsets;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool TileSet::isShared()
{
	return this->tileSetSpec->shared;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool TileSet::isOptimized()
{
	return this->tileSetSpec->optimized;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSet::setOffset(uint16 offset)
{
	ASSERT(offset < 2048, "TileSet::setOffset: offset out of bounds");

	if(this->offset != offset)
	{
		this->generation++;

		this->written = false;
		
		this->offset = offset;

		TileSet::fireEvent(this, kEventTileSetChangedOffset);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 TileSet::getOffset()
{
	return this->offset;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const TileSetSpec* TileSet::getSpec()
{
	return this->tileSetSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 TileSet::getNumberOfChars()
{
	return this->tileSetSpec->numberOfChars;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSet::addChar(uint32 charToAddTo, const uint32* newChar)
{
	if(NULL != newChar && charToAddTo < this->tileSetSpec->numberOfChars)
	{
		Mem::combineWORDs
		(
			(uint32*)(__TILE_SPACE_BASE_ADDRESS + (((uint32)this->offset + charToAddTo) << 4)),
			(uint32*)&this->tileSetSpec->tiles[__UINT32S_PER_TILES(charToAddTo) + 1] + this->tilesDisplacement,
			(uint32*)newChar,
			__UINT32S_PER_TILES(1)
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSet::putChar(uint32 charToReplace, const uint32* newChar)
{
	if(NULL != newChar && charToReplace < this->tileSetSpec->numberOfChars)
	{
		Mem::copyWORD
		(
			(uint32*)(__TILE_SPACE_BASE_ADDRESS + (((uint32)this->offset + charToReplace) << 4)),
			(uint32*)newChar,
			__UINT32S_PER_TILES(1)
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSet::putPixel(const uint32 charToReplace, const Pixel* tileSetPixel, uint8 newPixelColor)
{
	if(tileSetPixel && charToReplace < this->tileSetSpec->numberOfChars && tileSetPixel->x < 8 && tileSetPixel->y < 8)
	{
		static uint8 auxChar[] =
		{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};

		Mem::copyBYTE(auxChar, (uint8*)__TILE_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4) + (charToReplace << 4), (int32)(1 << 4));

		uint16 displacement = (tileSetPixel->y << 1) + (tileSetPixel->x >> 2);
		uint16 pixelToReplaceDisplacement = (tileSetPixel->x % 4) << 1;

		// TODO: review this, only works with non transparency pixels
		auxChar[displacement] &= (~(0x03 << pixelToReplaceDisplacement) | ((uint16)newPixelColor << pixelToReplaceDisplacement));
//		auxChar[displacement] |= (uint16)newPixelColor << pixelToReplaceDisplacement;

		Mem::copyBYTE
		(
			(uint8*)__TILE_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4) + (charToReplace << 4), auxChar, (int32)(sizeof(uint8) << 4)
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSet::setFrame(uint16 frame)
{	
	if(this->frame != frame || !this->written)
	{
		this->written = false;
		
		this->frame = frame;

		this->generation++;

		if(NULL != this->tileSetSpec->frameOffsets)
		{
			this->tilesDisplacement = this->tileSetSpec->frameOffsets[frame] - 1;
		}
		else
		{
			this->tilesDisplacement = __UINT32S_PER_TILES(this->tileSetSpec->numberOfChars * this->frame);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 TileSet::getFrame()
{
	return this->frame;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 TileSet::getGeneration()
{
	return this->generation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 TileSet::write()
{
	if(this->written)
	{
		return this->generation;
	}
	
	NM_ASSERT(0 < this->tileSetSpec->numberOfChars, "TileSet::write: 0 chars");

	uint16 tilesToWrite = this->tileSetSpec->numberOfChars;

#ifdef __SHOW_SPRITES_PROFILING
	extern int32 _writtenTiles;
	_writtenTiles += tilesToWrite;
#endif

	switch(this->tileSetSpec->tiles[0])
	{
		case __TILE_SET_COMPRESSION_RLE:
		{
			TileSet::writeRLE(this);
			break;
		}

		default:
		{
			if(100 < tilesToWrite)
			{
				CACHE_RESET;
			}

			Mem::copyWORD
			(
				(uint32*)(__TILE_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4)),
				&this->tileSetSpec->tiles[1] + this->tilesDisplacement,
				__UINT32S_PER_TILES(tilesToWrite)
			);

			break;
		}
	}

	this->written = true;
	
	return this->generation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TileSet::writeRLE()
{
	// 1 poxel = 2 pixels = 4 bits = 1 hex digit
	// So, each char has 32 poxels
	uint32 totalPoxels = this->tileSetSpec->numberOfChars << 5;

	uint32* destination = (uint32*)(__TILE_SPACE_BASE_ADDRESS + (((uint32)this->offset) << 4));
	uint32* limit = destination + __UINT32S_PER_TILES(this->tileSetSpec->numberOfChars);
	uint32* source = &this->tileSetSpec->tiles[1] + this->tilesDisplacement;

	uint32 uncompressedData = 0;
	uint32 uncompressedDataSize = 0;

	CACHE_RESET;

	for(uint32 poxel = 0; poxel < totalPoxels; poxel++)
	{
		uint32 compressedData = source[poxel];

		uint32 cycles = 4;

		while(cycles--)
		{
			uint8 pack = ((uint8*)&compressedData)[3];

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
