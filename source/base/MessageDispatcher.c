/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Clock.h>
#include <Printing.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "MessageDispatcher.h"


//=========================================================================================================
// CLASS'S ATTRIBUTES
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;
friend class Telegram;

static MessageDispatcher _messageDispatcher = NULL;


//=========================================================================================================
// CLASS'S STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static bool MessageDispatcher::dispatchMessage(uint32 delay, ListenerObject sender, ListenerObject receiver, int32 message, void* extraInfo)
{
	// make sure the receiver is valid
	ASSERT(sender, "MessageDispatcher::dispatchMessage: null sender");

	if(isDeleted(receiver))
	{
		return false;
	}

	if(0 >= delay)
	{
		// create the telegram
		bool result = false;

		// Only create a new telegram if the persistent one is in use
		if(_messageDispatcher->helperTelegramIsInUse)
		{
			Telegram telegram = new Telegram(sender, receiver, message, extraInfo);

			// send the telegram to the recipient
			result = ListenerObject::handleMessage(receiver, telegram);

			delete telegram;
		}
		else
		{
			_messageDispatcher->helperTelegram->sender = sender;
			_messageDispatcher->helperTelegram->receiver = receiver;
			_messageDispatcher->helperTelegram->message = message;
			_messageDispatcher->helperTelegram->extraInfo = extraInfo;

			_messageDispatcher->helperTelegramIsInUse = true;

			// send the telegram to the recipient
			result = ListenerObject::handleMessage(receiver, _messageDispatcher->helperTelegram);

			_messageDispatcher->helperTelegramIsInUse = false;
		}

		return result;
	}
	else
	{
		MessageDispatcher::dispatchDelayedMessage(MessageDispatcher::getInstance(), VUEngine::getMessagingClock(_vuEngine), delay, sender, receiver, message, extraInfo);
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS'S PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void MessageDispatcher::constructor()
{
	Base::constructor();

	this->delayedMessages = new VirtualList();
	this->helperTelegram = new Telegram(NULL, NULL, 0, NULL);
	this->helperTelegramIsInUse = false;

	_messageDispatcher = this;
}
//---------------------------------------------------------------------------------------------------------
void MessageDispatcher::destructor()
{
	_messageDispatcher = NULL;

	delete this->delayedMessages;
	delete this->helperTelegram;

	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void MessageDispatcher::dispatchDelayedMessage(Clock clock, uint32 delay,
 	ListenerObject sender, ListenerObject receiver, int32 message, void* extraInfo)
{
	// create the telegram
	DelayedMessage* delayedMessage = new DelayedMessage;

	delayedMessage->telegram = new Telegram(sender, receiver, message, extraInfo);
	delayedMessage->clock = clock ? clock : VUEngine::getMessagingClock(_vuEngine);
	delayedMessage->timeOfArrival = Clock::getMilliseconds(delayedMessage->clock) + delay;
	delayedMessage->discarded = false;

	VirtualList::pushBack(this->delayedMessages, delayedMessage);
}

/**
 * Take care of any discarded message
 */
//---------------------------------------------------------------------------------------------------------
void MessageDispatcher::processDiscardedMessages()
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

/**
 * Dispatch the delayed messages whose delay has expired
 */
//---------------------------------------------------------------------------------------------------------
bool MessageDispatcher::dispatchDelayedMessages()
{
	ASSERT(this->delayedMessages, "MessageDispatcher::dispatchDelayedMessages: null delayedMessages");

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
		else if(!delayedMessage->discarded && !Clock::isPaused(delayedMessage->clock) && Clock::getMilliseconds(delayedMessage->clock) > delayedMessage->timeOfArrival)
		{
			Telegram telegram = delayedMessage->telegram;

			if(!isDeleted(telegram))
			{
				void* sender = Telegram::getSender(telegram);
				void* receiver = Telegram::getReceiver(telegram);

				// check if sender and receiver are still alive
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
					Printing::setDebugMode(Printing::getInstance());
					Printing::clear(Printing::getInstance());
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

/**
 * Discarded delayed messages associated to the given clock
 *
 * @private
 * @param clock		the clock against which the message's delay is measured
 */
//---------------------------------------------------------------------------------------------------------
bool MessageDispatcher::discardDelayedMessagesWithClock(Clock clock)
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

/**
 * Discarded delayed messages sent by an object
 *
 * @private
 * @param sender	the object that originally sent the message
 * @param message	the actual message code
 */
//---------------------------------------------------------------------------------------------------------
bool MessageDispatcher::discardDelayedMessagesFromSender(ListenerObject sender, int32 message)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

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

/**
 * Discarded delayed messages sent to an object
 *
 * @private
 * @param sender	the object that the message was originally sent to
 * @param message	the actual message code
 */
//---------------------------------------------------------------------------------------------------------
bool MessageDispatcher::discardDelayedMessagesForReceiver(ListenerObject receiver, int32 message)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

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

/**
 * Discarded all delayed messages sent by an object
 *
 * @private
 * @param sender	the object that originally sent the message
 */
//---------------------------------------------------------------------------------------------------------
bool MessageDispatcher::discardAllDelayedMessagesFromSender(ListenerObject sender)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

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

/**
 * Print all delayed messages sent by an object
 *
 * @private
 * @param sender	the object that originally sent the message
 */
//---------------------------------------------------------------------------------------------------------
#ifndef __SHIPPING
void MessageDispatcher::printAllDelayedMessagesFromSender(ListenerObject sender, int16 x, int16 y)
{
	for(VirtualNode node = this->delayedMessages->head; NULL != node; node = node->next)
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

/**
 * Discarded all delayed messages sent to an object
 *
 * @private
 * @param sender	the object that the message was originally sent to
 */
//---------------------------------------------------------------------------------------------------------
bool MessageDispatcher::discardAllDelayedMessagesForReceiver(ListenerObject receiver)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

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


/**
 * Discarded all delayed messages sent to an object
 *
 * @private
 * @param sender	the object that the message was originally sent to
 */
//---------------------------------------------------------------------------------------------------------
bool MessageDispatcher::discardAllDelayedMessages(ListenerObject listenerObject)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

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

/**
 * Print status
 *
 * @param x			x screen coordinate
 * @param y			y screen coordinate
 */
//---------------------------------------------------------------------------------------------------------
#ifndef __RELEASE
void MessageDispatcher::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "MESSAGE DISPATCHER' STATUS", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Delayed messages:     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->delayedMessages), x + 19, y, NULL);
}
#endif
//---------------------------------------------------------------------------------------------------------
