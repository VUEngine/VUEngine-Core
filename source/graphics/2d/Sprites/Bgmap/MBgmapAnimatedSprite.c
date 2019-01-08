/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#include <MBgmapAnimatedSprite.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <ParamTableManager.h>
#include <Optics.h>
#include <Camera.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

void MBgmapAnimatedSprite::constructor(const MBgmapSpriteSpec* mBgmapSpriteSpec, Object owner)
{
	Base::constructor(mBgmapSpriteSpec, owner);

	ASSERT(this->texture, "MBgmapAnimatedSprite::constructor: null texture");

    this->animationController = new AnimationController(owner, Sprite::safeCast(this), mBgmapSpriteSpec->textureSpecs[0]->charSetSpec);

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

	// write according to the allocation type
	switch(CharSet::getAllocationType(charSet))
	{
		case __ANIMATED_SINGLE_OPTIMIZED:
			{
				// move charset spec to the next frame chars
				CharSet::setCharSpecDisplacement(charSet, Texture::getNumberOfChars(this->texture) *
						AnimationController::getActualFrameIndex(this->animationController));

				BgmapTexture bgmapTexture = BgmapTexture::safeCast(this->texture);

				// move map spec to the next frame
				Texture::setMapDisplacement(this->texture, Texture::getCols(this->texture) * Texture::getRows(this->texture) *
						(AnimationController::getActualFrameIndex(this->animationController) << 1));

				CharSet::write(charSet);
				BgmapTexture::rewrite(bgmapTexture);
			}
			break;

		case __ANIMATED_SINGLE:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
			{
				// move charset spec to the next frame chars
				CharSet::setCharSpecDisplacement(charSet, Texture::getNumberOfChars(this->texture) *
						AnimationController::getActualFrameIndex(this->animationController));

				// write charset
				CharSet::write(charSet);
			}
			break;

		case __ANIMATED_MULTI:
			{
				int totalColumns = 64 - (this->originalTextureSource.mx / 8);
				s32 frameColumn = Texture::getCols(this->texture) * AnimationController::getActualFrameIndex(this->animationController);
				this->drawSpec.textureSource.mx = this->originalTextureSource.mx + ((frameColumn % totalColumns) << 3);
				this->drawSpec.textureSource.my = this->originalTextureSource.my + ((frameColumn / totalColumns) << 3);
			}

			BgmapSprite::invalidateParamTable(this);
			break;
	}
}
