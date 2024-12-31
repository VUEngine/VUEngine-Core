/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <string.h>

#include <AnimationCoordinator.h>
#include <AnimationCoordinatorFactory.h>
#include <DebugConfig.h>

#include "AnimationController.h"


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

extern int32 strcmp(const char *, const char *);


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void AnimationController::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// initialize frame tracking
	this->actualFrame = 0;
	this->actualFrameIndex = -1;

	// initialize frame head
	this->frameDuration = 0;
	this->frameDurationDecrement = 1;

	// initialize animation function
	this->animationFunction = NULL;
	this->animationCoordinator = NULL;

	// not playing anything yet
	this->playing = false;
}
//---------------------------------------------------------------------------------------------------------
void AnimationController::destructor()
{
	if(!isDeleted(this->animationCoordinator))
	{
		AnimationCoordinator::removeAnimationController(this->animationCoordinator, this);
		this->animationCoordinator = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
bool AnimationController::play(const AnimationFunction* animationFunctions[], const char* animationName, ListenerObject scope, EventListener callback)
{
	if(NULL == animationFunctions || NULL == animationName)
	{
		return false;
	}

	ASSERT(NULL != animationFunctions, "AnimationController::play: null animationFunctions");
	ASSERT(NULL != animationName, "AnimationController::play: null animationName");

	if(!isDeleted(this->animationCoordinator))
	{
		if(!AnimationCoordinator::playAnimation(this->animationCoordinator, this, animationFunctions, animationName))
		{
			return false;
		}
	}

	bool functionNameFound = NULL != this->animationFunction ? 0 == strncmp((const char *)animationName, (const char *)this->animationFunction->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH) : false;

	if(!functionNameFound)
	{
		int32 i = 0;

		// Look for the animation function
		for(; NULL != animationFunctions[i]; i++ )
		{
			// compare function's names
			if(0 == strncmp((const char *)animationName, (const char *)animationFunctions[i]->name, __MAX_ANIMATION_FUNCTION_NAME_LENGTH))
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

	if(!isDeleted(scope))
	{
		if(NULL != this->animationFunction->onAnimationComplete)
		{
			AnimationController::addEventListener(this, scope, this->animationFunction->onAnimationComplete, kEventAnimationCompleted);
		}
		
		if(NULL != callback)
		{
			AnimationController::addEventListener(this, scope, callback, kEventAnimationCompleted);
		}		
	}

	this->actualFrame = 0;

	AnimationController::resetFrameDuration(this);

	this->playing = true;

	return true;
}
//--------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void AnimationController::pause(bool pause)
{
	this->playing = !pause;

	if(pause && -1 == this->actualFrame)
	{
		this->actualFrame = 0;
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimationController::stop()
{
	this->animationFunction = NULL;
	this->playing = false;
	this->actualFrame = 0;
}
//---------------------------------------------------------------------------------------------------------
bool AnimationController::updateAnimation()
{
	// first check for a valid animation function
	if(!this->playing || NULL == this->animationFunction)
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
	if(this->frameDuration > this->frameDurationDecrement)
	{
		this->frameDuration -= this->frameDurationDecrement;
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
			if(NULL != this->animationFunction->onAnimationComplete && !isDeleted(this->events))
			{
				AnimationController::fireEvent(this, kEventAnimationCompleted);
				NM_ASSERT(!isDeleted(this), "AnimationController::updateAnimation: deleted this during kEventAnimationCompleted");
			}
		}

		// Reset frame duration
		AnimationController::resetFrameDuration(this);

		uint16 actualFrameIndex = this->animationFunction->frames[this->actualFrame];

		bool frameValueChanged = this->actualFrameIndex != actualFrameIndex || actualFrameIndex != this->animationFunction->frames[actualFrame];
		this->actualFrameIndex = actualFrameIndex;
		
		return frameValueChanged;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
bool AnimationController::isPlaying()
{
	return this->playing;
}
//---------------------------------------------------------------------------------------------------------
bool AnimationController::isPlayingFunction(const char* animationName)
{
	if(NULL == animationName || NULL == this->animationFunction)
	{
		return false;
	}

	// compare function's names
	return !strcmp((const char *)animationName, (const char *)this->animationFunction->name);
}
//---------------------------------------------------------------------------------------------------------
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
	else if(!isDeleted(this->events))
	{
		AnimationController::fireEvent(this, kEventAnimationCompleted);
		NM_ASSERT(!isDeleted(this), "AnimationController::nextFrame: deleted this during kEventAnimationCompleted");
	}
}
//---------------------------------------------------------------------------------------------------------
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
	else if(!isDeleted(this->events))
	{
		AnimationController::fireEvent(this, kEventAnimationCompleted);
		NM_ASSERT(!isDeleted(this), "AnimationController::previousFrame: deleted this during kEventAnimationCompleted");
	}
}
//---------------------------------------------------------------------------------------------------------
bool AnimationController::setActualFrame(int16 actualFrame)
{
	if(0 > actualFrame)
	{
		actualFrame = -1;
	}

	if(NULL != this->animationFunction && 0 <= actualFrame)
	{
		bool updatedActualFrame = this->actualFrame != actualFrame && 0 <= actualFrame;

		this->actualFrame = actualFrame;

		return updatedActualFrame;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
int16 AnimationController::getActualFrame()
{
	return this->actualFrame;
}
//---------------------------------------------------------------------------------------------------------
int16 AnimationController::getActualFrameIndex()
{
	return NULL == this->animationFunction ? 0 : this->animationFunction->frames[this->actualFrame];
}
//---------------------------------------------------------------------------------------------------------
void AnimationController::setFrameDuration(uint8 frameDuration)
{
	this->frameDuration = frameDuration;
}
//---------------------------------------------------------------------------------------------------------
uint8 AnimationController::getFrameDuration()
{
	return this->frameDuration;
}
//---------------------------------------------------------------------------------------------------------
void AnimationController::setFrameDurationDecrement(uint8 frameDurationDecrement)
{
	this->frameDurationDecrement = frameDurationDecrement;
}
//---------------------------------------------------------------------------------------------------------
uint8 AnimationController::getFrameDurationDecrement()
{
	return this->frameDurationDecrement;
}
//---------------------------------------------------------------------------------------------------------
void AnimationController::setAnimationCoordinator(AnimationCoordinator animationCoordinator)
{
	// animation coordinator
	this->animationCoordinator = animationCoordinator; 
}
//---------------------------------------------------------------------------------------------------------
AnimationCoordinator AnimationController::getAnimationCoordinator()
{
	return this->animationCoordinator;
}
//---------------------------------------------------------------------------------------------------------
const AnimationFunction* AnimationController::getPlayingAnimationFunction()
{
	return this->playing ? this->animationFunction : NULL;
}
//---------------------------------------------------------------------------------------------------------
const char* AnimationController::getPlayingAnimationName()
{
	if(NULL != this->animationFunction)
	{
		return (const char *)this->animationFunction->name;
	}

	return "None";
}
//---------------------------------------------------------------------------------------------------------
int32 AnimationController::getNumberOfFrames()
{
	if(NULL != this->animationFunction)
	{
		return this->animationFunction->numberOfFrames;
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------

