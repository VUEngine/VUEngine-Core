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

#include <AnimatedEntity.h>
#include <Clock.h>
#include <MessageDispatcher.h>
#include <Optics.h>
#include <Camera.h>
#include <Shape.h>
#include <PhysicalWorld.h>
#include <Body.h>
#include <Box.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;
friend class Sprite;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void AnimatedEntity::constructor(AnimatedEntitySpec* animatedEntitySpec, int16 internalId, const char* const name)
{
	// construct base object
	Base::constructor(&animatedEntitySpec->entitySpec, internalId, name);

	// save ROM spec
	this->animationFunctions = animatedEntitySpec->animationFunctions;

	this->currentAnimationName = NULL;
}

// class's destructor
void AnimatedEntity::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

// ready method
void AnimatedEntity::ready(bool recursive)
{
	ASSERT(this->entitySpec, "AnimatedEntity::ready: null animatedEntitySpec");

	Base::ready(this, recursive);

	AnimatedEntity::playAnimation(this, ((AnimatedEntitySpec*)this->entitySpec)->initialAnimation);

	AnimatedEntity::setupListeners(this);
}

void AnimatedEntity::setupListeners()
{
	if(isDeleted(this->sprites))
	{
		return;
	}

	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		Sprite sprite = Sprite::safeCast(node->data);
		AnimationController animationController = Sprite::getAnimationController(sprite);

		if(!isDeleted(animationController) && !isDeleted(AnimationController::getAnimationCoordinator(animationController)))
		{
			AnimationController::addEventListener(animationController, ListenerObject::safeCast(this), (EventListener)AnimatedEntity::onAnimationStarted, kEventAnimationStarted);
		}
	}

	this->update = true;
}

void AnimatedEntity::onAnimationStarted(ListenerObject eventFirer __attribute__ ((unused)))
{
	this->update = true;
}

// execute character's logic
void AnimatedEntity::update()
{
	// call base
	Base::update(this);

	AnimatedEntity::animate(this);
}

// update animations
void AnimatedEntity::animate()
{
	if(isDeleted(this->sprites))
	{
		return;
	}

	bool stillPlaying = false;

	// move each child to a temporary list
	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		// first animate the frame
		stillPlaying |= Sprite::updateAnimation(node->data);
	}

	this->update = stillPlaying || AnimatedEntity::overrides(this, update);
}

// pause animation
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

	this->update = !pause || AnimatedEntity::overrides(this, update);
}

// play an animation
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
		this->update = true;
		this->currentAnimationName = animationName;
	}

	return result;
}

// play an animation
void AnimatedEntity::stopAnimation()
{
	if(!this->sprites)
	{
		return;
	}

	this->currentAnimationName = NULL;

	// play animation on each sprite
	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		Sprite::stop(node->data);
	}
}

// skip to next frame
void AnimatedEntity::nextFrame()
{
	if(!this->sprites)
	{
		return;
	}

	// do on each sprite
	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		Sprite::nextFrame(node->data);
	}
}

// rewind to previous frame
void AnimatedEntity::previousFrame()
{
	if(!this->sprites)
	{
		return;
	}

	// do on each sprite
	for(VirtualNode node = this->sprites->head; node && this->sprites; node = node->next)
	{
		Sprite::previousFrame(node->data);
	}
}

// is playing an animation
bool AnimatedEntity::isPlayingAnimation()
{
	ASSERT(this->sprites, "AnimatedEntity::isPlayingAnimation: null sprites");

	return Sprite::isPlaying(Sprite::safeCast(VirtualNode::getData(this->sprites->head)));
}

// is animation selected
bool AnimatedEntity::isAnimationLoaded(char* functionName)
{
	if(!this->sprites)
	{
		return false;
	}

	Sprite sprite = Sprite::safeCast(VirtualNode::getData(this->sprites->head));

	return Sprite::isPlayingFunction(sprite, functionName);
}

// get animation spec
const AnimationFunction** AnimatedEntity::getAnimationFunctions()
{
	return this->animationFunctions;
}

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

// set animation description
void AnimatedEntity::setAnimationFunction(const AnimationFunction** animationFunctions)
{
	this->animationFunctions = animationFunctions;

	AnimatedEntity::stopAnimation(this);
}

// resume method
void AnimatedEntity::resume()
{
	Base::resume(this);

	AnimatedEntity::playAnimation(this, this->currentAnimationName);

	AnimatedEntity::setupListeners(this);
}

/**
 * Handle propagated string
 *
 * @param message	Message

 * @return			Result
 */
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


int16 AnimatedEntity::getActualFrame()
{
	if(!this->sprites)
	{
		return -1;
	}

	for(VirtualNode node = this->sprites->head; node ; node = node->next)
	{
		return Sprite::getActualFrame(node->data);
	}

	return -1;
}

int32 AnimatedEntity::getNumberOfFrames()
{
	if(!this->sprites)
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

void AnimatedEntity::onAnimationCompleteHide(ListenerObject eventFirer __attribute__((unused)))
{
	AnimatedEntity::hide(this);
}