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

#include <AnimatedInGameEntity.h>
#include <Clock.h>
#include <AnimatedSprite.h>
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

__CLASS_DEFINITION(AnimatedInGameEntity);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// update animations
static void AnimatedInGameEntity_animate(AnimatedInGameEntity this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(AnimatedInGameEntity, __PARAMETERS(AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 ID))
__CLASS_NEW_END(AnimatedInGameEntity, __ARGUMENTS(animatedInGameEntityDefinition, ID));

// AnimatedInGameEntity.c
// class's conctructor
void AnimatedInGameEntity_constructor(AnimatedInGameEntity this, AnimatedInGameEntityDefinition* animatedInGameEntityDefinition, s16 ID)
{
	ASSERT(this, "AnimatedInGameEntity::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(InGameEntity, __ARGUMENTS(&animatedInGameEntityDefinition->inGameEntityDefinition, ID));

	// save ROM definition
	this->animatedInGameEntityDefinition = animatedInGameEntityDefinition;
	this->animationDescription = animatedInGameEntityDefinition->animationDescription;

	//set the direction
	this->direction.x = __RIGHT;
	this->previousDirection.x = __LEFT;
	this->direction.y = __DOWN;
	this->direction.z = __FAR;

	this->clock = Game_getInGameClock(Game_getInstance());

	AnimatedInGameEntity_playAnimation(this, animatedInGameEntityDefinition->initialAnimation);
}

// class's destructor
void AnimatedInGameEntity_destructor(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::destructor: null this");

	// destroy the super object
	__DESTROY_BASE(InGameEntity);
}

// updates the animation attributes
// graphically refresh of characters that are visible
void AnimatedInGameEntity_transform(AnimatedInGameEntity this, Transformation* environmentTransform)
{
	ASSERT(this, "AnimatedInGameEntity::transform: null this");

	// set sprite direction
	if (this->direction.x != this->previousDirection.x)
{
		// change sprite's direction
		Entity_setSpritesDirection((Entity)this, __XAXIS, this->direction.x);

		// save current direction
		this->previousDirection = this->direction;
	}

	// call base
	Entity_transform((Entity)this, environmentTransform);
}

// execute character's logic
void AnimatedInGameEntity_update(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::update: null this");

	// call base
	Container_update((Container)this);

	// if direction changed
	if (this->direction.x != this->previousDirection.x)
{
		ASSERT(this->sprites, "AnimatedInGameEntity::update: null sprites");

		// calculate gap again
		InGameEntity_setGap((InGameEntity)this);
	}

	if (this->sprites)
{
		AnimatedInGameEntity_animate(this);
	}
}

// update animations
static void AnimatedInGameEntity_animate(AnimatedInGameEntity this)
{
	VirtualNode node = VirtualList_begin(this->sprites);

	// move each child to a temporary list
	for (; node ; node = VirtualNode_getNext(node))
{
		Sprite sprite = (Sprite)VirtualNode_getData(node);

		// first animate the frame
		AnimatedSprite_update((AnimatedSprite)sprite, this->clock);
	}
}

// retrieve character's scale
Scale AnimatedInGameEntity_getScale(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::getScale: null this");
	ASSERT(this->sprites, "AnimatedInGameEntity::getScale: null sprites");

	Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));

	// get sprite's scale
	Scale scale = Sprite_getScale(sprite);

	// change direction
	scale.x = fabsf(scale.x) * this->direction.x;

	return scale;
}

// pause animation
void AnimatedInGameEntity_pauseAnimation(AnimatedInGameEntity this, int pause)
{
	ASSERT(this, "AnimatedInGameEntity::pauseAnimation: null this");
	ASSERT(this->sprites, "AnimatedInGameEntity::pauseAnimation: null sprites");

	if (this->sprites)
{
		VirtualNode node = VirtualList_begin(this->sprites);

		// play animation on each sprite
		for (; node ; node = VirtualNode_getNext(node))
{
			Sprite sprite = (Sprite)VirtualNode_getData(node);

			AnimatedSprite_pause((AnimatedSprite)sprite, pause);
		}
	}
}

// play an animation
void AnimatedInGameEntity_playAnimation(AnimatedInGameEntity this, char* animationName)
{
	ASSERT(this, "AnimatedInGameEntity::playAnimation: null this");
	ASSERT(this->sprites, "AnimatedInGameEntity::playAnimation: null sprites");

	if (this->sprites && animationName)
{
		VirtualNode node = VirtualList_begin(this->sprites);

		// play animation on each sprite
		for (; node ; node = VirtualNode_getNext(node))
{
			Sprite sprite = (Sprite)VirtualNode_getData(node);

			AnimatedSprite_play((AnimatedSprite)sprite, this->animationDescription, animationName);
		}
	}
}

// is play an animation
int AnimatedInGameEntity_isPlayingAnimation(AnimatedInGameEntity this)
{
	ASSERT(this, "AnimatedInGameEntity::isPlayingAnimation: null this");
	ASSERT(this->sprites, "AnimatedInGameEntity::isPlayingAnimation: null sprites");

	AnimatedSprite sprite = (AnimatedSprite)VirtualNode_getData(VirtualList_begin(this->sprites));

	return AnimatedSprite_isPlaying(sprite);
}

// is animation selected
int AnimatedInGameEntity_isAnimationLoaded(AnimatedInGameEntity this, char* functionName)
{
	ASSERT(this, "AnimatedInGameEntity::isAnimationLoaded: null this");
	ASSERT(this->sprites, "AnimatedInGameEntity::isAnimationLoaded: null sprites");

	Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));

	return AnimatedSprite_isPlayingFunction((AnimatedSprite)sprite, this->animationDescription, functionName);
}

// get animation definition
AnimationDescription* AnimatedInGameEntity_getAnimationDescription(AnimatedInGameEntity this)
{
	return this->animationDescription;
}

// set animation description
void AnimatedInGameEntity_setAnimationDescription(AnimatedInGameEntity this, AnimationDescription* animationDescription)
{
	this->animationDescription = animationDescription;
}

// set animation clock
void AnimatedInGameEntity_setClock(AnimatedInGameEntity this, Clock clock)
{
	this->clock = clock;
}

