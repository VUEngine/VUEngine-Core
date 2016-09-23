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

#include <Object.h>
#include <VirtualList.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class               Object
 * @brief               Base class for all other classes in the engine.
 *
 * @var VirtualList     events
 * @brief               List of registered events.
 * @memberof            Object
 */

// this is the base class for everything, so it derives from nothing but itself
__CLASS_DEFINITION(Object, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);

/**
 * An event
 */
typedef struct Event
{
	Object listener;
	EventListener method;
	u32 code;
} Event;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// to speed things up
extern MemoryPool _memoryPool;


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof    Object
 * @public
 *
 * @param this  Function scope
 */
void Object_constructor(Object this)
{
	ASSERT(this, "Object::destructor: null this");

	this->events = NULL;
}

/**
 * Class destructor
 *
 * @memberof    Object
 * @public
 *
 * @param this  Function scope
 */
void Object_destructor(Object this)
{
	ASSERT(this, "Object::destructor: null this");
	ASSERT(*(u32*)this, "Object::destructor: already deleted this");

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
    MemoryPool_free(_memoryPool, (void*)this);
}

/**
 * Handles incoming messages
 *
 * @memberof        Object
 * @public
 *
 * @param this      Function scope
 * @param telegram  The received message
 *
 * @return Always returns false, this is meant to be used only in derived classes
 */
bool Object_handleMessage(Object this __attribute__ ((unused)), void* telegram __attribute__ ((unused)))
{
	ASSERT(this, "Object::handleMessage: null this");

	return false;
}

/**
 * Registers an event listener
 *
 * @memberof            Object
 * @public
 *
 * @param this          Function scope
 * @param listener      Object to register event listener at
 * @param method        The method to execute on event
 * @param eventCode     The code of the event to listen to
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
 * @memberof            Object
 * @public
 *
 * @param this          Function scope
 * @param listener      Object where event listener is registered at
 * @param method        The method attached to event listener
 * @param eventCode     The code of the event
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
	}
}

/**
 * Removes event listeners without specifying a method
 *
 * @memberof            Object
 * @public
 *
 * @param this          Function scope
 * @param listener      Object where event listener is registered at
 * @param eventCode     The code of the event
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

		if(!VirtualList_getSize(this->events))
		{
            __DELETE(this->events);
            this->events = NULL;
		}
	}
}

/**
 * Removes event listeners without specifying a method nor a listener
 *
 * @memberof            Object
 * @public
 *
 * @param this          Function scope
 * @param eventCode     The code of the event
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

		if(!VirtualList_getSize(this->events))
		{
            __DELETE(this->events);
            this->events = NULL;
		}
	}
}

/**
 * Fires an event
 *
 * @memberof            Object
 * @public
 *
 * @param this          Function scope
 * @param eventCode     The code     of the event
 */
void Object_fireEvent(Object this,  u32 eventCode)
{
	ASSERT(this, "Object::fireEvent: null this");

	if(this->events)
	{
	    VirtualList eventsToRemove = __NEW(VirtualList);

		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

		    if(!*(u32*)event->listener)
            {
                VirtualList_pushBack(eventsToRemove, event);
            }
            else
		    {
                if(eventCode == event->code)
                {
                    event->method(event->listener, this);
                }
            }
		}

		for(node = eventsToRemove->head; node; node = node->next)
		{
			Event* event = (Event*)node->data;

            VirtualList_removeElement(this->events, event);

            __DELETE_BASIC(event);
		}

		__DELETE(eventsToRemove);
	}
}

/**
 * Casts an object to base class
 *
 * @memberof                            Object
 * @public
 *
 * @param this                          Function scope
 * @param targetClassGetClassMethod
 * @param baseClassGetClassMethod
 *
 * @return                              Casted Object
 */
Object Object_getCast(Object this, ObjectBaseClassPointer targetClassGetClassMethod, ObjectBaseClassPointer baseClassGetClassMethod)
{
	ASSERT(this, "Object::getCast: null this");

	if(!this)
	{
		return NULL;
	}

    if(!*(u32*)this)
    {
        Printing_text(Printing_getInstance(), "Object's address: ", 1, 15, NULL);
        Printing_hex(Printing_getInstance(), (u32)this, 18, 15, NULL);
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
 * @memberof    Object
 * @public
 *
 * @param this  Function scope
 *
 * @return      vTable pointer
 */
const void* Object_getVTable(Object this)
{
	ASSERT(this, "Object::getVTable: null this");

    return this->vTable;
}

