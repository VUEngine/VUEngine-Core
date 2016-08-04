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

#include <string.h>
#include <Object.h>
#include <VirtualList.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// it is the base class for everything.. so it does derives from nothing but itself
__CLASS_DEFINITION(Object, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


typedef struct Event
{
	Object listener;
	EventListener method;
	char name[__MAX_EVENT_NAME_LENGTH];

} Event;


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Object_constructor(Object this)
{
	this->events = NULL;
}

// class's destructor
void Object_destructor(Object this)
{
	Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_OBJECT_DESTROYED);

	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			__DELETE_BASIC(node->data);
		}

		__DELETE(this->events);
	}

	// an Object can not be instantiated, so there is no memory to free

	ASSERT(this, "Object::destructor: null this");
}

// on message
bool Object_handleMessage(Object this, void* telegram)
{
	ASSERT(this, "Object::handleMessage: null this");

	return false;
}

// register an event listener
void Object_addEventListener(Object this, Object listener, EventListener method, char* eventName)
{
	ASSERT(this, "Object::addEventListener: null this");

	if(!listener || !method || !eventName)
	{
		return;
	}

	if(NULL == this->events)
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

	// don't rely on the user, make it safe
	strncpy(event->name, eventName, __MAX_EVENT_NAME_LENGTH);

	VirtualList_pushBack(this->events, event);
}

// remove an event listener
void Object_removeEventListener(Object this, Object listener, EventListener method, char* eventName)
{
	ASSERT(this, "Object::removeEventListener: null this");

	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(listener == event->listener && method == event->method && !strncmp(event->name, eventName, __MAX_EVENT_NAME_LENGTH))
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

	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(!strncmp(event->name, eventName, __MAX_EVENT_NAME_LENGTH))
			{
				event->method(event->listener, this);
			}
		}
	}
}

// cast object to base class
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

		baseClassGetClassMethod = (ObjectBaseClassPointer)__VIRTUAL_CALL_UNSAFE(Object, getBaseClass, this);
	}

	if(!baseClassGetClassMethod || ((ObjectBaseClassPointer)&Object_getBaseClass == baseClassGetClassMethod && (ObjectBaseClassPointer)&Object_getBaseClass != targetClassGetClassMethod))
	{
		return NULL;
	}

	if(targetClassGetClassMethod == baseClassGetClassMethod)
	{
		return this;
	}

	return Object_getCast((Object)this, targetClassGetClassMethod, baseClassGetClassMethod(this));
}

const void* Object_getVTable(Object this)
{
	ASSERT(this, "Object::getVTable: null this");

    return this->vTable;
}

