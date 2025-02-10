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

#include <string.h>

#include <AnimationController.h>
#include <BgmapTexture.h>
#include <Texture.h>

#include "FrameBlendBgmapSprite.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameBlendBgmapSprite::constructor(Entity owner, const FrameBlendBgmapSpriteSpec* frameBlendBgmapSpriteSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &frameBlendBgmapSpriteSpec->bgmapSpriteSpec);

	this->actualFrame = 0;

	ASSERT(this->texture, "FrameBlendBgmapSprite::constructor: null texture");
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void FrameBlendBgmapSprite::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 FrameBlendBgmapSprite::doRender(int16 index)
{
	this->actualFrame = 0 == this->actualFrame ? 1 : 0;

	FrameBlendBgmapSprite::setMultiframe(this, this->actualFrame);

	return Base::doRender(this, index);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
