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

#include <string.h>

#include <ObjectAnimatedSprite.h>
#include <ObjectSpriteContainer.h>
#include <AnimationController.h>
#include <ObjectTexture.h>
#include <Game.h>


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

	this->animationController = __NEW(AnimationController, owner, __GET_CAST(Sprite, this), Texture_getCharSet(this->texture));
}

//destructor
void ObjectAnimatedSprite_destructor(ObjectAnimatedSprite this)
{
	ASSERT(this, "ObjectAnimatedSprite::destructor: null this");

	if(this->animationController)
	{
		__DELETE(this->animationController);
		this->animationController = NULL;
	}

	// destroy the super object
	__DESTROY_BASE;
}

// write char animation frame to char memory
void ObjectAnimatedSprite_writeAnimation(ObjectAnimatedSprite this)
{
	ASSERT(this, "ObjectAnimatedSprite::writeAnimation: null this");
	
	if(0 > this->objectIndex)
	{
		return;
	}
	
	int animationFrame = (int)AnimationController_getActualFrameIndex(this->animationController);
	
	if(0 > animationFrame)
	{
		return;
	}
	
	// write according to the allocation type
	switch(CharSet_getAllocationType(Texture_getCharSet(this->texture)))
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
			{
				CharSet charSet = Texture_getCharSet(this->texture);

				// move charset's charset's definition to the next frame chars
				CharSet_setCharDefinitionDisplacement(charSet, Texture_getNumberOfChars(this->texture) *
						(animationFrame << 4));

				//write charset
				CharSet_write(charSet);
			}

			break;

		case __ANIMATED_MULTI:

			ObjectTexture_resetBgmapDisplacement(__GET_CAST(ObjectTexture, this->texture));
			ObjectTexture_addBgmapDisplacement(__GET_CAST(ObjectTexture, this->texture), animationFrame);
			ObjectTexture_write(__GET_CAST(ObjectTexture, this->texture));
			break;
	}
}
