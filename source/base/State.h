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

#ifndef STATE_H_
#define STATE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define State_METHODS																					\
		Object_METHODS																					\
		__VIRTUAL_DEC(enter);																			\
		__VIRTUAL_DEC(execute);																			\
		__VIRTUAL_DEC(exit);																			\
		__VIRTUAL_DEC(suspend);																			\
		__VIRTUAL_DEC(resume);																			\

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
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void State_constructor(State this);
void State_destructor(State this);
void State_enter(State this, void* owner);
void State_execute(State this, void* owner);
void State_exit(State this, void* owner);
void State_suspend(State this, void* owner);
void State_resume(State this, void* owner);


#endif