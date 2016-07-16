/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef ANIMATION_CONTROLLER_H_
#define ANIMATION_CONTROLLER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Clock.h>
#include <CharSet.h>
#include <Sprite.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// event
#define __EVENT_ANIMATION_COMPLETE				"animationComplete"
#define __EVENT_ANIMATION_FRAME_CHANGED			"animationFrameChanged"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define AnimationController_METHODS(ClassName)															\

// declare the virtual methods which are redefined
#define AnimationController_SET_VTABLE(CsName)															\

#define AnimationController_ATTRIBUTES																	\
        /* super's attributes */																		\
        Object_ATTRIBUTES;																				\
        /* who owns the animated sprite */																\
        Object owner;																					\
        /* who owns the animated sprite */																\
        AnimationCoordinator animationCoordinator;														\
        /* actual animation's frame to show */															\
        s8 actualFrame;																					\
        /* previous animation's frame shown */															\
        s8 previousFrame;																				\
        /* actual frame cicle in a given direction (i.e. when walking) */								\
        s8 frameDelay;																					\
        /* frame delay increment update cycle */														\
        s8 frameDelayDelta;																				\
        /* a pointer to the animation function being played */											\
        const AnimationFunction* animationFunction;														\
        /* flag to know if playing an animation */														\
        u8 playing;																						\
        /* frame changed flag */																		\
        u8 animationFrameChanged;																		\

__CLASS(AnimationController);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(AnimationController, Object owner, Sprite sprite, const CharSetDefinition* charSetDefinition);

void AnimationController_constructor(AnimationController this, Object owner, Sprite sprite, const CharSetDefinition* charSetDefinition);
void AnimationController_destructor(AnimationController this);
void AnimationController_writeAnimation(AnimationController this);
s8 AnimationController_getActualFrameIndex(AnimationController this);
s8 AnimationController_getActualFrame(AnimationController this);
s8 AnimationController_getPreviousFrame(AnimationController this);
void AnimationController_setActualFrame(AnimationController this, s8 actualFrame);
s8 AnimationController_getFrameDelay(AnimationController this);
void AnimationController_setFrameDelay(AnimationController this, u8 frameDelay);
u8 AnimationController_geFrameDelayDelta(AnimationController this);
void AnimationController_setFrameDelayDelta(AnimationController this, u8 frameDelayDelta);
void AnimationController_animate(AnimationController this);
void AnimationController_fx(AnimationController this);
bool AnimationController_update(AnimationController this, Clock clock);
u8 AnimationController_getRows(AnimationController this);
u8 AnimationController_getCols(AnimationController this);
int AnimationController_getMapType(AnimationController this);
void AnimationController_playAnimationFunction(AnimationController this, const AnimationFunction* animationFunction);
const AnimationFunction* AnimationController_getPlayingAnimationFunction(AnimationController this);
void AnimationController_play(AnimationController this, const AnimationDescription* animationDescription, const char* functionName);
void AnimationController_stop(AnimationController this);
bool AnimationController_isPlayingFunction(AnimationController this, const AnimationDescription* animationDescription, const char* functionName);
bool AnimationController_isPlaying(AnimationController this);
void AnimationController_write(AnimationController this);
void AnimationController_pause(AnimationController this, bool pause);
bool AnimationController_didAnimationFrameChanged(AnimationController this);


#endif
