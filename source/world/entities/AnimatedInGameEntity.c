/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimatedInGameEntity.h>
#include <Clock.h>
#include <MessageDispatcher.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Screen.h>
#include <Shape.h>
#include <PhysicalWorld.h>
#include <Body.h>
#include <Cuboid.h>
#include <Prototypes.h>
#include <Game.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(AnimatedInGameEntity, InGameEntity);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void AnimatedInGameEntity_doProcessListeners(AnimatedInGameEntity this, void (*function)(Object this, Object listener, void (*method)(Object, Object),  char* eventName));
static void AnimatedInGameEntity_addListeners(AnimatedInGameEntity this);
static void AnimatedInGameEntity_removeListeners(AnimatedInGameEntity this);
static void AnimatedInGameEntity_animate(AnimatedInGameEntity this);
static void AnimatedInGameEntity_onFrameChanged(AnimatedInGameEntity this, Object firer);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(AnimatedInGameEntity, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 id, const char* const name)
__CLASS_NEW_END(AnimatedInGameEntity, animatedInGameEntityDefinition, id, name);

// AnimatedInGameEntity.c
// class's constructor
void AnimatedInGameEntity_constructor(AnimatedInGameEntity this, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 id, const char* const name)
{
	ASSERT(this, "AnimatedInGameEntity::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(&animatedInGameEntityDefinition->inGameEntityDefinition, id, name);

	// save ROM definition
	this->animatedInGameEntityDefinition = animatedInGameEntityDefinition;
	this->animationDescription = animatedInGameEntityDefinition->animationDescription;

	//set the direction
	this->direction.x = __RIGHT;
	this->direction.y = __DOWN;
	this->direction.z = __FAR;

	this->previousDirection = this->direction;

	this->clock = Game_getInGameClock(Game_getInstance());

	this->currentAnimationName = NULL;
	this->animationFrameChanged = false;
}

// class's destructor
void AnimatedInGameEntity_destructor(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::destructor: null this");

	AnimatedInGameEntity_removeListeners(this);

	// destroy the super object
	__DESTROY_BASE;
}

// initialize method
void AnimatedInGameEntity_initialize(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::initialize: null this");
	ASSERT(this->animatedInGameEntityDefinition, "AnimatedInGameEntity::initialize: null animatedInGameEntityDefinition");

	Entity_initialize(__GET_CAST(Entity, this));

	AnimatedInGameEntity_addListeners(this);

	AnimatedInGameEntity_playAnimation(this, this->animatedInGameEntityDefinition->initialAnimation);
}

// add listeners to sprites
static void AnimatedInGameEntity_doProcessListeners(AnimatedInGameEntity this, void (*function)(Object this, Object listener, void (*method)(Object, Object),  char* eventName))
{
	if(this->sprites)
	{
		VirtualNode node = VirtualList_begin(this->sprites);
	
		// setup listeners
		for(; node ; node = VirtualNode_getNext(node))
	    {
			Sprite sprite = __GET_CAST(Sprite, VirtualNode_getData(node));
	
			function(__GET_CAST(Object, sprite), __GET_CAST(Object, this), (void (*)(Object, Object))AnimatedInGameEntity_onFrameChanged, __EVENT_ANIMATION_FRAME_CHANGED);
		}
	}
}

// add listeners to sprites
static void AnimatedInGameEntity_addListeners(AnimatedInGameEntity this)
{
	AnimatedInGameEntity_doProcessListeners(this, Object_addEventListener);
}

// remove listeners to sprites
static void AnimatedInGameEntity_removeListeners(AnimatedInGameEntity this)
{
	AnimatedInGameEntity_doProcessListeners(this, Object_removeEventListener);
}

// called when one sprite changed its animation
static void AnimatedInGameEntity_onFrameChanged(AnimatedInGameEntity this, Object firer)
{
	this->animationFrameChanged = true;
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
		Entity_setSpritesDirection(__GET_CAST(Entity, this), __XAXIS, this->direction.x);

		// save current direction
		this->previousDirection = this->direction;
	}

	// call base
	Entity_transform(__GET_CAST(Entity, this), environmentTransform);
}

// execute character's logic
void AnimatedInGameEntity_update(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::update: null this");

	// call base
	Container_update(__GET_CAST(Container, this));

	// if direction changed
	if(this->direction.x != this->previousDirection.x)
	{
		ASSERT(this->sprites, "AnimatedInGameEntity::update: null sprites");

		// calculate gap again
		InGameEntity_setGap(__GET_CAST(InGameEntity, this));
	}

	if(this->sprites)
	{
		this->animationFrameChanged = false;
		AnimatedInGameEntity_animate(this);
	}
}

// update animations
static void AnimatedInGameEntity_animate(AnimatedInGameEntity this)
{
	VirtualNode node = VirtualList_begin(this->sprites);

	// move each child to a temporary list
	for(; node ; node = VirtualNode_getNext(node))
	{
		// first animate the frame
		Sprite_update(__GET_CAST(Sprite, VirtualNode_getData(node)), this->clock);
	}
}

// pause animation
void AnimatedInGameEntity_pauseAnimation(AnimatedInGameEntity this, int pause)
{
	ASSERT(this, "AnimatedInGameEntity::pauseAnimation: null this");
	ASSERT(this->sprites, "AnimatedInGameEntity::pauseAnimation: null sprites");

	if(this->sprites)
	{
		VirtualNode node = VirtualList_begin(this->sprites);

		// play animation on each sprite
		for(; node ; node = VirtualNode_getNext(node))
	    {
			Sprite_pause(__GET_CAST(Sprite, VirtualNode_getData(node)), pause);
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
		
		VirtualNode node = VirtualList_begin(this->sprites);

		// play animation on each sprite
		for(; node ; node = VirtualNode_getNext(node))
	    {
			Sprite_play(__GET_CAST(Sprite, VirtualNode_getData(node)), this->animationDescription, animationName);
		}
	}
}

// is play an animation
bool AnimatedInGameEntity_isPlayingAnimation(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::isPlayingAnimation: null this");
	ASSERT(this->sprites, "AnimatedInGameEntity::isPlayingAnimation: null sprites");

	return Sprite_isPlaying(__GET_CAST(Sprite, VirtualNode_getData(VirtualList_begin(this->sprites))));
}

// is animation selected
bool AnimatedInGameEntity_isAnimationLoaded(AnimatedInGameEntity this, char* functionName)
{
	ASSERT(this, "AnimatedInGameEntity::isAnimationLoaded: null this");

	if(this->sprites)
	{
		Sprite sprite = __GET_CAST(Sprite, VirtualNode_getData(VirtualList_begin(this->sprites)));

		return Sprite_isPlayingFunction(__GET_CAST(Sprite, sprite), this->animationDescription, functionName);
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

// set animation clock
void AnimatedInGameEntity_setClock(AnimatedInGameEntity this, Clock clock)
{
	ASSERT(this, "AnimatedInGameEntity::setClock: null this");

	this->clock = clock;
}

// resume method
void AnimatedInGameEntity_resume(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::resume: null this");

	Entity_resume(__GET_CAST(Entity, this));

	Entity_setSpritesDirection(__GET_CAST(Entity, this), __XAXIS, this->direction.x);

	AnimatedInGameEntity_playAnimation(this, this->currentAnimationName);
	
	AnimatedInGameEntity_addListeners(this);
}

