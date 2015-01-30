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

#include <Object.h>
#include <string.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// it is the base class for everything.. so it does derivates from nothing but itself
__CLASS_DEFINITION(Object, Object);

typedef struct Event
{
	Object listener;
	void (*method)(Object, Object);
	char name[__MAX_EVENT_NAME_LENGTH];

} Event;


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Object_constructor(Object this)
{
	this->dynamic = false;
	this->events = NULL;
}

// class's destructor
void Object_destructor(Object this)
{
	if (this->events)
	{
		VirtualNode node = VirtualList_begin(this->events);

		for (; node; node = VirtualNode_getNext(node))
		{
			__DELETE_BASIC(VirtualNode_getData(node));
		}

		__DELETE(this->events);
	}

	/* an Object can not be instantiated, so there is no
	 * memory to free
	 */

	ASSERT(this, "Object::destructor: null this");
}

// on message
bool Object_handleMessage(Object this, void* owner, void* telegram)
{
	ASSERT(this, "Object::handleMessage: null this");

	return false;
}

// register an event listener
void Object_addEventListener(Object this, Object listener, void (*method)(Object, Object),  char* eventName)
{
	ASSERT(this, "Object::addEventListener: null this");

	if (!listener || !method || !eventName)
	{
		return;
	}

	if (NULL == this->events)
	{
		this->events = __NEW(VirtualList);
	}
	else
	{
		Object_removeEventListener(this, listener, method, eventName);
	}

	Event* event = __NEW_BASIC(Event);
	event->listener = listener;
	event->method = method;
	// don't relay on the user, make it safe
	strncpy(event->name, eventName, __MAX_EVENT_NAME_LENGTH);

	VirtualList_pushBack(this->events, event);
}

// remove an event listener
void Object_removeEventListener(Object this, Object listener, void (*method)(Object, Object),  char* eventName)
{
	ASSERT(this, "Object::addEventListener: null this");

	if (this->events)
	{
		VirtualNode node = VirtualList_begin(this->events);

		for (; node; node = VirtualNode_getNext(node))
		{
			Event* event = (Event*)VirtualNode_getData(node);

			if (listener == event->listener && method == event->method && !strncmp(event->name, eventName, __MAX_EVENT_NAME_LENGTH))
			{
				VirtualList_removeElement(this->events, event);

				__DELETE_BASIC(event);
				break;
			}
		}
	}
}

// fire event
void Object_fireEvent(Object this,  char* eventName)
{
	ASSERT(this, "Object::fireEvent: null this");

	if (this->events)
	{
		VirtualNode node = VirtualList_begin(this->events);

		for (; node; node = VirtualNode_getNext(node))
		{
			Event* event = (Event*)VirtualNode_getData(node);

			if (!strncmp(event->name, eventName, __MAX_EVENT_NAME_LENGTH))
			{
				event->method(event->listener, this);
			}
		}
	}
}