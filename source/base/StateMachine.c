/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StateMachine.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	StateMachine
 * @extends Object
 * @ingroup base
 */
implements StateMachine : Object;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(StateMachine, void* owner)
__CLASS_NEW_END(StateMachine, owner);


/**
 * Class constructor
 *
 * @memberof		StateMachine
 * @public
 *
 * @param this		Function scope
 * @param owner		the StateMachine's owner
 */
void StateMachine::constructor(StateMachine this, void* owner)
{
	ASSERT(this, "StateMachine::constructor: null this");

	// construct base object
	Base::constructor();

	// set pointers
	this->owner = owner;
	this->currentState = NULL;
	this->previousState = NULL;
	this->stateStack = __NEW(VirtualList);
}

/**
 * Class destructor
 *
 * @memberof		StateMachine
 * @public
 *
 * @param this		Function scope
 */
void StateMachine::destructor(StateMachine this)
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
	Base::destructor();
}

/**
 * Method to propagate the update process to the current state
 *
 * @memberof		StateMachine
 * @public
 *
 * @param this		Function scope
 */
void StateMachine::update(StateMachine this)
{
	ASSERT(this, "StateMachine::update: null this");

	if(this->currentState)
	{
		 State::execute(this->currentState, this->owner);
	}
}

/**
 * Replace the current state for a new one
 *
 * @memberof		StateMachine
 * @public
 *
 * @param this		Function scope
 * @param newState	State to switch to
 */
void StateMachine::swapState(StateMachine this, State newState)
{
	ASSERT(this, "StateMachine::swapState: null this");
	ASSERT(newState, "StateMachine::swapState: null newState");

	// update the stack
	// remove current state
	VirtualList::popFront(this->stateStack);

	// finalize current state
	if(this->currentState)
	{
		this->previousState = this->currentState;

		// call the exit method from current state
		 State::exit(this->currentState, this->owner);
	}

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
 * @memberof		StateMachine
 * @public
 *
 * @param this		Function scope
 * @param newState	state to push
 *
 * @return 			Resulting stack's size
 */
u32 StateMachine::pushState(StateMachine this, State newState)
{
	ASSERT(this, "StateMachine::pushState: null this");

	if(!newState)
	{
		return StateMachine::getStackSize(this);
	}

	// finalize current state
	if(this->currentState)
	{
		// call the pause method from current state
		 State::suspend(this->currentState, this->owner);
	}

	// set new state
	this->currentState = newState;

	ASSERT(this->currentState, "StateMachine::pushState: null currentState");

	// push new state in the top of the stack
	VirtualList::pushFront(this->stateStack, (BYTE*)this->currentState);

	// call enter method from new state
	 State::enter(this->currentState, this->owner);

	// return the resulting stack size
	return StateMachine::getStackSize(this);
}

/**
 * Remove the current state, the one at the top of the stack
 *
 * @memberof		StateMachine
 * @public
 *
 * @param this		Function scope
 *
 * @return 			Resulting stack's size
 */
u32 StateMachine::popState(StateMachine this)
{
	ASSERT(this, "StateMachine::popState: null this");

	// return in case the stack is empty
	if(StateMachine::getStackSize(this) == 0)
	{
		return 0;
	}

	// finalize current state
	if(this->currentState)
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
	if(this->currentState)
	{
		 State::resume(this->currentState, this->owner);
	}

	// return the resulting stack size
	return StateMachine::getStackSize(this);
}

/**
 * Return the state just below the current one at the top of the stack
 *
 * @memberof		StateMachine
 * @public
 *
 * @param this		Function scope
 */
void StateMachine::returnToPreviousState(StateMachine this)
{
	ASSERT(this, "StateMachine::returnToPreviousState: null this");

	if(this->previousState)
	{
		if(this->currentState)
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
 * @memberof			StateMachine
 * @public
 *
 * @param this			Function scope
 * @param globalState	State to switch to
 */
void StateMachine::changeToGlobal(StateMachine this, State globalState)
{
	ASSERT(this, "StateMachine::changeToGlobal: null this");

	if(!globalState)
	{
		return;
	}
	if(this->currentState)
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
 * @memberof			StateMachine
 * @public
 *
 * @param this			Function scope
 * @param telegram		Telegram to forward
 *
 * @return				True if successfully processed, false otherwise
 */
bool StateMachine::handleMessage(StateMachine this, Telegram telegram)
{
	ASSERT(this, "StateMachine::handleMessage: null this");

	if(this->currentState )
	{
		return  State::processMessage(this->currentState, this->owner, telegram);
	}

	return false;
}

/**
 * Check if a given state is the current one
 *
 * @memberof			StateMachine
 * @public
 *
 * @param this			Function scope
 * @param state			State to check
 *
 * @return				True if the given state is the current one
 */
bool StateMachine::isInState(StateMachine this, const State state)
{
	ASSERT(this, "StateMachine::isInState: null this");

	return (this->currentState == state) ? true : false;
}

/**
 * Set the StageMachine's owner
 *
 * @memberof			StateMachine
 * @public
 *
 * @param this			Function scope
 * @param owner			New owner
 */
void StateMachine::setOwner(StateMachine this, void* owner)
{
	ASSERT(this, "StateMachine::setOwner: null this");

	this->owner = owner;
}

/**
 * Retrieve the current state
 *
 * @memberof			StateMachine
 * @public
 *
 * @param this			Function scope
 *
 * @return				Current state
 */
State StateMachine::getCurrentState(StateMachine this)
{
	ASSERT(this, "StateMachine::getCurrentState: null this");

	return this->currentState;
}

/**
 * Get the second state from the top of the stack
 *
 * @memberof			StateMachine
 * @public
 *
 * @param this			Function scope
 *
 * @return				Second state in the stack
 */
State StateMachine::getPreviousState(StateMachine this)
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

/**
 * Get the StateMachine's stack's size
 *
 * @memberof			StateMachine
 * @public
 *
 * @param this			Function scope
 *
 * @return				The size of the stack
 */
int StateMachine::getStackSize(StateMachine this)
{
	ASSERT(this, "StateMachine::getStackSize: null this");

	return VirtualList::getSize(this->stateStack);
}
