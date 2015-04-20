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

#include <OAnimatedSprite.h>
#include <OMegaSprite.h>
#include <OTexture.h>
#include <Game.h>
#include <string.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the OAnimatedSprite
__CLASS_DEFINITION(OAnimatedSprite, OSprite);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern int strcmp(const char *, const char *);

// class's constructor
static void OAnimatedSprite_constructor(OAnimatedSprite this, const SpriteDefinition* spriteDefinition, Object owner);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(OAnimatedSprite, const SpriteDefinition* spriteDefinition, Object owner)
__CLASS_NEW_END(OAnimatedSprite, spriteDefinition, owner);

// class's constructor
static void OAnimatedSprite_constructor(OAnimatedSprite this, const SpriteDefinition* spriteDefinition, Object owner)
{
	// construct base object
	__CONSTRUCT_BASE(spriteDefinition);

	this->animationController = __NEW(AnimationController, owner);
}

//destructor
void OAnimatedSprite_destructor(OAnimatedSprite this)
{
	ASSERT(this, "OAnimatedSprite::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

// write char animation frame to char memory
void OAnimatedSprite_writeAnimation(OAnimatedSprite this)
{
	ASSERT(this, "OAnimatedSprite::writeAnimation: null this");

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
				
//				OTexture_write(this->texture);
			}

			break;

		case __ANIMATED_SHARED:
			{
			}
			
			this->renderFlag |= __UPDATE_M;
			break;
	}
}
