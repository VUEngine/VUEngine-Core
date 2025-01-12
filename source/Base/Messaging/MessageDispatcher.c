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

#include <Clock.h>
#include <Printing.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "MessageDispatcher.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;
friend class Telegram;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool MessageDispatcher::dispatchMessage
(
	uint32 delay, ListenerObject sender, ListenerObject receiver, int32 message, void* extraInfo
)
{
	MessageDispatcher messageDispatcher = MessageDispatcher::getInstance();

	// Make sure the receiver is valid
	ASSERT(sender, "MessageDispatcher::dispatchMessage: null sender");

	if(isDeleted(receiver))
	{
		return false;
	}

	if(0 >= delay)
	{
		// Create the telegram
		bool result = false;

		// Only create a new telegram if the persistent one is in use
		if(messageDispatcher->helperTelegramIsInUse)
		{
			Telegram telegram = new Telegram(sender, receiver, message, extraInfo);

			// Send the telegram to the recipient
			result = ListenerObject::handleMessage(receiver, telegram);

			delete telegram;
		}
		else
		{
			messageDispatcher->helperTelegram->sender = sender;
			messageDispatcher->helperTelegram->receiver = receiver;
			messageDispatcher->helperTelegram->message = message;
			messageDispatcher->helperTelegram->extraInfo = extraInfo;

			messageDispatcher->helperTelegramIsInUse = true;

			// Send the telegram to the recipient
			result = ListenerObject::handleMessage(receiver, messageDispatcher->helperTelegram);

			messageDispatcher->helperTelegramIsInUse = false;
		}

		return result;
	}
	else
	{
		MessageDispatcher::dispatchDelayedMessage
		(
			VUEngine::getMessagingClock(), delay, sender, receiver, message, extraInfo
		);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void MessageDispatcher::dispatchDelayedMessage
(
	Clock clock, uint32 delay, ListenerObject sender, ListenerObject receiver, int32 message, void* extraInfo
)
{
	MessageDispatcher messageDispatcher = MessageDispatcher::getInstance();

	// Create the telegram
	DelayedMessage* delayedMessage = new DelayedMessage;

	delayedMessage->telegram = new Telegram(sender, receiver, message, extraInfo);
	delayedMessage->clock = clock ? clock : VUEngine::getMessagingClock();
	delayedMessage->timeOfArrival = Clock::getMilliseconds(delayedMessage->clock) + delay;
	delayedMessage->discarded = false;

	VirtualList::pushBack(messageDispatcher->delayedMessages, delayedMessage);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool MessageDispatcher::discardDelayedMessagesFromSender(ListenerObject sender, int32 message)
{
	MessageDispatcher messageDispatcher = MessageDispatcher::getInstance();
	
	bool messagesWereDiscarded = false;
	VirtualNode node = messageDispatcher->delayedMessages->head;

	for(; NULL != node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(!isDeleted(delayedMessage))
		{
			Telegram telegram = delayedMessage->telegram;

			if(isDeleted(telegram) || (Telegram::getMessage(telegram) == message && Telegram::getSender(telegram) == sender))
			{
				delayedMessage->discarded = true;
				messagesWereDiscarded |= true;
			}
		}
	}

	return messagesWereDiscarded;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool MessageDispatcher::discardDelayedMessagesForReceiver(ListenerObject receiver, int32 message)
{
	MessageDispatcher messageDispatcher = MessageDispatcher::getInstance();
	
	bool messagesWereDiscarded = false;
	VirtualNode node = messageDispatcher->delayedMessages->head;

	for(; NULL != node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(!isDeleted(delayedMessage))
		{
			Telegram telegram = delayedMessage->telegram;

			if(isDeleted(telegram) || (Telegram::getMessage(telegram) == message && Telegram::getReceiver(telegram) == receiver))
			{
				delayedMessage->discarded = true;
				messagesWereDiscarded |= true;
			}
		}
	}

	return messagesWereDiscarded;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool MessageDispatcher::discardAllDelayedMessagesFromSender(ListenerObject sender)
{
	MessageDispatcher messageDispatcher = MessageDispatcher::getInstance();

	bool messagesWereDiscarded = false;
	VirtualNode node = messageDispatcher->delayedMessages->head;

	for(; NULL != node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(!isDeleted(delayedMessage))
		{
			Telegram telegram = delayedMessage->telegram;

			if(isDeleted(telegram) || telegram->sender == sender)
			{
				delayedMessage->discarded = true;
				messagesWereDiscarded |= true;
			}
		}
	}

	return messagesWereDiscarded;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool MessageDispatcher::discardAllDelayedMessagesForReceiver(ListenerObject receiver)
{
	MessageDispatcher messageDispatcher = MessageDispatcher::getInstance();

	bool messagesWereDiscarded = false;
	VirtualNode node = messageDispatcher->delayedMessages->head;

	for(; NULL != node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(!isDeleted(delayedMessage))
		{
			Telegram telegram = delayedMessage->telegram;

			if(isDeleted(telegram) || telegram->receiver == receiver)
			{
				delayedMessage->discarded = true;
				messagesWereDiscarded |= true;
			}
		}
	}

	return messagesWereDiscarded;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool MessageDispatcher::discardAllDelayedMessages(ListenerObject listenerObject)
{
	MessageDispatcher messageDispatcher = MessageDispatcher::getInstance();

	bool messagesWereDiscarded = false;
	VirtualNode node = messageDispatcher->delayedMessages->head;

	for(; NULL != node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(!isDeleted(delayedMessage))
		{
			Telegram telegram = delayedMessage->telegram;

			if(isDeleted(telegram) || telegram->receiver == listenerObject || telegram->sender == listenerObject)
			{
				delayedMessage->discarded = true;
				messagesWereDiscarded |= true;
			}
		}
	}

	return messagesWereDiscarded;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __RELEASE
static void MessageDispatcher::print(int32 x, int32 y)
{
	MessageDispatcher messageDispatcher = MessageDispatcher::getInstance();

	Printing::text("MESSAGE DISPATCHER' STATUS", x, y++, NULL);
	Printing::text("Delayed messages:     ", x, ++y, NULL);
	Printing::int32(VirtualList::getCount(messageDispatcher->delayedMessages), x + 19, y, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void MessageDispatcher::printAllDelayedMessagesFromSender(ListenerObject sender, int16 x, int16 y)
{
	MessageDispatcher messageDispatcher = MessageDispatcher::getInstance();	

	for(VirtualNode node = messageDispatcher->delayedMessages->head; NULL != node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(!isDeleted(delayedMessage))
		{
			Telegram telegram = delayedMessage->telegram;

			if(Telegram::getSender(telegram) == sender)
			{
				PRINT_INT(telegram->message, x, y);
				PRINT_TEXT(__GET_CLASS_NAME(telegram->sender), x + 4, y);
				PRINT_TEXT(__GET_CLASS_NAME(telegram->receiver), x + 15, y++);

				if(27 < y)
				{
					x = 24;
					y = 1;
				}
			}
		}
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure bool MessageDispatcher::dispatchDelayedMessages()
{
	bool messagesDispatched = false;

	for(VirtualNode node = this->delayedMessages->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(isDeleted(delayedMessage))
		{	
			VirtualList::removeNode(this->delayedMessages, node);

			continue;	
		}
		else if
		(
			!delayedMessage->discarded 
			&& 
			!Clock::isPaused(delayedMessage->clock) 
			&& 
			Clock::getMilliseconds(delayedMessage->clock) > delayedMessage->timeOfArrival
		)
		{
			Telegram telegram = delayedMessage->telegram;

			if(!isDeleted(telegram))
			{
				void* sender = Telegram::getSender(telegram);
				void* receiver = Telegram::getReceiver(telegram);

				// Check if sender and receiver are still alive
				if(!isDeleted(sender) && !isDeleted(receiver))
				{
					messagesDispatched |= true;
					HardwareManager::suspendInterrupts();
					ListenerObject::handleMessage(receiver, telegram);
					HardwareManager::resumeInterrupts();
				}
#ifndef __RELEASE
				else if(isDeleted(sender) || isDeleted(receiver))
				{
					Printing::setDebugMode();
					Printing::clear();
					PRINT_TEXT("Message: ", 1, 16);
					PRINT_INT(Telegram::getMessage(telegram), 10, 16);

					if(!isDeleted(sender))
					{
						PRINT_TEXT("Sender: ", 1, 15);
						PRINT_TEXT(__GET_CLASS_NAME(sender), 10, 15);
					}

					if(!isDeleted(receiver))
					{
						PRINT_TEXT("Receiver: ", 1, 15);
						PRINT_TEXT(__GET_CLASS_NAME(receiver), 10, 15);
					}

					NM_ASSERT(!isDeleted(receiver), "MessageDispatcher::dispatchDelayedMessages: null receiver");
					NM_ASSERT(!isDeleted(sender), "MessageDispatcher::dispatchDelayedMessages: null sender");
				}
#endif			
			}

			delayedMessage->discarded = true;
		}

		if(delayedMessage->discarded)
		{
			if(!isDeleted(delayedMessage->telegram))
			{
				delete delayedMessage->telegram;
			}
	
			VirtualList::removeNode(this->delayedMessages, node);	

			delete delayedMessage;
		}
	}

	return messagesDispatched;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void MessageDispatcher::processDiscardedMessages()
{
	for(VirtualNode node = this->delayedMessages->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(!delayedMessage->discarded)
		{
			continue;
		}

		Telegram telegram = delayedMessage->telegram;
		ASSERT(telegram, "MessageDispatcher::processDiscardedMessages: null telegram");
		ASSERT(delayedMessage, "MessageDispatcher::processDiscardedMessages: null delayedMessage");

		VirtualList::removeNode(this->delayedMessages, node);

		if(!isDeleted(delayedMessage))
		{
			delete delayedMessage;
		}

		if(!isDeleted(telegram))
		{
			delete telegram;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure bool MessageDispatcher::discardDelayedMessagesWithClock(Clock clock)
{	
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; NULL != node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(!isDeleted(delayedMessage) && delayedMessage->clock == clock)
		{
			delayedMessage->discarded = true;
			messagesWereDiscarded |= true;
		}
	}

	return messagesWereDiscarded;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MessageDispatcher::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->delayedMessages = new VirtualList();
	this->helperTelegram = new Telegram(NULL, NULL, 0, NULL);
	this->helperTelegramIsInUse = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void MessageDispatcher::destructor()
{
	delete this->delayedMessages;
	delete this->helperTelegram;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}
