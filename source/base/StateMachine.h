/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <State.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
class StateMachine : Object
{
	// Object which owns this instance
	void* owner;
	// Pointer to the current state
	State currentState;
	// Pointer to the previous state
	State previousState;
	// Stack of states
	VirtualList stateStack;

	/// @publicsection
	void constructor(void* owner);
	void update();
	void swapState(State newState);
	uint32 pushState(State newState);
	uint32 popState();
	void popAllStates();
	void returnToPreviousState();
	void changeToGlobal(State globalState);
	bool isInState(State state);
	void setOwner(void* owner);
	bool hasStateInTheStack(State state);
	VirtualList getStateStack();
	State getCurrentState();
	State getPreviousState();
	int32 getStackSize();
	override bool handleMessage(Telegram telegram);
}


#endif
