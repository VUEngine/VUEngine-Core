/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
#include <ObjectSprite.h>
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
void ObjectTexture::constructor(ObjectTextureSpec* objectTextureSpec, u16 id, ObjectSprite owner)
{
	// construct base object
	Base::constructor((TextureSpec*)objectTextureSpec, id);

	this->owner = owner;
}

/**
 * Class destructor
 */
void ObjectTexture::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Set Texture's frame
 *
 * @param frame	Texture's frame to display
 */
void ObjectTexture::setFrameAnimatedMulti(u16 frame)
{
	ObjectTexture::setMapDisplacement(this, this->textureSpec->cols * this->textureSpec->rows * frame);
}

/**
 * Write the texture to DRAM
 */
void ObjectTexture::rewrite()
{
	Base::rewrite(this);

	if(isDeleted(this->owner))
	{
		return;
	}

	ObjectSprite_rewrite(this->owner);
}
