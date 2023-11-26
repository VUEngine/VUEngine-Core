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

#include <MBgmapAnimatedSprite.h>

#include <AnimationController.h>
#include <AnimationCoordinatorFactory.h>
#include <BgmapTexture.h>
#include <VIPManager.h>

#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

void MBgmapAnimatedSprite::constructor(const MBgmapAnimatedSpriteSpec* mBgmapAnimatedSpriteSpec, ListenerObject owner)
{
	Base::constructor(&mBgmapAnimatedSpriteSpec->mBgmapSpriteSpec, owner);

	ASSERT(this->texture, "MBgmapAnimatedSprite::constructor: null texture");

    this->animationController = new AnimationController();

	AnimationController::setAnimationCoordinator
	(
		this->animationController, 
		AnimationCoordinatorFactory::getCoordinator
		(
			AnimationCoordinatorFactory::getInstance(),
			this->animationController, 
			owner, 
			mBgmapAnimatedSpriteSpec->mBgmapSpriteSpec.textureSpecs[0]->charSetSpec
		)
	);
}

void MBgmapAnimatedSprite::destructor()
{
	if(this->animationController)
	{
		delete this->animationController;
		this->animationController = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void MBgmapAnimatedSprite::writeAnimation()
{
	CharSet charSet = Texture::getCharSet(this->texture, true);

	NM_ASSERT(!isDeleted(charSet), "MBgmapAnimatedSprite::writeAnimation: deleted charset");
	NM_ASSERT(!isDeleted(this->animationController), "MBgmapAnimatedSprite::writeAnimation: null animation controller");

	if(isDeleted(charSet) || isDeleted(this->animationController))
	{
		return;
	}

	if(1 < Texture::getNumberOfFrames(this->texture))
	{
		MBgmapAnimatedSprite::setFrameAnimatedMulti(this, AnimationController::getActualFrameIndex(this->animationController));
		MBgmapAnimatedSprite::invalidateParamTable(this);
	}
	else
	{
		Texture::setFrame(this->texture, AnimationController::getActualFrameIndex(this->animationController));
	}
}

void MBgmapAnimatedSprite::setFrameAnimatedMulti(uint16 frame)
{
	int16 mx = BgmapTexture::getXOffset(this->texture);
	int16 my = BgmapTexture::getYOffset(this->texture);
	int32 totalColumns = 64 - mx;
	int32 frameColumn = Texture::getCols(this->texture) * frame;
	this->textureSource.mx = (mx + (frameColumn % totalColumns)) << 3;
	this->textureSource.my = (my + (frameColumn % totalColumns)) << 3;;
}