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

#include <BgmapAnimatedSprite.h>

#include <AnimationController.h>
#include <BgmapTexture.h>
#include <Texture.h>
#include <VIPManager.h>

#include <DebugUtilities.h>
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
 * @param bgmapSpriteSpec		Sprite spec
 * @param owner						Owner
 */
void BgmapAnimatedSprite::constructor(SpatialObject owner, const BgmapAnimatedSpriteSpec* bgmapAnimatedSpriteSpec)
{
	// construct base object
	Base::constructor(owner, &bgmapAnimatedSpriteSpec->bgmapSpriteSpec);

	ASSERT(this->texture, "BgmapAnimatedSprite::constructor: null texture");

	BgmapAnimatedSprite::createAnimationController(this, bgmapAnimatedSpriteSpec->bgmapSpriteSpec.spriteSpec.textureSpec->charSetSpec);
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
	NM_ASSERT(!isDeleted(this->animationController), "BgmapAnimatedSprite::writeAnimation: null animation controller");

	if(isDeleted(this->animationController))
	{
		return;
	}

	if(Texture::isMultiframe(this->texture))
	{
		BgmapAnimatedSprite::setFrame(this, AnimationController::getActualFrameIndex(this->animationController));
		BgmapAnimatedSprite::invalidateParamTable(this);
	}
	else
	{
		Texture::setFrame(this->texture, AnimationController::getActualFrameIndex(this->animationController));
	}
}

void BgmapAnimatedSprite::setFrame(uint16 frame)
{
	int16 mx = BgmapTexture::getXOffset(this->texture) + Texture::getCols(this->texture) * frame;
	int16 my = BgmapTexture::getYOffset(this->texture) + Texture::getRows(this->texture) * (mx / 64);
	this->bgmapTextureSource.mx = __MODULO(mx, 64) << 3;
	this->bgmapTextureSource.my = my << 3;
	this->rendered = false;
}