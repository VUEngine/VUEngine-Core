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

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <State.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define StateMachine_METHODS																			\
		Object_METHODS																					\

// declare the virtual methods which are redefined
#define StateMachine_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, StateMachine, handleMessage);											\

__CLASS(StateMachine);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(StateMachine, void* owner);

void StateMachine_constructor(StateMachine this, void* owner);
void StateMachine_destructor(StateMachine this);
void StateMachine_update(StateMachine this);
void StateMachine_swapState(StateMachine this, State newState);
void StateMachine_pushState(StateMachine this, State newState);
void StateMachine_popState(StateMachine this);
void StateMachine_returnToPreviousState(StateMachine this);
void StateMachine_changeToGlobal(StateMachine this, State globalState);
bool StateMachine_handleMessage(StateMachine this, Telegram telegram);
bool StateMachine_isInState(StateMachine this, State state);
void StateMachine_setOwner(StateMachine this, void* owner);
State StateMachine_getCurrentState(StateMachine this);
State StateMachine_getPreviousState(StateMachine this);
int StateMachine_getStackSize(StateMachine this);


#endif