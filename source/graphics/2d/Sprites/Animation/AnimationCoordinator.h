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

#ifndef ANIMATION_COORDINATOR_H_
#define ANIMATION_COORDINATOR_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>
#include <AnimationController.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define AnimationCoordinator_METHODS														\
	Object_METHODS;																\
	__VIRTUAL_DEC(addAnimationController);										\
	__VIRTUAL_DEC(removeAnimationController);									\

#define AnimationCoordinator_SET_VTABLE(ClassName)								\
	Object_SET_VTABLE(ClassName);												\

#define AnimationCoordinator_ATTRIBUTES											\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* controllers to sync */													\
	VirtualList animationControllers;											\
																				\
	/* char set shared among entities */										\
	CharSet charSet;															\

__CLASS(AnimationCoordinator);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void AnimationCoordinator_constructor(AnimationCoordinator this, CharSet charSet);
void AnimationCoordinator_destructor(AnimationCoordinator this);
void AnimationCoordinator_addAnimationController(AnimationCoordinator this, AnimationController animationController);
void AnimationCoordinator_removeAnimationController(AnimationCoordinator this, AnimationController animationController);
const CharSet AnimationCoordinator_getCharSet(AnimationCoordinator this);


#endif