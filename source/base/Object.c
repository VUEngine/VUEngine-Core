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

#include <Object.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Object
 * @ingroup base
 * @brief	Base class for all other classes in the engine, it derives from nothing but itself
 */
__CLASS_DEFINITION(Object, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);

/**
 * An Event
 *
 * @memberof Object
 */
typedef struct Event
{
	/// Object to register event listener at
	Object listener;
	/// The method to execute on event
	EventListener method;
	/// The code of the event to listen to
	u32 code;

} Event;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// to speed things up
extern MemoryPool _memoryPool;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof	Object
 * @public
 *
 * @param this	Function scope
 */
void Object_constructor(Object this)
{
	ASSERT(this, "Object::destructor: null this");

	this->events = NULL;
}

/**
 * Class destructor
 *
 * @memberof	Object
 * @public
 *
 * @param this	Function scope
 */
void Object_destructor(Object this)
{
	ASSERT(this, "Object::destructor: null this");
	ASSERT(__IS_OBJECT_ALIVE(this), "Object::destructor: already deleted this");

	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			__DELETE_BASIC(node->data);
		}

		__DELETE(this->events);
		this->events = NULL;
	}

	// free the memory
#ifdef __DEBUG
	MemoryPool_free(_memoryPool, (void*)this);
#else
	*((u32*)this) = 0;
#endif
}

/**
 * Handles incoming messages
 *
 * @memberof		Object
 * @public
 *
 * @param this		Function scope
 * @param telegram	The received message
 *
 * @return			Always returns false, this is meant to be used only in derived classes
 */
bool Object_handleMessage(Object this __attribute__ ((unused)), void* telegram __attribute__ ((unused)))
{
	ASSERT(this, "Object::handleMessage: null this");

	return false;
}

/**
 * Registers an event listener
 *
 * @memberof			Object
 * @public
 *
 * @param this			Function scope
 * @param listener		Object to register event listener at
 * @param method		The method to execute on event
 * @param eventCode		The code of the event to listen to
 */
void Object_addEventListener(Object this, Object listener, EventListener method, u32 eventCode)
{
	ASSERT(this, "Object::addEventListener: null this");

	if(!listener || !method)
	{
		return;
	}

	if(NULL == this->events)
	{
		this->events = __NEW(VirtualList);
	}
	else
	{
		Object_removeEventListener(this, listener, method, eventCode);

		// the event list can be deleted when removed previous listeners
		if(NULL == this->events)
		{
			this->events = __NEW(VirtualList);
		}
	}

	Event* event = __NEW_BASIC(Event);
	event->listener = listener;
	event->method = method;
	event->code = eventCode;

	VirtualList_pushBack(this->events, event);
}

/**
 * Removes an event listener
 *
 * @memberof			Object
 * @public
 *
 * @param this			Function scope
 * @param listener		Object where event listener is registered at
 * @param method		The method attached to event listener
 * @param eventCode		The code of the event
 */
void Object_removeEventListener(Object this, Object listener, EventListener method, u32 eventCode)
{
	ASSERT(this, "Object::removeEventListener: null this");

	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(listener == event->listener && method == event->method && eventCode == event->code)
			{
				VirtualList_removeElement(this->events, event);

				__DELETE_BASIC(event);
				break;
			}
		}

		if(!this->events->head)
		{
			__DELETE(this->events);
			this->events = NULL;
		}
	}
}

/**
 * Removes event listeners without specifying a method
 *
 * @memberof			Object
 * @public
 *
 * @param this			Function scope
 * @param listener		Object where event listener is registered at
 * @param eventCode		The code of the event
 */
void Object_removeEventListeners(Object this, Object listener, u32 eventCode)
{
	ASSERT(this, "Object::removeEventListeners: null this");

	if(this->events)
	{
		VirtualList eventsToRemove = __NEW(VirtualList);

		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(listener == event->listener && eventCode == event->code)
			{
				VirtualList_pushBack(eventsToRemove, event);
			}
		}

		for(node = eventsToRemove->head; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			VirtualList_removeElement(this->events, event);

			__DELETE_BASIC(event);
		}

		__DELETE(eventsToRemove);

		if(!this->events->head)
		{
			__DELETE(this->events);
			this->events = NULL;
		}
	}
}

/**
 * Removes event listeners without specifying a method nor a listener
 *
 * @memberof			Object
 * @public
 *
 * @param this			Function scope
 * @param eventCode		The code of the event
 */
