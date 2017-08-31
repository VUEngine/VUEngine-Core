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

#include <AnimatedEntity.h>
#include <Clock.h>
#include <MessageDispatcher.h>
#include <Optics.h>
#include <Screen.h>
#include <Shape.h>
#include <PhysicalWorld.h>
#include <Body.h>
#include <Cuboid.h>
#include <Game.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	AnimatedEntity
 * @extends Entity
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(AnimatedEntity, Entity);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void AnimatedEntity_animate(AnimatedEntity this);

AnimationController Sprite_getAnimationController(Sprite this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(AnimatedEntity, AnimatedEntityDefinition* animatedEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(AnimatedEntity, animatedEntityDefinition, id, internalId, name);

// AnimatedEntity.c
// class's constructor
void AnimatedEntity_constructor(AnimatedEntity this, AnimatedEntityDefinition* animatedEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "AnimatedEntity::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Entity, &animatedEntityDefinition->entityDefinition, id, internalId, name);

	// save ROM definition
	this->animatedEntityDefinition = animatedEntityDefinition;
	this->animationDescription = animatedEntityDefinition->animationDescription;

	//set the direction
	this->direction.x = __RIGHT;
	this->direction.y = __DOWN;
	this->direction.z = __FAR;
	this->previousDirection = this->direction;

	this->currentAnimationName = NULL;
}

// class's destructor
void AnimatedEntity_destructor(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// set definition
void AnimatedEntity_setDefinition(AnimatedEntity this, void* animatedEntityDefinition)
{
	ASSERT(this, "AnimatedEntity::setDefinition: null this");
	ASSERT(animatedEntityDefinition, "AnimatedEntity::setDefinition: null definition");

	// save definition
	this->animatedEntityDefinition = animatedEntityDefinition;

	__CALL_BASE_METHOD(Entity, setDefinition, this, &((AnimatedEntityDefinition*)animatedEntityDefinition)->entityDefinition);
}

// ready method
void AnimatedEntity_ready(AnimatedEntity this, bool recursive)
{
	ASSERT(this, "AnimatedEntity::ready: null this");
	ASSERT(this->animatedEntityDefinition, "AnimatedEntity::ready: null animatedEntityDefinition");

	__CALL_BASE_METHOD(Entity, ready, this, recursive);

	AnimatedEntity_playAnimation(this, this->animatedEntityDefinition->initialAnimation);
}

// updates the animation attributes
// graphically refresh of characters that are visible
void AnimatedEntity_transform(AnimatedEntity this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "AnimatedEntity::transform: null this");

	VBVec3DFlag directionChanged = {false, false, false};

	// set sprite direction
	if(this->direction.x != this->previousDirection.x)
	{
		// change sprite's direction
		Entity_setSpritesDirection(__SAFE_CAST(Entity, this), __X_AXIS, this->direction.x);

		// save current direction
		this->previousDirection.x = this->direction.x;

		directionChanged.x = true;
	}

	if(this->direction.y != this->previousDirection.y)
	{
		// change sprite's direction
		Entity_setSpritesDirection(__SAFE_CAST(Entity, this), __Y_AXIS, this->direction.y);

		// save current direction
		this->previousDirection.y = this->direction.y;

		directionChanged.y = true;
	}

	if(this->direction.z != this->previousDirection.z)
	{
		// save current direction
		this->previousDirection.z = this->direction.z;

		directionChanged.z = true;
	}

	if(this->shapes && (directionChanged.x | directionChanged.y | directionChanged.z))
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			VBVec3D displacement = Shape_getDisplacement(__SAFE_CAST(Shape, node->data));

			if(directionChanged.x)
			{
				displacement.x = -displacement.x;
			}

			if(directionChanged.y)
			{
				displacement.y = -displacement.y;
			}

			if(directionChanged.z)
			{
				displacement.z = -displacement.z;
			}

			Shape_setDisplacement(__SAFE_CAST(Shape, node->data), displacement);
		}
	}

	// call base
	__CALL_BASE_METHOD(Entity, transform, this, environmentTransform, invalidateTransformationFlag);
}

// execute character's logic
void AnimatedEntity_update(AnimatedEntity this, u32 elapsedTime)
{
	ASSERT(this, "AnimatedEntity::update: null this");

	// call base
	__CALL_BASE_METHOD(Entity, update, this, elapsedTime);

	if(!elapsedTime)
	{
		return;
	}

	AnimatedEntity_animate(this);
}

