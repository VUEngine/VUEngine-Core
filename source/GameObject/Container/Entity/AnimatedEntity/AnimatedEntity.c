/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <SpriteManager.h>

#include "AnimatedEntity.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::constructor(AnimatedEntitySpec* animatedEntitySpec, int16 internalId, const char* const name)
{
	// Always explicitly call the base's constructor 
	Base::constructor(&animatedEntitySpec->entitySpec, internalId, name);

	this->playingAnimationName = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::ready(bool recursive)
{
	ASSERT(this->entitySpec, "AnimatedEntity::ready: null animatedEntitySpec");

	Base::ready(this, recursive);

	AnimatedEntity::playAnimation(this, ((AnimatedEntitySpec*)this->entitySpec)->initialAnimation);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::resume()
{
	Base::resume(this);

	AnimatedEntity::playAnimation(this, this->playingAnimationName);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool AnimatedEntity::handlePropagatedString(const char* string __attribute__ ((unused)))
{
	/* TODO: play only if the string contains the correct command */
	/*
	if (NULL == strnstr(string, __MAX_ANIMATION_FUNCTION_NAME_LENGTH, __ANIMATION_COMMAND)) 
	{
		return false;
	}
	*/

	AnimatedEntity::playAnimation(this, string);
	
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::playAnimation(const char* animationName)
{
	this->playingAnimationName = animationName;

	SpriteManager::propagateCommand
	(
		SpriteManager::getInstance(), cVisualComponentCommandPlay, GameObject::safeCast(this), 
		((AnimatedEntitySpec*)this->entitySpec)->animationFunctions, animationName, ListenerObject::safeCast(this), 
		(EventListener)AnimatedEntity::onAnimationComplete
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::pauseAnimation(bool pause)
{
	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandPause, GameObject::safeCast(this), pause);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::stopAnimation()
{
	this->playingAnimationName = NULL;

	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandStop, GameObject::safeCast(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::setActualFrame(int16 frame)
{
	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandSetFrame, GameObject::safeCast(this), frame);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::nextFrame()
{
	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandNextFrame, GameObject::safeCast(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimatedEntity::previousFrame()
{
	SpriteManager::propagateCommand(SpriteManager::getInstance(), cVisualComponentCommandPreviousFrame, GameObject::safeCast(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool AnimatedEntity::isPlaying()
{
	return NULL != this->playingAnimationName;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool AnimatedEntity::isPlayingAnimation(char* animationName)
{
	return 0 == strcmp(this->playingAnimationName, animationName);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

const char* AnimatedEntity::getPlayingAnimationName()
{
	return this->playingAnimationName;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool AnimatedEntity::onAnimationComplete(ListenerObject eventFirer __attribute__((unused)))
{
	this->playingAnimationName = NULL;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

