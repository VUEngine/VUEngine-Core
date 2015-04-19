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

#include <BTextureManager.h>

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define BTextureManager_ATTRIBUTES													\
																					\
	/* super's attributes */														\
	Object_ATTRIBUTES;																\
																					\
	/* number of chars ocuppied */													\
	u16 numberOfChars[__MAX_NUMBER_OF_BGMAPS_SEGMENTS];								\
																					\
	/* current x offset to set the next bgmap */									\
	s16 xOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];			\
																					\
	/* current y offset to set the next bgmap */									\
	s16 yOffset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS][__NUM_BGMAPS_PER_SEGMENT];			\
																					\
	/* 12 segments, 28 maps, 2 indexes (x,y) and bgmap segment */ 					\
	s8 offset[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT][3];		\
																					\
	/* next free bgmap used for text printing */									\
	u8 freeBgmapSegment;																	\
																					\
	/* the textures allocated */													\
	BTexture bTexture[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT];	\
																					\
	/* texture usage count */														\
	u8 textureUsageCount[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT];\
																					\
	/* number of available bgmap segments */										\
	u8 availableBgmapSegments;														\

// define the BTextureManager
__CLASS_DEFINITION(BTextureManager, Object);

enum OffsetIndex
{
	kXOffset = 0,
	kYOffset,
	kBgmapSegment
};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void BTextureManager_constructor(BTextureManager this);
static int BTextureManager_allocate(BTextureManager this, BTexture bTexture);
static BTexture BTextureManager_findTexture(BTextureManager this, BTextureDefinition* bTextureDefinition);
static BTexture BTextureManager_writeTexture(BTextureManager this, BTextureDefinition* bTextureDefinition, int isPreload);
BTexture BTextureManager_loadTexture(BTextureManager this, BTextureDefinition* bTextureDefinition, int isPreload);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(BTextureManager);

// class's constructor
static void BTextureManager_constructor(BTextureManager this)
{
	__CONSTRUCT_BASE();

	BTextureManager_reset(this);
}

// class's destructor
void BTextureManager_destructor(BTextureManager this)
{
	ASSERT(this, "BTextureManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// reset
void BTextureManager_reset(BTextureManager this)
{
	ASSERT(this, "BTextureManager::reset: null this");

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

		if (this->bTexture[i])
		{
			__DELETE(this->bTexture[i]);
		}
		
		this->bTexture[i] = NULL;
		this->textureUsageCount[i] = 0;
	}

	this->freeBgmapSegment = 0;
}

// allocate texture in bgmap graphic memory
static int BTextureManager_allocate(BTextureManager this, BTexture bTexture)
{
	ASSERT(this, "BTextureManager::allocate: null this");

	int i = 0;
	int j = 0;
	int aux = 0;

	u8 cols = Texture_getTotalCols(__UPCAST(Texture, bTexture));
	u8 rows = Texture_getTotalRows(__UPCAST(Texture, bTexture));
	
	cols += cols < 64? 1: 0;
	rows += rows < 64? 1: 0;

	u16 area = rows * cols;

	//if texture already defined, don't allocate
	if (Texture_getNumberOfChars(__UPCAST(Texture, bTexture)))
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
								u16 id = Texture_getId(__UPCAST(Texture, bTexture));

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
		NM_ASSERT(false, "BTextureManager::allocate: bgmap segments depleted");
	}

	//through exception if texture has 0 chars
	ASSERT(false, "BTextureManager::allocate: map has 0 chars");

	return false;
}

// retrieve free bgmap segment number
// allocate bgmap text boxes
// this bgmap segment is handled as one only bgmap defined inside so, only
// BTextureManager.xOffset[textbgmap][0] is used
void BTextureManager_allocateText(BTextureManager this, BTexture bTexture)
{
	ASSERT(this, "BTextureManager::allocateText: null this");

	int xDisplacement = 0;
	int yDisplacement = 0;

	u8 length = Texture_getCols(__UPCAST(Texture, bTexture));

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

	//Texture_setBgmapSegment(texture, this->freeBgmapSegment);

	//if there are no more rows in the segment... thrown and exception
	ASSERT(this->xOffset[this->freeBgmapSegment][0] < 4096, "BTextureManager::allocateText: mem depleted (TextBox)");
}

// deallocate texture from bgmap graphic memory
void BTextureManager_free(BTextureManager this, BTexture bTexture)
{
	ASSERT(this, "BTextureManager::free: null this");

	// if no one is using the texture anymore
	if (bTexture && !(--this->textureUsageCount[Texture_getId(__UPCAST(Texture, bTexture))]))
	{
		int allocationType = CharSet_getAllocationType(Texture_getCharSet(__UPCAST(Texture, bTexture)));

		// free char memory
		Texture_freeCharMemory(__UPCAST(Texture, bTexture));

		//determine the allocation type
		switch (allocationType)
		{
			case __ANIMATED:
				{
					int i = 0;

					// try to find a texture with the same bgmap definition
					for (; i < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; i++)
					{
						if (this->bTexture[i] == bTexture)
						{
							this->textureUsageCount[Texture_getId(__UPCAST(Texture, bTexture))] = 0;
							this->bTexture[i] = NULL;
							__DELETE(bTexture);
							break;
						}
					}
				}
				break;
		}
	}
}

