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
#include <AnimationController.h>
#include <AnimationCoordinator.h>
#include <AnimationCoordinatorFactory.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the AnimationController
__CLASS_DEFINITION(AnimationController, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern int strcmp(const char *, const char *);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(AnimationController, Object owner, Sprite sprite, const CharSetDefinition* charSetDefinition)
__CLASS_NEW_END(AnimationController, owner, sprite, charSetDefinition);

// class's constructor
void AnimationController_constructor(AnimationController this, Object owner, Sprite sprite, const CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "AnimationController::constructor: null this");

	__CONSTRUCT_BASE();

	// set the owner
	this->owner = owner;

	// initialize frame tracking
	this->actualFrame = 0;
	this->previousFrame = 0;

	// initialize frame head
	this->frameDelay = 0;
	this->frameDelayDelta = 1 << __FRAME_CYCLE;

	// intialize animation function
	this->animationFunction = NULL;

	// not playing anything yet
	this->playing = false;
	
	this->animationFrameChanged = false;
	
	ASSERT(charSetDefinition, "AnimationController::constructor: null charSetDefinition");

	// animation coordinator
	this->animationCoordinator = AnimationCoordinatorFactory_getCoordinator(AnimationCoordinatorFactory_getInstance(), this, sprite, charSetDefinition);
}

//destructor
void AnimationController_destructor(AnimationController this)
{
	ASSERT(this, "AnimationController::destructor: null this");

	if(this->animationCoordinator)
	{
		__VIRTUAL_CALL(void, AnimationCoordinator, removeAnimationController, this->animationCoordinator, this);
		this->animationCoordinator = NULL;
	}
	
	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// retrieve actual frame index of animation
s8 AnimationController_getActualFrameIndex(AnimationController this)
{
	ASSERT(this, "AnimationController::getActualAnimationController: null this");

	return this->animationFunction? this->animationFunction->frames[this->actualFrame]: 0;
}

// retrieve actual frame of animation
s8 AnimationController_getActualFrame(AnimationController this)
{
	ASSERT(this, "AnimationController::getActualAnimationController: null this");

	return this->actualFrame;
}

// retrieve previous frame index of animation
s8 AnimationController_getPreviousFrame(AnimationController this)
{
	ASSERT(this, "AnimationController::getPreviousAnimationController: null this");

	return this->previousFrame;
}

// set actual frame of animation
void AnimationController_setActualFrame(AnimationController this, s8 actualFrame)
{
	ASSERT(this, "AnimationController::setActualAnimationController: null this");

	this->actualFrame = actualFrame;
}

// retrieve frame delay
s8 AnimationController_getFrameDelay(AnimationController this)
{
	ASSERT(this, "AnimationController::getFrameDelay: null this");

	return this->frameDelay;
}

// set frame cicle
void AnimationController_setFrameDelay(AnimationController this, u8 frameDelay)
{
	ASSERT(this, "AnimationController::setFrameDelay: null this");

	this->frameDelay = frameDelay;
}

// retrieve frame delay delta
u8 AnimationController_geFrameDelayDelta(AnimationController this)
{
	ASSERT(this, "AnimationController::getAnimationControllerCicleDelta: null this");

	return this->frameDelayDelta;
}

// set frame delay delta
void AnimationController_setFrameDelayDelta(AnimationController this, u8 frameDelayDelta)
{
	ASSERT(this, "AnimationController::setAnimationControllerCicleDelta: null this");

	this->frameDelayDelta = frameDelayDelta << __FRAME_CYCLE;
}

// animate the frame
void AnimationController_animate(AnimationController this)
{
	ASSERT(this, "AnimationController::animate: null this");

	this->animationFrameChanged = false;
 
	// first check for a valid animation function
	if(!this->animationFunction)
	{
		return;
	}

	// if the actual frame was set to -1
	// it means that a not loop animation has been completed
	if(-1 == this->actualFrame)
	{
		return;
	}

	// show the next frame
	if(this->actualFrame >= this->animationFunction->numberOfFrames)
	{
		// the last frame has been reached
		if(this->animationFunction->onAnimationComplete)
		{
			Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_ANIMATION_COMPLETE);
		}

		// rewind to first frame
		this->actualFrame = 0;

		// if the animation is not a loop
		if(!this->animationFunction->loop)
		{
			// not playing anymore
			this->playing = false;

			// invalidate animation
			this->actualFrame = -1;

			return;
		}
	}

	// if the frame has changed
	if(this->actualFrame != this->previousFrame)
	{
		// write the new frame of animation
		this->animationFrameChanged = true;

		// don't write animation each time, only when the animation
		// has changed
		this->previousFrame = this->actualFrame;
		
		Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_ANIMATION_FRAME_CHANGED);
	}

	this->frameDelay -= this->frameDelayDelta;

	// reduce frame delay count
	if(0 > this->frameDelay)
	{
		// incrase the frame to show
		this->previousFrame = this->actualFrame++;

		// reset frame delay
		this->frameDelay = this->animationFunction->delay;

		// the minimum valid delay is 1
		if(0 == this->frameDelay)
		{
			this->frameDelay = 1;
		}
		else if(0 > this->frameDelay)
		{
			// pick up a random delay
			this->frameDelay = 1 + Utilities_random(Utilities_randomSeed(), abs(this->frameDelay));
		}
	}
}

