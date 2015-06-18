/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef ANIMATION_EDITOR_H_
#define ANIMATION_EDITOR_H_

#ifdef __ANIMATION_EDITOR

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <AnimatedInGameEntity.h>
#include <GameState.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define AnimationEditor_METHODS													\
		Object_METHODS															\


// declare the virtual methods which are redefined
#define AnimationEditor_SET_VTABLE(ClassName)									\
		Object_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, AnimationEditor, handleMessage);				\

// declare a AnimationEditor
__CLASS(AnimationEditor);

// for animation
typedef struct UserAnimatedInGameEntity
{
	const AnimatedInGameEntityDefinition* animatedInGameEntityDefinition;
	const char* name;

} UserAnimatedInGameEntity;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

AnimationEditor AnimationEditor_getInstance();

void AnimationEditor_destructor(AnimationEditor this);
void AnimationEditor_update(AnimationEditor this);
void AnimationEditor_start(AnimationEditor this, GameState gameState);
void AnimationEditor_stop(AnimationEditor this);
bool AnimationEditor_handleMessage(AnimationEditor this, Telegram telegram);

#endif


#endif