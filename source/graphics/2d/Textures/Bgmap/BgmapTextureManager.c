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

#include <BgmapTextureManager.h>
#include <HardwareManager.h>
#include <ParamTableManager.h>
#include <VIPManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S DECLARATIONS
//---------------------------------------------------------------------------------------------------------

friend class Texture;
friend class BgmapTexture;
friend class VirtualList;
friend class VirtualNode;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			BgmapTextureManager::getInstance()
 * @public
 * @return		BgmapTextureManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void BgmapTextureManager::constructor()
{
	Base::constructor();

	this->bgmapTextures = new VirtualList();
	BgmapTextureManager::reset(this);
}

/**
 * Class destructor
 */
void BgmapTextureManager::destructor()
{
	VirtualList::deleteData(this->bgmapTextures);
	delete this->bgmapTextures;
	this->bgmapTextures = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Reset manager's state
 */
void BgmapTextureManager::reset()
{
	NM_ASSERT(__BGMAP_SPACE_BASE_ADDRESS < __PARAM_TABLE_END, "BgmapTextureManager::reset: bgmap address space is negative");

	this->availableBgmapSegmentsForTextures = (uint32)((__PARAM_TABLE_END - __BGMAP_SPACE_BASE_ADDRESS) / __BGMAP_SEGMENT_SIZE);

	if(this->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}

	this->printingBgmapSegment = this->availableBgmapSegmentsForTextures - 1;

	int32 i = 0;

	// clear each bgmap segment usage
	for(; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS; i++)
	{
		this->numberOfChars[i] = 0;

		int32 j = 0;

		// clear the offsets
		for(j = 0; j <__NUM_BGMAPS_PER_SEGMENT; j++)
		{
			this->xOffset[i][j] = 0;
			this->yOffset[i][j] = 0;
		}
	}

	for(i = 0; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		this->offset[i][kXOffset] = -1;
		this->offset[i][kYOffset] = -1;
		this->offset[i][kCols] = 0;
		this->offset[i][kRows] = 0;
	}

	VirtualList::deleteData(this->bgmapTextures);

	this->deferTextureUpdate = false;
}

/**
 * Try to allocate a BGMAP memory space for a new Texture
 *
 * @private
 * @param bgmapTextureSpec		Texture to allocate space for
 * @param minimumSegment				Minimum bgmap segment to use
 * @param mustLiveAtEvenSegment			To force loading in an even bgmap segment
 * @param scValue					The increment applied when searching for the next free segment (used for MBgmapSprites)
 * @return 					True if the required space was successfully allocated
 */
int32 BgmapTextureManager::doAllocate(uint16 id, TextureSpec* textureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue)
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

	for(i = minimumSegment; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS && i < this->availableBgmapSegmentsForTextures; i += segmentStep)
	{
		int32 maximumRow = i == this->printingBgmapSegment ? 64 - __SCREEN_HEIGHT_IN_CHARS : 64;
		
		// if there is space in the segment memory
		// there are 4096 chars in each bgmap segment
		if((int32)(4096 - this->numberOfChars[i]) >= (int32)area )
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

							NM_ASSERT(!mustLiveAtEvenSegment || 0 == (i % 2), "BgmapTextureManager::doAllocate: cannot honor request for even bgmap");

							// increment the x offset
							this->xOffset[i][j] += cols + colsPad;

							// if the number of rows of the bgmap spec is greater than the
							// next y offset defined, increase the next y offset
							if(this->yOffset[i][j + 1] - this->yOffset[i][j] < rows + rowsPad)
							{
								this->yOffset[i][j + 1] = this->yOffset[i][j] + rows + rowsPad;
							}
							
							// update the number of chars defined inside the bgmap segment
							this->numberOfChars[i] += area;

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

#ifndef __SHIPPING
	Printing::setDebugMode(Printing::getInstance());
	Printing::clear(Printing::getInstance());
	BgmapTextureManager::print(this, 1, 10);
	Printing::text(Printing::getInstance(), "Texture ", 1, 19, NULL);
	Printing::text(Printing::getInstance(), "    Address: ", 1, 21, NULL);
	Printing::hex(Printing::getInstance(), (WORD)&textureSpec, 14, 21, 8, NULL);

	// throw an exception if there is no enough space to allocate the bgmap spec
	Error::triggerException("BgmapTextureManager::doAllocate: bgmap segments depleted", NULL);		
#endif

	return -1;
}

/**
 * Release a previously allocated Texture
 *
 * @param bgmapTexture		Texture to release
 */
void BgmapTextureManager::releaseTexture(BgmapTexture bgmapTexture)
{
	// if no one is using the texture anymore
	if(!isDeleted(bgmapTexture) && BgmapTexture::decreaseUsageCount(bgmapTexture))
	{
		switch(Texture::getAllocationType(bgmapTexture))
		{
			case __NO_ALLOCATION_TYPE:
			case __ANIMATED_SINGLE:
			case __ANIMATED_SINGLE_OPTIMIZED:
			case __ANIMATED_SHARED_COORDINATED:

				Texture::releaseCharSet(bgmapTexture);
				// Commented out because of the removal of bgmapTextureMaps array
				// VirtualList::removeElement(this->bgmapTextures, bgmapTexture);
				// delete bgmapTexture;
				break;

			case __ANIMATED_SHARED:
			case __ANIMATED_MULTI:
			case __NOT_ANIMATED:

				Texture::releaseCharSet(bgmapTexture);
				break;
		}
	}
}

/**
 * Update textures
 *
 * @public
 * @param value			Boolean flag
 */
void BgmapTextureManager::setDeferTextureUpdate(bool value)
{
	this->deferTextureUpdate = value;
}

/**
 * Update textures
 *
 * @public
 */
void BgmapTextureManager::updateTextures()
{
	for(VirtualNode node = this->bgmapTextures->head; NULL != node; node = node->next)
	{
		Texture texture = Texture::safeCast(node->data);

		if(kTextureWritten != texture->status && texture->update)
		{
			texture->update = !Texture::update(texture);

			if(this->deferTextureUpdate)
			{
				break;
			}
		}
	}
}

/**
 * Retrieve a previously allocated Texture
 *
 * @private
 * @param bgmapTextureSpec		Texture spec
 * @return								Allocated Texture
 */
BgmapTexture BgmapTextureManager::findTexture(BgmapTextureSpec* bgmapTextureSpec, bool recyclableOnly)
{
	TextureSpec* textureSpec = (TextureSpec*)bgmapTextureSpec;
	BgmapTexture selectedBgmapTexture = NULL;
	TextureSpec* selectedTextureSpec = NULL;

	// try to find a texture with the same bgmap spec
	for(VirtualNode node = this->bgmapTextures->head; NULL != node; node = node->next)
	{
		BgmapTexture allocatedBgmapTexture = BgmapTexture::safeCast(node->data);
		TextureSpec* allocatedTextureSpec = Texture::getTextureSpec(allocatedBgmapTexture);

		if(!recyclableOnly)
		{
			CharSet charSet = Texture::getCharSet(allocatedBgmapTexture, false);

			if(allocatedTextureSpec == textureSpec &&
				(NULL == charSet || allocatedTextureSpec->charSetSpec->allocationType == bgmapTextureSpec->charSetSpec->allocationType) &&
				(allocatedTextureSpec->padding.cols == bgmapTextureSpec->padding.cols && allocatedTextureSpec->padding.rows == bgmapTextureSpec->padding.rows)
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

		if(allocatedTextureSpec->recyclable && 0 == BgmapTexture::getUsageCount(allocatedBgmapTexture))
		{
			uint16 id = Texture::getId(allocatedBgmapTexture);
			uint16 cols = this->offset[id][kCols];
			uint16 rows = this->offset[id][kRows];


			if(textureSpec->cols <= cols && textureSpec->rows <= rows)
			{
				if(textureSpec->cols == cols && textureSpec->rows == rows)
				{
					selectedBgmapTexture = allocatedBgmapTexture;
					break;
				}
				else if(NULL == selectedBgmapTexture)
				{
					selectedBgmapTexture = allocatedBgmapTexture;
					selectedTextureSpec = allocatedTextureSpec;
				}
				else if(textureSpec->cols <= selectedTextureSpec->cols && textureSpec->rows <= selectedTextureSpec->rows)
				{
					if(textureSpec->cols * textureSpec->rows >= ((selectedTextureSpec->cols * selectedTextureSpec->rows) >> 1))
					{
						selectedBgmapTexture = allocatedBgmapTexture;
						selectedTextureSpec = allocatedTextureSpec;
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

/**
 * Allocate a BGMAP memory space for a new Texture
 *
 * @private
 * @param bgmapTextureSpec		Texture to allocate space for
 * @param minimumSegment				Minimum bgmap segment to use
 * @param mustLiveAtEvenSegment			To force loading in an even bgmap segment
 * @param scValue					The increment applied when searching for the next free segment (used for MBgmapSprites)
 * @return 								True if the required space was successfully allocated
 */
BgmapTexture BgmapTextureManager::allocateTexture(BgmapTextureSpec* bgmapTextureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue)
{
	uint16 id = VirtualList::getSize(this->bgmapTextures);

	//if not, then allocate
	int32 segment = BgmapTextureManager::doAllocate(this, id, (TextureSpec*)bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);

	if(0 > segment)
	{
		return NULL;
	}

	// create new texture and register it
	BgmapTexture bgmapTexture = new BgmapTexture(bgmapTextureSpec, id);
	BgmapTexture::setSegment(bgmapTexture, segment);

	VirtualList::pushBack(this->bgmapTextures, bgmapTexture);

	return bgmapTexture;
}

/**
 * Retrieve a Texture
 *
 * @private
 * @param bgmapTextureSpec		Texture spec to find o allocate a Texture
 * @param minimumSegment				Minimum bgmap segment to use
 * @param mustLiveAtEvenSegment			To force loading in an even bgmap segment
 * @param scValue					The increment applied when searching for the next free segment (used for MBgmapSprites)
 * @return 								Allocated Texture
 */
BgmapTexture BgmapTextureManager::getTexture(BgmapTextureSpec* bgmapTextureSpec, int16 minimumSegment, bool mustLiveAtEvenSegment, uint32 scValue)
{
	if(NULL == bgmapTextureSpec)
	{
		return NULL;
	}

	BgmapTexture bgmapTexture = NULL;

	if(NULL == bgmapTextureSpec->charSetSpec)
	{
		bgmapTexture = BgmapTextureManager::allocateTexture(this, bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);

		ASSERT(bgmapTexture, "BgmapTextureManager::getTexture: (animated) texture no allocated");
	}
	else
	{
		//determine the allocation type
		switch(bgmapTextureSpec->charSetSpec->allocationType)
		{
			case __ANIMATED_SINGLE:
			case __ANIMATED_SINGLE_OPTIMIZED:
			case __ANIMATED_SHARED_COORDINATED:

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
					bgmapTexture = BgmapTextureManager::allocateTexture(this, bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);
				}

				ASSERT(bgmapTexture, "BgmapTextureManager::getTexture: (animated) texture no allocated");
				break;

			case __ANIMATED_SHARED:
			case __ANIMATED_MULTI:
			case __NOT_ANIMATED:

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
					bgmapTexture = BgmapTextureManager::allocateTexture(this, bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment, scValue);
				}

				ASSERT(bgmapTexture, "BgmapTextureManager::getTexture: (shared) texture no allocated");
				break;

			default:

				NM_ASSERT(false, "BgmapTextureManager::getTexture: not valid allocation type");
				break;
		}
	}

	return bgmapTexture;
}

/**
 * Retrieve the x offset within a BGMAP segment of the Texture with the given id
 *
 * @private
 * @param id		Texture identification
 * @return 			X offset within a BGMAP segment
 */
int16 BgmapTextureManager::getXOffset(int32 id)
{
	return this->offset[id][kXOffset];
}

/**
 * Retrieve the y offset within a BGMAP segment of the Texture with the given id
 *
 * @private
 * @param id		Texture identification
 * @return 			Y offset within a BGMAP segment
 */
int16 BgmapTextureManager::getYOffset(int32 id)
{
	return this->offset[id][kYOffset];
}

/**
 * Retrieve the number of non used BGMAP segments for texture allocation
 *
 * @private
 * @return 			Number of non used BGMAP segments for texture allocation
 */
int16 BgmapTextureManager::getAvailableBgmapSegmentsForTextures()
{
	return this->availableBgmapSegmentsForTextures;
}

/**
 * Retrieve the BGMAP segment available for printing
 *
 * @private
 * @return 			BGMAP segment available for printing
 */
int16 BgmapTextureManager::getPrintingBgmapSegment()
{
	return this->printingBgmapSegment;
}

/**
 * Compute the available BGMAP segments based on texture usage
 *
 * @private
 */
void BgmapTextureManager::calculateAvailableBgmapSegments()
{
	uint32 paramTableBase = ParamTableManager::getParamTableBase(ParamTableManager::getInstance());

	this->availableBgmapSegmentsForTextures = (uint32)((paramTableBase - __BGMAP_SPACE_BASE_ADDRESS) / __BGMAP_SEGMENT_SIZE);

	if(this->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}

	this->printingBgmapSegment = this->availableBgmapSegmentsForTextures - 1;
}

/**
 * Print manager's status
 *
 * @private
 * @param x			Camera's x coocrinate
 * @param y			Camera's y coocrinate
 */
void BgmapTextureManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "BGMAP TEXTURES USAGE", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Segments for textures: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), BgmapTextureManager::getAvailableBgmapSegmentsForTextures(this), x + 23, y, NULL);
	Printing::text(Printing::getInstance(), "Printing segment: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(this), x + 23, y, NULL);
	Printing::text(Printing::getInstance(), "Textures count: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->bgmapTextures), x + 23, y, NULL);

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

		TextureSpec* allocatedTextureSpec = Texture::getTextureSpec(bgmapTexture);

		if(allocatedTextureSpec->recyclable)
		{
			recyclableTextures++;
			freeEntries += !BgmapTexture::getUsageCount(bgmapTexture)? 1 : 0;

//				Printing::text(Printing::getInstance(), BgmapTexture::getUsageCount(bgmapTexture) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + j + 1, y + i, NULL);
			Printing::hex(Printing::getInstance(), (int32)Texture::getTextureSpec(bgmapTexture), x + j, y + i, 8, NULL);
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
