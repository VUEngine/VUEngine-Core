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

#ifndef OBJECT_H_
#define OBJECT_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <stdarg.h>
#include <Types.h>
#include <Oop.h>
#include <Constants.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __MAX_EVENT_NAME_LENGTH	16

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the class from which all classes derivate
#define Object_ATTRIBUTES														\
																				\
	/* pointer to the class's virtual table */									\
	void* vTable;																\
																				\
	/* flag to know if it's a dynamic created object */ 						\
	u32 dynamic;																\
																				\
	/* events */																\
	VirtualList events;															\

// declare the virtual methods
#define Object_METHODS															\
	__VIRTUAL_DEC(handleMessage);												\

// define the virtual methods
#define Object_SET_VTABLE(ClassName)											\
	__VIRTUAL_SET(ClassName, Object, handleMessage);

// the root class for everything else!!
__CLASS(Object);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Object_constructor(Object this);
void Object_destructor(Object this);
bool Object_handleMessage(Object this, void* owner, void* telegram);
void Object_addEventListener(Object this, Object listener, void (*method)(Object, Object), char* eventName);
void Object_removeEventListener(Object this, Object listener, void (*method)(Object, Object), char* eventName);
void Object_fireEvent(Object this, char* eventName);
Object Object_upcast(Object this, void* (*targetClassGetClassMethod)(void), void* (*baseClassGetClassMethod)(void));

#endif