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

#ifndef BGMAP_ANIMATION_COORDINATOR_H_
#define BGMAP_ANIMATION_COORDINATOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimationCoordinator.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define BgmapAnimationCoordinator_METHODS(ClassName)													\
	    AnimationCoordinator_METHODS(ClassName)	    													\

#define BgmapAnimationCoordinator_SET_VTABLE(ClassName)													\
        AnimationCoordinator_SET_VTABLE(ClassName)														\
        __VIRTUAL_SET(ClassName, BgmapAnimationCoordinator, addAnimationController);					\
        __VIRTUAL_SET(ClassName, BgmapAnimationCoordinator, removeAnimationController);					\

#define BgmapAnimationCoordinator_ATTRIBUTES															\
        AnimationCoordinator_ATTRIBUTES																	\

__CLASS(BgmapAnimationCoordinator);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(BgmapAnimationCoordinator, const CharSetDefinition* charSetDefinition);

void BgmapAnimationCoordinator_constructor(BgmapAnimationCoordinator this, const CharSetDefinition* charSetDefinition);
void BgmapAnimationCoordinator_destructor(BgmapAnimationCoordinator this);

void BgmapAnimationCoordinator_addAnimationController(BgmapAnimationCoordinator this, AnimationController animationController);
void BgmapAnimationCoordinator_removeAnimationController(BgmapAnimationCoordinator this, AnimationController animationController);


#endif
