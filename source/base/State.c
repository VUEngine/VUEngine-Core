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

#include <State.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class 	State
 * @extends Object
 * @ingroup base
 */



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof	State
 * @public
 *
 * @param this	Function scope
 */
void State::constructor(State this)
{
	ASSERT(this, "State::constructor: null this");

	// construct base object
	Base::constructor();
}

/**
 * Class destructor
 *
 * @memberof	State
 * @public
 *
 * @param this	Function scope
 */
void State::destructor(State this)
{
	ASSERT(this, "State::destructor: null this");

	// free processor's memory
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Method called when the StateMachine enters to this state
 *
 * @memberof		State
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void State::enter(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::enter: null this");
}

/**
 * Method called when by the StateMachine's update method
 *
 * @memberof		State
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void State::execute(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::execute: null this");
}

/**
 * Method called when the StateMachine exits from this state
 *
 * @memberof		State
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void State::exit(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::exit: null this");
}

/**
 * Method called when the StateMachine enters another state without exiting this one
 *
 * @memberof		State
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void State::suspend(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::suspend: null this");
}

/**
 * Method called when the StateMachine returns to this state from another
 *
 * @memberof		State
 * @public
 *
 * @param this		Function scope
 * @param owner		StateMachine's owner
 */
void State::resume(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::resume: null this");
}
