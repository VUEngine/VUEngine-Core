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
#include <BgmapTexture.h>
#include <Texture.h>
#include <VIPManager.h>

#include "BgmapAnimatedSprite.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void BgmapAnimatedSprite::constructor(SpatialObject owner, const BgmapAnimatedSpriteSpec* bgmapAnimatedSpriteSpec)
{
	NM_ASSERT(NULL != bgmapAnimatedSpriteSpec, "BgmapAnimatedSprite::constructor: NULL bgmapAnimatedSpriteSpec");

	Base::constructor(owner, &bgmapAnimatedSpriteSpec->bgmapSpriteSpec);

	ASSERT(this->texture, "BgmapAnimatedSprite::constructor: null texture");

	BgmapAnimatedSprite::createAnimationController(this);
}
//---------------------------------------------------------------------------------------------------------
void BgmapAnimatedSprite::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void BgmapAnimatedSprite::updateAnimation()
{
	NM_ASSERT(!isDeleted(this->animationController), "BgmapAnimatedSprite::updateAnimation: null animation controller");

	if(isDeleted(this->animationController))
	{
		return;
	}

	if(Texture::isMultiframe(this->texture))
	{
		BgmapAnimatedSprite::setMultiframe(this, AnimationController::getActualFrameIndex(this->animationController));
		BgmapAnimatedSprite::invalidateParamTable(this);
	}
	else
	{
		Texture::setFrame(this->texture, AnimationController::getActualFrameIndex(this->animationController));
	}
}
//---------------------------------------------------------------------------------------------------------
