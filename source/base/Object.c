/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
#include <Utilities.h>
#include <MessageDispatcher.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;

// to speed things up
extern MemoryPool _memoryPool;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void Object::constructor()
{
	this->events = NULL;
}

/**
 * Class destructor
 */
void Object::destructor()
{
	ASSERT(!isDeleted(this), "Object::destructor: already deleted this");

	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		delete this->events;
		this->events = NULL;
	}

	// free the memory
#ifdef __DEBUG
	MemoryPool::free(_memoryPool, (void*)((u32)this - __DYNAMIC_STRUCT_PAD));
#else
	*((u32*)((u32)this - __DYNAMIC_STRUCT_PAD)) = __MEMORY_FREE_BLOCK_FLAG;
#endif
}

/**
 * Handles incoming messages
 *
 * @param telegram	The received message
 * @return			Always returns false, this is meant to be used only in derived classes
 */
bool Object::handleMessage(Telegram telegram __attribute__ ((unused)))
{
	return false;
}

/**
 * Registers an event listener
 *
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

				delete event;
				break;
			}
		}
	}
}

/**
 * Removes event listeners without specifying a scope
 *
 * @param listener		Object where event listener is registered at
 * @param eventCode		The code of the event
 */
void Object::removeEventListeners(EventListener method, u32 eventCode)
{
	if(this->events)
	{
		VirtualList eventsToRemove = new VirtualList();

		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(method == event->method && eventCode == event->code)
			{
				VirtualList::pushBack(eventsToRemove, event);
			}
		}

		for(node = eventsToRemove->head; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			VirtualList::removeElement(this->events, event);

			delete event;
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
 * Removes event listeners without specifying a method
 *
 * @param listener		Object where event listener is registered at
 * @param eventCode		The code of the event
 */
void Object::removeEventListenerScopes(Object listener, u32 eventCode)
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

			delete event;
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

			delete event;
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
 * Returns whether there are event listeners or not
 *
 * @returns				True if there are registered event listeners
 */
bool Object::hasActiveEventListeners()
{
	return !isDeleted(this->events) ? 0 < VirtualList::getSize(this->events) : false;
}

/**
 * Fires an event
 *
 * @param eventCode		The code of the event
 */
void Object::fireEvent(u32 eventCode)
{
	if(this->events)
	{
		// temporary lists to being able to modify the event lists while firing them
		VirtualList eventsToFire = new VirtualList();
		VirtualList eventsToRemove = new VirtualList();

		VirtualNode node = this->events->head;

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			// safety check in case that the there is a stacking up of firings within firings
			if(isDeleted(event) || isDeleted(event->listener))
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

			// safety check in case that the there is a stacking up of firings within firings
			if(!isDeleted(event))
			{
				delete event;
			}
		}

		delete eventsToRemove;

		node = eventsToFire->head;

#ifndef __RELEASE
		const char* className = __GET_CLASS_NAME(this);
#endif

		for(; node; node = node->next)
		{
			Event* event = (Event*)node->data;

			// safe check in case that the event have been deleted during the previous call to method
			if(!isDeleted(event))
			{
				event->method(event->listener, this);
			}

			// safe check in case that I have been deleted during the previous event
			if(isDeleted(this))
			{
#ifndef __RELEASE
				Printing::setDebugMode(Printing::getInstance());
				Printing::clear(Printing::getInstance());
				Printing::text(Printing::getInstance(), "Class:    ", 1, 12, NULL);
				Printing::text(Printing::getInstance(), className, 13, 12, NULL);
				Printing::text(Printing::getInstance(), "Method:    ", 1, 13, NULL);
				Printing::hex(Printing::getInstance(), (int)event->method, 13, 13, 8, NULL);
				Printing::text(Printing::getInstance(), "Event code: ", 1, 14, NULL);
				Printing::int(Printing::getInstance(), event->code, 13, 14, NULL);
				NM_ASSERT(!isDeleted(this), "Object::fireEvent: deleted during event listening");
#endif
				break;
			}
		}

		delete eventsToFire;
	}
}

/**
 * Casts an object to base class
 *
 * @param targetClassGetClassMethod
 * @param baseClassGetClassMethod
 * @return								Casted Object
 */
static Object Object::getCast(void* object, ClassPointer targetClassGetClassMethod, ClassPointer baseClassGetClassMethod)
{
#ifdef __BYPASS_CAST
	return object;
#endif

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

	if(isDeleted(object))
	{
	/*
		Printing::setDebugMode(Printing::getInstance());
		Printing::text(Printing::getInstance(), "Object's address: ", 1, 15, NULL);
		Printing::hex(Printing::getInstance(), (u32)object, 18, 15, 8, NULL);
	*/
		_vuengineLinkPointer = lp;
		_vuengineStackPointer = sp;
		NM_CAST_ASSERT(false, "Object::getCast: deleted object");
	}

	ASSERT(__VIRTUAL_CALL_ADDRESS(Object, getClassName, object), "Object::getCast: null getClassName");
	ASSERT(__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object), "Object::getCast: null getBaseClass");

	if(!baseClassGetClassMethod)
	{
		if(targetClassGetClassMethod == (ClassPointer)__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object))
		{
			lp = -1;
			sp = -1;
			return object;
		}

		// make my own virtual call, otherwise the macro will cause an infinite recursive call because of the
		// ObjectClass::getCast check
		baseClassGetClassMethod = (ClassPointer)__VIRTUAL_CALL_ADDRESS(Object, getBaseClass, object)(object);
	}

	if(!baseClassGetClassMethod || ((ClassPointer)&Object_getBaseClass == baseClassGetClassMethod && (ClassPointer)&Object_getBaseClass != targetClassGetClassMethod))
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

	return Object::getCast((Object)object, targetClassGetClassMethod, (ClassPointer)baseClassGetClassMethod(object));
}

/**
 * Send message to object
 *
 * @param receiver
 * @param message
 * @param delay
 * @param randomDelay
 */
void Object::sendMessageTo(Object receiver, u32 message, u32 delay, u32 randomDelay)
{
	MessageDispatcher::dispatchMessage(
		delay + (randomDelay ? Utilities::random(Utilities::randomSeed(), randomDelay) : 0), 
		Object::safeCast(this), 
		Object::safeCast(receiver), 
		message, 
		NULL
	);
}

/**
 * Send message to self
 *
 * @param receiver
 * @param message
 * @param delay
 * @param randomDelay
 */
void Object::sendMessageToSelf(u32 message, u32 delay, u32 randomDelay)
{
	Object::sendMessageTo(this, this, message, delay, randomDelay);
}

/**
 * Discard all delayed messages that I sent
 *
 * @param targetClassGetClassMethod
 */
void Object::discardAllMessages()
{
	MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), Object::safeCast(this));
}

/**
 * Discard a delayed messages that I sent
 *
 * @param message
 */
void Object::discardMessages(u32 message)
{
	MessageDispatcher::discardDelayedMessagesFromSender(MessageDispatcher::getInstance(), Object::safeCast(this), message);
}

/**
 * Get an Object's vTable
 *
 * @return		vTable pointer
 */
const void* Object::getVTable()
{
	return this->vTable;
}

