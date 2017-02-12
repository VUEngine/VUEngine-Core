/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#include <ObjectAnimatedSprite.h>
#include <ObjectSpriteContainer.h>
#include <AnimationController.h>
#include <ObjectTexture.h>
#include <Game.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	ObjectAnimatedSprite
 * @extends ObjectSprite
 * @ingroup graphics-2d-sprites-object
 */
__CLASS_DEFINITION(ObjectAnimatedSprite, ObjectSprite);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern int strcmp(const char *, const char *);

static void ObjectAnimatedSprite_constructor(ObjectAnimatedSprite this, const ObjectSpriteDefinition* objectSpriteDefinition, Object owner);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ObjectAnimatedSprite, const ObjectSpriteDefinition* objectSpriteDefinition, Object owner)
__CLASS_NEW_END(ObjectAnimatedSprite, objectSpriteDefinition, owner);

/**
 * Class constructor
 *
 * @memberof						ObjectAnimatedSprite
 * @public
 *
 * @param this						Function scope
 * @param bgmapSpriteDefinition		Sprite definition
 * @param owner						Owner
 */
static void ObjectAnimatedSprite_constructor(ObjectAnimatedSprite this, const ObjectSpriteDefinition* objectSpriteDefinition, Object owner)
{
	ASSERT(this, "ObjectAnimatedSprite::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(ObjectSprite, (const ObjectSpriteDefinition*)objectSpriteDefinition, owner);

	this->animationController = __NEW(AnimationController, owner, __SAFE_CAST(Sprite, this), objectSpriteDefinition->spriteDefinition.textureDefinition->charSetDefinition);
}

/**
 * Class destructor
 *
 * @memberof						ObjectAnimatedSprite
 * @public
 *
 * @param this						Function scope
 */
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

/**
 * Write animation
 *
 * @memberof		ObjectAnimatedSprite
 * @public
 *
 * @param this		Function scope
 */
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
	switch(CharSet_getAllocationType(Texture_getCharSet(this->texture, true)))
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
			{
				CharSet charSet = Texture_getCharSet(this->texture, true);

				// move charset definition to the next frame chars
				CharSet_setCharDefinitionDisplacement(charSet, Texture_getNumberOfChars(this->texture) * (animationFrame << 4));

				// write charset
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
