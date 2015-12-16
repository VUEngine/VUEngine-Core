/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
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

// globals
extern unsigned int volatile* _xpstts;

static void ObjectTexture_constructor(ObjectTexture this, ObjectTextureDefinition* objectTextureDefinition, u16 id);


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

	this->objectIndex = -1;

	// destroy the super object
	__DESTROY_BASE;
}

// write into memory the chars and this
void ObjectTexture_write(ObjectTexture this)
{
	ASSERT(this, "ObjectTexture::write: null this");

	if(0 > this->objectIndex)
	{
		return;
	}
	
	Texture_write(__SAFE_CAST(Texture, this));
	
	int palette = this->palette << 14;
	int charLocation = (CharSet_getSegment(this->charSet) << 9) + CharSet_getOffset(this->charSet);
	int rows = this->textureDefinition->rows;
	int cols = this->textureDefinition->cols;
	BYTE* framePointer = this->textureDefinition->bgmapDefinition + this->bgmapDisplacement;

	int i = 0;

	for(; i < rows; i++)
	{
		int j = 0;
		for(; j < cols; j++)
		{
			s32 objectIndex = this->objectIndex + i * cols + j;
			s32 charNumberIndex = (i * cols + j) << 1;
			u16 charNumber = charLocation + (framePointer[charNumberIndex] | (framePointer[charNumberIndex + 1] << 8));
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

