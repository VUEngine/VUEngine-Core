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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StateMachine.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// a state machine
#define StateMachine_ATTRIBUTES																			\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* that which owns this instance */																\
        void* owner;																					\
        /* pointer to the current state */																\
        State currentState;																				\
        /* pointer to the previous state */					    										\
        State previousState;																			\
        /* stack of states */																			\
        VirtualList stateStack;																			\

__CLASS_DEFINITION(StateMachine, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(StateMachine, void* owner)
__CLASS_NEW_END(StateMachine, owner);

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
	VirtualNode node = this->stateStack->head;

	for(; node; node = node->next)
	{
		__DELETE(node->data);
	}

	// deallocate the list
	__DELETE(this->stateStack);

	// free processor memory
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// update state
void StateMachine_update(StateMachine this)
{
	ASSERT(this, "StateMachine::update: null this");

	if(this->currentState)
	{
		__VIRTUAL_CALL(State, execute, this->currentState, this->owner);
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
	if(this->currentState)
	{
		this->previousState = this->currentState;

		// call the exit method from current state
		__VIRTUAL_CALL(State, exit, this->currentState, this->owner);
	}

	this->currentState = newState;

	// push new state in the top of the stack
	VirtualList_pushFront(this->stateStack, (BYTE*)this->currentState);

	// call enter method from new state
	__VIRTUAL_CALL(State, enter, this->currentState, this->owner);
}

// push a state in the stack
// returns the resulting stack size
u32 StateMachine_pushState(StateMachine this, State newState)
{
	ASSERT(this, "StateMachine::pushState: null this");

	if(!newState)
	{
		return StateMachine_getStackSize(this);
	}

	// finalize current state
	if(this->currentState)
	{
		// call the pause method from current state
		__VIRTUAL_CALL(State, suspend, this->currentState, this->owner);
	}

	// set new state
	this->currentState = newState;

	ASSERT(this->currentState, "StateMachine::pushState: null currentState");

	// push new state in the top of the stack
	VirtualList_pushFront(this->stateStack, (BYTE*)this->currentState);

	// call enter method from new state
	__VIRTUAL_CALL(State, enter, this->currentState, this->owner);

	// return the resulting stack size
	return StateMachine_getStackSize(this);
}

// pop a state from the stack
// returns the resulting stack size
u32 StateMachine_popState(StateMachine this)
{
	ASSERT(this, "StateMachine::popState: null this");

	// return in case the stack is empty
	if(StateMachine_getStackSize(this) == 0)
	{
		return 0;
	}

	// finalize current state
	if(this->currentState)
	{
		// call the exit method from current state
		__VIRTUAL_CALL(State, exit, this->currentState, this->owner);
	}

	// update the stack
	// remove the state in the top of the stack
	VirtualList_popFront(this->stateStack);

	// update current state
	this->currentState = VirtualList_front(this->stateStack);

	// call resume method from new state
	if(this->currentState)
	{
		__VIRTUAL_CALL(State, resume, this->currentState, this->owner);
	}

	// return the resulting stack size
	return StateMachine_getStackSize(this);
}

// return to previous state
void StateMachine_returnToPreviousState(StateMachine this)
{
	ASSERT(this, "StateMachine::returnToPreviousState: null this");

	if(this->previousState)
	{
		if(this->currentState)
		{
			__VIRTUAL_CALL(State, exit, this->currentState, this->owner);
		}

		this->currentState = this->previousState;

		this->previousState = NULL;
	}
}

// change to a global state
void StateMachine_changeToGlobal(StateMachine this, State globalState)
{
	ASSERT(this, "StateMachine::changeToGlobal: null this");

	if(!globalState)
	{
		return;
	}
	if(this->currentState)
	{
		__VIRTUAL_CALL(State, suspend, this->currentState, this->owner);

		this->previousState = this->currentState;
	}

	this->currentState = globalState;

	__VIRTUAL_CALL(State, enter, this->currentState, this->owner);
}

// return to previous state
bool StateMachine_handleMessage(StateMachine this, Telegram telegram)
{
	ASSERT(this, "StateMachine::handleMessage: null this");

	if(this->currentState )
	{
		return __VIRTUAL_CALL(State, processMessage, this->currentState, this->owner, telegram);
	}

	return false;
}

// returns true if the current state's type is equal to the type of the class passed as a parameter.
bool StateMachine_isInState(StateMachine this, const State state)
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
	ASSERT(this, "StateMachine::getPreviousState: null this");

	VirtualNode node = this->stateStack->head;

	if(node)
	{
		node = node->next;

		return __SAFE_CAST(State, node->data);
	}

	return NULL;
}

// retrieve the stack's size
int StateMachine_getStackSize(StateMachine this)
{
	ASSERT(this, "StateMachine::getStackSize: null this");

	return VirtualList_getSize(this->stateStack);
}
