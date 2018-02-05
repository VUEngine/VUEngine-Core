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

#ifndef OBJECT_H_
#define OBJECT_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Oop.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Object_METHODS(ClassName)												 						\
		__VIRTUAL_DEC(ClassName, bool, handleMessage, void* telegram);									\

// define the virtual methods
#define Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Object, handleMessage);												\

// declare the class from which all classes derive
#define Object_ATTRIBUTES																				\
		/**
		 * @var void*		vTable
		 * @brief			Pointer to the class's virtual table.
		 * @memberof		Object
		 */																								\
		void* vTable;																					\
		/**
		 * @var VirtualList events
		 * @brief			List of registered events.
		 * @memberof		Object
		 */																								\
		VirtualList events;																				\

__CLASS(Object);


//---------------------------------------------------------------------------------------------------------
//												TYPEDEFS
//---------------------------------------------------------------------------------------------------------

typedef void (*EventListener)(Object, Object);
typedef Object (*AllocatorPointer)();


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Object_constructor(Object this);
void Object_destructor(Object this);
bool Object_handleMessage(Object this, void* telegram);
void Object_addEventListener(Object this, Object listener, EventListener method, u32 eventCode);
void Object_removeEventListener(Object this, Object listener, EventListener method, u32 eventCode);
void Object_removeEventListeners(Object this, Object listener, u32 eventCode);
void Object_removeAllEventListeners(Object this, u32 eventCode);
void Object_fireEvent(Object this, u32 eventCode);
Object Object_getCast(Object this, ObjectBaseClassPointer targetClassGetClassMethod, ObjectBaseClassPointer baseClassGetClassMethod);
const void* Object_getVTable(Object this);


#endif
