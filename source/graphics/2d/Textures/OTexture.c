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
	this->bgmapDisplacement = 0;
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
	BYTE* framePointer = this->textureDefinition->bgmapDefinition;
	
	int i = 0;
	for (; i < rows; i++)
	{
		int j = 0;
		for (; j < cols; j++)
		{
			s32 objectIndex = this->objectIndex + i * cols + j;
			s32 charNumberIndex = objectIndex * 2;
			u16 charNumber = framePointer[charNumberIndex] | (framePointer[charNumberIndex + 1] << 8);
			charNumber += charLocation;
			OAM[(objectIndex << 2) + 3] = palette | (charNumber & 0x7FF);
		}
	}
}

// write an inanimated map
static void OTexture_writeNoAnimated(OTexture this)
{
	ASSERT(this, "OTexture::writeNoAnimated: null this");
	int palette = this->palette << 14;
	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int rows = this->textureDefinition->rows;
	int cols = this->textureDefinition->cols;
	BYTE* framePointer = this->textureDefinition->bgmapDefinition;

	int i = 0;
	for (; i < rows; i++)
	{
		int j = 0;
		for (; j < cols; j++)
		{
			s32 objectIndex = this->objectIndex + i * cols + j;
			s32 charNumberIndex = (i * cols + j) << 1;
			u16 charNumber = framePointer[charNumberIndex] | (framePointer[charNumberIndex + 1] << 8);
			charNumber += charLocation;
			OAM[(objectIndex << 2) + 3] = palette | (charNumber & 0x7FF);
		}
	}
}

// write an animated and shared map
static void OTexture_writeAnimatedShared(OTexture this)
{
	ASSERT(this, "OTexture::writeAnimatedShared: null this");
	int palette = this->palette << 14;
	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int rows = this->textureDefinition->rows;
	int cols = this->textureDefinition->cols;
	BYTE* framePointer = this->textureDefinition->bgmapDefinition + this->bgmapDisplacement;

	int i = 0;
	for (; i < rows; i++)
	{
		int j = 0;
		for (; j < cols; j++)
		{
			s32 objectIndex = this->objectIndex + i * cols + j;
			s32 charNumberIndex = (i * cols + j) << 1;
			u16 charNumber = framePointer[charNumberIndex] | (framePointer[charNumberIndex + 1] << 8);
			charNumber += charLocation;
			OAM[(objectIndex << 2) + 3] = palette | (charNumber & 0x7FF);
		}
	}
}

void OTexture_setObjectIndex(OTexture this, int objectIndex)
{
	ASSERT(this, "OTexture::setObjectIndex: null this");
	
	if(0 <= objectIndex && objectIndex < 1024)
	{
		this->objectIndex = objectIndex;
	}
}

void OTexture_resetBgmapDisplacement(OTexture this)
{
	ASSERT(this, "OTexture::resetBgmapDisplacement: null this");
	
	this->bgmapDisplacement = 0;
}

void OTexture_addBgmapDisplacement(OTexture this, int frame)
{
	ASSERT(this, "OTexture::setBgmapDisplacement: null this");
	ASSERT(0 <= frame, "OTexture::setBgmapDisplacement: negative frame");

	this->bgmapDisplacement = (this->textureDefinition->cols * this->textureDefinition->rows) * frame << 1;
}

