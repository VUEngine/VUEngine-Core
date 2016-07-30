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

#ifndef OBJECT_H_
#define OBJECT_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Oop.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __MAX_EVENT_NAME_LENGTH	                16

// events
#define __EVENT_OBJECT_DESTROYED				"objectDestroyed"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Object_METHODS(ClassName)											    						\
	    __VIRTUAL_DEC(ClassName, bool, handleMessage, void* telegram);						            \

// define the virtual methods
#define Object_SET_VTABLE(ClassName)																	\
	    __VIRTUAL_SET(ClassName, Object, handleMessage);												\

// declare the class from which all classes derive
#define Object_ATTRIBUTES																				\
        /* pointer to the class's virtual table */														\
        void* vTable;																				    \
        /* events */																					\
        VirtualList events;																		    	\

// the root class for everything else!!
__CLASS(Object);


//---------------------------------------------------------------------------------------------------------
// 											    TYPEDEFS
//---------------------------------------------------------------------------------------------------------

typedef void (*EventListener)(Object, Object);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Object_constructor(Object this);
void Object_destructor(Object this);
bool Object_handleMessage(Object this, void* telegram);
void Object_addEventListener(Object this, Object listener, EventListener method, char* eventName);
void Object_removeEventListener(Object this, Object listener, EventListener method, char* eventName);
void Object_fireEvent(Object this, char* eventName);
Object Object_getCast(Object this, void* (*targetClassGetClassMethod)(), void* (*baseClassGetClassMethod)());
const void* Object_getVTable(Object this);

#endif
