/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <MessageDispatcher.h>
#include <Printer.h>
#include <Telegram.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "ListenerObject.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::constructor()
{
#ifndef __RELEASE	
	// Always explicitly call the base's constructor 
	Base::constructor();
#endif

	this->events = NULL;
	this->eventFirings = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::destructor()
{	
	ASSERT(!isDeleted(this), "ListenerObject::destructor: already deleted this");
	NM_ASSERT(0 == this->eventFirings, "ListenerObject::destructor: called during event firing");

	MessageDispatcher::discardAllDelayedMessages(ListenerObject::safeCast(this));
	ListenerObject::removeAllEventListeners(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::addEventListener(ListenerObject listener, uint16 eventCode)
{
	// Don't remove these asserts!
	NM_ASSERT(!isDeleted(listener), "ListenerObject::addEventListener: invalid listener");

	if(NULL == listener)
	{
		return;
	}

	NM_ASSERT(NULL != __GET_CAST(ListenerObject, listener), "ListenerObject::addEventListener: wrong listener object type");

	if(NULL == this->events)
	{
		this->events = new VirtualList();
	}
	else
	{
		for(VirtualNode node = this->events->head; NULL != node; node = node->next)
		{
			Event* event = (Event*)node->data;

			if(listener == event->listener && eventCode == event->code)
			{
				event->remove = false;
				return;
			}
		}
	}

	Event* event = new Event;
	event->listener = listener;
	event->code = eventCode;
	event->remove = false;

	VirtualList::pushBack(this->events, event);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::removeEventListener(ListenerObject listener, uint16 eventCode)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::removeEventListeners(uint16 eventCode)
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

			if(eventCode == event->code)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ListenerObject::hasActiveEventListeners()
{
	return !isDeleted(this->events) ? NULL != VirtualList::begin(this->events) : false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

			// Safety check in case that the there is a stacking up of firings within firings
			if(isDeleted(event) || isDeleted(event->listener) || event->remove)
			{
				if(1 == this->eventFirings)
				{
					VirtualList::removeNode(this->events, node);

					// Safety check in case that the there is a stacking up of firings within firings
					if(!isDeleted(event))
					{
						delete event;
					}
				}
			}
			else if(eventCode == event->code)
			{
				event->remove = !ListenerObject::onEvent(event->listener, this, event->code);

				// Safe check in case that I have been deleted during the previous event
				if(isDeleted(this))
				{
#ifndef __RELEASE
					Printer::setDebugMode();
					Printer::clear();
					Printer::text("Class:    ", 1, 12, NULL);
					Printer::text(__GET_CLASS_NAME(this), 13, 12, NULL);
					Printer::text("Event code: ", 1, 14, NULL);
					Printer::int32(event->code, 13, 14, NULL);
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::sendMessageToSelf(uint32 message, uint32 delay, uint32 randomDelay)
{
	ListenerObject::sendMessageTo(this, this, message, delay, randomDelay);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::discardAllMessages()
{
	MessageDispatcher::discardAllDelayedMessagesFromSender(ListenerObject::safeCast(this));
	MessageDispatcher::discardAllDelayedMessagesForReceiver(ListenerObject::safeCast(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::discardMessages(uint32 message)
{
	MessageDispatcher::discardDelayedMessagesFromSender(ListenerObject::safeCast(this), message);
	MessageDispatcher::discardDelayedMessagesForReceiver(ListenerObject::safeCast(this), message);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ListenerObject::onEvent(ListenerObject eventFirer __attribute__((unused)), uint16 eventCode __attribute__((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ListenerObject::handleMessage(Telegram telegram __attribute__ ((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
