/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <string.h>

#include <AnimationController.h>
#include <ObjectSpriteContainer.h>

#include "ObjectAnimatedSprite.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ObjectAnimatedSprite::constructor(SpatialObject owner, const ObjectAnimatedSpriteSpec* objectAnimatedSpriteSpec)
{
	// construct base object
	Base::constructor(owner, &objectAnimatedSpriteSpec->objectSpriteSpec);

	ObjectAnimatedSprite::createAnimationController(this);
}
//---------------------------------------------------------------------------------------------------------
void ObjectAnimatedSprite::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void ObjectAnimatedSprite::updateAnimation()
{
	NM_ASSERT(!isDeleted(this->animationController), "ObjectAnimatedSprite::updateAnimation: null animation controller");

	if(isDeleted(this->animationController))
	{
		return;
	}

	if(Texture::isMultiframe(this->texture))
	{
		this->objectTextureSource.displacement = AnimationController::getActualFrameIndex(this->animationController) * this->rows * this->cols;
	}
	else
	{
		Texture::setFrame(this->texture, AnimationController::getActualFrameIndex(this->animationController));
	}
}
//---------------------------------------------------------------------------------------------------------