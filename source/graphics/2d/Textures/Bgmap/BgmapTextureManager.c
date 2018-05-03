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

#include <BgmapTextureManager.h>
#include <HardwareManager.h>
#include <ParamTableManager.h>
#include <VIPManager.h>
#include <VirtualList.h>

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define BgmapTextureManager_ATTRIBUTES																	\
		Object_ATTRIBUTES																				\
		/**
		 * @var u16 			numberOfChars
		 * @brief				Number of chars occupied
		 * @memberof			BgmapTextureManager
		 */																								\
		u16 numberOfChars[__MAX_NUMBER_OF_BGMAPS_SEGMENTS];												\
		/**
		 * @var s8 				xOffset
		 * @brief				Current x offset to set the next bgmap
		 * @memberof			BgmapTextureManager
		 */																								\
		s8 xOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];							\
		/**
		 * @var s8 				yOffset
		 * @brief				Current y offset to set the next bgmap
		 * @memberof			BgmapTextureManager
		 */																								\
		s8 yOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];							\
		/**
		 * @var s8 				offset
		 * @brief				12 segments, 28 maps, 2 indexes (x,y) and bgmap segment
		 * @memberof			BgmapTextureManager
		 */																								\
		s8 offset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT][4];						\
		/**
		 * @var s16 			freeBgmapSegment
		 * @brief				Next free bgmap used for text printing
		 * @memberof			BgmapTextureManager
		 */																								\
		s16 freeBgmapSegment;																			\
		/**
		 * @var BgmapTexture 	bgmapTextures
		 * @brief				The textures allocated
		 * @memberof			BgmapTextureManager
		 */																								\
		BgmapTexture bgmapTextures[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT];			\
		/**
		 * @var s16 			availableBgmapSegmentsForTextures
		 * @brief				Number of available bgmap segments
		 * @memberof			BgmapTextureManager
		 */																								\
		s16 availableBgmapSegmentsForTextures;															\
		/**
		 * @var s16 			printingBgmapSegment
		 * @brief				Segment for printing
		 * @memberof			BgmapTextureManager
		 */																								\
		s16 printingBgmapSegment;																		\

/**
 * @class 	BgmapTextureManager
 * @extends Object
 * @ingroup graphics-2d-textures-bgmap
 */
__CLASS_DEFINITION(BgmapTextureManager, Object);

enum OffsetIndex
{
	kXOffset = 0,
	kYOffset,
	kCols,
	kRows,
	kBgmapSegment
};


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void BgmapTextureManager_constructor(BgmapTextureManager this);
static int BgmapTextureManager_doAllocate(BgmapTextureManager this, BgmapTexture bgmapTexture);
static BgmapTexture BgmapTextureManager_findTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition);
static BgmapTexture BgmapTextureManager_allocateTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			BgmapTextureManager_getInstance()
 * @memberof	BgmapTextureManager
 * @public
 *
 * @return		BgmapTextureManager instance
 */
__SINGLETON(BgmapTextureManager);

/**
 * Class constructor
 *
 * @memberof			BgmapTextureManager
 * @private
 *
 * @param this			Function scope
 */
static void __attribute__ ((noinline)) BgmapTextureManager_constructor(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	BgmapTextureManager_reset(this);
}

/**
 * Class destructor
 *
 * @memberof			BgmapTextureManager
 * @public
 *
 * @param this			Function scope
 */
void BgmapTextureManager_destructor(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Reset manager's state
 *
 * @memberof			BgmapTextureManager
 * @public
 *
 * @param this			Function scope
 */
void BgmapTextureManager_reset(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::reset: null this");
	NM_ASSERT(__BGMAP_SPACE_BASE_ADDRESS < __PARAM_TABLE_END, "BgmapTextureManager::reset: bgmap address space is negative");

	this->availableBgmapSegmentsForTextures = (u32)((__PARAM_TABLE_END - __BGMAP_SPACE_BASE_ADDRESS) / __BGMAP_SEGMENT_SIZE);

	if(this->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}

	this->printingBgmapSegment = this->availableBgmapSegmentsForTextures - 1;

	int i = 0;
	int j = 0;

	// clear each bgmap segment usage
	for(; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS; i++)
	{
		this->numberOfChars[i] = 0;

		// clear the offsets
		for(j = 0;j<__NUM_BGMAPS_PER_SEGMENT; j++)
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
			__DELETE(this->bgmapTextures[i]);
		}

		this->bgmapTextures[i] = NULL;
	}

	this->freeBgmapSegment = 0;
}

