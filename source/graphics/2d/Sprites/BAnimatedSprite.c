/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BAnimatedSprite.h>
#include <Game.h>
#include <string.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the BAnimatedSprite
__CLASS_DEFINITION(BAnimatedSprite, BSprite);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern int strcmp(const char *, const char *);

// class's constructor
static void BAnimatedSprite_constructor(BAnimatedSprite this, const SpriteDefinition* spriteDefinition, Object owner);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BAnimatedSprite, const SpriteDefinition* spriteDefinition, Object owner)
__CLASS_NEW_END(BAnimatedSprite, spriteDefinition, owner);

// class's constructor
static void BAnimatedSprite_constructor(BAnimatedSprite this, const SpriteDefinition* spriteDefinition, Object owner)
{
	// construct base object
	__CONSTRUCT_BASE(spriteDefinition);

	this->animationController = __NEW(AnimationController, owner);
	
	// since the offset will be moved during animation, must save it
	this->originalTextureSource.mx = abs(BTexture_getXOffset(__UPCAST(BTexture, this->texture))) << 3;
	this->originalTextureSource.my = abs(BTexture_getYOffset(__UPCAST(BTexture, this->texture))) << 3;
}

//destructor
void BAnimatedSprite_destructor(BAnimatedSprite this)
{
	ASSERT(this, "BAnimatedSprite::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

// write char animation frame to char memory
void BAnimatedSprite_writeAnimation(BAnimatedSprite this)
{
	ASSERT(this, "BAnimatedSprite::writeAnimation: null this");

	// write according to the allocation type
	switch (CharSet_getAllocationType(Texture_getCharSet(this->texture)))
	{
		case __ANIMATED:

			{
				CharSet charSet = Texture_getCharSet(this->texture);

				// move charset's charset's definition to the next frame chars
				CharSet_setCharDefinitionDisplacement(charSet, Texture_getNumberOfChars(this->texture) *
						(AnimationController_getActualFrameIndex(this->animationController) << 4));

				//write charset
				CharSet_write(charSet);
			}

			break;

		case __ANIMATED_SHARED:
			{
				int totalColumns = 64 - (this->originalTextureSource.mx >> 3); 
				int frameColumn = Texture_getCols(this->texture) * AnimationController_getActualFrameIndex(this->animationController);
				this->drawSpec.textureSource.mx = this->originalTextureSource.mx + ((frameColumn % totalColumns) << 3);
				this->drawSpec.textureSource.my = this->originalTextureSource.my + ((frameColumn / totalColumns) << 3);
			}
			
			BSprite_invalidateParamTable(__UPCAST(BSprite, this));
			this->renderFlag |= __UPDATE_M;
			break;
	}
}
