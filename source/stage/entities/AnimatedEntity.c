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

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void AnimatedEntity::constructor(AnimatedEntitySpec* animatedEntitySpec, s16 id, s16 internalId, const char* const name)
{
	// construct base object
	Base::constructor(&animatedEntitySpec->entitySpec, id, internalId, name);

	// save ROM spec
	this->animatedEntitySpec = animatedEntitySpec;
	this->animationDescription = animatedEntitySpec->animationDescription;

	this->currentAnimationName = NULL;
}

// class's destructor
void AnimatedEntity::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

// set spec
void AnimatedEntity::setSpec(void* animatedEntitySpec)
{
	ASSERT(animatedEntitySpec, "AnimatedEntity::setSpec: null spec");

	// save spec
	this->animatedEntitySpec = animatedEntitySpec;

	Base::setSpec(this, &((AnimatedEntitySpec*)animatedEntitySpec)->entitySpec);
}

// ready method
void AnimatedEntity::ready(bool recursive)
{
	ASSERT(this->animatedEntitySpec, "AnimatedEntity::ready: null animatedEntitySpec");

	Base::ready(this, recursive);

	AnimatedEntity::playAnimation(this, this->animatedEntitySpec->initialAnimation);
}

// execute character's logic
void AnimatedEntity::update(u32 elapsedTime)
{
	// call base
	Base::update(this, elapsedTime);

	if(!elapsedTime)
	{
		return;
	}

	AnimatedEntity::animate(this);
}

// update animations
void AnimatedEntity::animate()
{
	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// move each child to a temporary list
	for(; node && this->sprites; node = node->next)
	{
		// first animate the frame
		Sprite::updateAnimation(node->data);
	}
}

// pause animation
void AnimatedEntity::pauseAnimation(bool pause)
{
	ASSERT(this->sprites, "AnimatedEntity::pauseAnimation: null sprites");

	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// play animation on each sprite
	for(; node && this->sprites; node = node->next)
	{
		Sprite::pause(node->data, pause);
	}
}

// play an animation
void AnimatedEntity::playAnimation(char* animationName)
{
	if(!this->sprites | !animationName)
	{
		return;
	}

	this->currentAnimationName = animationName;

	VirtualNode node = this->sprites->head;

	// play animation on each sprite
	for(; node && this->sprites; node = node->next)
	{
		Sprite::play(node->data, this->animationDescription, animationName);
	}
}

// skip to next frame
void AnimatedEntity::nextFrame()
{
	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;

	// do on each sprite
	for(; node && this->sprites; node = node->next)
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

	VirtualNode node = this->sprites->head;

	// do on each sprite
	for(; node && this->sprites; node = node->next)
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
AnimationDescription* AnimatedEntity::getAnimationDescription()
{
	return this->animationDescription;
}

void AnimatedEntity::setActualFrame(s16 frame)
{
	if(!this->sprites)
	{
		return;
	}

	VirtualNode node = this->sprites->head;
	for(; node ; node = node->next)
	{
		Sprite::setActualFrame(node->data, frame);
	}
}

// set animation description
void AnimatedEntity::setAnimationDescription(AnimationDescription* animationDescription)
{
	this->animationDescription = animationDescription;
}

// resume method
void AnimatedEntity::resume()
{
	Base::resume(this);

	AnimatedEntity::playAnimation(this, this->currentAnimationName);
}

s16 AnimatedEntity::getActualFrame()
{
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

int AnimatedEntity::getNumberOfFrames()
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

void AnimatedEntity::onAnimationCompleteHide(Object eventFirer __attribute__((unused)))
{
	AnimatedEntity::hide(this);
}