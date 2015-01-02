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

#ifndef ANIMATEDSPRITE_H_
#define ANIMATEDSPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// event
#define __EVENT_ANIMATION_COMPLETE	"animationComplete"

// max length of an animation function's name
#define __MAX_ANIMATION_FUNCTION_NAME_LENGTH	20

// max number of frames per animation function
#define __MAX_FRAMES_PER_ANIMATION_FUNCTION		16

// max number of animation functions per description
#define __MAX_ANIMATION_FUNCTIONS				32


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define AnimatedSprite_METHODS													\
		Sprite_METHODS															\

// declare the virtual methods which are redefined
#define AnimatedSprite_SET_VTABLE(ClassName)									\
		Sprite_SET_VTABLE(ClassName)											\

#define AnimatedSprite_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Sprite_ATTRIBUTES;															\
																				\
	/* who owns the animated sprite */											\
	Object owner;																\
																				\
	/* actual animation's frame to show */										\
	s8 actualFrame;																\
																				\
	/* previous animation's frame shown */										\
	s8 previousFrame;															\
																				\
	/* actual frame cicle in a given direction (i.e. when walking) */			\
	s8 frameDelay;																\
																				\
	/* frame delay increment update cycle */									\
	s8 frameDelayDelta;															\
																				\
	/* orignal position of the bgmap definition in ROM */						\
	int originalTextureXOffset;													\
																				\
	/* a pointer to the animation function being played */						\
	AnimationFunction* animationFunction;										\
																				\
	/* flag to project 3d to 2d position if needed */							\
	int calculatePositionFlag: 1;												\
																				\
	/* flag to know if playing an animation */									\
	int playing: 1;																\

// declare a Sprite, which holds a texture and a drawing specification
__CLASS(AnimatedSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// a function which defines the frames to play
typedef struct AnimationFunction
{
	// number of frames of this animation function
	int numberOfFrames;

	// frames to play in animation
	int frames[__MAX_FRAMES_PER_ANIMATION_FUNCTION];

	// number of cicles a frame of animation is displayed
	int delay;

	// whether to play it in loop or not
	int loop;

	// method to call function completion
	void* onAnimationComplete;

	// function's name
	char name[__MAX_ANIMATION_FUNCTION_NAME_LENGTH];

} AnimationFunction;

typedef const AnimationFunction AnimationFunctionROMDef;

// an animation definition
typedef struct AnimationDescription
{
	// total number of frames
	int numberOfFrames;

	// animation functions
	AnimationFunction* animationFunctions[__MAX_ANIMATION_FUNCTIONS];

} AnimationDescription;

typedef const AnimationDescription AnimationDescriptionROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(AnimatedSprite, __PARAMETERS(const SpriteDefinition* spriteDefinition, Object owner));

void AnimatedSprite_destructor(AnimatedSprite this);
int AnimatedSprite_getType(AnimatedSprite this);
Texture AnimatedSprite_getTexture(AnimatedSprite this);
void AnimatedSprite_writeAnimation(AnimatedSprite this);
void AnimatedSprite_setCalculatePositionFlag(AnimatedSprite this, int calculatePositionFlag);
s8 AnimatedSprite_getActualFrame(AnimatedSprite this);
s8 AnimatedSprite_getPreviousFrame(AnimatedSprite this);
void AnimatedSprite_setActualFrame(AnimatedSprite this, s8 actualFrame);
void AnimatedSprite_setPreviousFrame(AnimatedSprite this, s8 previousFrame);
s8 AnimatedSprite_getFrameDelay(AnimatedSprite this);
void AnimatedSprite_setFrameDelay(AnimatedSprite this, u8 frameDelay);
u8 AnimatedSprite_geFrameDelayDelta(AnimatedSprite this);
void AnimatedSprite_setFrameDelayDelta(AnimatedSprite this, u8 frameDelayDelta);
void AnimatedSprite_animate(AnimatedSprite this);
void AnimatedSprite_fx(AnimatedSprite this);
void AnimatedSprite_update(AnimatedSprite this, Clock clock);
u8 AnimatedSprite_getRows(AnimatedSprite this);
u8 AnimatedSprite_getCols(AnimatedSprite this);
int AnimatedSprite_getMapType(AnimatedSprite this);
void AnimatedSprite_playAnimationFunction(AnimatedSprite this, AnimationFunction* animationFunction);
void AnimatedSprite_play(AnimatedSprite this, AnimationDescription* animationDescription, char* functionName);
bool AnimatedSprite_isPlayingFunction(AnimatedSprite this, AnimationDescription* animationDescription, char* functionName);
bool AnimatedSprite_isPlaying(AnimatedSprite this);
void AnimatedSprite_write(AnimatedSprite this);
void AnimatedSprite_pause(AnimatedSprite this, int pause);


#endif