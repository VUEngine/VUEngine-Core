/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectTexture.h>
#include <Optics.h>
#include <VIPManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(ObjectTexture, Texture);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
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
	__CONSTRUCT_BASE(Texture, (TextureDefinition*)objectTextureDefinition, id);

	this->objectIndex = -1;
	this->bgmapDisplacement = 0;
}

// class's destructor
void ObjectTexture_destructor(ObjectTexture this)
{
	ASSERT(this, "ObjectTexture::destructor: null this");

	this->objectIndex = -1;

	// destroy the super object
	// must always be called at the end of the destructor
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

	if(!this->charSet)
	{
	    return;
	}

	int palette = this->palette << 14;
	int charLocation = CharSet_getOffset(this->charSet);
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
			_objectAttributesBaseAddress[(objectIndex << 2) + 3] = palette | (charNumber & 0x7FF);
		}
	}
}

void ObjectTexture_setObjectIndex(ObjectTexture this, int objectIndex)
{
	ASSERT(this, "ObjectTexture::setObjectIndex: null this");

	if(0 <= objectIndex && objectIndex < 1024)
	{
		this->objectIndex = objectIndex;
		this->written = false;
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

