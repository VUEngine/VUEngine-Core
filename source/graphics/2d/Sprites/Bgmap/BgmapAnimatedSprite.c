/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <BgmapAnimatedSprite.h>
#include <AnimationController.h>
#include <BgmapTextureManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the BgmapAnimatedSprite
__CLASS_DEFINITION(BgmapAnimatedSprite, BgmapSprite);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern int strcmp(const char *, const char *);

static void BgmapAnimatedSprite_constructor(BgmapAnimatedSprite this, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BgmapAnimatedSprite, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner)
__CLASS_NEW_END(BgmapAnimatedSprite, bgmapSpriteDefinition, owner);

// class's constructor
static void BgmapAnimatedSprite_constructor(BgmapAnimatedSprite this, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner)
{
	ASSERT(this, "BgmapAnimatedSprite::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(BgmapSprite, bgmapSpriteDefinition, owner);

	ASSERT(this->texture, "BgmapAnimatedSprite::constructor: null texture");

    this->animationController = __NEW(AnimationController, owner, __SAFE_CAST(Sprite, this), bgmapSpriteDefinition->spriteDefinition.textureDefinition->charSetDefinition);

    // since the offset will be moved during animation, must save it
    this->originalTextureSource.mx = BgmapTexture_getXOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
    this->originalTextureSource.my = BgmapTexture_getYOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
}

//destructor
void BgmapAnimatedSprite_destructor(BgmapAnimatedSprite this)
{
	ASSERT(this, "BgmapAnimatedSprite::destructor: null this");

	if(this->animationController)
	{
		__DELETE(this->animationController);
		this->animationController = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// write char animation frame to char memory
void BgmapAnimatedSprite_writeAnimation(BgmapAnimatedSprite this)
{
	ASSERT(this, "BgmapAnimatedSprite::writeAnimation: null this");

	ASSERT(Texture_getCharSet(this->texture, true), "BgmapAnimatedSprite::writeAnimation: null charset");

	// write according to the allocation type
	switch(CharSet_getAllocationType(Texture_getCharSet(this->texture, true)))
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
			{
				CharSet charSet = Texture_getCharSet(this->texture, true);

				// move charset definition to the next frame chars
				CharSet_setCharDefinitionDisplacement(charSet, Texture_getNumberOfChars(this->texture) *
						(AnimationController_getActualFrameIndex(this->animationController) << 4));

				// write charset
				CharSet_write(charSet);
			}
			break;

		case __ANIMATED_MULTI:
			{
				int totalColumns = 64 - (this->originalTextureSource.mx >> 3);
				int frameColumn = Texture_getCols(this->texture) * AnimationController_getActualFrameIndex(this->animationController);
				this->drawSpec.textureSource.mx = this->originalTextureSource.mx + ((frameColumn % totalColumns) << 3);
				this->drawSpec.textureSource.my = this->originalTextureSource.my + ((frameColumn / totalColumns) << 3);
			}

			BgmapSprite_invalidateParamTable(__SAFE_CAST(BgmapSprite, this));
			break;
	}
}
