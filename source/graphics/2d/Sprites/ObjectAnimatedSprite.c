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

#include <ObjectAnimatedSprite.h>
#include <ObjectSpriteContainer.h>
#include <ObjectTexture.h>
#include <Game.h>
#include <string.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ObjectAnimatedSprite
__CLASS_DEFINITION(ObjectAnimatedSprite, ObjectSprite);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern int strcmp(const char *, const char *);

// class's constructor
static void ObjectAnimatedSprite_constructor(ObjectAnimatedSprite this, const SpriteDefinition* spriteDefinition, Object owner);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectAnimatedSprite, const SpriteDefinition* spriteDefinition, Object owner)
__CLASS_NEW_END(ObjectAnimatedSprite, spriteDefinition, owner);

// class's constructor
static void ObjectAnimatedSprite_constructor(ObjectAnimatedSprite this, const SpriteDefinition* spriteDefinition, Object owner)
{
	// construct base object
	__CONSTRUCT_BASE(spriteDefinition);

	this->animationController = __NEW(AnimationController, owner);
}

//destructor
void ObjectAnimatedSprite_destructor(ObjectAnimatedSprite this)
{
	ASSERT(this, "ObjectAnimatedSprite::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

// write char animation frame to char memory
void ObjectAnimatedSprite_writeAnimation(ObjectAnimatedSprite this)
{
	ASSERT(this, "ObjectAnimatedSprite::writeAnimation: null this");
	
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

			ObjectTexture_addBgmapDisplacement(__UPCAST(ObjectTexture, this->texture), AnimationController_getActualFrameIndex(this->animationController));
			ObjectTexture_write(__UPCAST(ObjectTexture, this->texture));
			ObjectTexture_resetBgmapDisplacement(__UPCAST(ObjectTexture, this->texture));
			break;
	}
}
