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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StateMachine.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// a state machine
#define StateMachine_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* that which owns this instance */											\
	void* owner;																\
																				\
	/* pointer to the current state */											\
	State currentState;															\
																				\
	/* pointer to the previous state */											\
	State previousState;														\
																				\
	/* stack of states */														\
	VirtualList stateStack;														\

__CLASS_DEFINITION(StateMachine);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(StateMachine, __PARAMETERS(void* owner))
__CLASS_NEW_END(StateMachine, __ARGUMENTS(owner));

// allocate memory and call the constructor
void StateMachine_constructor(StateMachine this, void* owner)
{
	// construct base object
	__CONSTRUCT_BASE(Object);

	// set pointers
	this->owner = owner;

	this->currentState = NULL;

	this->previousState = NULL;

	this->stateStack = __NEW(VirtualList);
}

// class's destructor
void StateMachine_destructor(StateMachine this)
{
	ASSERT(this, "StateMachine::destructor: null this");
	ASSERT(this->stateStack, "StateMachine::destructor: null stateStack");

	// delete the stack
	VirtualNode node = VirtualList_begin(this->stateStack);

	for (; node; node = VirtualNode_getNext(node))
	{
		__DELETE(VirtualNode_getData(node));
	}

	// empty the list
	VirtualList_clear(this->stateStack);

	// deallocate the list
	__DELETE(this->stateStack);

	// free processor memory
	__DESTROY_BASE(Object);
}

// update state
void StateMachine_update(StateMachine this)
{
	ASSERT(this, "StateMachine::update: null this");

	if (this->currentState)
	{
		__VIRTUAL_CALL(void, State, execute, this->currentState, __ARGUMENTS(this->owner));
	}
}

// change state
void StateMachine_swapState(StateMachine this, State newState)
{
	ASSERT(this, "StateMachine::swapState: null this");
	ASSERT(newState, "StateMachine::swapState: null newState");

	// update the stack
	// remove current state
	VirtualList_popFront(this->stateStack);

	// finalize current state
	if (this->currentState)
	{
		this->previousState = this->currentState;

		// call the exit method from current state
		__VIRTUAL_CALL(void, State, exit, this->currentState, __ARGUMENTS(this->owner));
	}

	this->currentState = newState;

	// push new state in the top of the stack
	VirtualList_pushFront(this->stateStack, (BYTE*)this->currentState);

	// call enter method from new state
	__VIRTUAL_CALL(void, State, enter, this->currentState, __ARGUMENTS(this->owner));
}

// push a state in the stack
void StateMachine_pushState(StateMachine this, State newState)
{
	ASSERT(this, "StateMachine::pushState: null this");

	if (!newState)
	{
		return;
	}

	// finalize current state
	if (this->currentState)
	{
		// call the pause method from current state
		__VIRTUAL_CALL(void, State, pause, this->currentState, __ARGUMENTS(this->owner));
	}

	// set new state
	this->currentState = newState;

	ASSERT(this->currentState, "StateMachine::pushState: null currentState");

	// push new state in the top of the stack
	VirtualList_pushFront(this->stateStack, (BYTE*)this->currentState);

	// call enter method from new state
	__VIRTUAL_CALL(void, State, enter, this->currentState, __ARGUMENTS(this->owner));
}

// pop a state fromt the stack
void StateMachine_popState(StateMachine this)
{
	ASSERT(this, "StateMachine::popState: null this");
	ASSERT(VirtualList_getSize(this->stateStack) > 1, "StateMachine::popState: stack empty");

	// update the stack
	// remove the state in the top of the stack
	VirtualList_popFront(this->stateStack);

	// finalize current state
	if (this->currentState)
	{
		// call the exit method from current state
		__VIRTUAL_CALL(void, State, exit, this->currentState, __ARGUMENTS(this->owner));
	}

	// update current state
	this->currentState = (State)VirtualList_front(this->stateStack);

	ASSERT(this->currentState, "StateMachine::popState: null currentState");

	// call resume method from new state
	__VIRTUAL_CALL(void, State, resume, this->currentState, __ARGUMENTS(this->owner));
}

// return to previous state
void StateMachine_returnToPreviousState(StateMachine this)
{
	ASSERT(this, "StateMachine::returnToPreviousState: null this");

	if (this->previousState)
	{
		if (this->currentState)
	{
			__VIRTUAL_CALL(void, State, exit, this->currentState, __ARGUMENTS(this->owner));
		}

		this->currentState = this->previousState;

		this->previousState = NULL;
	}
}

// change to a global state
void StateMachine_changeToGlobal(StateMachine this, State globalState)
{
	ASSERT(this, "StateMachine::changeToGlobal: null this");

	if (!globalState)
	{
		return;
	}
	if (this->currentState)
	{
		__VIRTUAL_CALL(void, State, pause, this->currentState, __ARGUMENTS(this->owner));

		this->previousState = this->currentState;

	}

	this->currentState = globalState;

	__VIRTUAL_CALL(void, State, enter, this->currentState, __ARGUMENTS(this->owner));
}

// return to previous state
int StateMachine_handleMessage(StateMachine this, Telegram telegram)
{
	ASSERT(this, "StateMachine::handleMessage: null this");

	if (this->currentState )
	{
		return __VIRTUAL_CALL(int, State, handleMessage, this->currentState, __ARGUMENTS(this->owner, telegram));
	}

	return false;
}

// returns true if the current state's type is equal to the type of the
// class passed as a parameter.
int StateMachine_isInState(StateMachine this, const State state)
{
	ASSERT(this, "StateMachine::isInState: null this");

	return (this->currentState == state) ? true : false;
}

// set owner
void StateMachine_setOwner(StateMachine this, void* owner)
{
	ASSERT(this, "StateMachine::setOwner: null this");

	this->owner = owner;
}

// retrieve current state
State StateMachine_getCurrentState(StateMachine this)
{
	ASSERT(this, "StateMachine::getCurrentState: null this");

	return this->currentState;
}

// retrieve previous state in the stack
State StateMachine_getPreviousState(StateMachine this)
{
	VirtualNode node = VirtualList_begin(this->stateStack);

	if (node)
	{
		node = VirtualNode_getNext(node);

		return (State)VirtualNode_getData(node);
	}

	return NULL;
}
