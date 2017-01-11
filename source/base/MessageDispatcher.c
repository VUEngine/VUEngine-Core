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
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MessageDispatcher.h>
#include <Game.h>
#include <Clock.h>
#include <VirtualList.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void MessageDispatcher_constructor(MessageDispatcher this);
static void MessageDispatcher_destructor(MessageDispatcher this);
static void MessageDispatcher_dispatchDelayedMessage(MessageDispatcher this, u32 delay, Object sender,
	Object receiver, int message, void* extraInfo);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
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

__CLASS_DEFINITION(MessageDispatcher, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


typedef struct DelayedMessage
{
	// pointer to the telegram to dispatch
	Telegram telegram;

	// time of arrival
	u32 timeOfArrival;

	// reference to clock
	Clock clock;

} DelayedMessage;


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(MessageDispatcher);

// class's constructor
static void __attribute__ ((noinline)) MessageDispatcher_constructor(MessageDispatcher this)
{
	__CONSTRUCT_BASE(Object);

	this->delayedMessages = __NEW(VirtualList);
	this->delayedMessagesToDiscard = __NEW(VirtualList);
	this->delayedMessagesToDispatch = __NEW(VirtualList);
}

// class's destructor
__attribute__((unused)) void MessageDispatcher_destructor(MessageDispatcher this)
{
	ASSERT(this, "MessageDispatcher::destructor: null this");

	__DELETE(this->delayedMessages);
	__DELETE(this->delayedMessagesToDiscard);

	// allow a new construct
	__SINGLETON_DESTROY;
}

// dispatch a telegram
/*
static int MessageDispatcher_discharge(StateMachine receiver, Telegram telegram)
{
	return StateMachine_handleMessage(receiver, telegram);
}
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
		MessageDispatcher_dispatchDelayedMessage(MessageDispatcher_getInstance(), delay, sender, receiver, message, extraInfo);
	}

	return false;
}

// dispatch delayed messages
static void MessageDispatcher_dispatchDelayedMessage(MessageDispatcher this, u32 delay, Object sender,
		Object receiver, int message, void* extraInfo)
{
	ASSERT(this, "MessageDispatcher::dispatchDelayedMessage: null this");

	// create the telegram
	Telegram telegram = __NEW(Telegram, sender, receiver, message, extraInfo);

	DelayedMessage* delayMessage = __NEW_BASIC(DelayedMessage);

	delayMessage->telegram = telegram;
	delayMessage->clock = Game_getMessagingClock(Game_getInstance());
	delayMessage->timeOfArrival = Clock_getTime(delayMessage->clock) + delay;

	VirtualList_pushFront(this->delayedMessages, delayMessage);
}

// dispatch delayed messages
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

			VirtualList_removeElement(this->delayedMessages, delayedMessage);

			ASSERT(telegram, "MessageDispatcher::processDiscardedMessages: null telegram");
			ASSERT(delayedMessage, "MessageDispatcher::processDiscardedMessages: null delayedMessage");

			if(*(u32*)telegram)
			{
				__DELETE(telegram);
			}

			if(*(u32*)delayedMessage)
			{
				__DELETE_BASIC(delayedMessage);
			}
		}

		VirtualList_clear(this->delayedMessagesToDiscard);
	}

}

// dispatch delayed messages
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
				VirtualList_pushFront(this->delayedMessagesToDispatch, delayedMessage);
			}
		}

		node = this->delayedMessagesToDispatch->head;

		for(; node; node = node->next)
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
			Telegram telegram = delayedMessage->telegram;

			VirtualNode auxNode = this->delayedMessagesToDiscard->head;

			for(; auxNode; auxNode = auxNode->next)
			{
				if(delayedMessage == auxNode->data)
				{
					break;
				}
			}

			void* sender = Telegram_getSender(telegram);
			void* receiver = Telegram_getReceiver(telegram);

        	ASSERT(sender, "MessageDispatcher::dispatchDelayedMessages: null sender");
        	ASSERT(receiver, "MessageDispatcher::dispatchDelayedMessages: null receiver");

			// check if sender and receiver are still alive
			if(!auxNode && (sender && *(u32*)sender) && (receiver && *(u32*)receiver))
			{
    			messagesDispatched |= true;
				__VIRTUAL_CALL(Object, handleMessage, __SAFE_CAST(Object, receiver), telegram);
			}

			VirtualList_removeElement(this->delayedMessages, delayedMessage);

			if(auxNode)
			{
			    VirtualList_removeElement(this->delayedMessagesToDiscard, delayedMessage);
            }

			if(*(u32*)telegram)
			{
				__DELETE(telegram);
			}

			if(*(u32*)delayedMessage)
			{
				__DELETE_BASIC(delayedMessage);
			}
		}

		VirtualList_clear(this->delayedMessagesToDispatch);
	}

	return messagesDispatched;
}

// discard delayed messages
void MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher this, Clock clock)
{
	ASSERT(this, "MessageDispatcher::discardDelayedMessages: null this");

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

// discard delayed messages of an specific type
void MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher this, Object sender, int message)
{
	ASSERT(this, "MessageDispatcher::discardDelayedMessages: null this");

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
