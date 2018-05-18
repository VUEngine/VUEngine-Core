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


extern int strcmp(const char *, const char *);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

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
void AnimationController::constructor(Object owner, Sprite sprite, const CharSetDefinition* charSetDefinition)
{
	Base::constructor();

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
	this->animationCoordinator = AnimationCoordinatorFactory::getCoordinator(AnimationCoordinatorFactory::getInstance(), this, sprite, charSetDefinition);
}

/**
 * Class destructor
 *
 * @memberof	AnimationController
 * @public
 *
 * @param this	Function scope
 */
void AnimationController::destructor()
{
	if(this->animationCoordinator)
	{
		 AnimationCoordinator::removeAnimationController(this->animationCoordinator, this);
		this->animationCoordinator = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
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
s8 AnimationController::getActualFrameIndex()
{
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
s8 AnimationController::getActualFrame()
{
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
s8 AnimationController::getPreviousFrame()
{
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
void AnimationController::setActualFrame(s8 actualFrame)
{
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
s8 AnimationController::getFrameDuration()
{
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
void AnimationController::setFrameDuration(u8 frameDuration)
{
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
u8 AnimationController::getFrameCycleDecrement()
{
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
void AnimationController::setFrameCycleDecrement(u8 frameCycleDecrement)
{
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
bool AnimationController::updateAnimation()
{
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
				Object::fireEvent(__SAFE_CAST(Object, this), kEventAnimationCompleted);
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
			this->frameDuration = 1 + Utilities::random(Utilities::randomSeed(), __ABS(this->frameDuration));
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
void AnimationController::playAnimationFunction(const AnimationFunction* animationFunction)
{
	ASSERT(animationFunction, "AnimationController::playAnimationFunction: null animationFunction");

	// remove previous listeners
	if(this->animationFunction && this->animationFunction->onAnimationComplete)
	{
		Object::removeEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
	}

	// setup animation frame
	this->animationFunction = animationFunction;

	// register event callback
	if(this->animationFunction && this->animationFunction->onAnimationComplete)
	{
		Object::addEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
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
const AnimationFunction* AnimationController::getPlayingAnimationFunction()
{
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
bool AnimationController::play(const AnimationDescription* animationDescription, const char* functionName)
{
	ASSERT(animationDescription, "AnimationController::play: null animationDescription");
	ASSERT(functionName, "AnimationController::play: null functionName");

	if(this->animationCoordinator)
	{
		if(!AnimationCoordinator::playAnimation(this->animationCoordinator, this, animationDescription, functionName))
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
				Object::removeEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
			}

			// setup animation frame
			this->animationFunction = animationDescription->animationFunctions[i];

			// register event callback
			Object::addEventListener(__SAFE_CAST(Object, this), this->owner, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);

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
void AnimationController::stop()
{
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
void AnimationController::nextFrame()
{
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
		Object::fireEvent(__SAFE_CAST(Object, this), kEventAnimationCompleted);
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
void AnimationController::previousFrame()
{
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
		Object::fireEvent(__SAFE_CAST(Object, this), kEventAnimationCompleted);
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
bool AnimationController::isPlayingFunction(const char* functionName)
{
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
bool AnimationController::isPlaying()
{
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
void AnimationController::pause(bool pause)
{	this->playing = !pause;

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
int AnimationController::getNumberOfFrames()
{
	if(this->animationFunction)
	{
		return this->animationFunction->numberOfFrames;
	}

	return -1;
}
