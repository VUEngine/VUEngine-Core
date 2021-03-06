/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

#include <BgmapTextureManager.h>
#include <HardwareManager.h>
#include <ParamTableManager.h>
#include <VIPManager.h>
#include <VirtualList.h>


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

	BgmapTextureManager::reset(this);
}

/**
 * Class destructor
 */
void BgmapTextureManager::destructor()
{
	// allow a new construct
	Base::destructor();
}

/**
 * Reset manager's state
 */
void BgmapTextureManager::reset()
{
	NM_ASSERT(__BGMAP_SPACE_BASE_ADDRESS < __PARAM_TABLE_END, "BgmapTextureManager::reset: bgmap address space is negative");

	this->availableBgmapSegmentsForTextures = (u32)((__PARAM_TABLE_END - __BGMAP_SPACE_BASE_ADDRESS) / __BGMAP_SEGMENT_SIZE);

	if(this->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}

	this->printingBgmapSegment = this->availableBgmapSegmentsForTextures - 1;

	int i = 0;

	// clear each bgmap segment usage
	for(; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS; i++)
	{
		this->numberOfChars[i] = 0;

		int j = 0;

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

		if(this->bgmapTextures[i])
		{
			delete this->bgmapTextures[i];
		}

		this->bgmapTextures[i] = NULL;
	}
}

/**
 * Try to allocate a BGMAP memory space for a new Texture
 *
 * @private
 * @param bgmapTexture		Texture to allocate space for
 * @param minimumSegment				Minimum bgmap segment to use
 * @param mustLiveAtEvenSegment			To force loading in an even bgmap segment
 * @return 					True if the required space was successfully allocated
 */
