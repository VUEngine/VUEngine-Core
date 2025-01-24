/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <AnimationController.h>
#include <AnimationCoordinatorFactory.h>
#include <BgmapTexture.h>
#include <DebugConfig.h>
#include <VIPManager.h>

#include "MBgmapAnimatedSprite.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapAnimatedSprite::constructor(Entity owner, const MBgmapAnimatedSpriteSpec* mBgmapAnimatedSpriteSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &mBgmapAnimatedSpriteSpec->mBgmapSpriteSpec);

	ASSERT(this->texture, "MBgmapAnimatedSprite::constructor: null texture");

	MBgmapAnimatedSprite::createAnimationController(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MBgmapAnimatedSprite::destructor()
{
	if(this->animationController)
	{
		delete this->animationController;
		this->animationController = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
