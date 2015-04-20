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

#include <OTexture.h>
#include <Optics.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(OTexture, Texture);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void OTexture_constructor(OTexture this, OTextureDefinition* oTextureDefinition, u16 id);
static void OTexture_writeAnimated(OTexture this);
static void OTexture_writeNoAnimated(OTexture this);
static void OTexture_writeNoAnimated(OTexture this);
static void OTexture_writeAnimatedShared(OTexture this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(OTexture, OTextureDefinition* oTextureDefinition, u16 id)
__CLASS_NEW_END(OTexture, oTextureDefinition, id);

// class's constructor
static void OTexture_constructor(OTexture this, OTextureDefinition* oTextureDefinition, u16 id)
{
	// construct base object
	__CONSTRUCT_BASE((TextureDefinition*)oTextureDefinition, id);
	
	this->objectIndex = -1;
}

// class's destructor
void OTexture_destructor(OTexture this)
{
	ASSERT(this, "OTexture::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

// write into memory the chars and this
void OTexture_write(OTexture this)
{
	ASSERT(this, "OTexture::write: null this");

	Texture_write(__UPCAST(Texture, this));
	
	//determine the allocation type
	switch (CharSet_getAllocationType(this->charSet))
	{
		case __ANIMATED:

			// write the definition to graphic memory
			OTexture_writeAnimated(this);
			break;

		case __ANIMATED_SHARED:

			// write the definition to graphic memory
			OTexture_writeAnimatedShared(this);
			break;

		case __NO_ANIMATED:

			// write the definition to graphic memory
			OTexture_writeNoAnimated(this);
			break;

		default:

			ASSERT(false, "Texture::write: no allocation type");
	}
}

// write an animated map
static void OTexture_writeAnimated(OTexture this)
{
	ASSERT(this, "OTexture::writeAnimated: null this");

	int palette = this->palette << 14;
	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int rows = this->textureDefinition->rows;
	int cols = this->textureDefinition->cols;

	int i = 0;
	for (; i < rows; i++)
	{
		int j = 0;
		for (; j < cols; j++)
		{
			s32 objectIndex = this->objectIndex + i * cols + j;
			s32 charNumberIndex = objectIndex * 2;
			u16 charNumber = this->textureDefinition->bgmapDefinition[charNumberIndex] | (this->textureDefinition->bgmapDefinition[charNumberIndex + 1] << 8);
			charNumber += charLocation;
			OAM[ objectIndex << 2] = 0;
			OAM[(objectIndex << 2) + 1] = 0;
			OAM[(objectIndex << 2) + 2] = 0;
			OAM[(objectIndex << 2) + 3] = palette | (charNumber & 0x7FF);
		}
	}
}

// write an inanimated map
static void OTexture_writeNoAnimated(OTexture this)
{
	ASSERT(this, "OTexture::writeNoAnimated: null this");
/*
	int bgmapSegment = OTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int i = this->textureDefinition->rows;

	int xOffset = (int)OTextureManager_getXOffset(OTextureManager_getInstance(), this->id);
	int yOffset = (int)OTextureManager_getYOffset(OTextureManager_getInstance(), this->id);

	if (0 > xOffset || 0 > yOffset)
	{
		return;
	}
	
	//put the map into memory calculating the number of char for each reference
	for (; i--;)
	{
		//specifying the char displacement inside the char mem
		Mem_add ((u8*)BGMap(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (i << 6)) << 1),
				(const u8*)(this->textureDefinition->bgmapDefinition + ( i * (this->textureDefinition->cols) << 1)),
				this->textureDefinition->cols,
				(palette) | (charLocation));
	}
	*/
}

// write an animated and shared map
static void OTexture_writeAnimatedShared(OTexture this)
{
	ASSERT(this, "OTexture::writeAnimatedShared: null this");
/*
	int bgmapSegment = OTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	// determine the number of frames the map had
	int area = (this->textureDefinition->cols * this->textureDefinition->rows);
	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int frames = CharSet_getNumberOfChars(this->charSet) / area;

	int i = this->textureDefinition->rows;

	int xOffset = (int)OTextureManager_getXOffset(OTextureManager_getInstance(), this->id);
	int yOffset = (int)OTextureManager_getYOffset(OTextureManager_getInstance(), this->id);

	if (0 > xOffset || 0 > yOffset)
	{
		return;
	}

	//put the map into memory calculating the number of char for each reference
	for (; i--;)
	{
		int j = 1;
		//write into the specified bgmap segment plus the offset defined in the this structure, the this definition
		//specifying the char displacement inside the char mem
		for (; j <= frames; j++)
		{
			Mem_add ((u8*)BGMap(bgmapSegment) + ((xOffset + (this->textureDefinition->cols * (j - 1)) + (yOffset << 6) + (i << 6)) << 1),
					(const u8*)(this->textureDefinition->bgmapDefinition + ( i * (this->textureDefinition->cols) << 1)),
					this->textureDefinition->cols,
					(palette) | (charLocation + area * (j - 1)));
		}
	}
	*/
}

void OTexture_setObjectIndex(OTexture this, int objectIndex)
{
	ASSERT(this, "OTexture::setObjectIndex: null this");
	
	if(0 <= objectIndex && objectIndex < 1024)
	{
		this->objectIndex = objectIndex;
	}
}