// update animations
static void AnimatedEntity_animate(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::animate: null this");

	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// move each child to a temporary list
	for(; node ; node = node->next)
	{
		// first animate the frame
		Sprite_updateAnimation(__SAFE_CAST(Sprite, node->data));
	}
}

// pause animation
void AnimatedEntity_pauseAnimation(AnimatedEntity this, int pause)
{
	ASSERT(this, "AnimatedEntity::pauseAnimation: null this");
	ASSERT(this->sprites, "AnimatedEntity::pauseAnimation: null sprites");

	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// play animation on each sprite
	for(; node ; node = node->next)
	{
		Sprite_pause(__SAFE_CAST(Sprite, node->data), pause);
	}
}

// play an animation
void AnimatedEntity_playAnimation(AnimatedEntity this, char* animationName)
{
	ASSERT(this, "AnimatedEntity::playAnimation: null this");

	if(!this->sprites | !animationName)
	{
		return;
	}

	this->currentAnimationName = animationName;

	VirtualNode node = this->sprites->head;

	// play animation on each sprite
	for(; node ; node = node->next)
	{
		Sprite_play(__SAFE_CAST(Sprite, node->data), this->animationDescription, animationName);
	}
}

// skip to next frame
void AnimatedEntity_nextFrame(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::nextFrame: null this");

	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// do on each sprite
	for(; node ; node = node->next)
	{
		Sprite_nextFrame(__SAFE_CAST(Sprite, node->data));
	}
}

// rewind to previous frame
void AnimatedEntity_previousFrame(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::previousFrame: null this");

	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// do on each sprite
	for(; node ; node = node->next)
	{
		Sprite_previousFrame(__SAFE_CAST(Sprite, node->data));
	}
}

// is playing an animation
bool AnimatedEntity_isPlayingAnimation(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::isPlayingAnimation: null this");
	ASSERT(this->sprites, "AnimatedEntity::isPlayingAnimation: null sprites");

	return Sprite_isPlaying(__SAFE_CAST(Sprite, VirtualNode_getData(this->sprites->head)));
}

// is animation selected
bool AnimatedEntity_isAnimationLoaded(AnimatedEntity this, char* functionName)
{
	ASSERT(this, "AnimatedEntity::isAnimationLoaded: null this");

	if(!this->sprites)
	{
		return false;
	}

	Sprite sprite = __SAFE_CAST(Sprite, VirtualNode_getData(this->sprites->head));

	return Sprite_isPlayingFunction(__SAFE_CAST(Sprite, sprite), functionName);
}

// get animation definition
AnimationDescription* AnimatedEntity_getAnimationDescription(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::getAnimationDescription: null this");

	return this->animationDescription;
}

// set animation description
void AnimatedEntity_setAnimationDescription(AnimatedEntity this, AnimationDescription* animationDescription)
{
	ASSERT(this, "AnimatedEntity::setAnimationDescription: null this");

	this->animationDescription = animationDescription;
}

// resume method
void AnimatedEntity_resume(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::resume: null this");

	__CALL_BASE_METHOD(Entity, resume, this);

	Entity_setSpritesDirection(__SAFE_CAST(Entity, this), __X_AXIS, this->direction.x);

	AnimatedEntity_playAnimation(this, this->currentAnimationName);
}

s8 AnimatedEntity_getActualFrame(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::getActualFrame: null this");

	if(!this->sprites)
	{
		return -1;
	}

	VirtualNode node = this->sprites->head;

	for(; node ; node = node->next)
	{
		return Sprite_getActualFrame(node->data);
	}

	return -1;
}

int AnimatedEntity_getNumberOfFrames(AnimatedEntity this)
{
	if(!this->sprites)
	{
		return -1;
	}

	VirtualNode node = this->sprites->head;

	for(; node ; node = node->next)
	{
		AnimationController animationController = Sprite_getAnimationController(node->data);
		return AnimationController_getNumberOfFrames(animationController);
	}

	return -1;
}

// set direction
void AnimatedEntity_setDirection(AnimatedEntity this, Direction direction)
{
	ASSERT(this, "AnimatedEntity::setDirection: null this");

	this->direction = direction;
}

// get direction
Direction AnimatedEntity_getDirection(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::getDirection: null this");

	// TODO: must be recursive to account for parenting
	return this->direction;
}
