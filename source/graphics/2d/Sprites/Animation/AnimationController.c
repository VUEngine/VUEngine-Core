/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
#include <Game.h>
#include <Utilities.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern int strcmp(const char *, const char *);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param owner					Controller's owner
 * @param sprite				Sprite to animate
 * @param charSetSpec		CharSetSpec used to decide the animation allocation type
 */
void AnimationController::constructor()
{
	Base::constructor();

	// initialize frame tracking
	this->actualFrame = 0;
	this->previousFrameValue = -1;

	// initialize frame head
	this->frameDuration = 0;
	this->frameCycleDecrement = 1;

	// initialize animation function
	this->animationFunction = NULL;

	// not playing anything yet
	this->playing = false;
}

/**
 * Class destructor
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
 * Retrieve the animation coordinator
 *
 * @return 		Animation coordinator
 */
AnimationCoordinator AnimationController::getAnimationCoordinator()
{
	return this->animationCoordinator;
}

/**
 * Set the animation coordinator
 *
 * @param 		Animation coordinator
 */
void AnimationController::setAnimationCoordinator(AnimationCoordinator animationCoordinator)
{
	// animation coordinator
	this->animationCoordinator = animationCoordinator; 
}

/**
 * Retrieve the actual frame of animation index
 *
 * @private
 * @return 		Actual frame of animation index
 */
s16 AnimationController::getActualFrameIndex()
{
	return this->animationFunction ? this->animationFunction->frames[this->actualFrame] : 0;
}

/**
 * Retrieve the actual frame of animation
 *
 * @private
 * @return 		Actual frame of animation
 */
s16 AnimationController::getActualFrame()
{
	return this->actualFrame;
}

/**
 * Retrieve the previous frame of animation
 *
 * @publics
 * @return 		Previous frame of animation
 */
s16 AnimationController::getPreviousFrameValue()
{
	return this->previousFrameValue;
}

/**
 * Set the actual frame of animation
 *
 * @public
 * @param actualFrame	The new frame of animation
 *
 * @return bool			Whether the value was updated or not
 */
bool AnimationController::setActualFrame(s16 actualFrame)
{
	if(0 > actualFrame)
	{
		actualFrame = -1;
	}

	if(this->animationFunction && actualFrame < this->animationFunction->numberOfFrames)
	{
		bool updatedActualFrame = this->actualFrame != actualFrame && 0 <= actualFrame;

		this->actualFrame = actualFrame;

		return updatedActualFrame;
	}

	return false;
}

/**
 * Retrieve the number of cycles that each frame of animation is shown
 *
 * @private
 * @return		Frame duration in game cycles
 */
u8 AnimationController::getFrameDuration()
{
	return this->frameDuration;
}

/**
 * Set the number of cycles that each frame of animation is shown
 *
 * @private
 * @param frameDuration	Number of cycles that each frame of animation is shown
 */
void AnimationController::setFrameDuration(u8 frameDuration)
{
	this->frameDuration = frameDuration;
}

/**
 * Retrieve the frame duration decrement per cycle
 *
 * @private
 * @return		Frame cycle decrement
 */
u8 AnimationController::getFrameCycleDecrement()
{
	return this->frameCycleDecrement;
}

/**
 * Set the frame duration decrement per cycle
 *
 * @private
 * @param frameCycleDecrement	Decrement value for the frame cycle on each game cycle
 */
void AnimationController::setFrameCycleDecrement(u8 frameCycleDecrement)
{
	this->frameCycleDecrement = frameCycleDecrement;
}

/**
 * Update the animation
 *
 * @private
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
	// it means that a non looping animation has been completed
	if(-1 == this->actualFrame)
	{
		return false;
	}

	// reduce frame delay count
	if(this->frameDuration > this->frameCycleDecrement)
	{
		this->frameDuration -= this->frameCycleDecrement;
	}
	else
	{
		this->frameDuration = 0;
	}

	// reduce frame delay count
	if(0 == this->frameDuration)
	{
		s16 actualFrame = this->actualFrame;

		// increase the frame to show
		this->actualFrame++;

		// check if the actual frame is out of bounds
		if(this->actualFrame >= this->animationFunction->numberOfFrames)
		{
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

			// the last frame has been reached
			if(this->animationFunction->onAnimationComplete)
			{
				AnimationController::fireEvent(this, kEventAnimationCompleted);
				NM_ASSERT(!isDeleted(this), "AnimationController::updateAnimation: deleted this during kEventAnimationCompleted");
			}
		}

		// Reset frame duration
		AnimationController::resetFrameDuration(this);

		u8 actualFrameValue = this->animationFunction->frames[this->actualFrame];

		bool frameValueChanged = this->previousFrameValue != actualFrameValue || actualFrameValue != this->animationFunction->frames[actualFrame];
		this->previousFrameValue = actualFrameValue;

		return frameValueChanged;
	}

	return false;
}

/**
 * Reset frame duration
 *
 * @private
 */
void AnimationController::resetFrameDuration()
{
	// reset frame delay
	this->frameDuration = this->animationFunction->delay;

	// the minimum valid delay is 1
	if(0 == this->frameDuration)
	{
		this->frameDuration = 1;
	}
}

/**
 * Play an animation given an AnimationFunction
 *
 * @private
 * @param animationFunction		Animation function to play
 */
