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

#ifndef ANIMATION_COORDINATOR_FACTORY_H_
#define ANIMATION_COORDINATOR_FACTORY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <AnimationController.h>
#include <AnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define AnimationCoordinatorFactory_METHODS														\
	Object_METHODS;																\
	
#define AnimationCoordinatorFactory_SET_VTABLE(ClassName)						\
	Object_SET_VTABLE(ClassName);												\

__CLASS(AnimationCoordinatorFactory);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

AnimationCoordinatorFactory AnimationCoordinatorFactory_getInstance();

void AnimationCoordinatorFactory_destructor(AnimationCoordinatorFactory this);
void AnimationCoordinatorFactory_reset(AnimationCoordinatorFactory this);
AnimationCoordinator AnimationCoordinatorFactory_getCoordinator(AnimationCoordinatorFactory this, AnimationController animationController, Sprite sprite, CharSet charSet);


#endif