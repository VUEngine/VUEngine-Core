/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

#ifndef STATE_H_
#define STATE_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// declare the virtual methods
#define State_METHODS							\
		Object_METHODS							\
		__VIRTUAL_DEC(enter);					\
		__VIRTUAL_DEC(execute);					\
		__VIRTUAL_DEC(exit);					\
		__VIRTUAL_DEC(pause);					\
		__VIRTUAL_DEC(resume);					\
	

// declare the virtual methods which are redefined
#define State_SET_VTABLE(ClassName)						\
		Object_SET_VTABLE(ClassName)					\
		/*__VIRTUAL_SET(ClassName, State, enter);	*/	\
		__VIRTUAL_SET(ClassName, State, execute);		\
		__VIRTUAL_SET(ClassName, State, exit);			\
		__VIRTUAL_SET(ClassName, State, pause);			\
		__VIRTUAL_SET(ClassName, State, resume);		\
	
	
	

// declare a Sprite, which holds a texture and a drawing specification
__CLASS(State);

// a generic state
#define State_ATTRIBUTES								\
		Object_ATTRIBUTES								\
		

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//class's constructor
void State_constructor(State this);

//class's destructor
void State_destructor(State this);

// state's enter
void State_enter(State this, void* owner);

// state's execute
void State_execute(State this, void* owner);

// state's exit
void State_exit(State this, void* owner);

// state's pause
void State_pause(State this, void* owner);

// state's resume
void State_resume(State this, void* owner);

#endif /*STATE_H_*/
