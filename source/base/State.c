/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
 */
__CLASS_DEFINITION(State, Object);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// defined just to get rid of a warning
__CLASS_NEW_DEFINITION(State)
__CLASS_NEW_END(State);

// class's constructor
void State_constructor(State this)
{
	// construct base object
	__CONSTRUCT_BASE(Object);
}

// class's destructor
void State_destructor(State this)
{
	ASSERT(this, "State::destructor: null this");

	// free processor's memory
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// state's enter
void State_enter(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::enter: null this");
}

// state's execute
void State_execute(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::execute: null this");
}

// state's exit
void State_exit(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::exit: null this");
}

// state's suspend
void State_suspend(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::suspend: null this");
}

// state's resume
void State_resume(State this __attribute__ ((unused)), void* owner __attribute__ ((unused)))
{
	ASSERT(this, "State::resume: null this");
}
