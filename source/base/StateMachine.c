/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
	this->stateStack = new VirtualList();
}

/**
 * Class destructor
 */
void StateMachine::destructor()
{
	ASSERT(this->stateStack, "StateMachine::destructor: null stateStack");

	// delete the stack
	VirtualNode node = this->stateStack->head;

	for(; node; node = node->next)
	{
		delete node->data;
	}

	// deallocate the list
	delete this->stateStack;

	// free processor memory
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Method to propagate the update process to the current state
 */
void StateMachine::update()
{
	if(this->currentState)
	{
		State::execute(this->currentState, this->owner);
	}
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
	if(this->currentState)
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
 */
void StateMachine::returnToPreviousState()
{
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
 * @param globalState	State to switch to
 */
void StateMachine::changeToGlobal(State globalState)
{
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
int StateMachine::getStackSize()
{
	return VirtualList::getSize(this->stateStack);
}