/**
 * Try to allocate a BGMAP memory space for a new Texture
 *
 * @memberof				BgmapTextureManager
 * @private
 *
 * @param this				Function scope
 * @param bgmapTexture		Texture to allocate space for
 *
 * @return 					True if the required space was successfully allocated
 */
static int BgmapTextureManager_doAllocate(BgmapTextureManager this, BgmapTexture bgmapTexture)
{
	ASSERT(this, "BgmapTextureManager::doAllocate: null this");

	int i = 0;
	int j = 0;
	int aux = 0;

	int cols = Texture_getTotalCols(__SAFE_CAST(Texture, bgmapTexture));
	int rows = Texture_getTotalRows(__SAFE_CAST(Texture, bgmapTexture));

	TextureDefinition* textureDefinition = Texture_getTextureDefinition(__SAFE_CAST(Texture, bgmapTexture));

	u16 colsPad = textureDefinition->padding.cols << 1;
	u16 rowsPad = textureDefinition->padding.rows << 1;

	int area = (cols + colsPad) * (rows + rowsPad);

	// if texture already defined, don't allocate
	if(Texture_getNumberOfChars(__SAFE_CAST(Texture, bgmapTexture)))
	{
		for(i = 0; i < __MAX_NUMBER_OF_BGMAPS_SEGMENTS && i < this->availableBgmapSegmentsForTextures; i++)
		{
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
						aux = 64;
					}
					else
					{
						aux = this->yOffset[i][j + 1];
					}

					// determine if there is still mem space (columns) in the current y offset
					if(rows + rowsPad <= aux - this->yOffset[i][j] || (!this->yOffset[i][j + 1]))
					{
						if(rows + rowsPad <= 64 - this->yOffset[i][j])
						{
							if(cols + colsPad <= 64 - this->xOffset[i][j])
							{
								u16 id = Texture_getId(__SAFE_CAST(Texture, bgmapTexture));

								// register bgmap definition
								this->offset[id][kXOffset] = this->xOffset[i][j] + (colsPad >> 1);
								this->offset[id][kYOffset] = this->yOffset[i][j] + (rowsPad >> 1);
								this->offset[id][kCols] = cols;
								this->offset[id][kRows] = rows;

								BgmapTexture_setSegment(bgmapTexture, i);

								// increment the x offset
								this->xOffset[i][j] += cols + colsPad;

								// if the number of rows of the bgmap definition is greater than the
								// next y offset defined, increase the next y offset
								if(this->yOffset[i][j + 1] - this->yOffset[i][j] < rows + rowsPad)
								{
									this->yOffset[i][j + 1] = this->yOffset[i][j] + rows + rowsPad;
								}
								else
								{
									// there must be at least 2 columns free
									if(this->xOffset[i][j] >= 64)
									{
										// TODO: this was commented, don't know why
										// this->yOffset[i][j+1] = this->yOffset[i][j] + rows ;
									}
								}

								// update the number of chars defined inside the bgmap segment
								this->numberOfChars[i] += area;

								if(this->availableBgmapSegmentsForTextures >= __MAX_NUMBER_OF_BGMAPS_SEGMENTS && i + 1 > this->freeBgmapSegment)
								{
									this->freeBgmapSegment = i + 1;
								}

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

		// throw an exception if there is no enough space to allocate the bgmap definition
		NM_ASSERT(false, "BgmapTextureManager::doAllocate: bgmap segments depleted");
	}

	// through exception if texture has 0 chars
	ASSERT(false, "BgmapTextureManager::doAllocate: map has 0 chars");

	return false;
}

/**
 * Allocate a BGMAP memory space for text
 *
 * @memberof				BgmapTextureManager
 * @private
 *
 * @param this				Function scope
 * @param bgmapTexture		Texture to allocate space for
 */
/*
void BgmapTextureManager_allocateText(BgmapTextureManager this, BgmapTexture bgmapTexture)
{
	ASSERT(this, "BgmapTextureManager::allocateText: null this");

	//int xDisplacement = 0;
	int yDisplacement = 0;

	u32 length = Texture_getCols(__SAFE_CAST(Texture, bgmapTexture));

	// if there is space in the first row
	// calculate y displacement
	// offset/64->chars per row inside a bgmap
	yDisplacement = (this->xOffset[this->freeBgmapSegment][0] + length) >> 6;

	// move to the next row
	if(this->xOffset[this->freeBgmapSegment][0] < 64 * yDisplacement)
	{
		this->xOffset[this->freeBgmapSegment][0] = 64 * yDisplacement;
	}

	// offset%/64->chars per row inside a bgmap
	//xDisplacement = (this->xOffset[this->freeBgmapSegment][0]) % 64;

	// set next offset entry to modify within the free bgmap segment
	this->xOffset[this->freeBgmapSegment][0] += length;

	// if there are no more rows in the segment thrown an exception
	ASSERT(this->xOffset[this->freeBgmapSegment][0] < 64, "BgmapTextureManager::allocateText: mem depleted (TextBox)");
}
*/

/**
 * Release a previously allocated Texture
 *
 * @memberof				BgmapTextureManager
 * @public
 *
 * @param this				Function scope
 * @param bgmapTexture		Texture to release
 */
void BgmapTextureManager_releaseTexture(BgmapTextureManager this, BgmapTexture bgmapTexture)
{
	ASSERT(this, "BgmapTextureManager::free: null this");

	// if no one is using the texture anymore
	if(__IS_OBJECT_ALIVE(bgmapTexture) && BgmapTexture_decreaseUsageCount(bgmapTexture))
	{
		int i = Texture_getId(__SAFE_CAST(Texture, bgmapTexture));

		TextureDefinition* textureDefinition = Texture_getTextureDefinition(__SAFE_CAST(Texture, bgmapTexture));

		switch(textureDefinition->charSetDefinition->allocationType)
		{
			case __ANIMATED_SINGLE:
			case __ANIMATED_SINGLE_OPTIMIZED:

				__DELETE(bgmapTexture);
				this->bgmapTextures[i] = NULL;
				break;

			case __ANIMATED_SHARED:
			case __ANIMATED_SHARED_COORDINATED:
			case __ANIMATED_MULTI:
			case __NOT_ANIMATED:

				Texture_releaseCharSet(__SAFE_CAST(Texture, bgmapTexture));
				break;
		}
	}
}

/**
 * Retrieve a previously allocated Texture
 *
 * @memberof							BgmapTextureManager
 * @private
 *
 * @param this							Function scope
 * @param bgmapTextureDefinition		Texture definition
 *
 * @return								Allocated Texture
 */
static BgmapTexture BgmapTextureManager_findTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition)
{
	ASSERT(this, "BgmapTextureManager::findTexture: null this");

	int i = 0;
	TextureDefinition* textureDefinition = (TextureDefinition*)bgmapTextureDefinition;

	// try to find a texture with the same bgmap definition
	for(; i < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if(this->bgmapTextures[i])
		{
			CharSet charSet = Texture_getCharSet(__SAFE_CAST(Texture, this->bgmapTextures[i]), false);
			TextureDefinition* allocatedTextureDefinition = Texture_getTextureDefinition(__SAFE_CAST(Texture, this->bgmapTextures[i]));

			if(allocatedTextureDefinition == (TextureDefinition*)textureDefinition &&
				(!charSet || allocatedTextureDefinition->charSetDefinition->allocationType == bgmapTextureDefinition->charSetDefinition->allocationType) &&
				(allocatedTextureDefinition->padding.cols == bgmapTextureDefinition->padding.cols && allocatedTextureDefinition->padding.rows == bgmapTextureDefinition->padding.rows)
			)
			{
				// return if found
				return this->bgmapTextures[i];
			}
		}
	}

	if(!textureDefinition->recyclable)
	{
		return NULL;
	}

	BgmapTexture selectedBgmapTexture = NULL;
	TextureDefinition* selectedTextureDefinition = NULL;

	// try to find a texture with the same bgmap definition
	for(i = 0; i < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		BgmapTexture bgmapTexture = this->bgmapTextures[i];

		if(bgmapTexture && !BgmapTexture_getUsageCount(bgmapTexture))
		{
			u16 id = Texture_getId(__SAFE_CAST(Texture, bgmapTexture));
			u16 cols = this->offset[id][kCols];
			u16 rows = this->offset[id][kRows];

			TextureDefinition* allocatedTextureDefinition = Texture_getTextureDefinition(__SAFE_CAST(Texture, bgmapTexture));

			if(allocatedTextureDefinition->recyclable &&
				textureDefinition->cols <= cols &&
				textureDefinition->rows <= rows
			)
			{
				if(!selectedBgmapTexture)
				{
					selectedBgmapTexture = bgmapTexture;
					selectedTextureDefinition = allocatedTextureDefinition;
				}
				else if(textureDefinition->cols == cols && textureDefinition->rows == rows)
				{
					selectedBgmapTexture = bgmapTexture;
					selectedTextureDefinition = allocatedTextureDefinition;
					break;
				}
				else if(cols <= selectedTextureDefinition->cols && rows <= selectedTextureDefinition->rows)
				{
					selectedBgmapTexture = bgmapTexture;
					selectedTextureDefinition = allocatedTextureDefinition;
				}
			}
		}
	}

	if(selectedBgmapTexture)
	{
		Texture_setDefinition(__SAFE_CAST(Texture, selectedBgmapTexture), textureDefinition);
		Texture_setPalette(__SAFE_CAST(Texture, selectedBgmapTexture), textureDefinition->palette);
		 Texture_rewrite(selectedBgmapTexture);
	}

	return selectedBgmapTexture;
}

/**
 * Allocate a BGMAP memory space for a new Texture
 *
 * @memberof							BgmapTextureManager
 * @private
 *
 * @param this							Function scope
 * @param bgmapTextureDefinition		Texture to allocate space for
 *
 * @return 								True if the required space was successfully allocated
 */
static BgmapTexture BgmapTextureManager_allocateTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition)
{
	ASSERT(this, "BgmapTextureManager::allocateTexture: null this");

	int i = 0;

	// find an empty slot
	for(; i < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if(!this->bgmapTextures[i])
		{
			// create new texture and register it
			this->bgmapTextures[i] = __NEW(BgmapTexture, bgmapTextureDefinition, i);

			//if not, then allocate
			BgmapTextureManager_doAllocate(this, this->bgmapTextures[i]);

			return this->bgmapTextures[i];
		}
	}

	return NULL;
}

/**
 * Retrieve a Texture
 *
 * @memberof							BgmapTextureManager
 * @private
 *
 * @param this							Function scope
 * @param bgmapTextureDefinition		Texture definition to find o allocate a Texture
 *
 * @return 								Allocated Texture
 */
BgmapTexture BgmapTextureManager_getTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition)
{
	ASSERT(this, "BgmapTextureManager::getTexture: null this");

	BgmapTexture bgmapTexture = NULL;

	//determine the allocation type
	switch(bgmapTextureDefinition->charSetDefinition->allocationType)
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SINGLE_OPTIMIZED:

			// load a new texture
			bgmapTexture = BgmapTextureManager_allocateTexture(this, bgmapTextureDefinition);

			ASSERT(bgmapTexture, "BgmapTextureManager::getTexture: (animated) texture no allocated");
			break;

		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
		case __ANIMATED_MULTI:
		case __NOT_ANIMATED:

			// first try to find an already created texture
			bgmapTexture = BgmapTextureManager_findTexture(this, bgmapTextureDefinition);

			// if couldn't find the texture
			if(bgmapTexture)
			{
				BgmapTexture_increaseUsageCount(bgmapTexture);
			}
			else
			{
				// load it
				bgmapTexture = BgmapTextureManager_allocateTexture(this, bgmapTextureDefinition);
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
 * @memberof		BgmapTextureManager
 * @private
 *
 * @param this		Function scope
 * @param id		Texture identification
 *
 * @return 			X offset within a BGMAP segment
 */
s16 BgmapTextureManager_getXOffset(BgmapTextureManager this, int id)
{
	ASSERT(this, "BgmapTextureManager::getXOffset: null this");

	return this->offset[id][kXOffset];
}

/**
 * Retrieve the y offset within a BGMAP segment of the Texture with the given id
 *
 * @memberof		BgmapTextureManager
 * @private
 *
 * @param this		Function scope
 * @param id		Texture identification
 *
 * @return 			Y offset within a BGMAP segment
 */
s16 BgmapTextureManager_getYOffset(BgmapTextureManager this, int id)
{
	ASSERT(this, "BgmapTextureManager::getYOffset: null this");

	return this->offset[id][kYOffset];
}

/**
 * Retrieve the number of non used BGMAP segments for texture allocation
 *
 * @memberof		BgmapTextureManager
 * @private
 *
 * @param this		Function scope
 *
 * @return 			Number of non used BGMAP segments for texture allocation
 */
s16 BgmapTextureManager_getAvailableBgmapSegmentsForTextures(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::getAvailableBgmapSegmentsForTextures: null this");

	return this->availableBgmapSegmentsForTextures;
}

/**
 * Retrieve the BGMAP segment available for printing
 *
 * @memberof		BgmapTextureManager
 * @private
 *
 * @param this		Function scope
 *
 * @return 			BGMAP segment available for printing
 */
s16 BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::getPrintingBgmapSegment: null this");

	return this->printingBgmapSegment;
}

/**
 * Compute the available BGMAP segments based on texture usage
 *
 * @memberof		BgmapTextureManager
 * @private
 *
 * @param this		Function scope
 */
void BgmapTextureManager_calculateAvailableBgmapSegments(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::calculateAvailableBgmapSegments: null this");

	u32 paramTableBase = ParamTableManager_getParamTableBase(ParamTableManager_getInstance());

	this->availableBgmapSegmentsForTextures = this->printingBgmapSegment = (u32)((paramTableBase - __BGMAP_SPACE_BASE_ADDRESS - (__PRINTABLE_BGMAP_AREA << 1)) / __BGMAP_SEGMENT_SIZE);

	if(this->availableBgmapSegmentsForTextures > __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->availableBgmapSegmentsForTextures = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
	}
}

/**
 * Print manager's status
 *
 * @memberof		BgmapTextureManager
 * @private
 *
 * @param this		Function scope
 * @param x			Camera's x coocrinate
 * @param y			Camera's y coocrinate
 */
void BgmapTextureManager_print(BgmapTextureManager this, int x, int y)
{
	ASSERT(this, "BgmapTextureManager::print: null this");

	int index = 0;
	int textureCount = 0;
	for(;index < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; index++)
	{
		if(this->bgmapTextures[index])
		{
			textureCount++;
		}
	}

	Printing_text(Printing_getInstance(), "BGMAP TEXTURES' USAGE", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Segments for textures: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), BgmapTextureManager_getAvailableBgmapSegmentsForTextures(this), x + 23, y, NULL);
	Printing_text(Printing_getInstance(), "Textures' count: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), textureCount, x + 23, y, NULL);
	Printing_text(Printing_getInstance(), "Printing segment: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(this), x + 23, y, NULL);

	y++;
	y++;
	Printing_text(Printing_getInstance(), "Recyclable textures", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Total: ", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Free: ", x, y++, NULL);

	y++;
	Printing_text(Printing_getInstance(), "ROM      Refs", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Address  ", x, y++, NULL);

	int i = 0;
	int j = 0;
	int recyclableTextures = 0;
	int freeEntries = 0;

	// try to find a texture with the same bgmap definition
	for(index = 0; index < this->availableBgmapSegmentsForTextures * __NUM_BGMAPS_PER_SEGMENT; index++)
	{
		BgmapTexture bgmapTexture = this->bgmapTextures[index];

		if(bgmapTexture)
		{
			TextureDefinition* allocatedTextureDefinition = Texture_getTextureDefinition(__SAFE_CAST(Texture, bgmapTexture));

			if(allocatedTextureDefinition->recyclable)
			{
				recyclableTextures++;
				freeEntries += !BgmapTexture_getUsageCount(bgmapTexture)? 1 : 0;

//				Printing_text(Printing_getInstance(), BgmapTexture_getUsageCount(bgmapTexture)? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + j + 1, y + i, NULL);
				Printing_hex(Printing_getInstance(), (int)Texture_getTextureDefinition(__SAFE_CAST(Texture, bgmapTexture)), x + j, y + i, 8, NULL);
				Printing_int(Printing_getInstance(), BgmapTexture_getUsageCount(bgmapTexture), x + j + 9, y + i, NULL);

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

	Printing_int(Printing_getInstance(), recyclableTextures, x + 9, y - 5, NULL);
	Printing_int(Printing_getInstance(), freeEntries, x + 9, y - 4, NULL);
}
