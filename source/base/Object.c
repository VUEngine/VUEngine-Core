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

friend class VirtualNode;
friend class VirtualList;

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
void Object::constructor()
{
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
void Object::destructor()
{
	ASSERT(__IS_OBJECT_ALIVE(this), "Object::destructor: already deleted this");

	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			__DELETE_BASIC(node->data);
		}

		delete this->events;
		this->events = NULL;
	}

	// free the memory
#ifdef __DEBUG
	MemoryPool::free(_memoryPool, (void*)this);
#else
	*((u32*)this) = __MEMORY_FREE_BLOCK_FLAG;
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
bool Object::handleMessage(Telegram telegram __attribute__ ((unused)))
{
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
void Object::addEventListener(Object listener, EventListener method, u32 eventCode)
{
	if(!listener || !method)
	{
		return;
	}

	if(NULL == this->events)
	{
		this->events = new VirtualList();
	}
	else
	{
		// don't add the same listener twice
		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(listener == event->listener && method == event->method && eventCode == event->code)
			{
				return;
			}
		}
	}

	Event* event = new Event;
	event->listener = listener;
	event->method = method;
	event->code = eventCode;

	VirtualList::pushBack(this->events, event);
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
void Object::removeEventListener(Object listener, EventListener method, u32 eventCode)
{
	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(listener == event->listener && method == event->method && eventCode == event->code)
			{
				VirtualList::removeNode(this->events, node);

				__DELETE_BASIC(event);
				break;
			}
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
void Object::removeEventListeners(Object listener, u32 eventCode)
{
	if(this->events)
	{
		VirtualList eventsToRemove = new VirtualList();

		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(listener == event->listener && eventCode == event->code)
			{
				VirtualList::pushBack(eventsToRemove, event);
			}
		}

		for(node = eventsToRemove->head; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			VirtualList::removeElement(this->events, event);

			__DELETE_BASIC(event);
		}

		delete eventsToRemove;

		if(!this->events->head)
		{
			delete this->events;
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
void Object::removeAllEventListeners(u32 eventCode)
{
	if(this->events)
	{
		VirtualList eventsToRemove = new VirtualList();

		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(eventCode == event->code)
			{
				VirtualList::pushBack(eventsToRemove, event);
			}
		}

		for(node = eventsToRemove->head; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			VirtualList::removeElement(this->events, event);

			__DELETE_BASIC(event);
		}

		delete eventsToRemove;

		if(!this->events->head)
		{
			delete this->events;
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
void Object::fireEvent(u32 eventCode)
{
	if(this->events)
	{
		// temporal lists to being able to modify the event lists while firing them
		VirtualList eventsToFire = new VirtualList();
		VirtualList eventsToRemove = new VirtualList();

		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			// safe check in case that the there is a stacking up of firings within firings
			if(!__IS_BASIC_OBJECT_ALIVE(event) || !__IS_OBJECT_ALIVE(event->listener))
			{
				VirtualList::pushBack(eventsToRemove, event);
			}
			else if(eventCode == event->code)
			{
				VirtualList::pushBack(eventsToFire, event);
			}
		}

		for(node = eventsToRemove->head; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			VirtualList::removeElement(this->events, event);

			// safe check in case that the there is a stacking up of firings within firings
			if(__IS_BASIC_OBJECT_ALIVE(event))
			{
				__DELETE_BASIC(event);
			}
		}

		delete eventsToRemove;

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

		delete eventsToFire;
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
static Object Object::getCast(void* object, ObjectBaseClassPointer targetClassGetClassMethod, ObjectBaseClassPointer baseClassGetClassMethod)
{
	static int lp = -1;
	static int sp = -1;

	if(-1 == lp && -1 == sp)
	{
		asm(" mov sp,%0  ": "=r" (sp));
		asm(" mov lp,%0  ": "=r" (lp));
	}

	if(!object)
	{
		lp = -1;
		sp = -1;
		return NULL;
	}

	if(!__IS_OBJECT_ALIVE(object))
	{
	/*
		Printing::setDebugMode(Printing::getInstance());
		Printing::text(Printing::getInstance(), "Object's address: ", 1, 15, NULL);
		Printing::hex(Printing::getInstance(), (u32)object, 18, 15, 8, NULL);
*/
		_lp = lp;
		_sp = sp;
		NM_CAST_ASSERT(false, "Object::getCast: deleted object");
	}

	ASSERT(__VIRTUAL_CALL_ADDRESS(Object, getClassName, object), "Object::getCast: null getClassName");
	ASSERT(__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object), "Object::getCast: null getBaseClass");

	if(!baseClassGetClassMethod)
	{
		if(targetClassGetClassMethod == (ObjectBaseClassPointer)__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object))
		{
			lp = -1;
			sp = -1;
			return object;
		}

		// make my own virtual call, otherwise the macro will cause an infinite recursive call because of the
		// __SAFE_CAST check
		baseClassGetClassMethod = (ObjectBaseClassPointer)__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object)(object);
	}

	if(!baseClassGetClassMethod || ((ObjectBaseClassPointer)&Object_getBaseClass == baseClassGetClassMethod && (ObjectBaseClassPointer)&Object_getBaseClass != targetClassGetClassMethod))
	{
		lp = -1;
		sp = -1;
		return NULL;
	}

	if(targetClassGetClassMethod == baseClassGetClassMethod)
	{
		lp = -1;
		sp = -1;
		return object;
	}

	return Object::getCast((Object)object, targetClassGetClassMethod, (ObjectBaseClassPointer)baseClassGetClassMethod(object));
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
const void* Object::getVTable()
{
	return this->vTable;
}

