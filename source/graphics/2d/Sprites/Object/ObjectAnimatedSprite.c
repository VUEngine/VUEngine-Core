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
#include <AnimationCoordinatorFactory.h>
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

	this->animationController = new AnimationController();

	AnimationController::setAnimationCoordinator(
		this->animationController, 
		AnimationCoordinatorFactory::getCoordinator(
			AnimationCoordinatorFactory::getInstance(), 
			this->animationController, 
			owner,
			objectAnimatedSpriteSpec->objectSpriteSpec.spriteSpec.textureSpec->charSetSpec
			)
	);
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
	this->renderFlag = true;
	Texture::setFrame(this->texture, AnimationController::getActualFrameIndex(this->animationController));
}
