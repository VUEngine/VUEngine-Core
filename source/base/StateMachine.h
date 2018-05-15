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

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <State.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class StateMachine : Object
{
	/**
	 * @var void*		owner
	 * @brief			Object which owns this instance
	 * @memberof		StateMachine
	 */
	void* owner;
	/**
	 * @var State		currentState
	 * @brief			Pointer to the current state
	 * @memberof		StateMachine
	 */
	State currentState;
	/**
	 * @var State		previousState
	 * @brief			Pointer to the previous state
	 * @memberof		StateMachine
	 */
	State previousState;
	/**
	 * @var VirtualList	stateStack
	 * @brief			Stack of states
	 * @memberof		StateMachine
	 */
	VirtualList stateStack;

	void constructor(StateMachine this, void* owner);
	void update(StateMachine this);
	void swapState(StateMachine this, State newState);
	u32 pushState(StateMachine this, State newState);
	u32 popState(StateMachine this);
	void returnToPreviousState(StateMachine this);
	void changeToGlobal(StateMachine this, State globalState);
	bool isInState(StateMachine this, State state);
	void setOwner(StateMachine this, void* owner);
	State getCurrentState(StateMachine this);
	State getPreviousState(StateMachine this);
	int getStackSize(StateMachine this);
	override bool handleMessage(StateMachine this, Telegram telegram);
}


#endif
