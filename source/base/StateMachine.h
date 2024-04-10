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

#include <ListenerObject.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

enum StateOperations
{
	kStateMachineIdle = 0,
	kStateMachineSwapState,
	kStateMachineCleanStack,
	kStateMachinePushState,
	kStateMachinePopState
};

class Telegram;
class State;
class VirtualList;

/// @ingroup base
class StateMachine : ListenerObject
{
	// ListenerObject which owns this instance
	void* owner;
	// Stack of states
	VirtualList stateStack;
	// Pointer to the current state
	State currentState;
	// Pointer to the previous state
	State previousState;
	// game's next state
	State nextState;
	// game's next state operation
	int16 transition;
	
	/// @publicsection
	void constructor(void* owner);
	State update();
	bool prepareTransition(State state, int16 command);
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
	State getNextState();
	State getPreviousState();
	int32 getStackSize();
	override bool handleMessage(Telegram telegram);
}


#endif
