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

	this->animatedSprites = NULL;
	this->animationFunctions = animatedEntitySpec->animationFunctions;
	this->playingAnimationName = NULL;
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::destructor()
{
	if(!isDeleted(this->animatedSprites))
	{
		delete this->animatedSprites;
		this->animatedSprites = NULL;
	}
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
	if(NULL == animationName)
	{
		return false;
	}

	AnimatedEntity::getSprites(this);

	ListenerObject scope = ListenerObject::safeCast(this);

	bool result = false;

	// play animation on each sprite
	for(VirtualNode node = this->animatedSprites->head; NULL != node && NULL != this->animatedSprites; node = node->next)
	{
		NM_ASSERT(!isDeleted(Sprite::safeCast(node->data)), "AnimatedEntity::playAnimation: invalid sprite node");

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
	AnimatedEntity::getSprites(this);

	// play animation on each sprite
	for(VirtualNode node = this->animatedSprites->head; node && this->animatedSprites; node = node->next)
	{
		Sprite::pause(node->data, pause);
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::stopAnimation()
{
	AnimatedEntity::getSprites(this);

	this->playingAnimationName = NULL;

	// play animation on each sprite
	for(VirtualNode node = this->animatedSprites->head; node && this->animatedSprites; node = node->next)
	{
		Sprite::stop(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------
bool AnimatedEntity::isPlaying()
{
	AnimatedEntity::getSprites(this);

	return Sprite::isPlaying(Sprite::safeCast(VirtualNode::getData(this->animatedSprites->head)));
}
//---------------------------------------------------------------------------------------------------------
bool AnimatedEntity::isPlayingAnimation(char* animationName)
{
	AnimatedEntity::getSprites(this);

	Sprite sprite = Sprite::safeCast(VirtualNode::getData(this->animatedSprites->head));

	return Sprite::isPlayingAnimation(sprite, animationName);
}
//---------------------------------------------------------------------------------------------------------
const char* AnimatedEntity::getPlayingAnimationName()
{
	AnimatedEntity::getSprites(this);

	Sprite sprite = Sprite::safeCast(VirtualNode::getData(this->animatedSprites->head));

	return Sprite::getPlayingAnimationName(sprite);
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::setActualFrame(int16 frame)
{
	AnimatedEntity::getSprites(this);

	for(VirtualNode node = this->animatedSprites->head; node ; node = node->next)
	{
		Sprite::setActualFrame(node->data, frame);
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::nextFrame()
{
	AnimatedEntity::getSprites(this);

	// do on each sprite
	for(VirtualNode node = this->animatedSprites->head; node && this->animatedSprites; node = node->next)
	{
		Sprite::nextFrame(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::previousFrame()
{
	AnimatedEntity::getSprites(this);

	// do on each sprite
	for(VirtualNode node = this->animatedSprites->head; node && this->animatedSprites; node = node->next)
	{
		Sprite::previousFrame(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------
int16 AnimatedEntity::getActualFrame()
{
	AnimatedEntity::getSprites(this);

	for(VirtualNode node = this->animatedSprites->head; node ; node = node->next)
	{
		return Sprite::getActualFrame(node->data);
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------
int32 AnimatedEntity::getNumberOfFrames()
{
	AnimatedEntity::getSprites(this);

	for(VirtualNode node = this->animatedSprites->head; node ; node = node->next)
	{
		AnimationController animationController = Sprite::getAnimationController(node->data);
		return AnimationController::getNumberOfFrames(animationController);
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void AnimatedEntity::getSprites()
{
	if(isDeleted(this->animatedSprites))
	{
		this->animatedSprites = new VirtualList();	
	}

	SpriteManager::getSprites(SpriteManager::getInstance(), SpatialObject::safeCast(this), this->animatedSprites);
}
//---------------------------------------------------------------------------------------------------------