// retrieve a texture previuosly loaded
static BTexture BTextureManager_findTexture(BTextureManager this, BTextureDefinition* bTextureDefinition)
{
	ASSERT(this, "BTextureManager::findTexture: null this");

	int i = 0;

	// try to find a texture with the same bgmap definition
	for (; i < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if (this->bTexture[i] && Texture_getBgmapDefinition(__UPCAST(Texture, this->bTexture[i])) == bTextureDefinition->bgmapDefinition)
		{
			// return if found
			return this->bTexture[i];
		}
	}

	return NULL;
}

// load a texture
static BTexture BTextureManager_writeTexture(BTextureManager this, BTextureDefinition* bTextureDefinition, int isPreload)
{
	ASSERT(this, "BTextureManager::writeTexture: null this");

	int i = 0;

	// find and empty slot
	for (; i < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if (!this->bTexture[i])
		{
			// create new texture and register it
			this->bTexture[i] = __NEW(BTexture, bTextureDefinition, i);

			//if not, then allocate
			BTextureManager_allocate(this, this->bTexture[i]);

			// write texture to graphic memory
			BTexture_write(this->bTexture[i]);

			// set texture usage
			this->textureUsageCount[i] = isPreload ? 0 : 1;

			return this->bTexture[i];
		}
	}

	return NULL;
}

// load and retrieve a texture
BTexture BTextureManager_loadTexture(BTextureManager this, BTextureDefinition* bTextureDefinition, int isPreload)
{
	ASSERT(this, "BTextureManager::loadTexture: null this");

	BTexture bTexture = NULL;

	//determine the allocation type
	switch (bTextureDefinition->charSetDefinition.allocationType)
	{
		case __ANIMATED:

			// load a new texture
			bTexture = BTextureManager_writeTexture(this, bTextureDefinition, false);

			ASSERT(bTexture, "BTextureManager::get: (animated) texture no allocated");
			break;

		case __ANIMATED_SHARED:
		case __NO_ANIMATED:

			// first try to find an already created texture
			bTexture = BTextureManager_findTexture(this, bTextureDefinition);

			// if couldn't find the texture
			if (!bTexture)
			{
				// load it
				bTexture = BTextureManager_writeTexture(this, bTextureDefinition, isPreload);
			}
			else
			{
				// if no using texture yet
				if (!this->textureUsageCount[Texture_getId(__UPCAST(Texture, bTexture))])
				{
					// write texture to graphic memory
					BTexture_write(bTexture);
				}

				// increase texture usage count
				this->textureUsageCount[Texture_getId(__UPCAST(Texture, bTexture))]++;
			}

			ASSERT(bTexture, "BTextureManager::get: (shared) texture no allocated");
			break;
	}

	return bTexture;
}

// load and retrieve a texture
BTexture BTextureManager_get(BTextureManager this, BTextureDefinition* bTextureDefinition)
{
	ASSERT(this, "BTextureManager::get: null this");

	return bTextureDefinition? BTextureManager_loadTexture(this, bTextureDefinition, false): NULL;
}

// retrieve x offset
s8 BTextureManager_getXOffset(BTextureManager this, int id)
{
	ASSERT(this, "BTextureManager::getXOffset: null this");

	return this->offset[id][kXOffset];
}

// retrieve y offset
s8 BTextureManager_getYOffset(BTextureManager this, int id)
{
	ASSERT(this, "BTextureManager::getYOffset: null this");

	return this->offset[id][kYOffset];
}

// retrieve bgmap segment
u8 BTextureManager_getBgmapSegment(BTextureManager this, int id)
{
	ASSERT(this, "BTextureManager::getBgmapSegment: null this");

	return this->offset[id][kBgmapSegment];
}

// retrieve available bgmap segments
u8 BTextureManager_getAvailableBgmapSegments(BTextureManager this)
{
	ASSERT(this, "BTextureManager::print: null this");
	
	return this->availableBgmapSegments;
}

// retrieve available bgmap segments
u8 BTextureManager_getPrintingBgmapSegment(BTextureManager this)
{
	ASSERT(this, "BTextureManager::print: null this");
	
	return this->freeBgmapSegment + __NUMBER_OF_BGMAPS_SEGMENTS_ROOM;
}

// calculate the available bgmap segments based on usage
void BTextureManager_calculateAvailableBgmapSegments(BTextureManager this)
{
	this->availableBgmapSegments = this->freeBgmapSegment + __NUMBER_OF_BGMAPS_SEGMENTS_ROOM;
}

// set the available bgmap segments based to maximum defined
void BTextureManager_resetAvailableBgmapSegments(BTextureManager this)
{
	this->availableBgmapSegments = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
}

// print status
void BTextureManager_print(BTextureManager this, int x, int y)
{
	ASSERT(this, "BTextureManager::print: null this");

	int textureCount = 0;
	for (;this->bTexture[textureCount] && textureCount < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; textureCount++);

	Printing_text(Printing_getInstance(), "TEXTURES", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Texture count: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), textureCount, x + 15, y, NULL);
}