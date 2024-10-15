/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_


//=========================================================================================================
//												INCLUDES
//=========================================================================================================

#include <ListenerObject.h>


//---------------------------------------------------------------------------------------------------------
//											FORWARD DECLARATIONS
//---------------------------------------------------------------------------------------------------------

class Telegram;
class State;
class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S DATA
//---------------------------------------------------------------------------------------------------------

enum StateOperations
{
	kStateMachineIdle = 0,
	kStateMachineSwapState,
	kStateMachineCleanStack,
	kStateMachinePushState,
	kStateMachinePopState
};


//=========================================================================================================
//											CLASS'S DECLARATION
//=========================================================================================================

///
/// Class StateMachine
///
/// Inherits from ListenerObject
///
/// Implements a behavioral model of finite states.
/// @ingroup base
class StateMachine : ListenerObject
{
	/// Owner of this instance
	void* owner;

	/// Linked list of states
	VirtualList stateStack;

	/// State at the top of the stack of states
	State currentState;

	/// Previous state at the top of the stack of states
	State previousState;

	/// State to push on top of the stack of states
	State nextState;

	/// Enum that determines the next state transition
	int16 transition;
	
	/// @publicsection

	/// Class' constructor
	void constructor(void* owner);

	/// Class' destructor
	void destructor();

	/// Instructs the machine to change to the provided state by the transition specified by command
	/// @param state: State to transition to
	/// @param command: Enum that specifies the type of transition defined in StateOperations 
	/// @return true if the machine is not scheduled for a transition already, false otherwise
	bool transitionTo(State state, int16 command);

	/// Swap the state at the top of the stack by the provided state.
	/// @param newState: State to put at the top of the stack
	void swapState(State newState);

	/// Push the provided state at the top of the stack.
	/// @param newState: State to push at the top of the stack
	void pushState(State newState);

	/// Pop the top most state in the stack.
	void popState();

	/// Pop all the states in the stack and leave it empty.
	void popAllStates();

	/// Updates the state at the top of the stack.
	State update();

	/// Check if the top most state in the stack is the provided one.
	/// @param state: State to compare against the state at the top of the stack
	/// @return true if the provided state is the top most state in the stack
	bool isInState(State state);

	/// Check if the provided state is in the stack.
	/// @param state: State to check
	/// @return true if the provided state is in the stack
	bool hasStateInTheStack(State state);

	/// Retrieve the stack of states.
	/// @return Linked list of states
	VirtualList getStateStack();

	/// Retrieve the state at the top of the stack.
	/// @return State at the top of the stack of states
	State getCurrentState();

	/// Retrieve the state that the machine is pending transition to.
	/// @return State that the machine has to transition to
	State getNextState();

	/// Retrieve the state that was before at the top of the stack.
	/// @return State: the state that was previously at the top of the stack
	State getPreviousState();

	/// Retrieve the number of states in the stack.
	/// @return Number of states in the stack
	int32 getStackSize();

	/// Process a Telegram.
	/// @param telegram: Telegram to process
	/// @return True if the Telegram was processed
	override bool handleMessage(Telegram telegram);
}


#endif
