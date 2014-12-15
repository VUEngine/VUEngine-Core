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

//max leght of an animation function's name
#define __MAX_ANIMATION_FUNCTION_NAME_LENGHT	20

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
		__VIRTUAL_SET(ClassName, AnimatedSprite, update);						\

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
	char name[__MAX_ANIMATION_FUNCTION_NAME_LENGHT];

}AnimationFunction;

typedef const AnimationFunction AnimationFunctionROMDef;

// an animation definition
typedef struct AnimationDescription
{
	// total number of frames
	int numberOfFrames;

	// animation functions
	AnimationFunction* animationFunctions[__MAX_ANIMATION_FUNCTIONS];

}AnimationDescription;

typedef const AnimationDescription AnimationDescriptionROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

//class's allocator
__CLASS_NEW_DECLARE(AnimatedSprite, __PARAMETERS(Object owner, const SpriteDefinition* spriteDefinition));

 //destructor
void AnimatedSprite_destructor(AnimatedSprite this);

// retrieve frame allocation type
int AnimatedSprite_getType(AnimatedSprite this);

// retrieve frame's map
Texture AnimatedSprite_getTexture(AnimatedSprite this);

//write char animation frame to char memory
void AnimatedSprite_writeAnimation(AnimatedSprite this);

// if true, the frame's screen position will be calculated in the
// next render cicle
void AnimatedSprite_setCalculatePositionFlag(AnimatedSprite this, int calculatePositionFlag);

// retrieve actual frame index of animation
s8 AnimatedSprite_getActualFrame(AnimatedSprite this);

// retrieve previous frame index of animation
s8 AnimatedSprite_getPreviousFrame(AnimatedSprite this);

// set actual frame of animation
void AnimatedSprite_setActualFrame(AnimatedSprite this, s8 actualFrame);

// set previous frame index of animation
void AnimatedSprite_setPreviousFrame(AnimatedSprite this, s8 previousFrame);

// retrieve frame delay
s8 AnimatedSprite_getFrameDelay(AnimatedSprite this);

// set frame delay
void AnimatedSprite_setFrameDelay(AnimatedSprite this, u8 frameDelay);

// retrieve frame delay delta
u8 AnimatedSprite_geFrameDelayDelta(AnimatedSprite this);

// set frame delay delta
void AnimatedSprite_setFrameDelayDelta(AnimatedSprite this, u8 frameDelayDelta);

// animate the frame
void AnimatedSprite_animate(AnimatedSprite this);

// execute frame's map's FX
void AnimatedSprite_fx(AnimatedSprite this);

// render frame
void AnimatedSprite_update(AnimatedSprite this, Clock clock);

// retrieve frame's map's height
u8 AnimatedSprite_getRows(AnimatedSprite this);

// retrieve frame's map's width
u8 AnimatedSprite_getCols(AnimatedSprite this);

// retrieve frame's map's render mode
int AnimatedSprite_getMapType(AnimatedSprite this);

// allocate a write in graphic memory again
void AnimatedSprite_resetMemoryState(AnimatedSprite this, int worldLayer);

// play animation
void AnimatedSprite_playAnimationFunction(AnimatedSprite this, AnimationFunction* animationFunction);

// play animation
void AnimatedSprite_play(AnimatedSprite this, AnimationDescription* animationDescription, char* functionName);

// is playing a specific animation
int AnimatedSprite_isPlayingFunction(AnimatedSprite this, AnimationDescription* animationDescription, char* functionName);

// is playing animation
int AnimatedSprite_isPlaying(AnimatedSprite this);

// write sprite to graphic memory
void AnimatedSprite_write(AnimatedSprite this);

// pause animation
void AnimatedSprite_pause(AnimatedSprite this, int pause);

#endif