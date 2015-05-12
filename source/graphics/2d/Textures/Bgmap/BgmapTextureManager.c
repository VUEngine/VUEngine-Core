/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapTextureManager.h>

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define BgmapTextureManager_ATTRIBUTES															\
																								\
	/* super's attributes */																	\
	Object_ATTRIBUTES;																			\
																								\
	/* number of chars ocuppied */																\
	u16 numberOfChars[__MAX_NUMBER_OF_BGMAPS_SEGMENTS];											\
																								\
	/* current x offset to set the next bgmap */												\
	s16 xOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];						\
																								\
	/* current y offset to set the next bgmap */												\
	s16 yOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];						\
																								\
	/* 12 segments, 28 maps, 2 indexes (x,y) and bgmap segment */ 								\
	s8 offset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT][3];					\
																								\
	/* next free bgmap used for text printing */												\
	u8 freeBgmapSegment;																		\
																								\
	/* the textures allocated */																\
	BgmapTexture bgmapTextures[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT];		\
																								\
	/* number of available bgmap segments */													\
	u8 availableBgmapSegments;																	\

// define the BgmapTextureManager
__CLASS_DEFINITION(BgmapTextureManager, Object);

enum OffsetIndex
{
	kXOffset = 0,
	kYOffset,
	kBgmapSegment
};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void BgmapTextureManager_constructor(BgmapTextureManager this);
static int BgmapTextureManager_doAllocate(BgmapTextureManager this, BgmapTexture bgmapTexture);
static BgmapTexture BgmapTextureManager_findTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition);
static BgmapTexture BgmapTextureManager_allocateTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(BgmapTextureManager);

// class's constructor
static void BgmapTextureManager_constructor(BgmapTextureManager this)
{
	__CONSTRUCT_BASE();

	BgmapTextureManager_reset(this);
}

