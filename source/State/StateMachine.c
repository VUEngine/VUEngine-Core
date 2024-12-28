/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <State.h>
#include <Telegram.h>
#include <VirtualList.h>

#include "StateMachine.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void StateMachine::constructor(void* owner)
{
	Base::constructor();

	// set pointers
	this->owner = owner;
	this->currentState = NULL;
	this->previousState = NULL;
	this->nextState = NULL;
	this->stateStack = new VirtualList();
}
//---------------------------------------------------------------------------------------------------------
void StateMachine::destructor()
{
	NM_ASSERT(!isDeleted(this->stateStack), "StateMachine::destructor: null stateStack");

	// deallocate the list
	delete this->stateStack;

	this->owner = NULL;
	this->currentState = NULL;
	this->previousState = NULL;
	this->nextState = NULL;

	this->transition = kStateMachineIdle;

	// free processor memory

	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
bool StateMachine::handleMessage(Telegram telegram)
{
	if(this->currentState )
	{
		return  State::processMessage(this->currentState, this->owner, telegram);
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
bool StateMachine::transitionTo(State state, int16 transition)
{
	if(kStateMachineIdle != this->transition || NULL == state || kStateMachineIdle == transition)
	{
		return false;
	}

	this->nextState = state;
	this->transition = transition;

	if(NULL == this->currentState && NULL != this->nextState)
	{
		StateMachine::applyTransition(this);
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------
void StateMachine::swapState(State newState)
{
	if(isDeleted(newState))
	{
		return;
	}

	NM_ASSERT(!isDeleted(newState), "StateMachine::swapState: null newState");

	// update the stack
	// remove current state
	VirtualList::popFront(this->stateStack);

	// finalize current state
	if(NULL != this->currentState)
	{
		this->previousState = this->currentState;

		// call the exit method from current state
		State::exit(this->currentState, this->owner);
	}

	NM_ASSERT(!isDeleted(newState), "StateMachine::swapState: newState deleted during exit of previous state");

	this->currentState = newState;
	this->previousState = NULL;

	// push new state in the top of the stack
	VirtualList::pushFront(this->stateStack, (BYTE*)this->currentState);

	// call enter method from new state
	State::enter(this->currentState, this->owner);
}
//---------------------------------------------------------------------------------------------------------
void StateMachine::pushState(State newState)
{
	if(isDeleted(newState))
	{
		return;
	}

	// finalize current state
	if(NULL != this->currentState)
	{
		// call the pause method from current state
		State::suspend(this->currentState, this->owner);
	}

	// set new state
	this->currentState = newState;

	NM_ASSERT(!isDeleted(this->currentState), "StateMachine::pushState: null currentState");

	// push new state in the top of the stack
	VirtualList::pushFront(this->stateStack, (BYTE*)this->currentState);

	// call enter method from new state
	State::enter(this->currentState, this->owner);
}
//---------------------------------------------------------------------------------------------------------
void StateMachine::popState()
{
	// return in case the stack is empty
	if(StateMachine::getStackSize(this) == 0)
	{
		return;
	}

	// finalize current state
	if(NULL != this->currentState)
	{
		// call the exit method from current state
		State::exit(this->currentState, this->owner);
	}

	// update the stack
	// remove the state in the top of the stack
	VirtualList::popFront(this->stateStack);

	// update current state
	this->currentState = VirtualList::front(this->stateStack);

	// call resume method from new state
	if(NULL != this->currentState)
	{
		State::resume(this->currentState, this->owner);
	}
}
//---------------------------------------------------------------------------------------------------------
void StateMachine::popAllStates()
{
	while(0 < StateMachine::popStateWithoutResume(this));
}
//---------------------------------------------------------------------------------------------------------
State StateMachine::update()
{
	if(kStateMachineIdle != this->transition)
	{
		StateMachine::applyTransition(this);
	}
	else if(!isDeleted(this->currentState))
	{
		State::execute(this->currentState, this->owner);
	}

	return this->currentState;
}
//---------------------------------------------------------------------------------------------------------
bool StateMachine::isInState(const State state)
{
	return (this->currentState == state) ? true : false;
}
//---------------------------------------------------------------------------------------------------------
bool StateMachine::hasStateInTheStack(State state)
{
	return !isDeleted(state) && VirtualList::find(this->stateStack, state);
}
//---------------------------------------------------------------------------------------------------------
VirtualList StateMachine::getStateStack()
{
	return this->stateStack;
}
//---------------------------------------------------------------------------------------------------------
State StateMachine::getCurrentState()
{
	return this->currentState;
}
//---------------------------------------------------------------------------------------------------------
State StateMachine::getNextState()
{
	return this->nextState;
}
//---------------------------------------------------------------------------------------------------------
State StateMachine::getPreviousState()
{
	VirtualNode node = this->stateStack->head;

	if(node)
	{
		node = node->next;

		return State::safeCast(node->data);
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------
int32 StateMachine::getStackSize()
{
	return VirtualList::getCount(this->stateStack);
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
uint32 StateMachine::popStateWithoutResume()
{
	// return in case the stack is empty
	if(StateMachine::getStackSize(this) == 0)
	{
		return 0;
	}

	// finalize current state
	if(NULL != this->currentState)
	{
		// call the exit method from current state
		State::exit(this->currentState, this->owner);
	}

	// remove the state in the top of the stack
	VirtualList::popFront(this->stateStack);

	// update current state
	this->currentState = VirtualList::front(this->stateStack);

	// return the resulting stack size
	return StateMachine::getStackSize(this);
}
//---------------------------------------------------------------------------------------------------------
void StateMachine::applyTransition()
{
	if(kStateMachineIdle != this->transition)
	{
		switch(this->transition)
		{
			case kStateMachineCleanStack:

				if(NULL != this->events)
				{
					StateMachine::fireEvent(this, kEventStateMachineWillCleanStack);
				}

				StateMachine::popAllStates(this);

				if(NULL != this->nextState)
				{
					StateMachine::pushState(this, this->nextState);
				}

				if(NULL != this->events)
				{
					StateMachine::fireEvent(this, kEventStateMachineCleanedStack);
				}
				break;

			case kStateMachineSwapState:

				if(NULL != this->events)
				{
					StateMachine::fireEvent(this, kEventStateMachineWillSwapState);
				}

				StateMachine::swapState(this, this->nextState);

				if(NULL != this->events)
				{
					StateMachine::fireEvent(this, kEventStateMachineSwapedState);
				}

				break;

			case kStateMachinePushState:

				if(NULL != this->events)
				{
					StateMachine::fireEvent(this, kEventStateMachineWillPushState);
				}

				StateMachine::pushState(this, this->nextState);

				if(NULL != this->events)
				{
					StateMachine::fireEvent(this, kEventStateMachinePushedState);
				}

				break;

			case kStateMachinePopState:

				if(NULL != this->events)
				{
					StateMachine::fireEvent(this, kEventStateMachineWillPopState);
				}

				StateMachine::popState(this);

				if(NULL != this->events)
				{
					StateMachine::fireEvent(this, kEventStateMachinePoppedState);
				}
				break;
		}

		this->nextState = NULL;
		this->transition = kStateMachineIdle;
	}
}
//---------------------------------------------------------------------------------------------------------
