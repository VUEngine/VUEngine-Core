/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <State.h>
#include <Telegram.h>
#include <VirtualList.h>

#include "StateMachine.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param owner		the StateMachine's owner
 */
void StateMachine::constructor(void* owner)
{
	// construct base object
	Base::constructor();

	// set pointers
	this->owner = owner;
	this->currentState = NULL;
	this->previousState = NULL;
	this->nextState = NULL;
	this->stateStack = new VirtualList();
}

/**
 * Class destructor
 */
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
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Operation to take place on the next update cycle
 *
 * @param state	State to switch to
 */
bool StateMachine::prepareTransition(State state, int16 transition)
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

/**
 * Method to initiate a change in the state machine
 */
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

/**
 * Method to propagate the update process to the current state
 */
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

/**
 * Return the states stack
 *
 * @return VirtualList
 */
VirtualList StateMachine::getStateStack()
{
	return this->stateStack;
}

/**
 * Replace the current state for a new one
 *
 * @param newState	State to switch to
 */
void StateMachine::swapState(State newState)
{
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

/**
 * Push a new state at top of the stack, making it the current one
 *
 * @param newState	state to push
 * @return 			Resulting stack's size
 */
uint32 StateMachine::pushState(State newState)
{
	if(!newState)
	{
		return StateMachine::getStackSize(this);
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

	// return the resulting stack size
	return StateMachine::getStackSize(this);
}

/**
 * Remove the current state, the one at the top of the stack but don't call resume on the previous state
 *
 * @return 			Resulting stack's size
 */
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

/**
 * Remove all the states in the stack
 *
 */
void StateMachine::popAllStates()
{
	while(0 < StateMachine::popStateWithoutResume(this));
}

/**
 * Remove the current state, the one at the top of the stack
 *
 * @return 			Resulting stack's size
 */
uint32 StateMachine::popState()
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

	// return the resulting stack size
	return StateMachine::getStackSize(this);
}

/**
 * Return the state just below the current one at the top of the stack
 */
void StateMachine::returnToPreviousState()
{
	if(this->previousState)
	{
		if(NULL != this->currentState)
		{
			State::exit(this->currentState, this->owner);
		}

		this->currentState = this->previousState;

		this->previousState = NULL;
	}
}

/**
 * Change the current state to a new one but don't push it into the stack
 *
 * @param globalState	State to switch to
 */
void StateMachine::changeToGlobal(State globalState)
{
	if(!globalState)
	{
		return;
	}
	if(NULL != this->currentState)
	{
		State::suspend(this->currentState, this->owner);

		this->previousState = this->currentState;
	}

	this->currentState = globalState;

	State::enter(this->currentState, this->owner);
}

/**
 * Method to forward a message to the current state
 *
 * @param telegram		Telegram to forward
 * @return				True if successfully processed, false otherwise
 */
bool StateMachine::handleMessage(Telegram telegram)
{
	if(this->currentState )
	{
		return  State::processMessage(this->currentState, this->owner, telegram);
	}

	return false;
}

/**
 * Check if a given state is the current one
 *
 * @param state			State to check
 * @return				True if the given state is the current one
 */
bool StateMachine::isInState(const State state)
{
	return (this->currentState == state) ? true : false;
}

/**
 * Set the StageMachine's owner
 *
 * @param owner			New owner
 */
void StateMachine::setOwner(void* owner)
{
	this->owner = owner;
}

/**
 * Check if the state is in the stack
 *
 * @param state			State to check for
 */
bool StateMachine::hasStateInTheStack(State state)
{
	return !isDeleted(state) && VirtualList::find(this->stateStack, state);
}

/**
 * Retrieve the current state
 *
 * @return				Current state
 */
State StateMachine::getCurrentState()
{
	return this->currentState;
}

/**
 * Get the second state from the top of the stack
 *
 * @return				Second state in the stack
 */
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

/**
 * Get the StateMachine's stack's size
 *
 * @return				The size of the stack
 */
int32 StateMachine::getStackSize()
{
	return VirtualList::getSize(this->stateStack);
}
