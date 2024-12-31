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


#include <AnimationController.h>
#include <AnimationCoordinatorFactory.h>
#include <BgmapTexture.h>
#include <DebugConfig.h>
#include <VIPManager.h>

#include "MBgmapAnimatedSprite.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void MBgmapAnimatedSprite::constructor(GameObject owner, const MBgmapAnimatedSpriteSpec* mBgmapAnimatedSpriteSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &mBgmapAnimatedSpriteSpec->mBgmapSpriteSpec);

	ASSERT(this->texture, "MBgmapAnimatedSprite::constructor: null texture");

	MBgmapAnimatedSprite::createAnimationController(this);
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void MBgmapAnimatedSprite::updateAnimation()
{
	CharSet charSet = Texture::getCharSet(this->texture, true);

	NM_ASSERT(!isDeleted(charSet), "MBgmapAnimatedSprite::updateAnimation: deleted charset");
	NM_ASSERT(!isDeleted(this->animationController), "MBgmapAnimatedSprite::updateAnimation: null animation controller");

	if(isDeleted(charSet) || isDeleted(this->animationController))
	{
		return;
	}

	if(Texture::isMultiframe(this->texture))
	{
		MBgmapAnimatedSprite::setMultiframe(this, AnimationController::getActualFrameIndex(this->animationController));
		MBgmapAnimatedSprite::invalidateParamTable(this);
	}
	else
	{
		Texture::setFrame(this->texture, AnimationController::getActualFrameIndex(this->animationController));
	}
}
//---------------------------------------------------------------------------------------------------------
void MBgmapAnimatedSprite::setMultiframe(uint16 frame)
{
	int16 mx = BgmapTexture::getXOffset(this->texture);
	int16 my = BgmapTexture::getYOffset(this->texture);
	int32 totalColumns = 64 - mx;
	int32 frameColumn = Texture::getCols(this->texture) * frame;
	this->bgmapTextureSource.mx = (mx + (frameColumn % totalColumns)) << 3;
	this->bgmapTextureSource.my = (my + (frameColumn % totalColumns)) << 3;
}
//---------------------------------------------------------------------------------------------------------
