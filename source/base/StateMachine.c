/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <StateMachine.h>
#include <VirtualList.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// a state machine
#define StateMachine_ATTRIBUTES				\
											\
	/* super's attributes */				\
	Object_ATTRIBUTES;						\
											\
	/* that which owns this instance */		\
	void* owner;							\
											\
	/* pointer to the current state */		\
	State currentState;						\
											\
	/* pointer to the previous state */		\
	State previousState;					\
											\
	/* stack of states */					\
	VirtualList stateStack;
	
	
__CLASS_DEFINITION(StateMachine);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// always call these to macros next to each other
__CLASS_NEW_DEFINITION(StateMachine, __PARAMETERS(void* owner))
__CLASS_NEW_END(StateMachine, __ARGUMENTS(owner));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// allocate memory and call the constructor
void StateMachine_constructor(StateMachine this, void* owner){
	
	// construct base object
	__CONSTRUCT_BASE(Object);

	// set pointers
	this->owner = owner;
	
	this->currentState = NULL;
	
	this->previousState = NULL;
	
	this->stateStack = __NEW(VirtualList); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void StateMachine_destructor(StateMachine this){

	// delete the stack
	VirtualNode node = VirtualList_begin(this->stateStack);
	
	for(; node; node = VirtualNode_getNext(node)){
		
		__DELETE(VirtualNode_getData(node));
	}
	
	// empty the list
	VirtualList_clear(this->stateStack);
	
	// deallocate the list
	__DELETE(this->stateStack);
	
	// free processor memory
	__DESTROY_BASE(Object);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update state
void StateMachine_update(StateMachine this){

	ASSERT(this->currentState, StateMachine: null state);
	
	__VIRTUAL_CALL(void, State, execute, this->currentState, __ARGUMENTS(this->owner));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// change state
void StateMachine_swapState(StateMachine this, State newState){
	
	ASSERT(newState, StateMachine: swaping to NULL state); 
	
	// finalize current state
	if (this->currentState){

		this->previousState = this->currentState;
		
		// call the exit method from current state
		__VIRTUAL_CALL(void, State, exit, this->currentState, __ARGUMENTS(this->owner));
	}

	this->currentState = newState;
	
	// call enter method from new state
	__VIRTUAL_CALL(void, State, enter, this->currentState, __ARGUMENTS(this->owner));
	
	// update the stack
	// remove current state
	VirtualList_popFront(this->stateStack);
	
	// push new state in the top of the stack
	VirtualList_pushFront(this->stateStack, (BYTE*)this->currentState);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// push a state in the stack
void StateMachine_pushState(StateMachine this, State newState){
	
	if(!newState){
		
		return;
	}
	
	// finalize current state
	if (this->currentState){

		// call the pause method from current state
		__VIRTUAL_CALL(void, State, pause, this->currentState, __ARGUMENTS(this->owner));
	}

	// set new state 
	this->currentState = newState;
	
	// call enter method from new state
	__VIRTUAL_CALL(void, State, enter, this->currentState, __ARGUMENTS(this->owner));
	
	// push new state in the top of the stack
	VirtualList_pushFront(this->stateStack, (BYTE*)this->currentState);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pop a state fromt the stack
void StateMachine_popState(StateMachine this){

	ASSERT(VirtualList_getSize(this->stateStack) > 1, StateMachine: stack empty);
	
	// update the stack	
	// remove the state in the top of the stack
	VirtualList_popFront(this->stateStack);

	// finalize current state
	if (this->currentState){

		// call the exit method from current state
		__VIRTUAL_CALL(void, State, exit, this->currentState, __ARGUMENTS(this->owner));
	}

	// update current state
	this->currentState = (State)VirtualList_front(this->stateStack);
	
	// call resume method from new state
	__VIRTUAL_CALL(void, State, resume, this->currentState, __ARGUMENTS(this->owner));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// return to previous state
void StateMachine_returnToPreviousState(StateMachine this){

	if(this->previousState){
		
		if (this->currentState){			
			
			__VIRTUAL_CALL(void, State, exit, this->currentState, __ARGUMENTS(this->owner));
		}

		this->currentState = this->previousState;
		
		this->previousState = NULL;
	}			
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// change to a global state 
void StateMachine_changeToGlobal(StateMachine this, State globalState){
	
	if(!globalState){
		
		return;
	}
	if (this->currentState){

		__VIRTUAL_CALL(void, State, pause, this->currentState, __ARGUMENTS(this->owner));
		
		this->previousState = this->currentState;		
		
	}

	this->currentState = globalState;	
	
	__VIRTUAL_CALL(void, State, enter, this->currentState, __ARGUMENTS(this->owner));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// return to previous state
int StateMachine_handleMessage(StateMachine this, Telegram telegram){

	if(this->currentState ){

		return __VIRTUAL_CALL(int, State, handleMessage, this->currentState, __ARGUMENTS(this->owner, telegram));
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// returns true if the current state's type is equal to the type of the
// class passed as a parameter. 
int StateMachine_isInState(StateMachine this, const State state){

	return (this->currentState == state)? true: false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set owner
void StateMachine_setOwner(StateMachine this, void* owner){
	
	this->owner = owner;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve current state
State StateMachine_getCurrentState(StateMachine this){
	
	return this->currentState;
}
