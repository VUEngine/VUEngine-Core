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

#include <TextureManager.h>

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

//#undef __TOTAL_NUMBER_OF_BGMAPS_SEGMENTS
//#undef __MAX_NUMBER_OF_BGMAPS_SEGMENTS
//#define __MAX_NUMBER_OF_BGMAPS_SEGMENTS 6

#define TextureManager_ATTRIBUTES													\
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
	Texture texture[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT];	\
																					\
	/* texture usage count */														\
	u8 textureUsageCount[__MAX_NUMBER_OF_BGMAPS_SEGMENTS * __NUM_BGMAPS_PER_SEGMENT];\
																					\
	/* number of available bgmap segments */										\
	u8 availableBgmapSegments;														\

// define the TextureManager
__CLASS_DEFINITION(TextureManager);

enum OffsetIndex
{
	kXOffset = 0,
	kYOffset,
	kBgmapSegment
};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void TextureManager_constructor(TextureManager this);
static int TextureManager_allocate(TextureManager this, Texture texture);
static Texture TextureManager_findTexture(TextureManager this, TextureDefinition* textureDefinition);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(TextureManager);

// class's constructor
static void TextureManager_constructor(TextureManager this)
{
	__CONSTRUCT_BASE(Object);

	TextureManager_reset(this);
}

// class's destructor
void TextureManager_destructor(TextureManager this)
{
	ASSERT(this, "TextureManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// reset
void TextureManager_reset(TextureManager this)
{
	ASSERT(this, "TextureManager::reset: null this");

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

		if (this->texture[i])
		{
			__DELETE(this->texture[i]);
		}
		
		this->texture[i] = NULL;
		this->textureUsageCount[i] = 0;
	}

	this->freeBgmapSegment = 0;
}

// allocate texture in bgmap graphic memory
static int TextureManager_allocate(TextureManager this, Texture texture)
{
	ASSERT(this, "TextureManager::allocate: null this");

	int i = 0;
	int j = 0;
	int aux = 0;

	u8 cols = Texture_getTotalCols(texture);
	u8 rows = Texture_getRows(texture);
	
	cols += cols < 64? 1: 0;
	rows += rows < 64? 1: 0;

	u16 area = rows * cols;

	//if texture already defined, don't allocate
	CACHE_ENABLE;
	if (Texture_getNumberOfChars(texture))
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
								u16 id = Texture_getId(texture);

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

								CACHE_DISABLE;

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
		NM_ASSERT(false, "TextureManager::allocate: bgmap segments depleted");
	}
	CACHE_DISABLE;

	//through exception if texture has 0 chars
	ASSERT(false, "TextureManager::allocate: map has 0 chars");

	return false;
}

// retrieve free bgmap segment number
// allocate bgmap text boxes
// this bgmap segment is handled as one only bgmap defined inside so, only
// TextureManager.xOffset[textbgmap][0] is used
void TextureManager_allocateText(TextureManager this, Texture texture)
{
	ASSERT(this, "TextureManager::allocateText: null this");

	int xDisplacement = 0;
	int yDisplacement = 0;

	u8 length = Texture_getCols(texture);

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
	ASSERT(this->xOffset[this->freeBgmapSegment][0] < 4096, "TextureManager::allocateText: mem depleted (TextBox)");
}

// deallocate texture from bgmap graphic memory
void TextureManager_free(TextureManager this, Texture texture)
{
	ASSERT(this, "TextureManager::free: null this");

	// if no one is using the texture anymore
	if (texture && !(--this->textureUsageCount[Texture_getId(texture)]))
	{
		int allocationType = CharGroup_getAllocationType(Texture_getCharGroup(texture));

		// free char memory
		Texture_freeCharMemory(texture);

		//determine the allocation type
		switch (allocationType)
		{
			case __ANIMATED:
				{
					int i = 0;

					// try to find a texture with the same bgmap definition
					for (; i < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; i++)
					{
						if (this->texture[i] == texture)
						{
							this->textureUsageCount[Texture_getId(texture)] = 0;
							this->texture[i] = NULL;
							__DELETE(texture);
							break;
						}
					}
				}
				break;
		}
	}
}

// retrieve a texture previuosly loaded
static Texture TextureManager_findTexture(TextureManager this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "TextureManager::findTexture: null this");

	int i = 0;

	// try to find a texture with the same bgmap definition
	for (; i < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if (this->texture[i] && Texture_getBgmapDef(this->texture[i]) == textureDefinition->bgmapDefinition)
		{
			// return if found
			return this->texture[i];
		}
	}

	return NULL;
}