void Object_removeAllEventListeners(Object this, u32 eventCode)
{
	ASSERT(this, "Object::removeEventListeners: null this");

	if(this->events)
	{
		VirtualList eventsToRemove = __NEW(VirtualList);

		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(eventCode == event->code)
			{
				VirtualList_pushBack(eventsToRemove, event);
			}
		}

		for(node = eventsToRemove->head; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			VirtualList_removeElement(this->events, event);

			__DELETE_BASIC(event);
		}

		__DELETE(eventsToRemove);

		if(!this->events->head)
		{
			__DELETE(this->events);
			this->events = NULL;
		}
	}
}

/**
 * Fires an event
 *
 * @memberof			Object
 * @public
 *
 * @param this			Function scope
 * @param eventCode		The code of the event
 */
void Object_fireEvent(Object this, u32 eventCode)
{
	ASSERT(this, "Object::fireEvent: null this");

	if(this->events)
	{
		// temporal lists to being able to modify the event lists while firing them
		VirtualList eventsToFire = __NEW(VirtualList);
		VirtualList eventsToRemove = __NEW(VirtualList);

		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			// safe check in case that the there is a stacking up of firings within firings
			if(!__IS_BASIC_OBJECT_ALIVE(event) || !__IS_OBJECT_ALIVE(event->listener))
			{
				VirtualList_pushBack(eventsToRemove, event);
			}
			else if(eventCode == event->code)
			{
				VirtualList_pushBack(eventsToFire, event);
			}
		}

		for(node = eventsToRemove->head; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			VirtualList_removeElement(this->events, event);

			// safe check in case that the there is a stacking up of firings within firings
			if(__IS_BASIC_OBJECT_ALIVE(event))
			{
				__DELETE_BASIC(event);
			}
		}

		__DELETE(eventsToRemove);

		node = eventsToFire->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			// safe check in case that the event have been deleted during the previous call to method
			if(__IS_BASIC_OBJECT_ALIVE(event))
			{
				event->method(event->listener, this);
			}

			// safe check in case that I have been deleted during the previous event
			if(!__IS_OBJECT_ALIVE(this))
			{
				break;
			}
		}

		__DELETE(eventsToFire);
	}
}

/**
 * Casts an object to base class
 *
 * @memberof							Object
 * @public
 *
 * @param this							Function scope
 * @param targetClassGetClassMethod
 * @param baseClassGetClassMethod
 *
 * @return								Casted Object
 */
Object Object_getCast(Object this, ObjectBaseClassPointer targetClassGetClassMethod, ObjectBaseClassPointer baseClassGetClassMethod)
{
	ASSERT(this, "Object::getCast: null this");

	if(!this)
	{
		return NULL;
	}

	if(!__IS_OBJECT_ALIVE(this))
	{
		Printing_setDebugMode(Printing_getInstance());
		Printing_text(Printing_getInstance(), "Object's address: ", 1, 15, NULL);
		Printing_hex(Printing_getInstance(), (u32)this, 18, 15, 8, NULL);
		NM_ASSERT(false, "Object::getCast: deleted this");
	}

	ASSERT(__VIRTUAL_CALL_ADDRESS(Object, getClassName, this), "Object::getCast: null getClassName");
	ASSERT(__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, this), "Object::getCast: null getBaseClass");

	if(!baseClassGetClassMethod)
	{
		if(targetClassGetClassMethod == (ObjectBaseClassPointer)__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, this))
		{
			return this;
		}

		// make my own virtual call, otherwise the macro will cause an infinite recursive call because of the
		// __SAFE_CAST check
		baseClassGetClassMethod = (ObjectBaseClassPointer)__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, this)(this);
	}

	if(!baseClassGetClassMethod || ((ObjectBaseClassPointer)&Object_getBaseClass == baseClassGetClassMethod && (ObjectBaseClassPointer)&Object_getBaseClass != targetClassGetClassMethod))
	{
		return NULL;
	}

	if(targetClassGetClassMethod == baseClassGetClassMethod)
	{
		return this;
	}

	return Object_getCast((Object)this, targetClassGetClassMethod, (ObjectBaseClassPointer)baseClassGetClassMethod(this));
}

/**
 * Get an Object's vTable
 *
 * @memberof	Object
 * @public
 *
 * @param this	Function scope
 *
 * @return		vTable pointer
 */
const void* Object_getVTable(Object this)
{
	ASSERT(this, "Object::getVTable: null this");

	return this->vTable;
}

