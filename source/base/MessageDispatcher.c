/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
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
																										\
	/* super's attributes */																			\
	Object_ATTRIBUTES;																					\
																										\
	/* delayed messages */																				\
	VirtualList delayedMessages;																		\
																										\
	/* delayed messages */																				\
	VirtualList delayedMessagesToDiscard;																\

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
static void MessageDispatcher_constructor(MessageDispatcher this)
{
	__CONSTRUCT_BASE();

	this->delayedMessages = __NEW(VirtualList);
	this->delayedMessagesToDiscard = __NEW(VirtualList);
}

// class's destructor
void MessageDispatcher_destructor(MessageDispatcher this)
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
	//make sure the receiver is valid
	ASSERT(sender, "MessageDispatcher::dispatchMessage: null sender");
	ASSERT(receiver, "MessageDispatcher::dispatchMessage: null receiver");

	if(0 >= delay)
	{
		//create the telegram
		Telegram telegram = __NEW(Telegram, 0, sender, receiver, message, extraInfo);

		//send the telegram to the recipient
		bool result = __VIRTUAL_CALL(bool, Object, handleMessage, receiver, telegram);

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

	//create the telegram
	Telegram telegram = __NEW(Telegram, delay, sender, receiver, message, extraInfo);

	DelayedMessage* delayMessage = __NEW_BASIC(DelayedMessage);

	delayMessage->telegram = telegram;
	delayMessage->clock = Game_getInGameClock(Game_getInstance());
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
void MessageDispatcher_dispatchDelayedMessages(MessageDispatcher this)
{
	ASSERT(this, "MessageDispatcher::dispatchDelayedMessages: null this");
	ASSERT(this->delayedMessages, "MessageDispatcher::dispatchDelayedMessages: null delayedMessages");

	MessageDispatcher_processDiscardedMessages(this);
	
	if(this->delayedMessages->head)
	{
		VirtualList telegramsToDispatch = __NEW(VirtualList);

		VirtualNode node = this->delayedMessages->head;

		for(; node; node = node->next)
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)node->data;

			ASSERT(__SAFE_CAST(Telegram, delayedMessage->telegram), "MessageDispatcher::dispatchDelayedMessages: no telegram in queue")

			if(Clock_getTime(delayedMessage->clock) > delayedMessage->timeOfArrival)
			{
				VirtualList_pushFront(telegramsToDispatch, delayedMessage);
			}
		}

		node = telegramsToDispatch->head;

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

			// check if sender and receiver are still alive
			if(!auxNode && *(u32*)Telegram_getSender(telegram) && *(u32*)Telegram_getReceiver(telegram))
			{
				__VIRTUAL_CALL(bool, Object, handleMessage, Telegram_getReceiver(telegram), telegram);
			}
		}

		node = telegramsToDispatch->head;

		for(; node; node = node->next)
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)node->data;
			Telegram telegram = delayedMessage->telegram;

			VirtualList_removeElement(this->delayedMessages, delayedMessage);
			VirtualList_removeElement(this->delayedMessagesToDiscard, delayedMessage);

			if(*(u32*)telegram)
			{
				__DELETE(telegram);
			}
			
			if(*(u32*)delayedMessage)
			{
				__DELETE_BASIC(delayedMessage);
			}
		}

		__DELETE(telegramsToDispatch);
	}
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
