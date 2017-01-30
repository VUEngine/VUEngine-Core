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

#ifndef OBJECT_ANIMATION_COORDINATOR_H_
#define OBJECT_ANIMATION_COORDINATOR_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ObjectAnimationCoordinator_METHODS(ClassName)													\
    	AnimationCoordinator_METHODS(ClassName) 														\

#define ObjectAnimationCoordinator_SET_VTABLE(ClassName)												\
        AnimationCoordinator_SET_VTABLE(ClassName)														\
        __VIRTUAL_SET(ClassName, ObjectAnimationCoordinator, addAnimationController);					\
        __VIRTUAL_SET(ClassName, ObjectAnimationCoordinator, removeAnimationController);                \

#define ObjectAnimationCoordinator_ATTRIBUTES															\
        /* super's attributes */																		\
        AnimationCoordinator_ATTRIBUTES																\

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
