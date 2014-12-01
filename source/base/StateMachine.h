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

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <Telegram.h>
#include <State.h>


// declare the virtual methods
#define StateMachine_METHODS								\
		Object_METHODS										\

// declare the virtual methods which are redefined
#define StateMachine_SET_VTABLE(ClassName)								\
		Object_SET_VTABLE(ClassName)									\
		__VIRTUAL_SET(ClassName, StateMachine, handleMessage);			\


__CLASS(StateMachine);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(StateMachine, __PARAMETERS(void* owner));

// allocate memory and call the constructor
void StateMachine_constructor(StateMachine this, void* owner);

// class's destructor
void StateMachine_destructor(StateMachine this);

// update state
void StateMachine_update(StateMachine this);

// change state
void StateMachine_swapState(StateMachine this, State newState);

// push a state in the stack
void StateMachine_pushState(StateMachine this, State newState);

// pop a state fromt the stack
void StateMachine_popState(StateMachine this);

// return to previous state
void StateMachine_returnToPreviousState(StateMachine this);

// change to a global state 
void StateMachine_changeToGlobal(StateMachine this, State globalState);

// return to previous state
int StateMachine_handleMessage(StateMachine this, Telegram telegram);

// returns true if the current state's type is equal to the type of the
// class passed as a parameter. 
int StateMachine_isInState(StateMachine this, State state);

// set owner
void StateMachine_setOwner(StateMachine this, void* owner);

// retrieve current state
State StateMachine_getCurrentState(StateMachine this);

// retrieve previous state in the stack
State StateMachine_getPreviousState(StateMachine this);

#endif /*SCROLL_H_*/
