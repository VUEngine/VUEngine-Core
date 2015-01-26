/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimatedSprite.h>
#include <Game.h>
#include <string.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the AnimatedSprite
__CLASS_DEFINITION(AnimatedSprite);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern int strcmp(const char *, const char *);

// class's constructor
static void AnimatedSprite_constructor(AnimatedSprite this, const SpriteDefinition* spriteDefinition, Object owner);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(AnimatedSprite, __PARAMETERS(const SpriteDefinition* spriteDefinition, Object owner))
__CLASS_NEW_END(AnimatedSprite, __ARGUMENTS(spriteDefinition, owner));

// class's constructor
static void AnimatedSprite_constructor(AnimatedSprite this, const SpriteDefinition* spriteDefinition, Object owner)
{
	// construct base object
	__CONSTRUCT_BASE(Sprite, __ARGUMENTS(spriteDefinition));

	// since the offset will be moved during animation, must save it
	this->originalTextureSource.mx = abs(Texture_getXOffset(this->texture)) << 3;
	this->originalTextureSource.my = abs(Texture_getYOffset(this->texture)) << 3;

	// set the owner
	this->owner = owner;

	// initialize frame tracking
	this->actualFrame = 0;
	this->previousFrame = 0;

	// initialize frame head
	this->frameDelay = 0;
	this->frameDelayDelta = -1;

	// don't project position
	this->calculatePositionFlag = true;

	// intialize animation function
	this->animationFunction = NULL;

	// not playing anything yet
	this->playing = false;
}

//destructor
void AnimatedSprite_destructor(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::destructor: null this");

	// destroy the super object
	__DESTROY_BASE(Sprite);
}

// retrieve frame allocation type
int AnimatedSprite_getType(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::getType: null this");

	return CharSet_getAllocationType(Texture_getCharSet(this->texture));
}

// retrieve frame's map
Texture AnimatedSprite_getTexture(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::getTexture: null this");

	return this->texture;
}

// write char animation frame to char memory
void AnimatedSprite_writeAnimation(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::writeAnimation: null this");

	// write according to the allocation type
	switch (CharSet_getAllocationType(Texture_getCharSet(this->texture)))
	{
		case __ANIMATED:

			{
				CharSet charSet = Texture_getCharSet(this->texture);

				// move charset's charset's definition to the next frame chars
				CharSet_setCharDefinitionDisplacement(charSet, Texture_getNumberOfChars(this->texture) *
						(this->animationFunction->frames[this->actualFrame] << 4));

				//write charset
				CharSet_write(charSet);
			}

			break;

		case __ANIMATED_SHARED:
			{
				int totalColumns = 64 - (this->originalTextureSource.mx >> 3); 
				int frameColumn = Texture_getCols(this->texture) * this->animationFunction->frames[this->actualFrame];
				this->drawSpec.textureSource.mx = this->originalTextureSource.mx + ((frameColumn % totalColumns) << 3);
				this->drawSpec.textureSource.my = this->originalTextureSource.my + ((frameColumn / totalColumns) << 3);
			}
			break;
	}
	
	Sprite_scale((Sprite)this);
	this->renderFlag |= __UPDATE_M | (WRLD_AFFINE & this->head? __UPDATE_SIZE: 0);
}

// if true, the frame's screen position will be calculated in the
// next render cicle
void AnimatedSprite_setCalculatePositionFlag(AnimatedSprite this, int calculatePositionFlag)
{
	ASSERT(this, "AnimatedSprite::setCalculatePositionFlag: null this");

	this->calculatePositionFlag = calculatePositionFlag;
}

// retrieve actual frame index of animation
s8 AnimatedSprite_getActualFrame(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::getActualAnimatedSprite: null this");

	return this->actualFrame;
}

// retrieve previous frame index of animation
s8 AnimatedSprite_getPreviousFrame(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::getPreviousAnimatedSprite: null this");

	return this->previousFrame;
}

// set actual frame of animation
void AnimatedSprite_setActualFrame(AnimatedSprite this, s8 actualFrame)
{
	ASSERT(this, "AnimatedSprite::setActualAnimatedSprite: null this");

	this->actualFrame = actualFrame;
}

// set previous frame index of animation
void AnimatedSprite_setPreviousFrame(AnimatedSprite this, s8 previousFrame)
{
	ASSERT(this, "AnimatedSprite::setPreviousFrame: null this");

	// TODO: this method should not exist
	this->previousFrame = previousFrame;
}

// retrieve frame delay
s8 AnimatedSprite_getFrameDelay(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::getFrameDelay: null this");

	return this->frameDelay;
}

// set frame cicle
void AnimatedSprite_setFrameDelay(AnimatedSprite this, u8 frameDelay)
{
	ASSERT(this, "AnimatedSprite::setFrameDelay: null this");

	this->frameDelay = frameDelay;
}

// retrieve frame delay delta
u8 AnimatedSprite_geFrameDelayDelta(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::getAnimatedSpriteCicleDelta: null this");

	return this->frameDelayDelta;
}

// set frame delay delta
void AnimatedSprite_setFrameDelayDelta(AnimatedSprite this, u8 frameDelayDelta)
{
	ASSERT(this, "AnimatedSprite::setAnimatedSpriteCicleDelta: null this");

	this->frameDelayDelta = frameDelayDelta;
}

