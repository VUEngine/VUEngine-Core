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

#include <ObjectAnimatedSprite.h>

#include <AnimationController.h>
#include <ObjectSpriteContainer.h>

#include <string.h>


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern int32 strcmp(const char *, const char *);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param objectSpriteSpec	Sprite spec
 * @param owner						Owner
 */
void ObjectAnimatedSprite::constructor(const ObjectAnimatedSpriteSpec* objectAnimatedSpriteSpec, ListenerObject owner)
{
	// construct base object
	Base::constructor(&objectAnimatedSpriteSpec->objectSpriteSpec, owner);

	ObjectAnimatedSprite::createAnimationController(this, objectAnimatedSpriteSpec->objectSpriteSpec.spriteSpec.textureSpec->charSetSpec, owner);
}

/**
 * Class destructor
 *
 * @private
 */
void ObjectAnimatedSprite::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Write animation
 */
void ObjectAnimatedSprite::writeAnimation()
{
	NM_ASSERT(!isDeleted(this->animationController), "ObjectAnimatedSprite::writeAnimation: null animation controller");

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
