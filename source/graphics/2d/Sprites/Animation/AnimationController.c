/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
 * @ingroup graphics-2d-sprites-animation
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

/**
 * Class constructor
 *
 * @memberof					AnimationController
 * @public
 *
 * @param this					Function scope
 * @param owner					Controller's owner
 * @param sprite				Sprite to animate
 * @param charSetDefinition		CharSetDefinition used to decide the animation allocation type
 */
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
	this->frameDuration = 0;
	this->frameCycleDecrement = 1;

	// initialize animation function
	this->animationFunction = NULL;

	// not playing anything yet
	this->playing = false;

	ASSERT(charSetDefinition, "AnimationController::constructor: null charSetDefinition");

	// animation coordinator
	this->animationCoordinator = AnimationCoordinatorFactory_getCoordinator(AnimationCoordinatorFactory_getInstance(), this, sprite, charSetDefinition);
}

/**
 * Class destructor
 *
 * @memberof	AnimationController
 * @public
 *
 * @param this	Function scope
 */
void AnimationController_destructor(AnimationController this)
{
	ASSERT(this, "AnimationController::destructor: null this");

	if(this->animationCoordinator)
	{
		 AnimationCoordinator_removeAnimationController(this->animationCoordinator, this);
		this->animationCoordinator = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Retrieve the actual frame of animation index
 *
 * @memberof	AnimationController
 * @private
 *
 * @param this	Function scope
 *
 * @return 		Actual frame of animation index
 */
s8 AnimationController_getActualFrameIndex(AnimationController this)
{
	ASSERT(this, "AnimationController::getActualFrameIndex: null this");

	return this->animationFunction ? this->animationFunction->frames[this->actualFrame] : 0;
}

/**
 * Retrieve the actual frame of animation
 *
 * @memberof	AnimationController
 * @private
 *
 * @param this	Function scope
 *
 * @return 		Actual frame of animation
 */
s8 AnimationController_getActualFrame(AnimationController this)
{
	ASSERT(this, "AnimationController::getActualFrame: null this");

	return this->actualFrame;
}

/**
 * Retrieve the previous frame of animation
 *
 * @memberof	AnimationController
 * @private
 *
 * @param this	Function scope
 *
 * @return 		Previous frame of animation
 */
s8 AnimationController_getPreviousFrame(AnimationController this)
{
	ASSERT(this, "AnimationController::getPreviousFrame: null this");

	return this->previousFrame;
}

/**
 * Set the actual frame of animation
 *
 * @memberof			AnimationController
 * @private
 *
 * @param this			Function scope
 * @param actualFrame	The new frame of animation
 */
void AnimationController_setActualFrame(AnimationController this, s8 actualFrame)
{
	ASSERT(this, "AnimationController::setActualFrame: null this");

	this->actualFrame = actualFrame;
}

/**
 * Retrieve the number of cycles that each frame of animation is shown
 *
 * @memberof	AnimationController
 * @private
 *
 * @param this	Function scope
 *
 * @return		Frame duration in game cycles
 */
s8 AnimationController_getFrameDuration(AnimationController this)
{
	ASSERT(this, "AnimationController::getFrameDuration: null this");

	return this->frameDuration;
}

/**
 * Set the number of cycles that each frame of animation is shown
 *
 * @memberof			AnimationController
 * @private
 *
 * @param this			Function scope
 * @param frameDuration	Number of cycles that each frame of animation is shown
 */
void AnimationController_setFrameDuration(AnimationController this, u8 frameDuration)
{
	ASSERT(this, "AnimationController::setFrameDuration: null this");

	this->frameDuration = frameDuration;
}

/**
 * Retrieve the frame duration decrement per cycle
 *
 * @memberof	AnimationController
 * @private
 *
 * @param this	Function scope
 *
 * @return		Frame cycle decrement
 */
u8 AnimationController_getFrameCycleDecrement(AnimationController this)
{
	ASSERT(this, "AnimationController::getFrameCycleDecrement: null this");

	return this->frameCycleDecrement;
}

/**
 * Set the frame duration decrement per cycle
 *
 * @memberof					AnimationController
 * @private
 *
 * @param this					Function scope
 * @param frameCycleDecrement	Decrement value for the frame cycle on each game cycle
 */
void AnimationController_setFrameCycleDecrement(AnimationController this, u8 frameCycleDecrement)
{
	ASSERT(this, "AnimationController::setFrameCycleDecrement: null this");

	this->frameCycleDecrement = frameCycleDecrement;
}

/**
 * Update the animation
 *
 * @memberof	AnimationController
 * @private
 *
 * @param this	Function scope
 *
 * @return		True if the animation frame changed
 */
bool AnimationController_updateAnimation(AnimationController this)
{
	ASSERT(this, "AnimationController::updateAnimation: null this");

	// first check for a valid animation function
	if(!this->playing | !this->animationFunction)
	{
		return false;
	}

	// if the actual frame was set to -1
	// it means that a not loop animation has been completed
	if(-1 == this->actualFrame)
	{
		return false;
	}

	this->frameDuration -= this->frameCycleDecrement;

	// reduce frame delay count
	if(0 > this->frameDuration)
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
		this->frameDuration = this->animationFunction->delay;

		// the minimum valid delay is 1
		if(0 == this->frameDuration)
		{
			this->frameDuration = 1;
		}
		else if(0 > this->frameDuration)
		{
			// pick up a random delay
			this->frameDuration = 1 + Utilities_random(Utilities_randomSeed(), __ABS(this->frameDuration));
		}

		return true;
	}

	return false;
}

/**
 * Play an animation given an AnimationFunction
 *
 * @memberof					AnimationController
 * @private
 *
 * @param this					Function scope
 * @param animationFunction		Animation function to play
 */
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

	// set frame delay to 1 to force the writing of the first animation frame in the next update
	this->frameDuration = 1;

	// it's playing now
	this->playing = true;
}

