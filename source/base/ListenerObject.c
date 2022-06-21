/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <VirtualList.h>
#include <Utilities.h>
#include <MessageDispatcher.h>
#include <string.h>
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
void ListenerObject::constructor()
{
	this->events = NULL;
}

/**
 * Class destructor
 */
void ListenerObject::destructor()
{
	ASSERT(!isDeleted(this), "ListenerObject::destructor: already deleted this");

	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; NULL != node; node = node->next)
		{
			delete node->data;
		}

		delete this->events;
		this->events = NULL;
	}

	// free the memory
#ifdef __DEBUG
	MemoryPool::free(_memoryPool, (void*)((uint32)this - __DYNAMIC_STRUCT_PAD));
#else
	*((uint32*)((uint32)this - __DYNAMIC_STRUCT_PAD)) = __MEMORY_FREE_BLOCK_FLAG;
#endif

	this = NULL;
}

/**
 * Handles incoming messages
 *
 * @param telegram	The received message
 * @return			Always returns false, this is meant to be used only in derived classes
 */
bool ListenerObject::handleMessage(Telegram telegram __attribute__ ((unused)))
{
	return false;
}

/**
 * Registers an event listener
 *
 * @param listener		ListenerObject to register event listener at
 * @param method		The method to execute on event
 * @param eventCode		The code of the event to listen to
 */
void ListenerObject::addEventListener(ListenerObject listener, EventListener method, uint32 eventCode)
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

		for(; NULL != node; node = node->next)
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
 * @param listener		ListenerObject where event listener is registered at
 * @param method		The method attached to event listener
 * @param eventCode		The code of the event
 */
void ListenerObject::removeEventListener(ListenerObject listener, EventListener method, uint32 eventCode)
{
	if(this->events)
	{
		VirtualNode node = this->events->head;

		for(; NULL != node; node = node->next)
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
 * @param listener		ListenerObject where event listener is registered at
 * @param eventCode		The code of the event
 */
void ListenerObject::removeEventListeners(EventListener method, uint32 eventCode)
{
	if(this->events)
	{
		for(VirtualNode node = this->events->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Event* event = (Event*)node->data;

			if((NULL == method || method == event->method) && eventCode == event->code)
			{
				VirtualList::removeNode(this->events, node);

				delete event;
			}
		}


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
 * @param listener		ListenerObject where event listener is registered at
 * @param eventCode		The code of the event
 */
void ListenerObject::removeEventListenerScopes(ListenerObject listener, uint32 eventCode)
{
	if(this->events)
	{
		for(VirtualNode node = this->events->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Event* event = (Event*)node->data;

			if(listener == event->listener && eventCode == event->code)
			{
				VirtualList::removeNode(this->events, node);

				delete event;
			}
		}

		if(!this->events->head)
		{
			delete this->events;
			this->events = NULL;
		}
	}
}

/**
 * Removes all event listeners
 *
 */
void ListenerObject::removeAllEventListeners()
{
	if(this->events)
	{
		for(VirtualNode node = this->events->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Event* event = (Event*)node->data;

			VirtualList::removeNode(this->events, node);

			delete event;
		}

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
bool ListenerObject::hasActiveEventListeners()
{
	return !isDeleted(this->events) ? 0 < VirtualList::getSize(this->events) : false;
}

/**
 * Fires an event
 *
 * @param eventCode		The code of the event
 */
void ListenerObject::fireEvent(uint32 eventCode)
{
	if(this->events)
	{
		// temporary lists to being able to modify the event lists while firing them
		VirtualList eventsToFire = new VirtualList();

		for(VirtualNode node = this->events->head, nextNode; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Event* event = (Event*)node->data;

			// safety check in case that the there is a stacking up of firings within firings
			if(isDeleted(event) || isDeleted(event->listener))
			{
				VirtualList::removeNode(this->events, node);

				// safety check in case that the there is a stacking up of firings within firings
				if(!isDeleted(event))
				{
					delete event;
				}
			}
			else if(eventCode == event->code)
			{
				VirtualList::pushBack(eventsToFire, event);
			}
		}

		for(VirtualNode node = eventsToFire->head; NULL != node; node = node->next)
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
				Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 13, 12, NULL);
				Printing::text(Printing::getInstance(), "Method:    ", 1, 13, NULL);
				Printing::hex(Printing::getInstance(), (int32)event->method, 13, 13, 8, NULL);
				Printing::text(Printing::getInstance(), "Event code: ", 1, 14, NULL);
				Printing::int32(Printing::getInstance(), event->code, 13, 14, NULL);
				NM_ASSERT(!isDeleted(this), "ListenerObject::fireEvent: deleted during event listening");
#endif
				break;
			}
		}

		delete eventsToFire;
	}
}

/**
 * Send message to object
 *
 * @param receiver
 * @param message
 * @param delay
 * @param randomDelay
 */
void ListenerObject::sendMessageTo(ListenerObject receiver, uint32 message, uint32 delay, uint32 randomDelay)
{
	MessageDispatcher::dispatchMessage(
		delay + (randomDelay ? Utilities::random(Utilities::randomSeed(), randomDelay) : 0), 
		ListenerObject::safeCast(this), 
		ListenerObject::safeCast(receiver), 
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
void ListenerObject::sendMessageToSelf(uint32 message, uint32 delay, uint32 randomDelay)
{
	ListenerObject::sendMessageTo(this, this, message, delay, randomDelay);
}

/**
 * Discard all delayed messages that I sent
 *
 * @param targetClassGetClassMethod
 */
void ListenerObject::discardAllMessages()
{
	MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), ListenerObject::safeCast(this));
}

/**
 * Discard a delayed messages that I sent
 *
 * @param message
 */
void ListenerObject::discardMessages(uint32 message)
{
	MessageDispatcher::discardDelayedMessagesFromSender(MessageDispatcher::getInstance(), ListenerObject::safeCast(this), message);
}
