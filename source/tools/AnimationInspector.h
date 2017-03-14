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

#ifndef ANIMATION_INSPECTOR_H_
#define ANIMATION_INSPECTOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <AnimatedInGameEntity.h>
#include <GameState.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define AnimationInspector_METHODS(ClassName)																			\
		Object_METHODS(ClassName)																					\


// declare the virtual methods which are redefined
#define AnimationInspector_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\

// declare a AnimationInspector
__CLASS(AnimationInspector);

/**
 * For animation
 *
 * @memberof	AnimationInspector
 */
typedef struct UserAnimatedInGameEntity
{
	/// definition
	const AnimatedInGameEntityDefinition* animatedInGameEntityDefinition;
	/// name
	const char* name;

} UserAnimatedInGameEntity;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

AnimationInspector AnimationInspector_getInstance();

void AnimationInspector_destructor(AnimationInspector this);
void AnimationInspector_update(AnimationInspector this);
void AnimationInspector_show(AnimationInspector this, GameState gameState);
void AnimationInspector_hide(AnimationInspector this);
void AnimationInspector_processUserInput(AnimationInspector this, u16 pressedKey);


#endif
