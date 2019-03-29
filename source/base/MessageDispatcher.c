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
	this->delayedMessagesToDiscard = new VirtualList();
	this->delayedMessagesToDispatch = new VirtualList();
}

/**
 * Class destructor
 */
void MessageDispatcher::destructor()
{
	delete this->delayedMessages;
	delete this->delayedMessagesToDiscard;

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
static bool MessageDispatcher::dispatchMessage(u32 delay, Object sender, Object receiver, int message, void* extraInfo)
{
	// make sure the receiver is valid
	ASSERT(sender, "MessageDispatcher::dispatchMessage: null sender");
	ASSERT(receiver, "MessageDispatcher::dispatchMessage: null receiver");

	if(0 >= delay)
	{
		// create the telegram
		Telegram telegram = new Telegram(sender, receiver, message, extraInfo);

		// send the telegram to the recipient
		bool result =  Object::handleMessage(receiver, telegram);

		delete telegram;
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
void MessageDispatcher::dispatchDelayedMessage(Clock clock, u32 delay,
 	Object sender, Object receiver, int message, void* extraInfo)
{
	// create the telegram
	Telegram telegram = new Telegram(sender, receiver, message, extraInfo);

	DelayedMessage* delayMessage = new DelayedMessage;

	delayMessage->telegram = telegram;
	delayMessage->clock = clock ? clock : Game::getMessagingClock(Game::getInstance());
	delayMessage->timeOfArrival = Clock::getTime(delayMessage->clock) + delay;

	VirtualList::pushBack(this->delayedMessages, delayMessage);
}

/**
 * Take care of any discarded message
 */
void MessageDispatcher::processDiscardedMessages()
{
	ASSERT(this->delayedMessagesToDiscard, "MessageDispatcher::processDiscardedMessages: null delayedMessagesToDiscard");

	if(this->delayedMessagesToDiscard->head)
	{
		VirtualNode node = this->delayedMessagesToDiscard->head;

		for(; node; node = node->next)
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
			Telegram telegram = delayedMessage->telegram;
			ASSERT(telegram, "MessageDispatcher::processDiscardedMessages: null telegram");
			ASSERT(delayedMessage, "MessageDispatcher::processDiscardedMessages: null delayedMessage");

			VirtualList::removeElement(this->delayedMessages, delayedMessage);

			if(!isDeleted(delayedMessage))
			{
				delete delayedMessage;
			}

			if(!isDeleted(telegram))
			{
				delete telegram;
			}
		}

		VirtualList::clear(this->delayedMessagesToDiscard);
	}

}

/**
 * Dispatch the delayed messages whose delay has expired
 */
u32 MessageDispatcher::dispatchDelayedMessages()
{
	ASSERT(this->delayedMessages, "MessageDispatcher::dispatchDelayedMessages: null delayedMessages");

	u32 messagesDispatched = false;

	MessageDispatcher::processDiscardedMessages(this);

	if(this->delayedMessages->head)
	{
		VirtualNode node = this->delayedMessages->head;

		for(; node; node = node->next)
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

			ASSERT(Telegram::safeCast(delayedMessage->telegram), "MessageDispatcher::dispatchDelayedMessages: no telegram in queue")

			if(!Clock::isPaused(delayedMessage->clock) && Clock::getTime(delayedMessage->clock) > delayedMessage->timeOfArrival)
			{
				Telegram telegram = delayedMessage->telegram;

				void* sender = Telegram::getSender(telegram);
				void* receiver = Telegram::getReceiver(telegram);

				ASSERT(sender, "MessageDispatcher::dispatchDelayedMessages: null sender");
				ASSERT(receiver, "MessageDispatcher::dispatchDelayedMessages: null receiver");

				// check if sender and receiver are still alive
				if(!VirtualList::find(this->delayedMessagesToDiscard, delayedMessage) && !isDeleted(sender) && !isDeleted(receiver))
				{
					messagesDispatched |= true;
					Object::handleMessage(receiver, telegram);
				}

				VirtualList::removeElement(this->delayedMessages, delayedMessage);

				if(VirtualList::find(this->delayedMessagesToDiscard, delayedMessage))
				{
					VirtualList::removeElement(this->delayedMessagesToDiscard, delayedMessage);
				}

				if(!isDeleted(telegram))
				{
					delete telegram;
				}

				if(!isDeleted(delayedMessage))
				{
					delete delayedMessage;
				}

				break;
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

		if(delayedMessage->clock == clock && !VirtualList::find(this->delayedMessagesToDiscard, delayedMessage))
		{
			VirtualList::pushBack(this->delayedMessagesToDiscard, delayedMessage);
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
bool MessageDispatcher::discardDelayedMessagesFromSender(Object sender, int message)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(Telegram::getMessage(telegram) == message && Telegram::getSender(telegram) == sender && !VirtualList::find(this->delayedMessagesToDiscard, delayedMessage))
		{
			VirtualList::pushBack(this->delayedMessagesToDiscard, delayedMessage);
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
bool MessageDispatcher::discardDelayedMessagesForReceiver(Object receiver, int message)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(!isDeleted(delayedMessage) && !isDeleted(telegram) && Telegram::getMessage(telegram) == message && Telegram::getReceiver(telegram) == receiver && !VirtualList::find(this->delayedMessagesToDiscard, delayedMessage))
		{
			if(!VirtualList::find(this->delayedMessagesToDiscard, delayedMessage))
			{
				VirtualList::pushBack(this->delayedMessagesToDiscard, delayedMessage);
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
bool MessageDispatcher::discardAllDelayedMessagesFromSender(Object sender)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(!isDeleted(delayedMessage) && !isDeleted(telegram) && Telegram::getSender(telegram) == sender && !VirtualList::find(this->delayedMessagesToDiscard, delayedMessage))
		{
			if(!VirtualList::find(this->delayedMessagesToDiscard, delayedMessage))
			{
				VirtualList::pushBack(this->delayedMessagesToDiscard, delayedMessage);
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
bool MessageDispatcher::discardAllDelayedMessagesForReceiver(Object receiver)
{
	bool messagesWereDiscarded = false;
	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(!isDeleted(delayedMessage) && !isDeleted(telegram) && Telegram::getReceiver(telegram) == receiver && !VirtualList::find(this->delayedMessagesToDiscard, delayedMessage))
		{
			if(!VirtualList::find(this->delayedMessagesToDiscard, delayedMessage))
			{
				VirtualList::pushBack(this->delayedMessagesToDiscard, delayedMessage);
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
void MessageDispatcher::print(int x, int y)
{
	Printing::text(Printing::getInstance(), "MESSAGE DISPATCHER' STATUS", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Delayed messages:     ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->delayedMessages), x + 19, y, NULL);
	Printing::text(Printing::getInstance(), "Discarded messages:         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->delayedMessagesToDiscard), x + 19, y, NULL);
}
