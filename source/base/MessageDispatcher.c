/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void MessageDispatcher_constructor(MessageDispatcher this);
static void MessageDispatcher_destructor(MessageDispatcher this);
void MessageDispatcher_dispatchDelayedMessage(MessageDispatcher this, Clock clock, u32 delay, Object sender,
	Object receiver, int message, void* extraInfo);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// text box based on bgmaps
#define MessageDispatcher_ATTRIBUTES																	\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* delayed messages */																			\
		VirtualList delayedMessages;																	\
		/* delayed messages */																			\
		VirtualList delayedMessagesToDiscard;															\
		/* delayed messages */																			\
		VirtualList delayedMessagesToDispatch;															\

/**
 * @class 	MessageDispatcher
 * @extends Object
 * @ingroup base
 */
__CLASS_DEFINITION(MessageDispatcher, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);

/**
 * Delayed Message
 *
 * @memberof MessageDispatcher
 */
typedef struct DelayedMessage
{
	/// pointer to the telegram to dispatch
	Telegram telegram;
	/// time of arrival
	u32 timeOfArrival;
	/// reference to clock
	Clock clock;

} DelayedMessage;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			MessageDispatcher_getInstance()
 * @memberof	MessageDispatcher
 * @public
 *
 * @return		MessageDispatcher instance
 */
 __SINGLETON(MessageDispatcher);

/**
 * Class constructor
 *
 * @memberof		MessageDispatcher
 * @public
 *
 * @param this		Function scope
 */
 static void __attribute__ ((noinline)) MessageDispatcher_constructor(MessageDispatcher this)
{
	ASSERT(this, "MessageDispatcher::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->delayedMessages = __NEW(VirtualList);
	this->delayedMessagesToDiscard = __NEW(VirtualList);
	this->delayedMessagesToDispatch = __NEW(VirtualList);
}

/**
 * Class destructor
 *
 * @memberof		MessageDispatcher
 * @public
 *
 * @param this		Function scope
 */
__attribute__((unused)) void MessageDispatcher_destructor(MessageDispatcher this)
{
	ASSERT(this, "MessageDispatcher::destructor: null this");

	__DELETE(this->delayedMessages);
	__DELETE(this->delayedMessagesToDiscard);

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Dispatch a message
 *
 * @memberof		MessageDispatcher
 * @public
 *
 * @param delay		milliseconds to wait before dispatching the message
 * @param sender	the object that sends the message
 * @param receiver	the object that receives the message
 * @param message	the actual message code
 * @param extraInfo	pointer to any extra data that must accompany the message
 *
 * @return			a flag indicating the status of the processing of the message
 */
bool MessageDispatcher_dispatchMessage(u32 delay, Object sender, Object receiver, int message, void* extraInfo)
{
	// make sure the receiver is valid
	ASSERT(sender, "MessageDispatcher::dispatchMessage: null sender");
	ASSERT(receiver, "MessageDispatcher::dispatchMessage: null receiver");

	if(0 >= delay)
	{
		// create the telegram
		Telegram telegram = __NEW(Telegram, sender, receiver, message, extraInfo);

		// send the telegram to the recipient
		bool result = __VIRTUAL_CALL(Object, handleMessage, receiver, telegram);

		__DELETE(telegram);
		return result;
	}
	else
	{
		MessageDispatcher_dispatchDelayedMessage(MessageDispatcher_getInstance(), Game_getMessagingClock(Game_getInstance()), delay, sender, receiver, message, extraInfo);
	}

	return false;
}

/**
 * Dispatch delayed message
 *
 * @memberof		MessageDispatcher
 * @private
 *
 * @param this		Function scope
 * @param delay		milliseconds to wait before dispatching the message
 * @param sender	the object that sends the message
 * @param receiver	the object that receives the message
 * @param message	the actual message code
 * @param extraInfo	pointer to any extra data that must accompany the message
 */
void MessageDispatcher_dispatchDelayedMessage(MessageDispatcher this, Clock clock, u32 delay,
 	Object sender, Object receiver, int message, void* extraInfo)
{
	ASSERT(this, "MessageDispatcher::dispatchDelayedMessage: null this");

	// create the telegram
	Telegram telegram = __NEW(Telegram, sender, receiver, message, extraInfo);

	DelayedMessage* delayMessage = __NEW_BASIC(DelayedMessage);

	delayMessage->telegram = telegram;
	delayMessage->clock = clock ? clock : Game_getMessagingClock(Game_getInstance());
	delayMessage->timeOfArrival = Clock_getTime(delayMessage->clock) + delay;

	VirtualList_pushBack(this->delayedMessages, delayMessage);
}

/**
 * Take care of any discarded message
 *
 * @memberof		MessageDispatcher
 * @public
 *
 * @param this		Function scope
 */
void MessageDispatcher_processDiscardedMessages(MessageDispatcher this)
{
	ASSERT(this, "MessageDispatcher::processDiscardedMessages: null this");
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

			VirtualList_removeElement(this->delayedMessages, delayedMessage);

			if(__IS_BASIC_OBJECT_ALIVE(delayedMessage))
			{
				__DELETE_BASIC(delayedMessage);
			}

			if(__IS_OBJECT_ALIVE(telegram))
			{
				__DELETE(telegram);
			}
		}

		VirtualList_clear(this->delayedMessagesToDiscard);
	}

}

