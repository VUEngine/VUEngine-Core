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

#include <BgmapTexture.h>
#include <Mem.h>
#include <ParamTableManager.h>
#include <Printing.h>
#include <VirtualList.h>
#include <VIPManager.h>

#include "BgmapTextureManager.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Texture;
friend class BgmapTexture;
friend class VirtualList;
friend class VirtualNode;

	
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapTextureManager::reset()
{
	NM_ASSERT(__BGMAP_SPACE_BASE_ADDRESS < __PARAM_TABLE_END, "BgmapTextureManager::reset: bgmap address space is negative");

	VirtualList::deleteData(this->bgmapTextures);

	this->availableBgmapSegmentsForTextures = (uint32)((__PARAM_TABLE_END - __BGMAP_SPACE_BASE_ADDRESS) / __BGMAP_SEGMENT_SIZE);

	if(this->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}

	this->printingBgmapSegment = this->availableBgmapSegmentsForTextures - 1;

	// clear each bgmap segment usage
	for(int32 i = 0; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS; i++)
	{
		this->usedTiles[i] = 0;

		// clear the offsets
		for(int32 j = 0; j <__NUM_BGMAPS_PER_SEGMENT; j++)
		{
			this->xOffset[i][j] = 0;
			this->yOffset[i][j] = 0;
		}
	}

	for(int32 i = 0; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		this->offset[i][kXOffset] = -1;
		this->offset[i][kYOffset] = -1;
		this->offset[i][kCols] = 0;
		this->offset[i][kRows] = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapTextureManager::clearBgmapSegment(int32 segment)
{
	Mem::clear((BYTE*)__BGMAP_SEGMENT(segment), 64 * 64 * 2);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapTextureManager::calculateAvailableBgmapSegments()
{
	uint32 paramTableBase = ParamTableManager::getParamTableBase(ParamTableManager::getInstance());

	this->availableBgmapSegmentsForTextures = (uint32)((paramTableBase - __BGMAP_SPACE_BASE_ADDRESS) / __BGMAP_SEGMENT_SIZE);

	if(this->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}

	this->printingBgmapSegment = this->availableBgmapSegmentsForTextures - 1;

	Printing::setPrintingBgmapSegment(Printing::getInstance(), this->printingBgmapSegment);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int8 BgmapTextureManager::getAvailableBgmapSegmentsForTextures()
{
	return this->availableBgmapSegmentsForTextures;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int8 BgmapTextureManager::getPrintingBgmapSegment()
{
	return this->printingBgmapSegment;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapTextureManager::loadTextures(const TextureSpec** textureSpecs)
{
	// textures
	if(NULL != textureSpecs)
	{
		VirtualList sortedTextureSpecs = new VirtualList();
		VirtualList preloadedTextures = new VirtualList();

		for(int16 i = 0; NULL != textureSpecs[i]; i++)
		{
			if(NULL == textureSpecs[i]->charSetSpec)
			{
				continue;
			}
			
			if
			(
				textureSpecs[i]->recyclable 
				|| 
				(
					textureSpecs[i]->charSetSpec->shared
				)
			)
			{
				VirtualNode node = NULL;

				for(node = sortedTextureSpecs->head; NULL != node; node = node->next)
				{
					TextureSpec* sortedTextureSpec = (TextureSpec*)node->data;

					if(textureSpecs[i] == sortedTextureSpec)
					{
						break;
					}

					if(textureSpecs[i]->rows < sortedTextureSpec->rows)
					{
						continue;
					}

					VirtualList::insertBefore(sortedTextureSpecs, node, textureSpecs[i]);
					break;
				}

				if(NULL == node)
				{
					VirtualList::pushBack(sortedTextureSpecs, textureSpecs[i]);
				}
			}
		}

		// Must not release the texture just after loading them so
		// the recyclable ones are not recycled immediately
		for(VirtualNode node = sortedTextureSpecs->head; NULL != node; node = node->next)
		{
			TextureSpec* textureSpec = (TextureSpec*)node->data;

			BgmapTexture bgmapTexture = BgmapTextureManager::getTexture(this, textureSpec, 0, false, __WORLD_1x1);

			NM_ASSERT(!isDeleted(bgmapTexture), "BgmapTextureManager::loadTextures: failed to load bgmapTexture");
			VirtualList::pushBack(preloadedTextures, bgmapTexture);
		}

		delete sortedTextureSpecs;

		for(VirtualNode node = VirtualList::begin(preloadedTextures); NULL != node; node = node->next)
		{
			BgmapTextureManager::releaseTexture(this, BgmapTexture::safeCast(node->data));
		}

		delete preloadedTextures;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

BgmapTexture BgmapTextureManager::getTexture
(
	BgmapTextureSpec* bgmapTextureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue
)
{
	if(NULL == bgmapTextureSpec)
	{
		NM_ASSERT(false, "BgmapTextureManager::getTexture: NULL spec provided");
		return NULL;
	}

	BgmapTexture bgmapTexture = NULL;

	if(NULL == bgmapTextureSpec->charSetSpec)
	{
		bgmapTexture = BgmapTextureManager::allocateTexture(this, bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);

		ASSERT(!isDeleted(bgmapTexture), "BgmapTextureManager::getTexture: (animated) texture no allocated");
	}
	else
	{
		if(!bgmapTextureSpec->charSetSpec->shared)
		{
			if(bgmapTextureSpec->recyclable)
			{
				bgmapTexture = BgmapTextureManager::findTexture(this, bgmapTextureSpec, true);
			}
			
			if(NULL != bgmapTexture)
			{
				BgmapTexture::increaseUsageCount(bgmapTexture);
			}
			else
			{
				// load a new texture
				bgmapTexture = 
					BgmapTextureManager::allocateTexture(this, bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);
			}

			ASSERT(bgmapTexture, "BgmapTextureManager::getTexture: (animated) texture no allocated");
		}
		else
		{
			// first try to find an already created texture
			bgmapTexture = BgmapTextureManager::findTexture(this, bgmapTextureSpec, false);

			// if couldn't find the texture
			if(NULL != bgmapTexture)
			{
				BgmapTexture::increaseUsageCount(bgmapTexture);
			}
			else
			{
				// load it
				bgmapTexture = 
					BgmapTextureManager::allocateTexture(this, bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);
			}

			ASSERT(!isDeleted(bgmapTexture), "BgmapTextureManager::getTexture: (shared) texture no allocated");
		}
	}

	if(!isDeleted(bgmapTexture) && kTextureWritten != bgmapTexture->status)
	{
		BgmapTexture::prepare(bgmapTexture);
	}

	return bgmapTexture;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapTextureManager::releaseTexture(BgmapTexture bgmapTexture)
{
	// if no one is using the texture anymore
	if(!isDeleted(bgmapTexture))
	{
		BgmapTexture::decreaseUsageCount(bgmapTexture);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapTextureManager::getXOffset(int32 id)
{
	return this->offset[id][kXOffset];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 BgmapTextureManager::getYOffset(int32 id)
{
	return this->offset[id][kYOffset];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void BgmapTextureManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "BGMAP TEXTURES USAGE", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Segments for textures: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), BgmapTextureManager::getAvailableBgmapSegmentsForTextures(this), x + 23, y, NULL);
	Printing::text(Printing::getInstance(), "Printing segment: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(this), x + 23, y, NULL);
	Printing::text(Printing::getInstance(), "Textures count: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->bgmapTextures), x + 23, y, NULL);

	y++;
	y++;
	Printing::text(Printing::getInstance(), "Recyclable textures", x, y++, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Total: ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Free: ", x, y++, NULL);

	y++;
	Printing::text(Printing::getInstance(), "ROM", x, y++, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Address   Refs", x, y++, NULL);
	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", x, y++, NULL);

	int32 i = 0;
	int32 j = 0;
	int32 recyclableTextures = 0;
	int32 freeEntries = 0;

	// try to find a texture with the same bgmap spec
	for(VirtualNode node = this->bgmapTextures->head; NULL != node; node = node->next)
	{
		BgmapTexture bgmapTexture = BgmapTexture::safeCast(node->data);

		TextureSpec* allocatedTextureSpec = Texture::getSpec(bgmapTexture);

		if(allocatedTextureSpec->recyclable)
		{
			recyclableTextures++;
			freeEntries += !BgmapTexture::getUsageCount(bgmapTexture)? 1 : 0;

			Printing::hex(Printing::getInstance(), (int32)Texture::getSpec(bgmapTexture), x + j, y + i, 8, NULL);
			Printing::int32(Printing::getInstance(), BgmapTexture::getUsageCount(bgmapTexture), x + j + 9, y + i, NULL);

			if(++i + y > __SCREEN_HEIGHT / 8)
			{
				i = 0;
				j += 11;

				if(j + x > __SCREEN_WIDTH / 8)
				{
					i = 0;
					j = 0;
				}
			}
		}
	}

	Printing::int32(Printing::getInstance(), recyclableTextures, x + 7, y - 7, NULL);
	Printing::int32(Printing::getInstance(), freeEntries, x + 7, y - 6, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapTextureManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->bgmapTextures = new VirtualList();
	BgmapTextureManager::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapTextureManager::destructor()
{
	VirtualList::deleteData(this->bgmapTextures);
	delete this->bgmapTextures;
	this->bgmapTextures = NULL;

	// allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

BgmapTexture BgmapTextureManager::findTexture(BgmapTextureSpec* bgmapTextureSpec, bool recyclableOnly)
{
	TextureSpec* textureSpec = (TextureSpec*)bgmapTextureSpec;
	BgmapTexture selectedBgmapTexture = NULL;

	CACHE_RESET;

	// try to find a texture with the same bgmap spec
	for(VirtualNode node = this->bgmapTextures->head; NULL != node; node = node->next)
	{
		BgmapTexture allocatedBgmapTexture = BgmapTexture::safeCast(node->data);
		TextureSpec* allocatedTextureSpec = allocatedBgmapTexture->textureSpec;

		if(!recyclableOnly && allocatedTextureSpec == textureSpec)
		{
			if
			(
				(
					NULL == allocatedBgmapTexture->charSet 
					|| 
					allocatedTextureSpec->charSetSpec->shared == bgmapTextureSpec->charSetSpec->shared
				) 
				&&
				(
					allocatedTextureSpec->padding.cols == bgmapTextureSpec->padding.cols 
					&& 
					allocatedTextureSpec->padding.rows == bgmapTextureSpec->padding.rows
				)
			)
			{
				// return if found
				return allocatedBgmapTexture;
			}
		}

		if(!textureSpec->recyclable)
		{
			continue;
		}

		if(allocatedTextureSpec->recyclable && 0 == allocatedBgmapTexture->usageCount)
		{
			uint16 id = allocatedBgmapTexture->id;
			uint16 cols = this->offset[id][kCols];
			uint16 rows = this->offset[id][kRows];

			if
			(
				textureSpec->cols > (cols >> 2) && textureSpec->cols <= cols 
				&& 
				(textureSpec->rows > (rows >> 2)) && textureSpec->rows <= rows
			)
			{
				if(textureSpec->cols == cols && textureSpec->rows == rows)
				{
					selectedBgmapTexture = allocatedBgmapTexture;
					break;
				}
				else if(NULL == selectedBgmapTexture)
				{
					selectedBgmapTexture = allocatedBgmapTexture;
				}
				else 
				{
					uint16 selectedBgmapTextureId = selectedBgmapTexture->id;
					uint16 selectedBgmapTextureCols = this->offset[selectedBgmapTextureId][kCols];
					uint16 selectedBgmapTextureRows = this->offset[selectedBgmapTextureId][kRows];

					if(cols < selectedBgmapTextureCols || rows < selectedBgmapTextureRows)
					{
						selectedBgmapTexture = allocatedBgmapTexture;
					}
				}
			}
		}
	}

	if(!isDeleted(selectedBgmapTexture))
	{
		Texture::setSpec(selectedBgmapTexture, textureSpec);
	}

	return selectedBgmapTexture;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

BgmapTexture BgmapTextureManager::allocateTexture
(
	BgmapTextureSpec* bgmapTextureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue
)
{
	uint16 id = VirtualList::getCount(this->bgmapTextures);

	//if not, then allocate
	int32 segment = 
		BgmapTextureManager::doAllocate(this, id, (TextureSpec*)bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);

	if(0 > segment)
	{
		return NULL;
	}

	// create new texture and register it
	BgmapTexture bgmapTexture = new BgmapTexture(bgmapTextureSpec, id);
	BgmapTexture::setSegment(bgmapTexture, segment);
	BgmapTexture::setOffsets(bgmapTexture, BgmapTextureManager::getXOffset(this, id), BgmapTextureManager::getYOffset(this, id));

	VirtualList::pushBack(this->bgmapTextures, bgmapTexture);

	return bgmapTexture;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 BgmapTextureManager::doAllocate
(
	uint16 id, TextureSpec* textureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue
)
{
	int32 i = 0;
	int32 j = 0;
	int32 aux = 0;

	int32 cols = Texture::getTotalCols(textureSpec);
	int32 rows = Texture::getTotalRows(textureSpec);

	uint16 colsPad = (textureSpec->padding.cols << 1);// + (cols < 64 ? 1 : 0);
	uint16 rowsPad = (textureSpec->padding.rows << 1);// + (rows < 64 ? 1 : 0);

	int32 area = (cols + colsPad) * (rows + rowsPad);

	int16 segmentStep = 1;

	if(mustLiveAtEvenSegment)
	{
		switch(scValue)
		{
			case __WORLD_8x1:
			case __WORLD_4x2:
			case __WORLD_2x4:
			case __WORLD_1x8:

				segmentStep = 8;
				break;

			case __WORLD_4x1:
			case __WORLD_2x2:
			case __WORLD_1x4:

				segmentStep = 4;
				break;

			case __WORLD_2x1:
			case __WORLD_1x2:

				segmentStep = 2;
				break;
		}

		if(0 != minimumSegment % segmentStep)
		{
			minimumSegment += (segmentStep - minimumSegment % segmentStep);
		}

		NM_ASSERT(0 == minimumSegment % 2, "BgmapTextureManager::doAllocate: cannot honor request for even bgmap");
	}

	CACHE_RESET;

	for(i = minimumSegment; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS && i < this->availableBgmapSegmentsForTextures; i += segmentStep)
	{
		int32 maximumRow = i == this->printingBgmapSegment ? 64 - __SCREEN_HEIGHT_IN_CHARS : 64;
		
		// if there is space in the segment memory
		// there are 4096 chars in each bgmap segment
		if((int32)(4096 - this->usedTiles[i]) >= (int32)area )
		{
			// check if there is space within the segment
			// we check the next so don't go to the last element
			for(j = 0; j < __NUM_BGMAPS_PER_SEGMENT - 1; j++)
			{
				// determine the y offset inside the bgmap segment
				if(!this->yOffset[i][j + 1])
				{
					aux = maximumRow;
				}
				else
				{
					aux = this->yOffset[i][j + 1];
				}

				// determine if there is still mem space (columns) in the current y offset
				if(rows + rowsPad <= aux - this->yOffset[i][j] || (!this->yOffset[i][j + 1]))
				{
					if(rows + rowsPad <= maximumRow - this->yOffset[i][j])
					{
						if(cols + colsPad <= 64 - this->xOffset[i][j])
						{
							// register bgmap spec
							this->offset[id][kXOffset] = this->xOffset[i][j] + (colsPad >> 1);
							this->offset[id][kYOffset] = this->yOffset[i][j] + (rowsPad >> 1);
							this->offset[id][kCols] = cols;
							this->offset[id][kRows] = rows;

							NM_ASSERT
							(
								!mustLiveAtEvenSegment || 0 == (i % 2), 
								"BgmapTextureManager::doAllocate: cannot honor request for even bgmap"
							);

							// increment the x offset
							this->xOffset[i][j] += cols + colsPad;

							// if the number of rows of the bgmap spec is greater than the
							// next y offset defined, increase the next y offset
							if(this->yOffset[i][j + 1] - this->yOffset[i][j] < rows + rowsPad)
							{
								this->yOffset[i][j + 1] = this->yOffset[i][j] + rows + rowsPad;
							}
							
							// update the number of chars defined inside the bgmap segment
							this->usedTiles[i] += area;

							// if there is a free bgmap segment
							return i;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					if(rows + rowsPad > 64 - this->yOffset[i][j])
					{
						break;
					}
				}
			}
		}
	}

#ifdef __DEBUG
	Printing::setDebugMode(Printing::getInstance());
	Printing::clear(Printing::getInstance());
	BgmapTextureManager::print(this, 1, 10);
	Printing::text(Printing::getInstance(), "Texture ", 1, 19, NULL);
	Printing::text(Printing::getInstance(), "    Address: ", 1, 21, NULL);
	Printing::hex(Printing::getInstance(), (WORD)textureSpec, 14, 21, 8, NULL);

	// throw an exception if there is no enough space to allocate the bgmap spec
	Error::triggerException("BgmapTextureManager::doAllocate: bgmap segments depleted", NULL);		
#endif

	return -1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