void AnimationController::playAnimationFunction(const AnimationFunction* animationFunction, Object scope)
{
	ASSERT(animationFunction, "AnimationController::playAnimationFunction: null animationFunction");

	// remove previous listeners
	if(this->animationFunction && this->animationFunction->onAnimationComplete)
	{
		AnimationController::removeEventListeners(this, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
	}

	// setup animation frame
	this->animationFunction = animationFunction;

	// register event callback
	if(!isDeleted(scope) && this->animationFunction && this->animationFunction->onAnimationComplete)
	{
		AnimationController::addEventListener(this, scope, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
	}

	// reset frame to play
	this->actualFrame = 0;

	// Reset frame duration
	AnimationController::resetFrameDuration(this);

	// it's playing now
	this->playing = true;

	// Tell anyone listening
	AnimationController::fireEvent(this, kEventAnimationStarted);
}

/**
 * Retrieve the currently playing AnimationFunction
 *
 * @private
 * @return			Animation function
 */
const AnimationFunction* AnimationController::getPlayingAnimationFunction()
{
	return this->playing ? this->animationFunction : NULL;
}

/**
 * Play an animation given an AnimationDescription and the name of an AnimationFunction
 *
 * @public
 * @param animationDescription		Animation description holding the animation function
 * @param functionName				Name of the animation function's to play
 * @return							True if the animation started playing
 */
bool AnimationController::play(const AnimationDescription* animationDescription, const char* functionName, Object scope)
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

	if(NULL == this->animationFunction || strncmp((const char *)functionName, (const char *)this->animationFunction->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH))
	{
		int i = 0;

		// search for the animation function
		for(; animationDescription->animationFunctions[i]; i++ )
		{
			// compare function's names
			if(!strncmp((const char *)functionName, (const char *)animationDescription->animationFunctions[i]->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH))
			{
				// remove previous listeners
				if(this->animationFunction && this->animationFunction->onAnimationComplete)
				{
					AnimationController::removeEventListeners(this, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
				}

				this->animationFunction = animationDescription->animationFunctions[i];

				break;
			}
		}
	}

	if(NULL == this->animationFunction)
	{
		return false;
	}

	// setup animation frame
	if(!isDeleted(scope) && this->animationFunction->onAnimationComplete)
	{
		// register event callback
		AnimationController::addEventListener(this, scope, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
	}

	// reset frame to play
	this->actualFrame = 0;

	// Reset frame duration
	AnimationController::resetFrameDuration(this);

	// it's playing now
	this->playing = true;

	// Tell anyone listening
	AnimationController::fireEvent(this, kEventAnimationStarted);

	return true;
}

/**
 * Replay the last animation if any
 *
 * @public
 * @param animationDescription		Animation description holding the animation function
 * @return							True if the animation started playing
 */
bool AnimationController::replay(const AnimationDescription* animationDescription)
{
	if(NULL == this->animationFunction)
	{
		return false;
	}

	if(this->animationCoordinator)
	{
		if(!AnimationCoordinator::playAnimation(this->animationCoordinator, this, animationDescription, this->animationFunction->name))
		{
			return false;
		}
	}

	// reset frame to play
	this->actualFrame = 0;

	// Reset frame duration
	AnimationController::resetFrameDuration(this);

	// it's playing now
	this->playing = true;

	// Tell anyone listening
	AnimationController::fireEvent(this, kEventAnimationStarted);

	return true;
}

/**
 * Stop the currently playing animation
 *
 * @private
 */
void AnimationController::stop()
{
	this->animationFunction = NULL;
	this->playing = false;
	this->actualFrame = 0;
}

/**
 * Skip to next frame
 */
void AnimationController::nextFrame()
{
	if(!this->animationFunction)
	{
		return;
	}

	if(this->actualFrame < (this->animationFunction->numberOfFrames - 1))
	{
		this->actualFrame++;
	}
	else if(this->animationFunction->loop)
	{
		this->actualFrame = 0;
	}
	else
	{
		AnimationController::fireEvent(this, kEventAnimationCompleted);
		NM_ASSERT(!isDeleted(this), "AnimationController::nextFrame: deleted this during kEventAnimationCompleted");
	}
}

/**
 * Rewind to previous frame
 */
void AnimationController::previousFrame()
{
	if(!this->animationFunction)
	{
		return;
	}

	if(this->actualFrame > 0)
	{
		this->actualFrame--;
	}
	else if(this->animationFunction->loop)
	{
		this->actualFrame = (this->animationFunction->numberOfFrames - 1);
	}
	else
	{
		AnimationController::fireEvent(this, kEventAnimationCompleted);
		NM_ASSERT(!isDeleted(this), "AnimationController::previousFrame: deleted this during kEventAnimationCompleted");
	}
}

/**
 * Check if a given animation function is playing
 *
 * @private
 * @param functionName			The animation function's name
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
 * @private
 * @return							True if there is an animation playing
 */
bool AnimationController::isPlaying()
{
	return this->playing;
}

/**
 * Pause the currently playing animation
 *
 * @private
 * @param pause			Flag to pause or to resume animation
 */
void AnimationController::pause(bool pause)
{
	this->playing = !pause;

	if(pause && -1 == this->actualFrame)
	{
		this->actualFrame = 0;
	}
}

/**
 * Get the total number of frames of the current animation
 *
 * @private
 */
int AnimationController::getNumberOfFrames()
{
	if(this->animationFunction)
	{
		return this->animationFunction->numberOfFrames;
	}

	return -1;
}
