/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
void ObjectTexture::constructor(ObjectTextureSpec* objectTextureSpec, uint16 id, ObjectSprite owner)
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
void ObjectTexture::setFrameAnimatedMulti(uint16 frame)
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