// load a texture
static Texture TextureManager_writeTexture(TextureManager this, TextureDefinition* textureDefinition, int isPreload)
{
	ASSERT(this, "TextureManager::writeTexture: null this");

	int i = 0;

	// find and empty slot
	for (; i < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; i++)
	{
		if (!this->texture[i])
		{
			// create new texture and register it
			this->texture[i] = __NEW(Texture, __ARGUMENTS(textureDefinition, i));

			//if not, then allocate
			TextureManager_allocate(this, this->texture[i]);

			// write texture to graphic memory
			Texture_write(this->texture[i]);

			// set texture usage
			this->textureUsageCount[i] = isPreload ? 0 : 1;

			return this->texture[i];
		}
	}

	return NULL;
}

// load and retrieve a texture
Texture TextureManager_loadTexture(TextureManager this, TextureDefinition* textureDefinition, int isPreload)
{
	ASSERT(this, "TextureManager::loadTexture: null this");

	Texture texture = NULL;

	//determine the allocation type
	switch (textureDefinition->charGroupDefinition.allocationType)
	{
		case __ANIMATED:

			// load a new texture
			texture = TextureManager_writeTexture(this, textureDefinition, isPreload);

			ASSERT(texture, "TextureManager::get: (animated) texture no allocated");

			break;

		case __ANIMATED_SHARED:
		case __NO_ANIMATED:

			// first try to find an already created texture
			texture = TextureManager_findTexture(this, textureDefinition);

			// if coudn't find the texture
			if (!texture)
			{
				// load it
				texture = TextureManager_writeTexture(this, textureDefinition, isPreload);
			}
			else
			{
				// if no using texture yet
				if (!this->textureUsageCount[Texture_getId(texture)])
				{
					// write texture to graphic memory
					Texture_write(texture);
				}

				// increase texture usage count
				this->textureUsageCount[Texture_getId(texture)]++;
			}

			ASSERT(texture, "TextureManager::get: (shared) texture no allocated");

			break;
	}

	return texture;
}

// load and retrieve a texture
Texture TextureManager_get(TextureManager this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "TextureManager::get: null this");

	return textureDefinition? TextureManager_loadTexture(this, textureDefinition, false): NULL;
}

// retrieve x offset
s8 TextureManager_getXOffset(TextureManager this, int id)
{
	ASSERT(this, "TextureManager::getXOffset: null this");

	return this->offset[id][kXOffset];
}

// retrieve y offset
s8 TextureManager_getYOffset(TextureManager this, int id)
{
	ASSERT(this, "TextureManager::getYOffset: null this");

	return this->offset[id][kYOffset];
}

// retrieve bgmap segment
u8 TextureManager_getBgmapSegment(TextureManager this, int id)
{
	ASSERT(this, "TextureManager::getBgmapSegment: null this");

	return this->offset[id][kBgmapSegment];
}

// retrieve available bgmap segments
u8 TextureManager_getAvailableBgmapSegments(TextureManager this)
{
	ASSERT(this, "TextureManager::print: null this");
	return this->availableBgmapSegments;
}

// retrieve available bgmap segments
void TextureManager_setAvailableBgmapSegments(TextureManager this, u8 availableBgmapSegments)
{
	ASSERT(this, "TextureManager::print: null this");
	
	this->availableBgmapSegments = 0 < availableBgmapSegments && availableBgmapSegments <= __MAX_NUMBER_OF_BGMAPS_SEGMENTS? availableBgmapSegments: this->availableBgmapSegments; 
}

// retrieve available bgmap segments
u8 TextureManager_getPrintingBgmapSegment(TextureManager this)
{
	ASSERT(this, "TextureManager::print: null this");
	
	return this->freeBgmapSegment;
}

// calculate the available bgmap segments based on usage
void TextureManager_calculateAvailableBgmapSegments(TextureManager this)
{
	this->availableBgmapSegments = this->freeBgmapSegment;
}

// set the available bgmap segments based to maximum defined
void TextureManager_resetAvailableBgmapSegments(TextureManager this)
{
	this->availableBgmapSegments = __MAX_NUMBER_OF_BGMAPS_SEGMENTS;
}

// print status
void TextureManager_print(TextureManager this, int x, int y)
{
	ASSERT(this, "TextureManager::print: null this");

	int textureCount = 0;
	for (;this->texture[textureCount] && textureCount < this->availableBgmapSegments * __NUM_BGMAPS_PER_SEGMENT; textureCount++);

	Printing_text(Printing_getInstance(), "TEXTURES' USAGE", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Texture count: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), textureCount, x + 15, y, NULL);
}