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

#ifndef STATE_H_
#define STATE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------


// declare the virtual methods
#define State_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, enter, void* owner);												\
		__VIRTUAL_DEC(ClassName, void, execute, void* owner);											\
		__VIRTUAL_DEC(ClassName, void, exit, void* owner);												\
		__VIRTUAL_DEC(ClassName, void, suspend, void* owner);											\
		__VIRTUAL_DEC(ClassName, void, resume, void* owner);											\
		__VIRTUAL_DEC(ClassName, bool, processMessage, void* owner, void* telegram);					\

// declare the virtual methods which are redefined
#define State_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, State, enter);															\
		__VIRTUAL_SET(ClassName, State, execute);														\
		__VIRTUAL_SET(ClassName, State, exit);															\
		__VIRTUAL_SET(ClassName, State, suspend);														\
		__VIRTUAL_SET(ClassName, State, resume);														\

// a generic state
#define State_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\

__CLASS(State);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void State_constructor(State this);
void State_destructor(State this);
void State_enter(State this, void* owner);
void State_execute(State this, void* owner);
void State_exit(State this, void* owner);
void State_suspend(State this, void* owner);
void State_resume(State this, void* owner);


#endif
