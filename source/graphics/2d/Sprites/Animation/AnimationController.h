/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef ANIMATION_CONTROLLER_H_
#define ANIMATION_CONTROLLER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Clock.h>
#include <CharSet.h>
#include <Sprite.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define AnimationController_METHODS(ClassName)															\

// declare the virtual methods which are redefined
#define AnimationController_SET_VTABLE(CsName)															\

#define AnimationController_ATTRIBUTES																	\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* who owns the animated sprite */																\
		Object owner;																					\
		/* who owns the animated sprite */																\
		AnimationCoordinator animationCoordinator;														\
		/* actual animation's frame to show */															\
		s8 actualFrame;																					\
		/* previous animation's frame shown */															\
		s8 previousFrame;																				\
		/* actual frame cycle in a given direction (i.e. when walking) */								\
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
//										PUBLIC INTERFACE
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
bool AnimationController_animate(AnimationController this);
void AnimationController_fx(AnimationController this);
bool AnimationController_update(AnimationController this, Clock clock);
u8 AnimationController_getRows(AnimationController this);
u8 AnimationController_getCols(AnimationController this);
int AnimationController_getMapType(AnimationController this);
void AnimationController_playAnimationFunction(AnimationController this, const AnimationFunction* animationFunction);
const AnimationFunction* AnimationController_getPlayingAnimationFunction(AnimationController this);
bool AnimationController_play(AnimationController this, const AnimationDescription* animationDescription, const char* functionName);
void AnimationController_stop(AnimationController this);
bool AnimationController_isPlayingFunction(AnimationController this, const char* functionName);
bool AnimationController_isPlaying(AnimationController this);
void AnimationController_write(AnimationController this);
void AnimationController_pause(AnimationController this, bool pause);



#endif
