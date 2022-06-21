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

#include <string.h>

#include <BgmapAnimatedSprite.h>
#include <AnimationController.h>
#include <BgmapTextureManager.h>
#include <AnimationCoordinatorFactory.h>
#include <debugUtilities.h>


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
 * @param bgmapSpriteSpec		Sprite spec
 * @param owner						Owner
 */
void BgmapAnimatedSprite::constructor(const BgmapAnimatedSpriteSpec* bgmapAnimatedSpriteSpec, ListenerObject owner)
{
	// construct base object
	Base::constructor(&bgmapAnimatedSpriteSpec->bgmapSpriteSpec, owner);

	ASSERT(this->texture, "BgmapAnimatedSprite::constructor: null texture");

    this->animationController = new AnimationController();

	AnimationController::setAnimationCoordinator(
		this->animationController, 
		AnimationCoordinatorFactory::getCoordinator(
			AnimationCoordinatorFactory::getInstance(), 
			this->animationController, 
			owner, 
			bgmapAnimatedSpriteSpec->bgmapSpriteSpec.spriteSpec.textureSpec->charSetSpec
			)
	);


    // since the offset will be moved during animation, must save it
    this->originalTextureSource.mx = BgmapTexture::getXOffset(this->texture) << 3;
    this->originalTextureSource.my = BgmapTexture::getYOffset(this->texture) << 3;
}

/**
 * Class destructor
 */
void BgmapAnimatedSprite::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Write animation
 */
void BgmapAnimatedSprite::writeAnimation()
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

			BgmapAnimatedSprite::setFrameAnimatedMulti(this, AnimationController::getActualFrameIndex(this->animationController));
			BgmapAnimatedSprite::invalidateParamTable(this);
			break;

		default:

			Texture::setFrame(this->texture, AnimationController::getActualFrameIndex(this->animationController));
			break;
	}
}

void BgmapAnimatedSprite::setFrameAnimatedMulti(uint16 frame)
{
	int32 totalColumns = 64 - (this->originalTextureSource.mx / 8);
	int32 frameColumn = Texture::getCols(this->texture) * frame;
	this->drawSpec.textureSource.mx = this->originalTextureSource.mx + ((frameColumn % totalColumns) << 3);
	this->drawSpec.textureSource.my = this->originalTextureSource.my + ((frameColumn / totalColumns) << 3);
}