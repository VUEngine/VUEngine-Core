/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <State.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(State, Object);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// defined just to get rid of a warning
__CLASS_NEW_DEFINITION(State)
__CLASS_NEW_END(State);

// class's constructor
void State_constructor(State this)
{
	// construct base object
	__CONSTRUCT_BASE();
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
void State_enter(State this, void* owner)
{
	ASSERT(this, "State::enter: null this");
}

// state's execute
void State_execute(State this, void* owner)
{
	ASSERT(this, "State::execute: null this");
}

// state's exit
void State_exit(State this, void* owner)
{
	ASSERT(this, "State::exit: null this");
}

// state's suspend
void State_suspend(State this, void* owner)
{
	ASSERT(this, "State::suspend: null this");
}

// state's resume
void State_resume(State this, void* owner)
{
	ASSERT(this, "State::resume: null this");
}