/**
 * Retrieve the currently playing AnimationFunction
 *
 * @memberof		AnimationController
 * @private
 *
 * @param this		Function scope
 *
 * @return			Animation function
 */
const AnimationFunction* AnimationController_getPlayingAnimationFunction(AnimationController this)
{
	ASSERT(this, "AnimationController::getPlayingAnimationFunction: null this");

	return this->playing ? this->animationFunction : NULL;
}

/**
 * Play an animation given an AnimationDescription and the name of an AnimationFunction
 *
 * @memberof					AnimationController
 * @private
 *
 * @param this						Function scope
 * @param animationDescription		Animation description holding the animation function
 * @param functionName				Name of the animation function's to play
 *
 * @return							True if the animation started playing
 */
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

			// set frame delay to 1 to force the writing of the first animation frame in the next update
			this->frameDuration = 1;

			// it's playing now
			this->playing = true;

			return true;
		}
	}

	return false;
}

/**
 * Stop the currently playing animation
 *
 * @memberof	AnimationController
 * @private
 *
 * @param this	Function scope
 */
void AnimationController_stop(AnimationController this)
{
	ASSERT(this, "AnimationController::stop: null this");

	this->animationFunction = NULL;
	this->playing = false;
	this->actualFrame = 0;
}

/**
 * Skip to next frame
 *
 * @memberof	AnimationController
 * @public
 *
 * @param this	Function scope
 */
void AnimationController_nextFrame(AnimationController this)
{
	ASSERT(this, "AnimationController::nextFrame: null this");

	if(!this->animationFunction)
	{
		return;
	}

	if(this->actualFrame < (this->animationFunction->numberOfFrames - 1))
	{
		this->previousFrame = this->actualFrame++;
	}
	else if(this->animationFunction->loop)
	{
		this->previousFrame = this->actualFrame;
		this->actualFrame = 0;
	}
	else
	{
		Object_fireEvent(__SAFE_CAST(Object, this), kEventAnimationCompleted);
	}
}

/**
 * Rewind to previous frame
 *
 * @memberof	AnimationController
 * @public
 *
 * @param this	Function scope
 */
void AnimationController_previousFrame(AnimationController this)
{
	ASSERT(this, "AnimationController::previousFrame: null this");

	if(!this->animationFunction)
	{
		return;
	}

	if(this->actualFrame > 0)
	{
		this->previousFrame = this->actualFrame--;
	}
	else if(this->animationFunction->loop)
	{
		this->previousFrame = this->actualFrame;
		this->actualFrame = (this->animationFunction->numberOfFrames - 1);
	}
	else
	{
		Object_fireEvent(__SAFE_CAST(Object, this), kEventAnimationCompleted);
	}
}

/**
 * Check if a given animation function is playing
 *
 * @memberof					AnimationController
 * @private
 *
 * @param this					Function scope
 * @param functionName			The animation function's name
 *
 * @return						True if the animation is playing
 */
bool AnimationController_isPlayingFunction(AnimationController this, const char* functionName)
{
	ASSERT(this, "AnimationController::isPlayingFunction: null this");

	// compare function's names
	return !strcmp((const char *)functionName, (const char *)this->animationFunction->name);
}

/**
 * Check if any animation is playing
 *
 * @memberof						AnimationController
 * @private
 *
 * @param this						Function scope
 *
 * @return							True if there is an animation playing
 */
bool AnimationController_isPlaying(AnimationController this)
{
	ASSERT(this, "AnimationController::isPlaying: null this");

	return this->playing;
}

/**
 * Pause the currently playing animation
 *
 * @memberof			AnimationController
 * @private
 *
 * @param this			Function scope
 * @param pause			Flag to pause or to resume animation
 */
void AnimationController_pause(AnimationController this, bool pause)
{
	ASSERT(this, "AnimationController::pause: null this");
	this->playing = !pause;

	if(-1 == this->actualFrame)
	{
		this->actualFrame = 0;
	}
}

/**
 * Get the total number of frames of the current animation
 *
 * @memberof	AnimationController
 * @private
 *
 * @param this	Function scope
 */
int AnimationController_getNumberOfFrames(AnimationController this)
{
	ASSERT(this, "AnimationController::getNumberOfFrames: null this");

	if(this->animationFunction)
	{
		return this->animationFunction->numberOfFrames;
	}

	return -1;
}
