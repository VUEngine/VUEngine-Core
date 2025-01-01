/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <MessageDispatcher.h>
#include <Printing.h>
#include <Telegram.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "ListenerObject.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::constructor()
{
#ifndef __RELEASE	
	// Always explicitly call the base's constructor 
	Base::constructor();
#endif

	this->events = NULL;
	this->eventFirings = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::destructor()
{	
	ASSERT(!isDeleted(this), "ListenerObject::destructor: already deleted this");
	NM_ASSERT(0 == this->eventFirings, "ListenerObject::destructor: called during event firing");

	MessageDispatcher::discardAllDelayedMessages(MessageDispatcher::getInstance(), ListenerObject::safeCast(this));
	ListenerObject::removeAllEventListeners(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::addEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	if(NULL == listener || NULL == callback)
	{
		return;
	}

	if(NULL == this->events)
	{
		this->events = new VirtualList();
	}
	else
	{
		for(VirtualNode node = this->events->head; NULL != node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(listener == event->listener && callback == event->callback && eventCode == event->code)
			{
				event->remove = false;
				return;
			}
		}
	}

	Event* event = new Event;
	event->listener = listener;
	event->callback = callback;
	event->code = eventCode;
	event->remove = false;

	VirtualList::pushBack(this->events, event);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::removeEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	if(NULL != this->events)
	{
		HardwareManager::suspendInterrupts();

		for(VirtualNode node = this->events->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Event* event = (Event*)node->data;

			if(isDeleted(event))
			{
				if(0 == this->eventFirings)
				{
					VirtualList::removeNode(this->events, node);

					continue;
				}

			}

			if(listener == event->listener && callback == event->callback && eventCode == event->code)
			{
				if(0 < this->eventFirings)
				{
					event->remove = true;
				}
				else
				{
					VirtualList::removeNode(this->events, node);
					delete event;
				}
			}
		}

		if(NULL != this->events && NULL == this->events->head)
		{
			delete this->events;
			this->events = NULL;
		}

		HardwareManager::resumeInterrupts();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::removeEventListeners(EventListener callback, uint16 eventCode)
{
	if(NULL != this->events)
	{
		HardwareManager::suspendInterrupts();

		for(VirtualNode node = this->events->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Event* event = (Event*)node->data;

			if(isDeleted(event))
			{
				if(0 == this->eventFirings)
				{
					VirtualList::removeNode(this->events, node);

					continue;
				}

			}

			if((NULL == callback || callback == event->callback) && eventCode == event->code)
			{
				if(0 < this->eventFirings)
				{
					event->remove = true;
				}
				else
				{
					VirtualList::removeNode(this->events, node);
					delete event;
				}
			}
		}

		if(NULL != this->events && NULL == this->events->head)
		{
			delete this->events;
			this->events = NULL;
		}

		HardwareManager::resumeInterrupts();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::removeEventListenerScopes(ListenerObject listener, uint16 eventCode)
{
	if(NULL != this->events)
	{
		HardwareManager::suspendInterrupts();

		for(VirtualNode node = this->events->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Event* event = (Event*)node->data;

			if(isDeleted(event))
			{
				if(0 == this->eventFirings)
				{
					VirtualList::removeNode(this->events, node);
				}

				continue;
			}

			if(listener == event->listener && eventCode == event->code)
			{
				if(0 < this->eventFirings)
				{
					event->remove = true;
				}
				else
				{
					VirtualList::removeNode(this->events, node);
					delete event;
				}
			}
		}

		if(NULL != this->events && NULL == this->events->head)
		{
			delete this->events;
			this->events = NULL;
		}

		HardwareManager::resumeInterrupts();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::removeAllEventListeners()
{
	if(NULL != this->events)
	{
		HardwareManager::suspendInterrupts();

		if(0 == this->eventFirings)
		{
			VirtualList::deleteData(this->events);
			delete this->events;
			this->events = NULL;
		}
		else
		{
			for(VirtualNode node = this->events->head, nextNode = NULL; NULL != node; node = nextNode)
			{
				nextNode = node->next;

				Event* event = (Event*)node->data;

				if(!isDeleted(event))
				{
					event->remove = true;
				}
			}			
		}

		HardwareManager::resumeInterrupts();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ListenerObject::hasActiveEventListeners()
{
	return !isDeleted(this->events) ? NULL != VirtualList::begin(this->events) : false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::fireEvent(uint16 eventCode)
{
	if(NULL != this->events)
	{
		this->eventFirings++;

		HardwareManager::suspendInterrupts();

		for(VirtualNode node = this->events->head, nextNode; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Event* event = (Event*)node->data;

			// safety check in case that the there is a stacking up of firings within firings
			if(isDeleted(event) || isDeleted(event->listener) || event->remove)
			{
				if(1 == this->eventFirings)
				{
					VirtualList::removeNode(this->events, node);

					// safety check in case that the there is a stacking up of firings within firings
					if(!isDeleted(event))
					{
						delete event;
					}
				}
			}
			else if(eventCode == event->code)
			{
				event->remove = !event->callback(event->listener, this);

				// safe check in case that I have been deleted during the previous event
				if(isDeleted(this))
				{
#ifndef __RELEASE
					Printing::setDebugMode(Printing::getInstance());
					Printing::clear(Printing::getInstance());
					Printing::text(Printing::getInstance(), "Class:    ", 1, 12, NULL);
					Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this), 13, 12, NULL);
					Printing::text(Printing::getInstance(), "Method:    ", 1, 13, NULL);
					Printing::hex(Printing::getInstance(), (int32)event->callback, 13, 13, 8, NULL);
					Printing::text(Printing::getInstance(), "Event code: ", 1, 14, NULL);
					Printing::int32(Printing::getInstance(), event->code, 13, 14, NULL);
					NM_ASSERT(!isDeleted(this), "ListenerObject::fireEvent: deleted during event listening");
#endif
					break;
				}
			}
		}
		
		if(NULL != this->events && NULL == this->events->head)
		{
			delete this->events;
			this->events = NULL;
		}

		HardwareManager::resumeInterrupts();

		this->eventFirings--;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::sendMessageTo(ListenerObject receiver, uint32 message, uint32 delay, uint32 randomDelay)
{
	MessageDispatcher::dispatchMessage
	(
		delay + (randomDelay ? Math::random(Math::randomSeed(), randomDelay) : 0), 
		ListenerObject::safeCast(this), 
		ListenerObject::safeCast(receiver), 
		message, 
		NULL
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::sendMessageToSelf(uint32 message, uint32 delay, uint32 randomDelay)
{
	ListenerObject::sendMessageTo(this, this, message, delay, randomDelay);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::discardAllMessages()
{
	MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), ListenerObject::safeCast(this));
	MessageDispatcher::discardAllDelayedMessagesForReceiver(MessageDispatcher::getInstance(), ListenerObject::safeCast(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::discardMessages(uint32 message)
{
	MessageDispatcher::discardDelayedMessagesFromSender(MessageDispatcher::getInstance(), ListenerObject::safeCast(this), message);
	MessageDispatcher::discardDelayedMessagesForReceiver(MessageDispatcher::getInstance(), ListenerObject::safeCast(this), message);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ListenerObject::handleMessage(Telegram telegram __attribute__ ((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

