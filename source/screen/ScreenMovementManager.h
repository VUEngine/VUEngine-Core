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

#ifndef SCREEN_MOVEMENT_MANAGER_H_
#define SCREEN_MOVEMENT_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ScreenMovementManager_METHODS(ClassName)														\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, focus, u32 checkIfFocusEntityIsMoving);							\

// declare the virtual methods which are redefined
#define ScreenMovementManager_SET_VTABLE(ClassName)														\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, ScreenMovementManager, focus); 										\

#define ScreenMovementManager_ATTRIBUTES																\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\

// declare a ScreenMovementManager
__CLASS(ScreenMovementManager);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

ScreenMovementManager ScreenMovementManager_getInstance();

void ScreenMovementManager_constructor(ScreenMovementManager this);
void ScreenMovementManager_destructor(ScreenMovementManager this);
void ScreenMovementManager_focus(ScreenMovementManager this, u32 checkIfFocusEntityIsMoving);


#endif