/**
 * Dispatch the delayed messages whose delay has expired
 *
 * @memberof		MessageDispatcher
 * @public
 *
 * @param this		Function scope
 */
u32 MessageDispatcher_dispatchDelayedMessages(MessageDispatcher this)
{
	ASSERT(this, "MessageDispatcher::dispatchDelayedMessages: null this");
	ASSERT(this->delayedMessages, "MessageDispatcher::dispatchDelayedMessages: null delayedMessages");

	u32 messagesDispatched = false;

	MessageDispatcher_processDiscardedMessages(this);

	if(this->delayedMessages->head)
	{
		VirtualNode node = this->delayedMessages->head;

		for(; node; node = node->next)
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

			ASSERT(__SAFE_CAST(Telegram, delayedMessage->telegram), "MessageDispatcher::dispatchDelayedMessages: no telegram in queue")

			if(!Clock_isPaused(delayedMessage->clock) && Clock_getTime(delayedMessage->clock) > delayedMessage->timeOfArrival)
			{
				Telegram telegram = delayedMessage->telegram;

				void* sender = Telegram_getSender(telegram);
				void* receiver = Telegram_getReceiver(telegram);

				ASSERT(sender, "MessageDispatcher::dispatchDelayedMessages: null sender");
				ASSERT(receiver, "MessageDispatcher::dispatchDelayedMessages: null receiver");

				// check if sender and receiver are still alive
				if(!VirtualList_find(this->delayedMessagesToDiscard, delayedMessage) && __IS_OBJECT_ALIVE(sender) && __IS_OBJECT_ALIVE(receiver))
				{
					messagesDispatched |= true;
					__VIRTUAL_CALL(Object, handleMessage, receiver, telegram);
				}

				VirtualList_removeElement(this->delayedMessages, delayedMessage);

				if(VirtualList_find(this->delayedMessagesToDiscard, delayedMessage))
				{
					VirtualList_removeElement(this->delayedMessagesToDiscard, delayedMessage);
				}

				if(__IS_OBJECT_ALIVE(telegram))
				{
					__DELETE(telegram);
				}

				if(__IS_BASIC_OBJECT_ALIVE(delayedMessage))
				{
					__DELETE_BASIC(delayedMessage);
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
 * @memberof		MessageDispatcher
 * @private
 *
 * @param this		Function scope
 * @param clock		the clock against which the message's delay is measured
 */
void MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher this, Clock clock)
{
	ASSERT(this, "MessageDispatcher::discardDelayedMessagesWithClock: null this");

	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

		if(delayedMessage->clock == clock && !VirtualList_find(this->delayedMessagesToDiscard, delayedMessage))
		{
			VirtualList_pushBack(this->delayedMessagesToDiscard, delayedMessage);
		}
	}
}

/**
 * Discarded delayed messages sent by an object
 *
 * @memberof		MessageDispatcher
 * @private
 *
 * @param this		Function scope
 * @param sender	the object that originally sent the message
 * @param message	the actual message code
 */
void MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher this, Object sender, int message)
{
	ASSERT(this, "MessageDispatcher::discardDelayedMessagesFromSender: null this");

	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(Telegram_getMessage(telegram) == message && Telegram_getSender(telegram) == sender && !VirtualList_find(this->delayedMessagesToDiscard, delayedMessage))
		{
			VirtualList_pushBack(this->delayedMessagesToDiscard, delayedMessage);
		}
	}
}

