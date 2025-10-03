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

	if(!isDeleted(this->events))
	{
		VirtualList events = this->events;
		this->events = NULL;
		VirtualList::deleteData(events);
		delete events;
	}

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

	HardwareManager::suspendInterrupts();

	VirtualNode node = NULL;

	if(NULL == this->events)
	{
		this->events = new VirtualList();
	}
	else
	{
		node = this->events->head;
		
		for(VirtualNode nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			Event* event = (Event*)node->data;

			if(event->remove || (listener == event->listener && eventCode == event->code))
			{
				event->listener = listener;
				event->code = eventCode;
				event->remove = false;
				break;
			}
		}
	}

	if(NULL == node)
	{
		Event* event = new Event;
		event->listener = listener;
		event->code = eventCode;
		event->remove = false;

		VirtualList::pushBack(this->events, event);		
	}

	HardwareManager::resumeInterrupts();
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

			if
			(
				NULL == event 
				|| 
				event->remove 
				|| 
				(
					(NULL == listener || listener == event->listener) 
					&& 
					(kEventEngineFirst == eventCode || eventCode == event->code)
				)
			)
			{
				event->remove = true;
				
				if(0 == this->eventFirings)
				{
					VirtualList::removeNode(this->events, node);

					if(!isDeleted(event))
					{
						delete event;
					}

					continue;
				}
			}
		}

        if(NULL == this->events->head && 0 == this->eventFirings)
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
	ListenerObject::removeEventListener(this, NULL, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ListenerObject::removeAllEventListeners()
{
	ListenerObject::removeEventListener(this, NULL, kEventEngineFirst);
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
		HardwareManager::suspendInterrupts();

		this->eventFirings++;

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

					if(!isDeleted(event))
					{
						delete event;
					}
				}
			}
			else if(eventCode == event->code)
			{
				event->remove = !ListenerObject::onEvent(event->listener, this, event->code);

				if(1 == this->eventFirings)
				{
					if(isDeleted(event) || isDeleted(event->listener) || event->remove)
					{
						VirtualList::removeNode(this->events, node);

						// Safety check in case that the there is a stacking up of firings within firings
						if(!isDeleted(event))
						{
							delete event;
						}
					}
				}

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
			VirtualList events = this->events;
			this->events = NULL;
			delete events;
		}

		this->eventFirings--;

        if(NULL != this->events && NULL == this->events->head && 0 == this->eventFirings)
        {
            delete this->events;
            this->events = NULL;
        }
		
		HardwareManager::resumeInterrupts();
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
