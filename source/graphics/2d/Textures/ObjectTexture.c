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

#include <ObjectTexture.h>
#include <Optics.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(ObjectTexture, Texture);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void ObjectTexture_constructor(ObjectTexture this, ObjectTextureDefinition* objectTextureDefinition, u16 id);
static void ObjectTexture_writeAnimated(ObjectTexture this);
static void ObjectTexture_writeNoAnimated(ObjectTexture this);
static void ObjectTexture_writeNoAnimated(ObjectTexture this);
static void ObjectTexture_writeAnimatedShared(ObjectTexture this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectTexture, ObjectTextureDefinition* objectTextureDefinition, u16 id)
__CLASS_NEW_END(ObjectTexture, objectTextureDefinition, id);

// class's constructor
static void ObjectTexture_constructor(ObjectTexture this, ObjectTextureDefinition* objectTextureDefinition, u16 id)
{
	// construct base object
	__CONSTRUCT_BASE((TextureDefinition*)objectTextureDefinition, id);
	
	this->objectIndex = -1;
	this->bgmapDisplacement = 0;
}

// class's destructor
void ObjectTexture_destructor(ObjectTexture this)
{
	ASSERT(this, "ObjectTexture::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

// write into memory the chars and this
void ObjectTexture_write(ObjectTexture this)
{
	ASSERT(this, "ObjectTexture::write: null this");

	Texture_write(__UPCAST(Texture, this));
	
	//determine the allocation type
	switch (CharSet_getAllocationType(this->charSet))
	{
		case __ANIMATED:

			// write the definition to graphic memory
			ObjectTexture_writeAnimated(this);
			break;

		case __ANIMATED_SHARED:

			// write the definition to graphic memory
			ObjectTexture_writeAnimatedShared(this);
			break;

		case __NO_ANIMATED:

			// write the definition to graphic memory
			ObjectTexture_writeNoAnimated(this);
			break;

		default:

			ASSERT(false, "Texture::write: no allocation type");
	}
}

// write an animated map
static void ObjectTexture_writeAnimated(ObjectTexture this)
{
	ASSERT(this, "ObjectTexture::writeAnimated: null this");

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
static void ObjectTexture_writeNoAnimated(ObjectTexture this)
{
	ASSERT(this, "ObjectTexture::writeNoAnimated: null this");
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
static void ObjectTexture_writeAnimatedShared(ObjectTexture this)
{
	ASSERT(this, "ObjectTexture::writeAnimatedShared: null this");
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

void ObjectTexture_setObjectIndex(ObjectTexture this, int objectIndex)
{
	ASSERT(this, "ObjectTexture::setObjectIndex: null this");
	
	if(0 <= objectIndex && objectIndex < 1024)
	{
		this->objectIndex = objectIndex;
	}
}

void ObjectTexture_resetBgmapDisplacement(ObjectTexture this)
{
	ASSERT(this, "ObjectTexture::resetBgmapDisplacement: null this");
	
	this->bgmapDisplacement = 0;
}

void ObjectTexture_addBgmapDisplacement(ObjectTexture this, int frame)
{
	ASSERT(this, "ObjectTexture::setBgmapDisplacement: null this");
	ASSERT(0 <= frame, "ObjectTexture::setBgmapDisplacement: negative frame");

	this->bgmapDisplacement = (this->textureDefinition->cols * this->textureDefinition->rows) * frame << 1;
}

