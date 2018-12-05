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

#include <ObjectTexture.h>
#include <Optics.h>
#include <VIPManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 * @param objectTextureSpec		Texture spec
 * @param id							Identifier
 */
void ObjectTexture::constructor(ObjectTextureSpec* objectTextureSpec, u16 id)
{
	// construct base object
	Base::constructor((TextureSpec*)objectTextureSpec, id);

	this->objectIndex = -1;
	this->mapDisplacement = 0;
}

/**
 * Class destructor
 */
void ObjectTexture::destructor()
{
	this->objectIndex = -1;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Write the texture to DRAM
 */
void ObjectTexture::write()
{
	if(0 > this->objectIndex)
	{
		return;
	}

	Base::write(this);

	if(!this->charSet)
	{
		return;
	}

	int palette = this->palette << 14;
	int charLocation = CharSet::getOffset(this->charSet);
	int rows = this->textureSpec->rows;
	int cols = this->textureSpec->cols;
	BYTE* framePointer = this->textureSpec->mapSpec + this->mapDisplacement;

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

/**
 * Set the start OBJECT index
 *
 * @param objectIndex	OBJECT index
 */
void ObjectTexture::setObjectIndex(int objectIndex)
{
	if(0 <= objectIndex && objectIndex < 1024)
	{
		this->objectIndex = objectIndex;
		this->written = false;
	}
}
