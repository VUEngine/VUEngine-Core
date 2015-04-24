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

#ifndef ANIMATION_CONTROLLER_H_
#define ANIMATION_CONTROLLER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// event
#define __EVENT_ANIMATION_COMPLETE				"animationComplete"
#define __EVENT_ANIMATION_FRAME_CHANGED			"animationFrameChanged"

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
#define AnimationController_METHODS												\

// declare the virtual methods which are redefined
#define AnimationController_SET_VTABLE(CsName)									\

#define AnimationController_ATTRIBUTES											\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
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
	/* a pointer to the animation function being played */						\
	AnimationFunction* animationFunction;										\
																				\
	/* flag to know if playing an animation */									\
	int playing: 1;																\

// declare a Sprite, which holds a texture and a drawing specification
__CLASS(AnimationController);


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
	// animation functions
	AnimationFunction* animationFunctions[__MAX_ANIMATION_FUNCTIONS];

} AnimationDescription;

typedef const AnimationDescription AnimationDescriptionROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(AnimationController, Object owner);

void AnimationController_constructor(AnimationController this, Object owner);
void AnimationController_destructor(AnimationController this);
void AnimationController_writeAnimation(AnimationController this);
s8 AnimationController_getActualFrameIndex(AnimationController this);
s8 AnimationController_getActualFrame(AnimationController this);
s8 AnimationController_getPreviousFrame(AnimationController this);
void AnimationController_setActualFrame(AnimationController this, s8 actualFrame);
void AnimationController_setPreviousFrame(AnimationController this, s8 previousFrame);
s8 AnimationController_getFrameDelay(AnimationController this);
void AnimationController_setFrameDelay(AnimationController this, u8 frameDelay);
u8 AnimationController_geFrameDelayDelta(AnimationController this);
void AnimationController_setFrameDelayDelta(AnimationController this, u8 frameDelayDelta);
bool AnimationController_animate(AnimationController this);
void AnimationController_fx(AnimationController this);
bool AnimationController_update(AnimationController this, Clock clock);
u8 AnimationController_getRows(AnimationController this);
u8 AnimationController_getCols(AnimationController this);
int AnimationController_getMapType(AnimationController this);
void AnimationController_playAnimationFunction(AnimationController this, AnimationFunction* animationFunction);
void AnimationController_play(AnimationController this, AnimationDescription* animationDescription, char* functionName);
bool AnimationController_isPlayingFunction(AnimationController this, AnimationDescription* animationDescription, char* functionName);
bool AnimationController_isPlaying(AnimationController this);
void AnimationController_write(AnimationController this);
void AnimationController_pause(AnimationController this, bool pause);


#endif