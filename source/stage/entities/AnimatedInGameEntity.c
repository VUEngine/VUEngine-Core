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
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimatedInGameEntity.h>
#include <Clock.h>
#include <MessageDispatcher.h>
#include <Optics.h>
#include <Screen.h>
#include <Shape.h>
#include <PhysicalWorld.h>
#include <Body.h>
#include <Cuboid.h>
#include <Prototypes.h>
#include <Game.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(AnimatedInGameEntity, InGameEntity);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void AnimatedInGameEntity_animate(AnimatedInGameEntity this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(AnimatedInGameEntity, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(AnimatedInGameEntity, animatedInGameEntityDefinition, id, internalId, name);

// AnimatedInGameEntity.c
// class's constructor
void AnimatedInGameEntity_constructor(AnimatedInGameEntity this, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "AnimatedInGameEntity::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(InGameEntity, &animatedInGameEntityDefinition->inGameEntityDefinition, id, internalId, name);

	// save ROM definition
	this->animatedInGameEntityDefinition = animatedInGameEntityDefinition;
	this->animationDescription = animatedInGameEntityDefinition->animationDescription;

	//set the direction
	this->direction.x = __RIGHT;
	this->direction.y = __DOWN;
	this->direction.z = __FAR;

	this->previousDirection = this->direction;

	this->currentAnimationName = NULL;
}

// class's destructor
void AnimatedInGameEntity_destructor(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// set definition
void AnimatedInGameEntity_setDefinition(AnimatedInGameEntity this, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition)
{
	ASSERT(this, "AnimatedInGameEntity::setDefinition: null this");
	ASSERT(animatedInGameEntityDefinition, "AnimatedInGameEntity::setDefinition: null definition");

	// save definition
	this->animatedInGameEntityDefinition = animatedInGameEntityDefinition;

	InGameEntity_setDefinition(__SAFE_CAST(InGameEntity, this), &animatedInGameEntityDefinition->inGameEntityDefinition);
}

// ready method
void AnimatedInGameEntity_ready(AnimatedInGameEntity this, u32 recursive)
{
	ASSERT(this, "AnimatedInGameEntity::ready: null this");
	ASSERT(this->animatedInGameEntityDefinition, "AnimatedInGameEntity::ready: null animatedInGameEntityDefinition");

	Entity_ready(__SAFE_CAST(Entity, this), recursive);

	AnimatedInGameEntity_playAnimation(this, this->animatedInGameEntityDefinition->initialAnimation);
}

// updates the animation attributes
// graphically refresh of characters that are visible
void AnimatedInGameEntity_transform(AnimatedInGameEntity this, const Transformation* environmentTransform)
{
	ASSERT(this, "AnimatedInGameEntity::transform: null this");

	// set sprite direction
	if(this->direction.x != this->previousDirection.x)
	{
		// change sprite's direction
		Entity_setSpritesDirection(__SAFE_CAST(Entity, this), __XAXIS, this->direction.x);

		// save current direction
		this->previousDirection.x = this->direction.x;
	}


	if(this->direction.y != this->previousDirection.y)
	{
		// change sprite's direction
		Entity_setSpritesDirection(__SAFE_CAST(Entity, this), __YAXIS, this->direction.y);

		// save current direction
		this->previousDirection.y = this->direction.y;
	}

    this->previousDirection.z = this->direction.z;

	// call base
	Entity_transform(__SAFE_CAST(Entity, this), environmentTransform);
}

// execute character's logic
void AnimatedInGameEntity_update(AnimatedInGameEntity this, u32 elapsedTime)
{
	ASSERT(this, "AnimatedInGameEntity::update: null this");

	// call base
	Container_update(__SAFE_CAST(Container, this), elapsedTime);

	// if direction changed
	if(this->direction.x != this->previousDirection.x ||
	   this->direction.y != this->previousDirection.y ||
	   this->direction.z != this->previousDirection.z
	)
	{
		ASSERT(this->sprites, "AnimatedInGameEntity::update: null sprites");

		// calculate gap again
		InGameEntity_setGap(__SAFE_CAST(InGameEntity, this));
	}

    if(elapsedTime)
    {
        AnimatedInGameEntity_animate(this);
    }
}

// update animations
static void AnimatedInGameEntity_animate(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::animate: null this");

	if(this->sprites)
	{
        VirtualNode node = this->sprites->head;

        // move each child to a temporary list
        for(; node ; node = node->next)
        {
            // first animate the frame
            Sprite_animate(__SAFE_CAST(Sprite, node->data));
        }
    }
}

// pause animation
void AnimatedInGameEntity_pauseAnimation(AnimatedInGameEntity this, int pause)
{
	ASSERT(this, "AnimatedInGameEntity::pauseAnimation: null this");
	ASSERT(this->sprites, "AnimatedInGameEntity::pauseAnimation: null sprites");

	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		// play animation on each sprite
		for(; node ; node = node->next)
	    {
			Sprite_pause(__SAFE_CAST(Sprite, node->data), pause);
		}
	}
}

// play an animation
void AnimatedInGameEntity_playAnimation(AnimatedInGameEntity this, char* animationName)
{
	ASSERT(this, "AnimatedInGameEntity::playAnimation: null this");

	if(this->sprites && animationName)
	{
		this->currentAnimationName = animationName;

		VirtualNode node = this->sprites->head;

		// play animation on each sprite
		for(; node ; node = node->next)
	    {
			Sprite_play(__SAFE_CAST(Sprite, node->data), this->animationDescription, animationName);
		}
	}
}

// is playing an animation
bool AnimatedInGameEntity_isPlayingAnimation(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::isPlayingAnimation: null this");
	ASSERT(this->sprites, "AnimatedInGameEntity::isPlayingAnimation: null sprites");

	return Sprite_isPlaying(__SAFE_CAST(Sprite, VirtualNode_getData(this->sprites->head)));
}

// is animation selected
bool AnimatedInGameEntity_isAnimationLoaded(AnimatedInGameEntity this, char* functionName)
{
	ASSERT(this, "AnimatedInGameEntity::isAnimationLoaded: null this");

	if(this->sprites)
	{
		Sprite sprite = __SAFE_CAST(Sprite, VirtualNode_getData(this->sprites->head));

		return Sprite_isPlayingFunction(__SAFE_CAST(Sprite, sprite), functionName);
	}

	return false;
}

// get animation definition
AnimationDescription* AnimatedInGameEntity_getAnimationDescription(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::getAnimationDescription: null this");

	return this->animationDescription;
}

// set animation description
void AnimatedInGameEntity_setAnimationDescription(AnimatedInGameEntity this, AnimationDescription* animationDescription)
{
	ASSERT(this, "AnimatedInGameEntity::setAnimationDescription: null this");

	this->animationDescription = animationDescription;
}

// resume method
void AnimatedInGameEntity_resume(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::resume: null this");

	Entity_resume(__SAFE_CAST(Entity, this));

	Entity_setSpritesDirection(__SAFE_CAST(Entity, this), __XAXIS, this->direction.x);

	AnimatedInGameEntity_playAnimation(this, this->currentAnimationName);
}

