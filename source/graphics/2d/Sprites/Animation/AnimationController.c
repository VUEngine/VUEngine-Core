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
#include <AnimationController.h>
#include <AnimationCoordinator.h>
#include <AnimationCoordinatorFactory.h>
#include <Utilities.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	AnimationController
 * @extends Object
 */
__CLASS_DEFINITION(AnimationController, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern int strcmp(const char *, const char *);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(AnimationController, Object owner, Sprite sprite, const CharSetDefinition* charSetDefinition)
__CLASS_NEW_END(AnimationController, owner, sprite, charSetDefinition);

// class's constructor
void AnimationController_constructor(AnimationController this, Object owner, Sprite sprite, const CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "AnimationController::constructor: null this");

	__CONSTRUCT_BASE(Object);

	// set the owner
	this->owner = owner;

	// initialize frame tracking
	this->actualFrame = 0;
	this->previousFrame = 0;

	// initialize frame head
	this->frameDelay = 0;
	this->frameDelayDelta = 1 << __FRAME_CYCLE;

	// initialize animation function
	this->animationFunction = NULL;

	// not playing anything yet
	this->playing = false;

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
		__VIRTUAL_CALL(AnimationCoordinator, removeAnimationController, this->animationCoordinator, this);
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

	return this->animationFunction ? this->animationFunction->frames[this->actualFrame] : 0;
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

// set frame cycle
void AnimationController_setFrameDelay(AnimationController this, u8 frameDelay)
{
	ASSERT(this, "AnimationController::setFrameDelay: null this");

	this->frameDelay = frameDelay;
}

// retrieve frame delay delta
u8 AnimationController_geFrameDelayDelta(AnimationController this)
{
	ASSERT(this, "AnimationController::getAnimationControllerCycleDelta: null this");

	return this->frameDelayDelta;
}

// set frame delay delta
void AnimationController_setFrameDelayDelta(AnimationController this, u8 frameDelayDelta)
{
	ASSERT(this, "AnimationController::setAnimationControllerCycleDelta: null this");

	this->frameDelayDelta = frameDelayDelta << __FRAME_CYCLE;
}

bool AnimationController_animate(AnimationController this)
{
	ASSERT(this, "AnimationController::animate: null this");

	// first check for a valid animation function
	if(!this->playing || !this->animationFunction)
	{
		return false;
	}

	// if the actual frame was set to -1
	// it means that a not loop animation has been completed
	if(-1 == this->actualFrame)
	{
		return false;
	}

	this->frameDelay -= this->frameDelayDelta;

	// reduce frame delay count
	if(0 > this->frameDelay)
	{
		// increase the frame to show
		this->previousFrame = this->actualFrame++;

		// check if the actual frame is out of bounds
		if(this->actualFrame >= this->animationFunction->numberOfFrames)
		{
			// the last frame has been reached
			if(this->animationFunction->onAnimationComplete)
			{
				Object_fireEvent(__SAFE_CAST(Object, this), kEventAnimationCompleted);
			}

			// rewind to first frame
			this->actualFrame = 0;

			// if the animation is not a loop
			if(!this->animationFunction->loop)
			{
				// not playing anymore
				this->playing = false;

				// invalidate animation
				this->actualFrame = this->animationFunction->numberOfFrames - 1;
			}
		}

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
			this->frameDelay = 1 + Utilities_random(Utilities_randomSeed(), __ABS(this->frameDelay));
		}

		return true;
	}

	return false;
}

// play animation
void AnimationController_playAnimationFunction(AnimationController this, const AnimationFunction* animationFunction)
{
	ASSERT(this, "AnimationController::playAnimationFunction: null this");
	ASSERT(animationFunction, "AnimationController::playAnimationFunction: null animationFunction");

	// remove previous listeners
	if(this->animationFunction && this->animationFunction->onAnimationComplete)
	{
		Object_removeEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
	}

	// setup animation frame
	this->animationFunction = animationFunction;

	// register event callback
	if(this->animationFunction && this->animationFunction->onAnimationComplete)
	{
		Object_addEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
	}

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
bool AnimationController_play(AnimationController this, const AnimationDescription* animationDescription, const char* functionName)
{
	ASSERT(this, "AnimationController::play: null this");
	ASSERT(animationDescription, "AnimationController::play: null animationDescription");
	ASSERT(functionName, "AnimationController::play: null functionName");

	if(this->animationCoordinator)
	{
		if(!AnimationCoordinator_playAnimation(this->animationCoordinator, this, animationDescription, functionName))
		{
			return false;
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
				Object_removeEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
			}

			// setup animation frame
			this->animationFunction = animationDescription->animationFunctions[i];

			// register event callback
			Object_addEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);

			// force frame writing in the next update
			this->previousFrame = 0;

			// reset frame to play
			this->actualFrame = 0;

			// set frame delay to 1 to force the writing of the first
			// animation frame in the next update
			this->frameDelay = 1;

			// it's playing now
			this->playing = true;

			return true;
		}
	}

	return false;
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
bool AnimationController_isPlayingFunction(AnimationController this, const char* functionName)
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
