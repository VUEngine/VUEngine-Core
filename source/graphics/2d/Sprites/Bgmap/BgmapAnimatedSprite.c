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

#include <string.h>

#include <BgmapAnimatedSprite.h>
#include <AnimationController.h>
#include <BgmapTextureManager.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	BgmapAnimatedSprite
 * @extends BgmapSprite
 * @ingroup graphics-2d-sprites-bgmap
 */



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
 * @memberof						BgmapAnimatedSprite
 * @public
 *
 * @param this						Function scope
 * @param bgmapSpriteDefinition		Sprite definition
 * @param owner						Owner
 */
void BgmapAnimatedSprite::constructor(const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner)
{
	// construct base object
	Base::constructor(bgmapSpriteDefinition, owner);

	ASSERT(this->texture, "BgmapAnimatedSprite::constructor: null texture");

    this->animationController = new AnimationController(owner, __SAFE_CAST(Sprite, this), bgmapSpriteDefinition->spriteDefinition.textureDefinition->charSetDefinition);

    // since the offset will be moved during animation, must save it
    this->originalTextureSource.mx = BgmapTexture::getXOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
    this->originalTextureSource.my = BgmapTexture::getYOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
}

/**
 * Class destructor
 *
 * @memberof						BgmapAnimatedSprite
 * @public
 *
 * @param this						Function scope
 */
void BgmapAnimatedSprite::destructor()
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

/**
 * Write animation
 *
 * @memberof		BgmapAnimatedSprite
 * @public
 *
 * @param this		Function scope
 */
void BgmapAnimatedSprite::writeAnimation()
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
				// move charset definition to the next frame chars
				CharSet::setCharDefinitionDisplacement(charSet, Texture::getNumberOfChars(this->texture) *
						((int)AnimationController::getActualFrameIndex(this->animationController) << 4));

				BgmapTexture bgmapTexture = __SAFE_CAST(BgmapTexture, this->texture);

				// move map definition to the next frame
				Texture::setMapDisplacement(this->texture, Texture::getCols(this->texture) * Texture::getRows(this->texture) *
						((int)AnimationController::getActualFrameIndex(this->animationController) << 1));

				CharSet::write(charSet);
				BgmapTexture::rewrite(bgmapTexture);
			}
			break;

		case __ANIMATED_SINGLE:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
			{
				// move charset definition to the next frame chars
				CharSet::setCharDefinitionDisplacement(charSet, Texture::getNumberOfChars(this->texture) *
						((int)AnimationController::getActualFrameIndex(this->animationController) << 4));

				// write charset
				CharSet::write(charSet);
			}
			break;

		case __ANIMATED_MULTI:
			{
				int totalColumns = 64 - (this->originalTextureSource.mx / 8);
				int frameColumn = Texture::getCols(this->texture) * AnimationController::getActualFrameIndex(this->animationController);
				this->drawSpec.textureSource.mx = this->originalTextureSource.mx + ((frameColumn % totalColumns) << 3);
				this->drawSpec.textureSource.my = this->originalTextureSource.my + ((frameColumn / totalColumns) << 3);
			}

			BgmapSprite::invalidateParamTable(__SAFE_CAST(BgmapSprite, this));
			break;
	}
}