int BgmapTextureManager::doAllocate(BgmapTexture bgmapTexture, s16 minimumSegment, bool mustLiveAtEvenSegment)
{
	int i = 0;
	int j = 0;
	int aux = 0;

	int cols = Texture::getTotalCols(bgmapTexture);
	int rows = Texture::getTotalRows(bgmapTexture);

	TextureSpec* textureSpec = Texture::getTextureSpec(bgmapTexture);

	u16 colsPad = (textureSpec->padding.cols << 1);// + (cols < 64 ? 1 : 0);
	u16 rowsPad = (textureSpec->padding.rows << 1);// + (rows < 64 ? 1 : 0);

	int area = (cols + colsPad) * (rows + rowsPad);

	if(mustLiveAtEvenSegment)
	{
		if(0 != minimumSegment % 2)
		{
			minimumSegment++;
		}

		NM_ASSERT(0 == minimumSegment % 2, "BgmapTextureManager::doAllocate: cannot honor request for even bgmap");
	}

	// if texture already defined, don't allocate
	if(Texture::getNumberOfChars(bgmapTexture))
	{
		for(i = minimumSegment; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS && i < this->availableBgmapSegmentsForTextures; i += mustLiveAtEvenSegment ? 2 : 1)
		{
			int maximumRow = i == this->printingBgmapSegment ? 64 - __SCREEN_HEIGHT_IN_CHARS : 64;
			
			// if there is space in the segment memory
			// there are 4096 chars in each bgmap segment
			if((int)(4096 - this->numberOfChars[i]) >= (int)area )
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
								u16 id = Texture::getId(bgmapTexture);

								// register bgmap spec
								this->offset[id][kXOffset] = this->xOffset[i][j] + (colsPad >> 1);
								this->offset[id][kYOffset] = this->yOffset[i][j] + (rowsPad >> 1);
								this->offset[id][kCols] = cols;
								this->offset[id][kRows] = rows;

								NM_ASSERT(!mustLiveAtEvenSegment || 0 == (i % 2), "BgmapTextureManager::doAllocate: cannot honor request for even bgmap");
	
								BgmapTexture::setSegment(bgmapTexture, i);

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
								return true;
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

		// throw an exception if there is no enough space to allocate the bgmap spec
		NM_ASSERT(false, "BgmapTextureManager::doAllocate: bgmap segments depleted");
	}

	// through exception if texture has 0 chars
	ASSERT(false, "BgmapTextureManager::doAllocate: map has 0 chars");

	return false;
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
		int i = Texture::getId(bgmapTexture);

		TextureSpec* textureSpec = Texture::getTextureSpec(bgmapTexture);

		switch(textureSpec->charSetSpec->allocationType)
		{
			case __ANIMATED_SINGLE:
			case __ANIMATED_SINGLE_OPTIMIZED:
			case __ANIMATED_SHARED_COORDINATED:

				delete bgmapTexture;
				this->bgmapTextures[i] = NULL;
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
 * Retrieve a previously allocated Texture
 *
 * @private
 * @param bgmapTextureSpec		Texture spec
 * @return								Allocated Texture
 */
BgmapTexture BgmapTextureManager::findTexture(BgmapTextureSpec* bgmapTextureSpec)
{
	int i = 0;
	TextureSpec* textureSpec = (TextureSpec*)bgmapTextureSpec;

	// try to find a texture with the same bgmap spec
	for(; i < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if(this->bgmapTextures[i])
		{
			CharSet charSet = Texture::getCharSet(this->bgmapTextures[i], false);
			TextureSpec* allocatedTextureSpec = Texture::getTextureSpec(this->bgmapTextures[i]);

			if(allocatedTextureSpec == (TextureSpec*)textureSpec &&
				(!charSet || allocatedTextureSpec->charSetSpec->allocationType == bgmapTextureSpec->charSetSpec->allocationType) &&
				(allocatedTextureSpec->padding.cols == bgmapTextureSpec->padding.cols && allocatedTextureSpec->padding.rows == bgmapTextureSpec->padding.rows)
			)
			{
				// return if found
				return this->bgmapTextures[i];
			}
		}
	}

	if(!textureSpec->recyclable)
	{
		return NULL;
	}

	BgmapTexture selectedBgmapTexture = NULL;
	TextureSpec* selectedTextureSpec = NULL;

	// try to find a texture with the same bgmap spec
	for(i = 0; i < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		BgmapTexture bgmapTexture = this->bgmapTextures[i];

		if(bgmapTexture && 0 == BgmapTexture::getUsageCount(bgmapTexture))
		{
			u16 id = Texture::getId(bgmapTexture);
			u16 cols = this->offset[id][kCols];
			u16 rows = this->offset[id][kRows];

			TextureSpec* allocatedTextureSpec = Texture::getTextureSpec(bgmapTexture);

			if(allocatedTextureSpec->recyclable &&
				textureSpec->cols <= cols &&
				textureSpec->rows <= rows
			)
			{
				if(textureSpec->cols == cols && textureSpec->rows == rows)
				{
					selectedBgmapTexture = bgmapTexture;
					break;
				}
				else if(!selectedBgmapTexture)
				{
					selectedBgmapTexture = bgmapTexture;
					selectedTextureSpec = allocatedTextureSpec;
				}
				else if(cols <= selectedTextureSpec->cols && rows <= selectedTextureSpec->rows)
				{
					selectedBgmapTexture = bgmapTexture;
					selectedTextureSpec = allocatedTextureSpec;
				}
			}
		}
	}

	if(selectedBgmapTexture)
	{
		Texture::setSpec(selectedBgmapTexture, textureSpec);
		Texture::setPalette(selectedBgmapTexture, textureSpec->palette);
		Texture::rewrite(selectedBgmapTexture);
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
 * @return 								True if the required space was successfully allocated
 */
BgmapTexture BgmapTextureManager::allocateTexture(BgmapTextureSpec* bgmapTextureSpec, s16 minimumSegment, bool mustLiveAtEvenSegment)
{
	int i = 0;

	// find an empty slot
	for(; i < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if(!this->bgmapTextures[i])
		{
			// create new texture and register it
			this->bgmapTextures[i] = new BgmapTexture(bgmapTextureSpec, i);

			//if not, then allocate
			BgmapTextureManager::doAllocate(this, this->bgmapTextures[i], minimumSegment, mustLiveAtEvenSegment);

			return this->bgmapTextures[i];
		}
	}

	return NULL;
}

/**
 * Retrieve a Texture
 *
 * @private
 * @param bgmapTextureSpec		Texture spec to find o allocate a Texture
 * @param minimumSegment				Minimum bgmap segment to use
 * @param mustLiveAtEvenSegment			To force loading in an even bgmap segment
 * @return 								Allocated Texture
 */
BgmapTexture BgmapTextureManager::getTexture(BgmapTextureSpec* bgmapTextureSpec, s16 minimumSegment, bool mustLiveAtEvenSegment)
{
	BgmapTexture bgmapTexture = NULL;

	//determine the allocation type
	switch(bgmapTextureSpec->charSetSpec->allocationType)
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SINGLE_OPTIMIZED:
		case __ANIMATED_SHARED_COORDINATED:

			// load a new texture
			bgmapTexture = BgmapTextureManager::allocateTexture(this, bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment);

			ASSERT(bgmapTexture, "BgmapTextureManager::getTexture: (animated) texture no allocated");
			break;

		case __ANIMATED_SHARED:
		case __ANIMATED_MULTI:
		case __NOT_ANIMATED:

			// first try to find an already created texture
			bgmapTexture = BgmapTextureManager::findTexture(this, bgmapTextureSpec);

			// if couldn't find the texture
			if(bgmapTexture)
			{
				BgmapTexture::increaseUsageCount(bgmapTexture);
			}
			else
			{
				// load it
				bgmapTexture = BgmapTextureManager::allocateTexture(this, bgmapTextureSpec, minimumSegment, mustLiveAtEvenSegment);
			}

			ASSERT(bgmapTexture, "BgmapTextureManager::getTexture: (shared) texture no allocated");
			break;

		default:

			NM_ASSERT(false, "BgmapTextureManager::getTexture: not valid allocation type");
			break;
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
s16 BgmapTextureManager::getXOffset(int id)
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
s16 BgmapTextureManager::getYOffset(int id)
{
	return this->offset[id][kYOffset];
}

/**
 * Retrieve the number of non used BGMAP segments for texture allocation
 *
 * @private
 * @return 			Number of non used BGMAP segments for texture allocation
 */
s16 BgmapTextureManager::getAvailableBgmapSegmentsForTextures()
{
	return this->availableBgmapSegmentsForTextures;
}

/**
 * Retrieve the BGMAP segment available for printing
 *
 * @private
 * @return 			BGMAP segment available for printing
 */
s16 BgmapTextureManager::getPrintingBgmapSegment()
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
	u32 paramTableBase = ParamTableManager::getParamTableBase(ParamTableManager::getInstance());

	this->printingBgmapSegment = (u32)((paramTableBase - __BGMAP_SPACE_BASE_ADDRESS - (__PRINTABLE_BGMAP_AREA << 1)) / __BGMAP_SEGMENT_SIZE);

	this->availableBgmapSegmentsForTextures = this->printingBgmapSegment + 1;

	if(this->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}
}

/**
 * Print manager's status
 *
 * @private
 * @param x			Camera's x coocrinate
 * @param y			Camera's y coocrinate
 */
void BgmapTextureManager::print(int x, int y)
{
	int index = 0;
	int textureCount = 0;
	for(;index < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; index++)
	{
		if(this->bgmapTextures[index])
		{
			textureCount++;
		}
	}

	Printing::text(Printing::getInstance(), "BGMAP TEXTURES USAGE", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Segments for textures: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), BgmapTextureManager::getAvailableBgmapSegmentsForTextures(this), x + 23, y, NULL);
	Printing::text(Printing::getInstance(), "Textures count: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), textureCount, x + 23, y, NULL);
	Printing::text(Printing::getInstance(), "Printing segment: ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(this), x + 23, y, NULL);

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

	int i = 0;
	int j = 0;
	int recyclableTextures = 0;
	int freeEntries = 0;

	// try to find a texture with the same bgmap spec
	for(index = 0; index < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; index++)
	{
		BgmapTexture bgmapTexture = this->bgmapTextures[index];

		if(bgmapTexture)
		{
			TextureSpec* allocatedTextureSpec = Texture::getTextureSpec(bgmapTexture);

			if(allocatedTextureSpec->recyclable)
			{
				recyclableTextures++;
				freeEntries += !BgmapTexture::getUsageCount(bgmapTexture)? 1 : 0;

//				Printing::text(Printing::getInstance(), BgmapTexture::getUsageCount(bgmapTexture) ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + j + 1, y + i, NULL);
				Printing::hex(Printing::getInstance(), (int)Texture::getTextureSpec(bgmapTexture), x + j, y + i, 8, NULL);
				Printing::int(Printing::getInstance(), BgmapTexture::getUsageCount(bgmapTexture), x + j + 10, y + i, NULL);

				if(++i + y > __SCREEN_HEIGHT / 8)
				{
					i = 0;
					j += 14;

					if(j + x > __SCREEN_WIDTH / 8)
					{
						i = 0;
						j = 0;
					}
				}
			}
		}
	}

	Printing::int(Printing::getInstance(), recyclableTextures, x + 7, y - 7, NULL);
	Printing::int(Printing::getInstance(), freeEntries, x + 7, y - 6, NULL);
}
