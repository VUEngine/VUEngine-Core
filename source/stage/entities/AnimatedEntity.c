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

#include <AnimatedEntity.h>
#include <Clock.h>
#include <MessageDispatcher.h>
#include <Optics.h>
#include <Camera.h>
#include <Shape.h>
#include <PhysicalWorld.h>
#include <Body.h>
#include <Box.h>
#include <Game.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	AnimatedEntity
 * @extends Entity
 * @ingroup stage-entities
 */
implements AnimatedEntity : Entity;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void AnimatedEntity::animate(AnimatedEntity this);

AnimationController Sprite::getAnimationController(Sprite this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(AnimatedEntity, AnimatedEntityDefinition* animatedEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(AnimatedEntity, animatedEntityDefinition, id, internalId, name);

// class's constructor
void AnimatedEntity::constructor(AnimatedEntity this, AnimatedEntityDefinition* animatedEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "AnimatedEntity::constructor: null this");

	// construct base object
	Base::constructor(&animatedEntityDefinition->entityDefinition, id, internalId, name);

	// save ROM definition
	this->animatedEntityDefinition = animatedEntityDefinition;
	this->animationDescription = animatedEntityDefinition->animationDescription;

	this->currentAnimationName = NULL;
}

// class's destructor
void AnimatedEntity::destructor(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

// set definition
void AnimatedEntity::setDefinition(AnimatedEntity this, void* animatedEntityDefinition)
{
	ASSERT(this, "AnimatedEntity::setDefinition: null this");
	ASSERT(animatedEntityDefinition, "AnimatedEntity::setDefinition: null definition");

	// save definition
	this->animatedEntityDefinition = animatedEntityDefinition;

	Base::setDefinition(this, &((AnimatedEntityDefinition*)animatedEntityDefinition)->entityDefinition);
}

// ready method
void AnimatedEntity::ready(AnimatedEntity this, bool recursive)
{
	ASSERT(this, "AnimatedEntity::ready: null this");
	ASSERT(this->animatedEntityDefinition, "AnimatedEntity::ready: null animatedEntityDefinition");

	Base::ready(this, recursive);

	AnimatedEntity::playAnimation(this, this->animatedEntityDefinition->initialAnimation);
}


// execute character's logic
void AnimatedEntity::update(AnimatedEntity this, u32 elapsedTime)
{
	ASSERT(this, "AnimatedEntity::update: null this");

	// call base
	Base::update(this, elapsedTime);

	if(!elapsedTime)
	{
		return;
	}

	AnimatedEntity::animate(this);
}

// update animations
static void AnimatedEntity::animate(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::animate: null this");

	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// move each child to a temporary list
	for(; node ; node = node->next)
	{
		// first animate the frame
		Sprite::updateAnimation(__SAFE_CAST(Sprite, node->data));
	}
}

// pause animation
void AnimatedEntity::pauseAnimation(AnimatedEntity this, int pause)
{
	ASSERT(this, "AnimatedEntity::pauseAnimation: null this");
	ASSERT(this->sprites, "AnimatedEntity::pauseAnimation: null sprites");

	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// play animation on each sprite
	for(; node ; node = node->next)
	{
		Sprite::pause(__SAFE_CAST(Sprite, node->data), pause);
	}
}

// play an animation
void AnimatedEntity::playAnimation(AnimatedEntity this, char* animationName)
{
	ASSERT(this, "AnimatedEntity::playAnimation: null this");

	if(!this->sprites | !animationName)
	{
		return;
	}

	this->currentAnimationName = animationName;

	VirtualNode node = this->sprites->head;

	// play animation on each sprite
	for(; node ; node = node->next)
	{
		Sprite::play(__SAFE_CAST(Sprite, node->data), this->animationDescription, animationName);
	}
}

// skip to next frame
void AnimatedEntity::nextFrame(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::nextFrame: null this");

	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// do on each sprite
	for(; node ; node = node->next)
	{
		Sprite::nextFrame(__SAFE_CAST(Sprite, node->data));
	}
}

// rewind to previous frame
void AnimatedEntity::previousFrame(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::previousFrame: null this");

	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// do on each sprite
	for(; node ; node = node->next)
	{
		Sprite::previousFrame(__SAFE_CAST(Sprite, node->data));
	}
}

// is playing an animation
bool AnimatedEntity::isPlayingAnimation(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::isPlayingAnimation: null this");
	ASSERT(this->sprites, "AnimatedEntity::isPlayingAnimation: null sprites");

	return Sprite::isPlaying(__SAFE_CAST(Sprite, VirtualNode::getData(this->sprites->head)));
}

// is animation selected
bool AnimatedEntity::isAnimationLoaded(AnimatedEntity this, char* functionName)
{
	ASSERT(this, "AnimatedEntity::isAnimationLoaded: null this");

	if(!this->sprites)
	{
		return false;
	}

	Sprite sprite = __SAFE_CAST(Sprite, VirtualNode::getData(this->sprites->head));

	return Sprite::isPlayingFunction(__SAFE_CAST(Sprite, sprite), functionName);
}

// get animation definition
AnimationDescription* AnimatedEntity::getAnimationDescription(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::getAnimationDescription: null this");

	return this->animationDescription;
}

// set animation description
void AnimatedEntity::setAnimationDescription(AnimatedEntity this, AnimationDescription* animationDescription)
{
	ASSERT(this, "AnimatedEntity::setAnimationDescription: null this");

	this->animationDescription = animationDescription;
}

// resume method
void AnimatedEntity::resume(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::resume: null this");

	Base::resume(this);

	AnimatedEntity::playAnimation(this, this->currentAnimationName);
}

s8 AnimatedEntity::getActualFrame(AnimatedEntity this)
{
	ASSERT(this, "AnimatedEntity::getActualFrame: null this");

	if(!this->sprites)
	{
		return -1;
	}

	VirtualNode node = this->sprites->head;

	for(; node ; node = node->next)
	{
		return Sprite::getActualFrame(node->data);
	}

	return -1;
}

int AnimatedEntity::getNumberOfFrames(AnimatedEntity this)
{
	if(!this->sprites)
	{
		return -1;
	}

	VirtualNode node = this->sprites->head;

	for(; node ; node = node->next)
	{
		AnimationController animationController = Sprite::getAnimationController(node->data);
		return AnimationController::getNumberOfFrames(animationController);
	}

	return -1;
}