// animate the frame
void AnimatedSprite_animate(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::animate: null this");

	// first check for a valid animation function
	if (!this->animationFunction)
	{
		return;
	}

	// if the actual frame was set to -1
	// it means that a not loop animation has been completed
	if (-1 == this->actualFrame)
	{
		return;
	}

	// show the next frame
	if (this->actualFrame >= this->animationFunction->numberOfFrames)
	{
		// the last frame has been reached
		if (this->animationFunction->onAnimationComplete)
		{
			Object_fireEvent((Object)this, __EVENT_ANIMATION_COMPLETE);
		}

		// rewind to first frame
		this->actualFrame = 0;

		// if the animation is not a loop
		if (!this->animationFunction->loop)
		{
			// not playing anymore
			this->playing = false;

			// invalidate animation
			this->actualFrame = -1;

			return;
		}
	}

	// if the frame has changed
	if (this->actualFrame != this->previousFrame)
	{
		// write the new frame of animation
		AnimatedSprite_writeAnimation(this);

		// don't write animation each time, only when the animation
		// has changed
		this->previousFrame = this->actualFrame;
		
		Object_fireEvent((Object)this, __EVENT_ANIMATION_FRAME_CHANGED);
	}

	this->frameDelay += this->frameDelayDelta;

	// reduce frame delay count
	if (0 >= this->frameDelay)
	{
		// incrase the frame to show
		this->previousFrame = this->actualFrame++;

		// reset frame delay
		this->frameDelay = this->animationFunction->delay;

		// if the delay is negative
		if (0 > this->frameDelay)
		{
			// pick up a random delay
			this->frameDelay = Utilities_random(Utilities_randomSeed(), abs(this->frameDelay));
		}
	}
}

// render frame
void AnimatedSprite_update(AnimatedSprite this, Clock clock)
{
	ASSERT(this, "AnimatedSprite::update: null this");

	if (this->playing && !Clock_isPaused(clock))
	{
		// first animate the frame
		AnimatedSprite_animate(this);
	}
}

// retrieve frame's map's height
u8 AnimatedSprite_getRows(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::getRows: null this");

	return Texture_getRows(this->texture);
}

// retrieve frame's map's width
u8 AnimatedSprite_getCols(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::getCols: null this");

	return Texture_getCols(this->texture);
}

// play animation
void AnimatedSprite_playAnimationFunction(AnimatedSprite this, AnimationFunction* animationFunction)
{
	ASSERT(this, "AnimatedSprite::playAnimation: null this");

	// setup animation frame
	this->animationFunction = animationFunction;

	// register event callback
	Object_addEventListener((Object)this, this->owner, this->animationFunction->onAnimationComplete, __EVENT_ANIMATION_COMPLETE);

	// force frame writing in the next update
	this->previousFrame = -1;

	// reset frame to play
	this->actualFrame = 0;

	// set frame delay to 1 to force the writing of the first
	// animation frame in the next update
	this->frameDelay = 1;

	// it's playing now
	this->playing = true;
}
// play animation
void AnimatedSprite_play(AnimatedSprite this, AnimationDescription* animationDescription, char* functionName)
{
	ASSERT(this, "AnimatedSprite::play: null this");

	int i = 0;

	// search for the animation function
	for (; animationDescription->animationFunctions[i]; i++ )
	{
		// compare function's names
		if (!strcmp((const char *)functionName, (const char *)animationDescription->animationFunctions[i]->name))
		{
			// setup animation frame
			this->animationFunction = animationDescription->animationFunctions[i];

			// register event callback
			Object_addEventListener((Object)this, this->owner, this->animationFunction->onAnimationComplete, __EVENT_ANIMATION_COMPLETE);

			// force frame writing in the next update
			this->previousFrame = -1;

			// reset frame to play
			this->actualFrame = 0;

			// set frame delay to 1 to force the writing of the first
			// animation frame in the next update
			this->frameDelay = 1;

			// it's playing now
			this->playing = true;
		}
	}
}

// is play animation
bool AnimatedSprite_isPlayingFunction(AnimatedSprite this, AnimationDescription* animationDescription, char* functionName)
{
	ASSERT(this, "AnimatedSprite::isPlayingFunction: null this");

	// compare function's names
	return !strcmp((const char *)functionName, (const char *)this->animationFunction->name);
}

// is playing animation
bool AnimatedSprite_isPlaying(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::isPlaying: null this");

	return this->playing;
}

// pause animation
void AnimatedSprite_pause(AnimatedSprite this, int pause)
{
	ASSERT(this, "AnimatedSprite::pause: null this");
	this->playing = !pause;

	if (-1 == this->actualFrame)
	{
		this->actualFrame = 0;
	}
}

// write sprite to graphic memory
void AnimatedSprite_write(AnimatedSprite this)
{
	ASSERT(this, "AnimatedSprite::write: null this");

	// write the texture
	Texture_write(this->texture);

	// save the original offset
	this->originalTextureSource.mx = abs(Texture_getXOffset(this->texture)) << 3;
	this->originalTextureSource.my = abs(Texture_getYOffset(this->texture)) << 3;
}