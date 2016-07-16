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

#ifndef ANIMATION_COORDINATOR_H_
#define ANIMATION_COORDINATOR_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>
#include <Sprite.h>
#include <AnimationController.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define AnimationCoordinator_METHODS(ClassName)															\
        Object_METHODS(ClassName);																		\
        __VIRTUAL_DEC(ClassName, void, addAnimationController, AnimationController animationController);\
        __VIRTUAL_DEC(ClassName, void, removeAnimationController, AnimationController animationController);\

#define AnimationCoordinator_SET_VTABLE(ClassName)														\
	    Object_SET_VTABLE(ClassName);																	\

#define AnimationCoordinator_ATTRIBUTES																	\
        /* super's attributes */																		\
        Object_ATTRIBUTES;																				\
        /* controllers to sync */																		\
        VirtualList animationControllers;																\
        /* char set definition shared among entities */													\
        const CharSetDefinition* charSetDefinition;														\

__CLASS(AnimationCoordinator);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void AnimationCoordinator_constructor(AnimationCoordinator this, const CharSetDefinition* charSetDefinition);
void AnimationCoordinator_destructor(AnimationCoordinator this);
void AnimationCoordinator_addAnimationController(AnimationCoordinator this, AnimationController animationController);
void AnimationCoordinator_removeAnimationController(AnimationCoordinator this, AnimationController animationController);
const CharSetDefinition* AnimationCoordinator_getCharSetDefinition(AnimationCoordinator this);
bool AnimationCoordinator_playAnimation(AnimationCoordinator this, AnimationController animationController, const AnimationDescription* animationDescription, const char* functionName);


#endif
