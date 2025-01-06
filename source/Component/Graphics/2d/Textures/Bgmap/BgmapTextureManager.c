/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with bgmapTextureManager source code.
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

static void BgmapTextureManager::reset()
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	NM_ASSERT(__BGMAP_SPACE_BASE_ADDRESS < __PARAM_TABLE_END, "BgmapTextureManager::reset: bgmap address space is negative");

	VirtualList::deleteData(bgmapTextureManager->bgmapTextures);

	bgmapTextureManager->availableBgmapSegmentsForTextures = (uint32)((__PARAM_TABLE_END - __BGMAP_SPACE_BASE_ADDRESS) / __BGMAP_SEGMENT_SIZE);

	if(bgmapTextureManager->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		bgmapTextureManager->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}

	bgmapTextureManager->printingBgmapSegment = bgmapTextureManager->availableBgmapSegmentsForTextures - 1;

	// Clear each bgmap segment usage
	for(int32 i = 0; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS; i++)
	{
		bgmapTextureManager->usedTiles[i] = 0;

		// Clear the offsets
		for(int32 j = 0; j <__NUM_BGMAPS_PER_SEGMENT; j++)
		{
			bgmapTextureManager->xOffset[i][j] = 0;
			bgmapTextureManager->yOffset[i][j] = 0;
		}
	}

	for(int32 i = 0; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		bgmapTextureManager->offset[i][kXOffset] = -1;
		bgmapTextureManager->offset[i][kYOffset] = -1;
		bgmapTextureManager->offset[i][kCols] = 0;
		bgmapTextureManager->offset[i][kRows] = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void BgmapTextureManager::clearBgmapSegment(int32 segment)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	Mem::clear((BYTE*)__BGMAP_SEGMENT(segment), 64 * 64 * 2);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void BgmapTextureManager::calculateAvailableBgmapSegments()
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	uint32 paramTableBase = ParamTableManager::getParamTableBase(ParamTableManager::getInstance());

	bgmapTextureManager->availableBgmapSegmentsForTextures = (uint32)((paramTableBase - __BGMAP_SPACE_BASE_ADDRESS) / __BGMAP_SEGMENT_SIZE);

	if(bgmapTextureManager->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		bgmapTextureManager->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}

	bgmapTextureManager->printingBgmapSegment = bgmapTextureManager->availableBgmapSegmentsForTextures - 1;

	Printing::setPrintingBgmapSegment(bgmapTextureManager->printingBgmapSegment);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int8 BgmapTextureManager::getAvailableBgmapSegmentsForTextures()
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	return bgmapTextureManager->availableBgmapSegmentsForTextures;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int8 BgmapTextureManager::getPrintingBgmapSegment()
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	return bgmapTextureManager->printingBgmapSegment;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void BgmapTextureManager::loadTextures(const TextureSpec** textureSpecs)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	// Textures
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
		// The recyclable ones are not recycled immediately
		for(VirtualNode node = sortedTextureSpecs->head; NULL != node; node = node->next)
		{
			TextureSpec* textureSpec = (TextureSpec*)node->data;

			BgmapTexture bgmapTexture = BgmapTextureManager::getTexture(textureSpec, 0, false, __WORLD_1x1);

			NM_ASSERT(!isDeleted(bgmapTexture), "BgmapTextureManager::loadTextures: failed to load bgmapTexture");
			VirtualList::pushBack(preloadedTextures, bgmapTexture);
		}

		delete sortedTextureSpecs;

		for(VirtualNode node = VirtualList::begin(preloadedTextures); NULL != node; node = node->next)
		{
			BgmapTextureManager::releaseTexture(BgmapTexture::safeCast(node->data));
		}

		delete preloadedTextures;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static BgmapTexture BgmapTextureManager::getTexture
(
	BgmapTextureSpec* bgmapTextureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue
)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	if(NULL == bgmapTextureSpec)
	{
		NM_ASSERT(false, "BgmapTextureManager::getTexture: NULL spec provided");
		return NULL;
	}

	BgmapTexture bgmapTexture = NULL;

	if(NULL == bgmapTextureSpec->charSetSpec)
	{
		bgmapTexture = BgmapTextureManager::allocateTexture(bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);

		ASSERT(!isDeleted(bgmapTexture), "BgmapTextureManager::getTexture: (animated) texture no allocated");
	}
	else
	{
		if(!bgmapTextureSpec->charSetSpec->shared)
		{
			if(bgmapTextureSpec->recyclable)
			{
				bgmapTexture = BgmapTextureManager::findTexture(bgmapTextureSpec, true);
			}
			
			if(NULL != bgmapTexture)
			{
				BgmapTexture::increaseUsageCount(bgmapTexture);
			}
			else
			{
				// Load a new texture
				bgmapTexture = 
					BgmapTextureManager::allocateTexture(bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);
			}

			ASSERT(bgmapTexture, "BgmapTextureManager::getTexture: (animated) texture no allocated");
		}
		else
		{
			// First try to find an already created texture
			bgmapTexture = BgmapTextureManager::findTexture(bgmapTextureSpec, false);

			// If couldn't find the texture
			if(NULL != bgmapTexture)
			{
				BgmapTexture::increaseUsageCount(bgmapTexture);
			}
			else
			{
				// Load it
				bgmapTexture = 
					BgmapTextureManager::allocateTexture(bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);
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

static void BgmapTextureManager::releaseTexture(BgmapTexture bgmapTexture)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	// If no one is using the texture anymore
	if(!isDeleted(bgmapTexture))
	{
		BgmapTexture::decreaseUsageCount(bgmapTexture);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 BgmapTextureManager::getXOffset(int32 id)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	return bgmapTextureManager->offset[id][kXOffset];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int16 BgmapTextureManager::getYOffset(int32 id)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	return bgmapTextureManager->offset[id][kYOffset];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void BgmapTextureManager::print(int32 x, int32 y)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	Printing::text("BGMAP TEXTURES USAGE", x, y++, NULL);
	Printing::text("Segments for textures: ", x, ++y, NULL);
	Printing::int32(BgmapTextureManager::getAvailableBgmapSegmentsForTextures(bgmapTextureManager), x + 23, y, NULL);
	Printing::text("Printing segment: ", x, ++y, NULL);
	Printing::int32(BgmapTextureManager::getPrintingBgmapSegment(bgmapTextureManager), x + 23, y, NULL);
	Printing::text("Textures count: ", x, ++y, NULL);
	Printing::int32(VirtualList::getCount(bgmapTextureManager->bgmapTextures), x + 23, y, NULL);

	y++;
	y++;
	Printing::text("Recyclable textures", x, y++, NULL);
	y++;
	Printing::text("Total: ", x, y++, NULL);
	Printing::text("Free: ", x, y++, NULL);

	y++;
	Printing::text("ROM", x, y++, NULL);
	y++;
	Printing::text("Address   Refs", x, y++, NULL);
	Printing::text("\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", x, y++, NULL);

	int32 i = 0;
	int32 j = 0;
	int32 recyclableTextures = 0;
	int32 freeEntries = 0;

	// Try to find a texture with the same bgmap spec
	for(VirtualNode node = bgmapTextureManager->bgmapTextures->head; NULL != node; node = node->next)
	{
		BgmapTexture bgmapTexture = BgmapTexture::safeCast(node->data);

		TextureSpec* allocatedTextureSpec = Texture::getSpec(bgmapTexture);

		if(allocatedTextureSpec->recyclable)
		{
			recyclableTextures++;
			freeEntries += !BgmapTexture::getUsageCount(bgmapTexture)? 1 : 0;

			Printing::hex((int32)Texture::getSpec(bgmapTexture), x + j, y + i, 8, NULL);
			Printing::int32(BgmapTexture::getUsageCount(bgmapTexture), x + j + 9, y + i, NULL);

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

	Printing::int32(recyclableTextures, x + 7, y - 7, NULL);
	Printing::int32(freeEntries, x + 7, y - 6, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static BgmapTexture BgmapTextureManager::findTexture(BgmapTextureSpec* bgmapTextureSpec, bool recyclableOnly)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	TextureSpec* textureSpec = (TextureSpec*)bgmapTextureSpec;
	BgmapTexture selectedBgmapTexture = NULL;

	CACHE_RESET;

	// Try to find a texture with the same bgmap spec
	for(VirtualNode node = bgmapTextureManager->bgmapTextures->head; NULL != node; node = node->next)
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
				// Return if found
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
			uint16 cols = bgmapTextureManager->offset[id][kCols];
			uint16 rows = bgmapTextureManager->offset[id][kRows];

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
					uint16 selectedBgmapTextureCols = bgmapTextureManager->offset[selectedBgmapTextureId][kCols];
					uint16 selectedBgmapTextureRows = bgmapTextureManager->offset[selectedBgmapTextureId][kRows];

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

static BgmapTexture BgmapTextureManager::allocateTexture
(
	BgmapTextureSpec* bgmapTextureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue
)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

	uint16 id = VirtualList::getCount(bgmapTextureManager->bgmapTextures);

	//if not, then allocate
	int32 segment = 
		BgmapTextureManager::doAllocate(id, (TextureSpec*)bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);

	if(0 > segment)
	{
		return NULL;
	}

	// Create new texture and register it
	BgmapTexture bgmapTexture = new BgmapTexture(bgmapTextureSpec, id);
	BgmapTexture::setSegment(bgmapTexture, segment);
	BgmapTexture::setOffsets(bgmapTexture, BgmapTextureManager::getXOffset(id), BgmapTextureManager::getYOffset(id));

	VirtualList::pushBack(bgmapTextureManager->bgmapTextures, bgmapTexture);

	return bgmapTexture;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int32 BgmapTextureManager::doAllocate
(
	uint16 id, TextureSpec* textureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue
)
{
	BgmapTextureManager bgmapTextureManager = BgmapTextureManager::getInstance();

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

	for(i = minimumSegment; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS && i < bgmapTextureManager->availableBgmapSegmentsForTextures; i += segmentStep)
	{
		int32 maximumRow = i == bgmapTextureManager->printingBgmapSegment ? 64 - __SCREEN_HEIGHT_IN_CHARS : 64;
		
		// If there is space in the segment memory
		// There are 4096 chars in each bgmap segment
		if((int32)(4096 - bgmapTextureManager->usedTiles[i]) >= (int32)area )
		{
			// Check if there is space within the segment
			// We check the next so don't go to the last element
			for(j = 0; j < __NUM_BGMAPS_PER_SEGMENT - 1; j++)
			{
				// Determine the y offset inside the bgmap segment
				if(!bgmapTextureManager->yOffset[i][j + 1])
				{
					aux = maximumRow;
				}
				else
				{
					aux = bgmapTextureManager->yOffset[i][j + 1];
				}

				// Determine if there is still mem space (columns) in the current y offset
				if(rows + rowsPad <= aux - bgmapTextureManager->yOffset[i][j] || (!bgmapTextureManager->yOffset[i][j + 1]))
				{
					if(rows + rowsPad <= maximumRow - bgmapTextureManager->yOffset[i][j])
					{
						if(cols + colsPad <= 64 - bgmapTextureManager->xOffset[i][j])
						{
							// Register bgmap spec
							bgmapTextureManager->offset[id][kXOffset] = bgmapTextureManager->xOffset[i][j] + (colsPad >> 1);
							bgmapTextureManager->offset[id][kYOffset] = bgmapTextureManager->yOffset[i][j] + (rowsPad >> 1);
							bgmapTextureManager->offset[id][kCols] = cols;
							bgmapTextureManager->offset[id][kRows] = rows;

							NM_ASSERT
							(
								!mustLiveAtEvenSegment || 0 == (i % 2), 
								"BgmapTextureManager::doAllocate: cannot honor request for even bgmap"
							);

							// Increment the x offset
							bgmapTextureManager->xOffset[i][j] += cols + colsPad;

							// If the number of rows of the bgmap spec is greater than the
							// Next y offset defined, increase the next y offset
							if(bgmapTextureManager->yOffset[i][j + 1] - bgmapTextureManager->yOffset[i][j] < rows + rowsPad)
							{
								bgmapTextureManager->yOffset[i][j + 1] = bgmapTextureManager->yOffset[i][j] + rows + rowsPad;
							}
							
							// Update the number of chars defined inside the bgmap segment
							bgmapTextureManager->usedTiles[i] += area;

							// If there is a free bgmap segment
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
					if(rows + rowsPad > 64 - bgmapTextureManager->yOffset[i][j])
					{
						break;
					}
				}
			}
		}
	}

#ifdef __DEBUG
	Printing::setDebugMode();
	Printing::clear();
	BgmapTextureManager::print(1, 10);
	Printing::text("Texture ", 1, 19, NULL);
	Printing::text("    Address: ", 1, 21, NULL);
	Printing::hex((WORD)textureSpec, 14, 21, 8, NULL);

	// Throw an exception if there is no enough space to allocate the bgmap spec
	Error::triggerException("BgmapTextureManager::doAllocate: bgmap segments depleted", NULL);		
#endif

	return -1;
}

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
	BgmapTextureManager::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BgmapTextureManager::destructor()
{
	VirtualList::deleteData(this->bgmapTextures);
	delete this->bgmapTextures;
	this->bgmapTextures = NULL;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