// class's destructor
void BgmapTextureManager_destructor(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// reset
void BgmapTextureManager_reset(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::reset: null this");

	this->availableBgmapSegments = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;

	int i = 0;
	int j = 0;

	// clear each bgmap segment usage
	for (; i < this->availableBgmapSegments; i++)
	{
		this->numberOfChars[i] = 0;

		// clear the offsets
		for (j = 0;j<__NUM_BGMAPS_PER_SEGMENT; j++)
		{
			this->xOffset[i][j] = 0;
			this->yOffset[i][j] = 0;
		}
	}

	for (i = 0; i < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		this->offset[i][kXOffset] = -1;
		this->offset[i][kYOffset] = -1;
		this->offset[i][kBgmapSegment] = -1;

		if (this->bgmapTextures[i])
		{
			__DELETE(this->bgmapTextures[i]);
		}
		
		this->bgmapTextures[i] = NULL;
	}

	this->freeBgmapSegment = 0;
}

// allocate texture in bgmap graphic memory
static int BgmapTextureManager_doAllocate(BgmapTextureManager this, BgmapTexture bgmapTexture)
{
	ASSERT(this, "BgmapTextureManager::allocate: null this");

	int i = 0;
	int j = 0;
	int aux = 0;

	u8 cols = Texture_getTotalCols(__GET_CAST(Texture, bgmapTexture));
	u8 rows = Texture_getTotalRows(__GET_CAST(Texture, bgmapTexture));
	
	cols += cols < 64? 1: 0;
	rows += rows < 64? 1: 0;

	u16 area = rows * cols;

	//if texture already defined, don't allocate
	if (Texture_getNumberOfChars(__GET_CAST(Texture, bgmapTexture)))
	{
		for (i = 0; i < this->availableBgmapSegments; i++)
		{
			// if there is space in the segment memory
			// there are 4096 chars in each bgmap segment
			if ((int)(4096 - this->numberOfChars[i]) >= (int)area )
			{
				//check if there is space within the segment
				// we check the next so don't go to the last element
				for (j = 0; j < __NUM_BGMAPS_PER_SEGMENT - 1; j++)
				{
					//determine the y offset inside the bgmap segment
					if (!this->yOffset[i][j + 1])
					{
						aux = 64;
					}
					else
					{
						aux = this->yOffset[i][j + 1];
					}

					//determine if there is still mem space (columns) in the current y offset
					if (rows <= aux - this->yOffset[i][j] || (!this->yOffset[i][j + 1]))
					{
						if (rows <= 64 - this->yOffset[i][j])
						{
							if (cols <= 64 - this->xOffset[i][j])
							{
								u16 id = Texture_getId(__GET_CAST(Texture, bgmapTexture));

								//registry bgmap definition
								this->offset[id][kXOffset] = this->xOffset[i][j];
								this->offset[id][kYOffset] = this->yOffset[i][j];
								this->offset[id][kBgmapSegment] = i;

								//increment the x offset
								this->xOffset[i][j] += cols;

								//if the number of rows of the bgmap definition is greater than the
								//next y offset defined, increase the next y offset
								if (this->yOffset[i][j + 1] - this->yOffset[i][j] < rows)
								{
									this->yOffset[i][j + 1] = this->yOffset[i][j] + rows ;
								}
								else
								{
									// there must be at least 2 columns free
									if (this->xOffset[i][j] >= 62)
									{
										//TODO: this was commented, don't know why
										//this->yOffset[i][j+1] = this->yOffset[i][j] + rows ;
									}
								}

								//update the number of chars defined inside the bgmap segment
								this->numberOfChars[i] += area;

								if (i + 1 > this->freeBgmapSegment)
								{
									this->freeBgmapSegment = i + 1;
								}

								//if there is a free bgmap segment
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
						if (rows > 64 - this->yOffset[i][j])
						{
							break;
						}
					}
				}
			}
		}

		//throw an exception if there is no enough space to allocate the bgmap definition
		NM_ASSERT(false, "BgmapTextureManager::allocate: bgmap segments depleted");
	}

	//through exception if texture has 0 chars
	ASSERT(false, "BgmapTextureManager::allocate: map has 0 chars");

	return false;
}

// retrieve free bgmap segment number
// allocate bgmap text boxes
// this bgmap segment is handled as one only bgmap defined inside so, only
// BgmapTextureManager.xOffset[textbgmap][0] is used
void BgmapTextureManager_allocateText(BgmapTextureManager this, BgmapTexture bgmapTexture)
{
	ASSERT(this, "BgmapTextureManager::allocateText: null this");

	int xDisplacement = 0;
	int yDisplacement = 0;

	u8 length = Texture_getCols(__GET_CAST(Texture, bgmapTexture));

	//if there is space in the first row
	//calculate y displacement
	//offset/64->chars per row inside a bgmap
	yDisplacement = (this->xOffset[this->freeBgmapSegment][0] + length) >> 6;

	//move to the next row
	if (this->xOffset[this->freeBgmapSegment][0] < 64 * yDisplacement)
	{
		this->xOffset[this->freeBgmapSegment][0] = 64 * yDisplacement;
	}

	//offset%/64->chars per row inside a bgmap
	xDisplacement = (this->xOffset[this->freeBgmapSegment][0]) % 64;

	//set next ofsset entry to modify within the free bgmap segment
	this->xOffset[this->freeBgmapSegment][0] += length;

	//if there are no more rows in the segment... thrown and exception
	ASSERT(this->xOffset[this->freeBgmapSegment][0] < 4096, "BgmapTextureManager::allocateText: mem depleted (TextBox)");
}

// deallocate texture from bgmap graphic memory
void BgmapTextureManager_releaseTexture(BgmapTextureManager this, BgmapTexture bgmapTexture)
{
	ASSERT(this, "BgmapTextureManager::free: null this");
	
	// if no one is using the texture anymore
	if (bgmapTexture && BgmapTexture_decreaseUsageCount(bgmapTexture))
	{
		int i = Texture_getId(__GET_CAST(Texture, bgmapTexture));
		
		switch(CharSet_getAllocationType(Texture_getCharSet(__GET_CAST(Texture, bgmapTexture))))
		{
			case __ANIMATED_SINGLE:

				__DELETE(bgmapTexture);
				this->bgmapTextures[i] = NULL;
				break;

			case __ANIMATED_SHARED:
			case __ANIMATED_SHARED_COORDINATED:
			case __ANIMATED_MULTI:
			case __NOT_ANIMATED:

				Texture_releaseCharSet(__GET_CAST(Texture, bgmapTexture));
				break;
		}
	}
}

// retrieve a texture previuosly loaded
static BgmapTexture BgmapTextureManager_findTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition)
{
	ASSERT(this, "BgmapTextureManager::findTexture: null this");

	int i = 0;

	// try to find a texture with the same bgmap definition
	for (; i < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if (this->bgmapTextures[i])
		{
			CharSet charSet = Texture_getCharSet(__GET_CAST(Texture, this->bgmapTextures[i]));

			if(Texture_getBgmapDefinition(__GET_CAST(Texture, this->bgmapTextures[i])) == bgmapTextureDefinition->bgmapDefinition &&
				(!charSet || CharSet_getAllocationType(charSet) == bgmapTextureDefinition->charSetDefinition.allocationType)
			)
			{
				if(!charSet)
				{
					BgmapTexture_write(this->bgmapTextures[i]);
				}
				
				// return if found
				return this->bgmapTextures[i];
			}
		}
	}

	return NULL;
}

// load a texture
static BgmapTexture BgmapTextureManager_allocateTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition)
{
	ASSERT(this, "BgmapTextureManager::writeTexture: null this");

	int i = 0;

	// find and empty slot
	for (; i < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if (!this->bgmapTextures[i])
		{
			// create new texture and register it
			this->bgmapTextures[i] = __NEW(BgmapTexture, bgmapTextureDefinition, i);

			//if not, then allocate
			BgmapTextureManager_doAllocate(this, this->bgmapTextures[i]);

			// write texture to graphic memory
			BgmapTexture_write(this->bgmapTextures[i]);

			return this->bgmapTextures[i];
		}
	}

	return NULL;
}

// load and retrieve a texture
BgmapTexture BgmapTextureManager_getTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition)
{
	ASSERT(this, "BgmapTextureManager::getTexture: null this");

	BgmapTexture bgmapTexture = NULL;

	//determine the allocation type
	switch (bgmapTextureDefinition->charSetDefinition.allocationType)
	{
		case __ANIMATED_SINGLE:

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
			if (bgmapTexture)
			{
				BgmapTexture_increaseUsageCount(bgmapTexture);
				
				if(!Texture_getCharSet(__GET_CAST(Texture, bgmapTexture)))
				{
					__VIRTUAL_CALL(void, Texture, write, bgmapTexture);
				}
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

// retrieve x offset
s8 BgmapTextureManager_getXOffset(BgmapTextureManager this, int id)
{
	ASSERT(this, "BgmapTextureManager::getXOffset: null this");

	return this->offset[id][kXOffset];
}

// retrieve y offset
s8 BgmapTextureManager_getYOffset(BgmapTextureManager this, int id)
{
	ASSERT(this, "BgmapTextureManager::getYOffset: null this");

	return this->offset[id][kYOffset];
}

// retrieve bgmap segment
u8 BgmapTextureManager_getBgmapSegment(BgmapTextureManager this, int id)
{
	ASSERT(this, "BgmapTextureManager::getBgmapSegment: null this");

	return this->offset[id][kBgmapSegment];
}

// retrieve available bgmap segments
u8 BgmapTextureManager_getAvailableBgmapSegmentForParamTable(BgmapTextureManager this)
{
	ASSERT(this, "getAvailableBgmapSegmentForParamTable::print: null this");
	
	return this->freeBgmapSegment + __NUMBER_OF_BGMAPS_SEGMENTS_ROOM;
}

// retrieve available bgmap segments
u8 BgmapTextureManager_getAvailableBgmapSegments(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::print: null this");
	
	return this->availableBgmapSegments;
}

// retrieve available bgmap segments
u8 BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager this)
{
	ASSERT(this, "BgmapTextureManager::print: null this");
	
	return this->freeBgmapSegment + __NUMBER_OF_BGMAPS_SEGMENTS_ROOM;
}

// calculate the available bgmap segments based on usage
void BgmapTextureManager_calculateAvailableBgmapSegments(BgmapTextureManager this)
{
	this->availableBgmapSegments = this->freeBgmapSegment + __NUMBER_OF_BGMAPS_SEGMENTS_ROOM;
}

// set the available bgmap segments based to maximum defined
void BgmapTextureManager_resetAvailableBgmapSegments(BgmapTextureManager this)
{
	this->availableBgmapSegments = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
}

// print status
void BgmapTextureManager_print(BgmapTextureManager this, int x, int y)
{
	ASSERT(this, "BgmapTextureManager::print: null this");

	int textureCount = 0;
	for (;this->bgmapTextures[textureCount] && textureCount < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; textureCount++);

	Printing_text(Printing_getInstance(), "BGMAP TEXTURES' USAGE", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Segments for textures: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), BgmapTextureManager_getAvailableBgmapSegments(this), x + 23, y, NULL);
	Printing_text(Printing_getInstance(), "Textures' count: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), textureCount, x + 23, y, NULL);
	Printing_text(Printing_getInstance(), "Printing segment: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), BgmapTextureManager_getPrintingBgmapSegment(this), x + 23, y, NULL);
}