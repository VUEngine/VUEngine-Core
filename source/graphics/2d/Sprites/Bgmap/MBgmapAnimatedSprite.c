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
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <ParamTableManager.h>
#include <Optics.h>
#include <Camera.h>
#include <AnimationCoordinatorFactory.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

void MBgmapAnimatedSprite::constructor(const MBgmapAnimatedSpriteSpec* mBgmapAnimatedSpriteSpec, ListenerObject owner)
{
	Base::constructor(&mBgmapAnimatedSpriteSpec->mBgmapSpriteSpec, owner);

	ASSERT(this->texture, "MBgmapAnimatedSprite::constructor: null texture");

    this->animationController = new AnimationController();

	AnimationController::setAnimationCoordinator(
		this->animationController, 
		AnimationCoordinatorFactory::getCoordinator(
			AnimationCoordinatorFactory::getInstance(), 
			this->animationController, 
			owner, 
			mBgmapAnimatedSpriteSpec->mBgmapSpriteSpec.textureSpecs[0]->charSetSpec
			)
	);

    // since the offset will be moved during animation, must save it
    this->originalTextureSource.mx = BgmapTexture::getXOffset(this->texture) << 3;
    this->originalTextureSource.my = BgmapTexture::getYOffset(this->texture) << 3;
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

	ASSERT(charSet, "BgmapAnimatedSprite::writeAnimation: null charset");

	if(!charSet)
	{
		return;
	}

	switch(CharSet::getAllocationType(charSet))
	{
		case __ANIMATED_MULTI:

			MBgmapAnimatedSprite::setFrameAnimatedMulti(this, AnimationController::getActualFrameIndex(this->animationController));
			MBgmapAnimatedSprite::invalidateParamTable(this);
			break;

		default:

			Texture::setFrame(this->texture, AnimationController::getActualFrameIndex(this->animationController));
			break;
	}
}

void MBgmapAnimatedSprite::setFrameAnimatedMulti(uint16 frame)
{
	int32 totalColumns = 64 - (this->originalTextureSource.mx / 8);
	int32 frameColumn = Texture::getCols(this->texture) * frame;
	this->drawSpec.textureSource.mx = this->originalTextureSource.mx + ((frameColumn % totalColumns) << 3);
	this->drawSpec.textureSource.my = this->originalTextureSource.my + ((frameColumn / totalColumns) << 3);
}