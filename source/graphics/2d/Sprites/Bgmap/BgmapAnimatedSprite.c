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
#include <AnimationCoordinatorFactory.h>
#include <BgmapTexture.h>
#include <Texture.h>
#include <VIPManager.h>

#include <debugUtilities.h>
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
void BgmapAnimatedSprite::constructor(const BgmapAnimatedSpriteSpec* bgmapAnimatedSpriteSpec, ListenerObject owner)
{
	// construct base object
	Base::constructor(&bgmapAnimatedSpriteSpec->bgmapSpriteSpec, owner);

	ASSERT(this->texture, "BgmapAnimatedSprite::constructor: null texture");

    this->animationController = new AnimationController();

	AnimationController::setAnimationCoordinator
	(
		this->animationController, 
		AnimationCoordinatorFactory::getCoordinator
		(
			AnimationCoordinatorFactory::getInstance(),
			this->animationController, 
			owner, 
			bgmapAnimatedSpriteSpec->bgmapSpriteSpec.spriteSpec.textureSpec->charSetSpec
		)
	);
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

	NM_ASSERT(!isDeleted(charSet), "BgmapAnimatedSprite::writeAnimation: deleted charset");
	NM_ASSERT(!isDeleted(this->animationController), "BgmapAnimatedSprite::writeAnimation: null animation controller");

	if(isDeleted(charSet) || isDeleted(this->animationController))
	{
		return;
	}

	if(kCharSetSharedMulti == CharSet::getSharingScheme(charSet))
	{
		BgmapAnimatedSprite::setFrameAnimatedMulti(this, AnimationController::getActualFrameIndex(this->animationController));
		BgmapAnimatedSprite::invalidateParamTable(this);
	}
	else
	{
		Texture::setFrame(this->texture, AnimationController::getActualFrameIndex(this->animationController));
	}
}

void BgmapAnimatedSprite::setFrameAnimatedMulti(uint16 frame)
{
	int16 mx = BgmapTexture::getXOffset(this->texture) + Texture::getCols(this->texture) * frame;
	int16 my = BgmapTexture::getYOffset(this->texture) + Texture::getRows(this->texture) * (mx / 64);
	this->textureSource.mx = __MODULO(mx, 64) << 3;
	this->textureSource.my = my << 3;
}