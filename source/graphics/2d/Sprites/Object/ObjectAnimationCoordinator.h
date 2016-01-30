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

#ifndef OBJECT_ANIMATION_COORDINATOR_H_
#define OBJECT_ANIMATION_COORDINATOR_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ObjectAnimationCoordinator_METHODS																\
	AnimationCoordinator_METHODS;																		\
	
#define ObjectAnimationCoordinator_SET_VTABLE(ClassName)												\
	AnimationCoordinator_SET_VTABLE(ClassName);															\
	__VIRTUAL_SET(ClassName, ObjectAnimationCoordinator, addAnimationController);						\
	__VIRTUAL_SET(ClassName, ObjectAnimationCoordinator, addAnimationController);						\
	__VIRTUAL_SET(ClassName, ObjectAnimationCoordinator, removeAnimationController);\

#define ObjectAnimationCoordinator_ATTRIBUTES															\
																										\
	/* super's attributes */																			\
	AnimationCoordinator_ATTRIBUTES;																	\

__CLASS(ObjectAnimationCoordinator);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ObjectAnimationCoordinator, const CharSetDefinition* charSetDefinition);

void ObjectAnimationCoordinator_constructor(ObjectAnimationCoordinator this, const CharSetDefinition* charSetDefinition);
void ObjectAnimationCoordinator_destructor(ObjectAnimationCoordinator this);
void ObjectAnimationCoordinator_addAnimationController(ObjectAnimationCoordinator this, AnimationController animationController);
void ObjectAnimationCoordinator_removeAnimationController(ObjectAnimationCoordinator this, AnimationController animationController);


#endif