// render frame
/*
bool AnimationController_update(AnimationController this, Clock clock)
{
	ASSERT(this, "AnimationController::update: null this");

	if(this->playing && !Clock_isPaused(clock))
	{
		// first animate the frame
		return AnimationController_animate(this);
	}
	
	return false;
}
*/

// play animation
void AnimationController_playAnimationFunction(AnimationController this, const AnimationFunction* animationFunction)
{
	ASSERT(this, "AnimationController::playAnimation: null this");
	ASSERT(animationFunction, "AnimationController::playAnimation: null animationFunction");

	// remove previous listeners
	if(this->animationFunction && this->animationFunction->onAnimationComplete)
	{
		Object_removeEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, __EVENT_ANIMATION_COMPLETE);
	}
	
	// setup animation frame
	this->animationFunction = animationFunction;

	// register event callback
	Object_addEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, __EVENT_ANIMATION_COMPLETE);

	// force frame writing in the next update
	this->previousFrame = 0;

	// reset frame to play
	this->actualFrame = 0;

	// set frame delay to 1 to force the writing of the first
	// animation frame in the next update
	this->frameDelay = 1;

	// it's playing now
	this->playing = true;
}

const AnimationFunction* AnimationController_getPlayingAnimationFunction(AnimationController this)
{
	ASSERT(this, "AnimationController::getPlayingAnimationFunction: null this");

	return this->playing? this->animationFunction: NULL;
}

// play animation
void AnimationController_play(AnimationController this, const AnimationDescription* animationDescription, const char* functionName)
{
	ASSERT(this, "AnimationController::play: null this");
	ASSERT(animationDescription, "AnimationController::play: null animationDescription");
	ASSERT(functionName, "AnimationController::play: null functionName");
	
	if(this->animationCoordinator)
	{
		if(!AnimationCoordinator_playAnimation(this->animationCoordinator, this, animationDescription, functionName))
		{
			return;
		}
	}

	int i = 0;

	// search for the animation function
	for(; animationDescription->animationFunctions[i]; i++ )
	{
		// compare function's names
		if(!strcmp((const char *)functionName, (const char *)animationDescription->animationFunctions[i]->name))
		{
			// remove previous listeners
			if(this->animationFunction && this->animationFunction->onAnimationComplete)
			{
				Object_removeEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, __EVENT_ANIMATION_COMPLETE);
			}

			// setup animation frame
			this->animationFunction = animationDescription->animationFunctions[i];

			// register event callback
			Object_addEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, __EVENT_ANIMATION_COMPLETE);

			// force frame writing in the next update
			this->previousFrame = 0;

			// reset frame to play
			this->actualFrame = 0;

			// set frame delay to 1 to force the writing of the first
			// animation frame in the next update
			this->frameDelay = 1;

			// it's playing now
			this->playing = true;
			
			// force writing in the next render cycle
			this->animationFrameChanged = true;
		}
	}
}

// stop animation
void AnimationController_stop(AnimationController this)
{
	ASSERT(this, "AnimationController::stop: null this");

	this->animationFunction = NULL;
	this->playing = false;
	this->actualFrame = 0;
}

// is play animation
bool AnimationController_isPlayingFunction(AnimationController this, const AnimationDescription* animationDescription, const char* functionName)
{
	ASSERT(this, "AnimationController::isPlayingFunction: null this");

	// compare function's names
	return !strcmp((const char *)functionName, (const char *)this->animationFunction->name);
}

// is playing animation
bool AnimationController_isPlaying(AnimationController this)
{
	ASSERT(this, "AnimationController::isPlaying: null this");

	return this->playing;
}

// pause animation
void AnimationController_pause(AnimationController this, bool pause)
{
	ASSERT(this, "AnimationController::pause: null this");
	this->playing = !pause;

	if(-1 == this->actualFrame)
	{
		this->actualFrame = 0;
	}
}

bool AnimationController_didAnimationFrameChanged(AnimationController this)
{
	ASSERT(this, "AnimationController::didAnimationFrameChanged: null this");

	return this->animationFrameChanged;
}
