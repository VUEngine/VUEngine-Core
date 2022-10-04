/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern int32 strcmp(const char *, const char *);


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
int16 AnimationController::getActualFrameIndex()
{
	return this->animationFunction ? this->animationFunction->frames[this->actualFrame] : 0;
}

/**
 * Retrieve the actual frame of animation
 *
 * @private
 * @return 		Actual frame of animation
 */
int16 AnimationController::getActualFrame()
{
	return this->actualFrame;
}

/**
 * Retrieve the previous frame of animation
 *
 * @publics
 * @return 		Previous frame of animation
 */
int16 AnimationController::getPreviousFrameValue()
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
bool AnimationController::setActualFrame(int16 actualFrame)
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
uint8 AnimationController::getFrameDuration()
{
	return this->frameDuration;
}

/**
 * Set the number of cycles that each frame of animation is shown
 *
 * @private
 * @param frameDuration	Number of cycles that each frame of animation is shown
 */
void AnimationController::setFrameDuration(uint8 frameDuration)
{
	this->frameDuration = frameDuration;
}

/**
 * Retrieve the frame duration decrement per cycle
 *
 * @private
 * @return		Frame cycle decrement
 */
uint8 AnimationController::getFrameCycleDecrement()
{
	return this->frameCycleDecrement;
}

/**
 * Set the frame duration decrement per cycle
 *
 * @private
 * @param frameCycleDecrement	Decrement value for the frame cycle on each game cycle
 */
void AnimationController::setFrameCycleDecrement(uint8 frameCycleDecrement)
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
		int16 actualFrame = this->actualFrame;

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

		uint8 actualFrameValue = this->animationFunction->frames[this->actualFrame];

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
void AnimationController::playAnimationFunction(const AnimationFunction* animationFunction, ListenerObject scope)
{
	ASSERT(animationFunction, "AnimationController::playAnimationFunction: null animationFunction");

	// remove previous listeners
	if(NULL != this->animationFunction && NULL != this->animationFunction->onAnimationComplete)
	{
		AnimationController::removeEventListeners(this, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
	}

	// setup animation frame
	this->animationFunction = animationFunction;

	// register event callback
	if(!isDeleted(scope) && NULL != this->animationFunction && NULL != this->animationFunction->onAnimationComplete)
	{
		AnimationController::addEventListener(this, scope, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
	}

	// reset frame to play
	this->actualFrame = 0;

	// Reset frame duration
	AnimationController::resetFrameDuration(this);

	// it's playing now
	this->playing = true;
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
 * Play an animation given an animation function and the name of an AnimationFunction
 *
 * @public
 * @param animationFunctions		Animation description holding the animation function
 * @param functionName				Name of the animation function's to play
 * @return							True if the animation started playing
 */
bool AnimationController::play(const AnimationFunction* animationFunctions[], const char* functionName, ListenerObject scope)
{
	if(NULL == animationFunctions || NULL == functionName)
	{
		return false;
	}

	ASSERT(NULL != animationFunctions, "AnimationController::play: null animationFunctions");
	ASSERT(NULL != functionName, "AnimationController::play: null functionName");

	if(!isDeleted(this->animationCoordinator))
	{
		if(!AnimationCoordinator::playAnimation(this->animationCoordinator, this, animationFunctions, functionName))
		{
			return false;
		}
	}

	bool functionNameFound = false;

	if(NULL == this->animationFunction || 0 != strncmp((const char *)functionName, (const char *)this->animationFunction->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH))
	{
		int32 i = 0;

		// search for the animation function
		for(; NULL != animationFunctions[i]; i++ )
		{
			// compare function's names
			if(0 == strncmp((const char *)functionName, (const char *)animationFunctions[i]->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH))
			{
				// remove previous listeners
				if(NULL != this->animationFunction && NULL != this->animationFunction->onAnimationComplete)
				{
					AnimationController::removeEventListeners(this, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
				}

				this->animationFunction = animationFunctions[i];

				functionNameFound = true;

				break;
			}
		}
	}

	if(NULL == this->animationFunction || !functionNameFound)
	{
		return false;
	}

	// setup animation frame
	if(!isDeleted(scope) && NULL != this->animationFunction->onAnimationComplete)
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

	return true;
}

/**
 * Replay the last animation if any
 *
 * @public
 * @param animationFunctions		Animation description holding the animation function
 * @return							True if the animation started playing
 */
bool AnimationController::replay(const AnimationFunction* animationFunctions[])
{
	if(NULL == this->animationFunction)
	{
		return false;
	}

	if(!isDeleted(this->animationCoordinator))
	{
		if(!AnimationCoordinator::playAnimation(this->animationCoordinator, this, animationFunctions, this->animationFunction->name))
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
int32 AnimationController::getNumberOfFrames()
{
	if(this->animationFunction)
	{
		return this->animationFunction->numberOfFrames;
	}

	return -1;
}
