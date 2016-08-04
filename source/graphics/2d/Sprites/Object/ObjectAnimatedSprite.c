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

static void ObjectAnimatedSprite_constructor(ObjectAnimatedSprite this, const ObjectSpriteDefinition* oSpriteDefinition, Object owner);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectAnimatedSprite, const ObjectSpriteDefinition* oSpriteDefinition, Object owner)
__CLASS_NEW_END(ObjectAnimatedSprite, oSpriteDefinition, owner);

// class's constructor
static void ObjectAnimatedSprite_constructor(ObjectAnimatedSprite this, const ObjectSpriteDefinition* oSpriteDefinition, Object owner)
{
	ASSERT(this, "ObjectAnimatedSprite::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(ObjectSprite, (const ObjectSpriteDefinition*)oSpriteDefinition, owner);

	this->animationController = __NEW(AnimationController, owner, __SAFE_CAST(Sprite, this), oSpriteDefinition->spriteDefinition.textureDefinition->charSetDefinition);
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
	// must always be called at the end of the destructor
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

	// force rendering
	this->renderFlag = true;

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

			ObjectTexture_resetBgmapDisplacement(__SAFE_CAST(ObjectTexture, this->texture));
			ObjectTexture_addBgmapDisplacement(__SAFE_CAST(ObjectTexture, this->texture), animationFrame);
			ObjectTexture_write(__SAFE_CAST(ObjectTexture, this->texture));
			break;
	}
}
