/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

extern int strcmp(const char *, const char *);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param bgmapSpriteSpec		Sprite spec
 * @param owner						Owner
 */
void BgmapAnimatedSprite::constructor(const BgmapAnimatedSpriteSpec* bgmapAnimatedSpriteSpec, Object owner)
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

void BgmapAnimatedSprite::setFrameAnimatedMulti(u16 frame)
{
	int totalColumns = 64 - (this->originalTextureSource.mx / 8);
	s32 frameColumn = Texture::getCols(this->texture) * frame;
	this->drawSpec.textureSource.mx = this->originalTextureSource.mx + ((frameColumn % totalColumns) << 3);
	this->drawSpec.textureSource.my = this->originalTextureSource.my + ((frameColumn / totalColumns) << 3);
}