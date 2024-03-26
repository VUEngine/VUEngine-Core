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

#include <AnimationController.h>
#include <BgmapTexture.h>
#include <DebugUtilities.h>
#include <Texture.h>
#include <VIPManager.h>

#include "FrameBlendBgmapSprite.h"


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
void FrameBlendBgmapSprite::constructor(SpatialObject owner, const FrameBlendBgmapSpriteSpec* frameBlendBgmapSpriteSpec)
{
	// construct base object
	Base::constructor(owner, &frameBlendBgmapSpriteSpec->bgmapSpriteSpec);

	this->actualFrame = 0;

	ASSERT(this->texture, "FrameBlendBgmapSprite::constructor: null texture");
}

/**
 * Class destructor
 */
void FrameBlendBgmapSprite::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

int16 FrameBlendBgmapSprite::doRender(int16 index)
{
	FrameBlendBgmapSprite::swapFrame(this);

	return Base::doRender(this, index);
}

void FrameBlendBgmapSprite::swapFrame()
{
	this->actualFrame = 0 == this->actualFrame ? 1 : 0;

	int16 mx = BgmapTexture::getXOffset(this->texture) + Texture::getCols(this->texture) * this->actualFrame;
	int16 my = BgmapTexture::getYOffset(this->texture) + Texture::getRows(this->texture) * (mx / 64);
	this->bgmapTextureSource.mx = __MODULO(mx, 64) << 3;
	this->bgmapTextureSource.my = my << 3;
	this->rendered = false;
}