/**
 * Discarded delayed messages sent to an object
 *
 * @memberof		MessageDispatcher
 * @private
 *
 * @param this		Function scope
 * @param sender	the object that the message was originally sent to
 * @param message	the actual message code
 */
void MessageDispatcher_discardDelayedMessagesForReceiver(MessageDispatcher this, Object receiver, int message)
{
	ASSERT(this, "MessageDispatcher::discardDelayedMessagesFromSender: null this");

	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(__IS_BASIC_OBJECT_ALIVE(delayedMessage) && __IS_OBJECT_ALIVE(telegram) && Telegram_getMessage(telegram) == message && Telegram_getReceiver(telegram) == receiver && !VirtualList_find(this->delayedMessagesToDiscard, delayedMessage))
		{
			if(!VirtualList_find(this->delayedMessagesToDiscard, delayedMessage))
			{
				VirtualList_pushBack(this->delayedMessagesToDiscard, delayedMessage);
			}
		}
	}
}

/**
 * Discarded all delayed messages sent by an object
 *
 * @memberof		MessageDispatcher
 * @private
 *
 * @param this		Function scope
 * @param sender	the object that originally sent the message
 */
void MessageDispatcher_discardAllDelayedMessagesFromSender(MessageDispatcher this, Object sender)
{
	ASSERT(this, "MessageDispatcher::discardAllDelayedMessagesFromSender: null this");

	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(__IS_BASIC_OBJECT_ALIVE(delayedMessage) && __IS_OBJECT_ALIVE(telegram) && Telegram_getSender(telegram) == sender && !VirtualList_find(this->delayedMessagesToDiscard, delayedMessage))
		{
			if(!VirtualList_find(this->delayedMessagesToDiscard, delayedMessage))
			{
				VirtualList_pushBack(this->delayedMessagesToDiscard, delayedMessage);
			}
		}
	}
}

/**
 * Discarded all delayed messages sent to an object
 *
 * @memberof		MessageDispatcher
 * @private
 *
 * @param this		Function scope
 * @param sender	the object that the message was originally sent to
 */
void MessageDispatcher_discardAllDelayedMessagesForReceiver(MessageDispatcher this, Object receiver)
{
	ASSERT(this, "MessageDispatcher::discardAllDelayedMessagesForReceiver: null this");

	VirtualNode node = this->delayedMessages->head;

	for(; node; node = node->next)
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
		Telegram telegram = delayedMessage->telegram;

		if(__IS_BASIC_OBJECT_ALIVE(delayedMessage) && __IS_OBJECT_ALIVE(telegram) && Telegram_getReceiver(telegram) == receiver && !VirtualList_find(this->delayedMessagesToDiscard, delayedMessage))
		{
			if(!VirtualList_find(this->delayedMessagesToDiscard, delayedMessage))
			{
				VirtualList_pushBack(this->delayedMessagesToDiscard, delayedMessage);
			}
		}
	}
}
