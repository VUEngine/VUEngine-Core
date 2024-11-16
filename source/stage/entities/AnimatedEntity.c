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

#include <AnimationController.h>
#include <Sprite.h>
#include <VirtualList.h>

#include "AnimatedEntity.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;
friend class Sprite;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::constructor(AnimatedEntitySpec* animatedEntitySpec, int16 internalId, const char* const name)
{
	// construct base object
	Base::constructor(&animatedEntitySpec->entitySpec, internalId, name);

	// save ROM spec
	this->animationFunctions = animatedEntitySpec->animationFunctions;

	this->playingAnimationName = NULL;
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::ready(bool recursive)
{
	ASSERT(this->entitySpec, "AnimatedEntity::ready: null animatedEntitySpec");

	Base::ready(this, recursive);

	AnimatedEntity::playAnimation(this, ((AnimatedEntitySpec*)this->entitySpec)->initialAnimation);
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::resume()
{
	Base::resume(this);

	AnimatedEntity::playAnimation(this, this->playingAnimationName);
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
bool AnimatedEntity::playAnimation(const char* animationName)
{
	if(NULL == this->sprites || NULL == animationName)
	{
		return false;
	}

	ListenerObject scope = ListenerObject::safeCast(this);

	bool result = false;

	// play animation on each sprite
	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		if(Sprite::play(node->data, this->animationFunctions, animationName, scope))
		{
			result = true;
			scope = NULL;
		}
	}

	if(result)
	{
		this->playingAnimationName = animationName;
	}

	return result;
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::pauseAnimation(bool pause)
{
	ASSERT(this->sprites, "AnimatedEntity::pauseAnimation: null sprites");

	if(isDeleted(this->sprites))
	{
		return;
	}

	// play animation on each sprite
	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		Sprite::pause(node->data, pause);
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::stopAnimation()
{
	if(isDeleted(this->sprites))
	{
		return;
	}

	this->playingAnimationName = NULL;

	// play animation on each sprite
	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		Sprite::stop(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------
bool AnimatedEntity::isPlaying()
{
	ASSERT(this->sprites, "AnimatedEntity::isPlaying: null sprites");

	return Sprite::isPlaying(Sprite::safeCast(VirtualNode::getData(this->sprites->head)));
}
//---------------------------------------------------------------------------------------------------------
bool AnimatedEntity::isPlayingAnimation(char* animationName)
{
	if(isDeleted(this->sprites))
	{
		return false;
	}

	Sprite sprite = Sprite::safeCast(VirtualNode::getData(this->sprites->head));

	return Sprite::isPlayingAnimation(sprite, animationName);
}
//---------------------------------------------------------------------------------------------------------
const char* AnimatedEntity::getPlayingAnimationName()
{
	if(isDeleted(this->sprites))
	{
		return "None";
	}

	Sprite sprite = Sprite::safeCast(VirtualNode::getData(this->sprites->head));

	return Sprite::getPlayingAnimationName(sprite);
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::setActualFrame(int16 frame)
{
	if(isDeleted(this->sprites))
	{
		return;
	}

	for(VirtualNode node = this->sprites->head; node ; node = node->next)
	{
		Sprite::setActualFrame(node->data, frame);
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::nextFrame()
{
	if(isDeleted(this->sprites))
	{
		return;
	}

	// do on each sprite
	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		Sprite::nextFrame(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::previousFrame()
{
	if(isDeleted(this->sprites))
	{
		return;
	}

	// do on each sprite
	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		Sprite::previousFrame(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------
int16 AnimatedEntity::getActualFrame()
{
	if(isDeleted(this->sprites))
	{
		return -1;
	}

	for(VirtualNode node = this->sprites->head; node ; node = node->next)
	{
		return Sprite::getActualFrame(node->data);
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------
int32 AnimatedEntity::getNumberOfFrames()
{
	if(isDeleted(this->sprites))
	{
		return -1;
	}

	for(VirtualNode node = this->sprites->head; node ; node = node->next)
	{
		AnimationController animationController = Sprite::getAnimationController(node->data);
		return AnimationController::getNumberOfFrames(animationController);
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------
