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

#include <MessageDispatcher.h>
#include <Game.h>
#include <Clock.h>
#include <VirtualList.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;
friend class Telegram;

static MessageDispatcher _messageDispatcher = NULL;

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			MessageDispatcher::getInstance()
 * @memberof	MessageDispatcher
 * @public
 * @return		MessageDispatcher instance
 */


/**
 * Class constructor
 */
 void MessageDispatcher::constructor()
{
	Base::constructor();

	this->delayedMessages = new VirtualList();
	this->helperTelegram = new Telegram(NULL, NULL, 0, NULL);
	this->helperTelegramIsInUse = false;

	_messageDispatcher = this;
}

/**
 * Class destructor
 */
void MessageDispatcher::destructor()
{
	_messageDispatcher = NULL;

	delete this->delayedMessages;
	delete this->helperTelegram;

	// allow a new construct
	Base::destructor();
}

/**
 * Dispatch a message
 *
 * @param delay		milliseconds to wait before dispatching the message
 * @param sender	the object that sends the message
 * @param receiver	the object that receives the message
 * @param message	the actual message code
 * @param extraInfo	pointer to any extra data that must accompany the message
 * @return			a flag indicating the status of the processing of the message
 */
static bool MessageDispatcher::dispatchMessage(uint32 delay, Object sender, Object receiver, int32 message, void* extraInfo)
{
	// make sure the receiver is valid
	ASSERT(sender, "MessageDispatcher::dispatchMessage: null sender");
	ASSERT(receiver, "MessageDispatcher::dispatchMessage: null receiver");

	if(0 >= delay)
	{
		// create the telegram
		bool result = false;

		// Only create a new telegram if the persistent one is in use
		if(_messageDispatcher->helperTelegramIsInUse)
		{
			Telegram telegram = new Telegram(sender, receiver, message, extraInfo);

			// send the telegram to the recipient
			result = Object::handleMessage(receiver, telegram);

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
			result = Object::handleMessage(receiver, _messageDispatcher->helperTelegram);

			_messageDispatcher->helperTelegramIsInUse = false;
		}

		return result;
	}
	else
	{
		MessageDispatcher::dispatchDelayedMessage(MessageDispatcher::getInstance(), Game::getMessagingClock(Game::getInstance()), delay, sender, receiver, message, extraInfo);
	}

	return false;
}

/**
 * Dispatch delayed message
 *
 * @param delay		milliseconds to wait before dispatching the message
 * @param sender	the object that sends the message
 * @param receiver	the object that receives the message
 * @param message	the actual message code
 * @param extraInfo	pointer to any extra data that must accompany the message
 */
void MessageDispatcher::dispatchDelayedMessage(Clock clock, uint32 delay,
 	Object sender, Object receiver, int32 message, void* extraInfo)
{
	// create the telegram
	Telegram telegram = new Telegram(sender, receiver, message, extraInfo);

	DelayedMessage* delayMessage = new DelayedMessage;

	delayMessage->telegram = telegram;
	delayMessage->clock = clock ? clock : Game::getMessagingClock(Game::getInstance());
	delayMessage->timeOfArrival = Clock::getTime(delayMessage->clock) + delay;
	delayMessage->discarded = false;

	VirtualList::pushFront(this->delayedMessages, delayMessage);
}

/**
 * Take care of any discarded message
 */
void MessageDispatcher::processDiscardedMessages()
{
	VirtualNode nextNode = NULL;

	for(VirtualNode node = this->delayedMessages->head; node; node = nextNode)
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
uint32 MessageDispatcher::dispatchDelayedMessages()
{
	ASSERT(this->delayedMessages, "MessageDispatcher::dispatchDelayedMessages: null delayedMessages");

	uint32 messagesDispatched = false;

	VirtualNode nextNode = NULL;

	for(VirtualNode node = this->delayedMessages->head; node; node = nextNode)
	{
		nextNode = node->next;

		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(isDeleted(delayedMessage))
		{
			VirtualList::removeNode(this->delayedMessages, node);
			continue;
		}

		if(delayedMessage->discarded)
		{
			if(!isDeleted(delayedMessage->telegram))
			{
				delete delayedMessage->telegram;
			}

			delete delayedMessage;

			VirtualList::removeNode(this->delayedMessages, node);
			continue;
		}

		ASSERT(Telegram::safeCast(delayedMessage->telegram), "MessageDispatcher::dispatchDelayedMessages: no telegram in queue")

		if(!Clock::isPaused(delayedMessage->clock) && Clock::getTime(delayedMessage->clock) > delayedMessage->timeOfArrival)
		{
			Telegram telegram = delayedMessage->telegram;

			void* sender = Telegram::getSender(telegram);
			void* receiver = Telegram::getReceiver(telegram);

			ASSERT(sender, "MessageDispatcher::dispatchDelayedMessages: null sender");
			ASSERT(receiver, "MessageDispatcher::dispatchDelayedMessages: null receiver");

			// check if sender and receiver are still alive
			if(!isDeleted(sender) && !isDeleted(receiver))
			{
				messagesDispatched |= true;
				Object::handleMessage(receiver, telegram);

				delete delayedMessage->telegram;
				delete delayedMessage;

				VirtualList::removeNode(this->delayedMessages, node);
			}
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
bool MessageDispatcher::discardDelayedMessagesWithClock(Clock clock)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(delayedMessage->clock == clock)
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
bool MessageDispatcher::discardDelayedMessagesFromSender(Object sender, int32 message)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(Telegram::getMessage(telegram) == message && Telegram::getSender(telegram) == sender)
		{
			delayedMessage->discarded = true;
			messagesWereDiscarded |= true;
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
bool MessageDispatcher::discardDelayedMessagesForReceiver(Object receiver, int32 message)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(!isDeleted(delayedMessage) && !isDeleted(telegram) && Telegram::getMessage(telegram) == message && Telegram::getReceiver(telegram) == receiver)
		{
			delayedMessage->discarded = true;
			messagesWereDiscarded |= true;
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
bool MessageDispatcher::discardAllDelayedMessagesFromSender(Object sender)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(!isDeleted(delayedMessage) && !isDeleted(telegram) && Telegram::getSender(telegram) == sender)
		{
			delayedMessage->discarded = true;
			messagesWereDiscarded |= true;
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
bool MessageDispatcher::discardAllDelayedMessagesForReceiver(Object receiver)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(!isDeleted(delayedMessage) && !isDeleted(telegram) && Telegram::getReceiver(telegram) == receiver)
		{
			delayedMessage->discarded = true;
			messagesWereDiscarded |= true;
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
void MessageDispatcher::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "MESSAGE DISPATCHER' STATUS", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Delayed messages:     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->delayedMessages), x + 19, y, NULL);
}
