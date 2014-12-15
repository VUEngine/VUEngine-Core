/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <State.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(State);


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
	__CONSTRUCT_BASE(Object);
}

// class's destructor
void State_destructor(State this)
{
	ASSERT(this, "State::destructor: null this");

	// free processor's memory
	__DESTROY_BASE(Object);
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

// state's pause
void State_pause(State this, void* owner)
{
	ASSERT(this, "State::pause: null this");
}

// state's resume
void State_resume(State this, void* owner)
{
	ASSERT(this, "State::resume: null this");
}

