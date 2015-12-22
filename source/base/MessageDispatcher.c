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
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void MessageDispatcher_constructor(MessageDispatcher this);

// class's destructor
static void MessageDispatcher_destructor(MessageDispatcher this);

// dispatch delayed messages
static void MessageDispatcher_dispatchDelayedMessage(MessageDispatcher this, u32 delay, Object sender,
		Object receiver, int message, void* extraInfo);

// discart delayed messages
void MessageDispatcher_discardAllDelayedMessages(MessageDispatcher this);


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

typedef struct DelayedMessage
{
	// pointer to the telegram to dispatch
	Telegram telegram;

	// time of arrival
	u32 timeOfArrival;

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
	delayMessage->timeOfArrival = Clock_getTime(Game_getClock(Game_getInstance())) + delay;

	VirtualList_pushFront(this->delayedMessages, delayMessage);
}

// dispatch delayed messages
void MessageDispatcher_processDiscardedMessages(MessageDispatcher this)
{
	ASSERT(this, "MessageDispatcher::processDiscardedMessages: null this");
	ASSERT(this->delayedMessagesToDiscard, "MessageDispatcher::processDiscardedMessages: null delayedMessagesToDiscard");

	if(VirtualList_begin(this->delayedMessagesToDiscard))
	{
		VirtualNode node = VirtualList_begin(this->delayedMessagesToDiscard);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)VirtualNode_getData(node);
			Telegram telegram = delayedMessage->telegram;

			VirtualList_removeElement(this->delayedMessages, delayedMessage);

			ASSERT(telegram, "MessageDispatcher::processDiscardedMessages: null telegram");
			ASSERT(delayedMessage, "MessageDispatcher::processDiscardedMessages: null delayedMessage");

			__DELETE(telegram);
			__DELETE_BASIC(delayedMessage);
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
	
	if(VirtualList_begin(this->delayedMessages))
	{
		VirtualList telegramsToDispatch = __NEW(VirtualList);

		VirtualNode node = VirtualList_begin(this->delayedMessages);

		for(; node; node = VirtualNode_getNext(node))
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)VirtualNode_getData(node);

			ASSERT(__SAFE_CAST(Telegram, delayedMessage->telegram), "MessageDispatcher::dispatchDelayedMessages: no telegram in queue")

			if(Clock_getTime(Game_getClock(Game_getInstance())) > delayedMessage->timeOfArrival)
			{
				VirtualList_pushFront(telegramsToDispatch, delayedMessage);
			}
		}

		node = VirtualList_begin(telegramsToDispatch);

		for(; node; node = VirtualNode_getNext(node))
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)VirtualNode_getData(node);
			Telegram telegram = delayedMessage->telegram;

			VirtualNode auxNode = VirtualList_begin(this->delayedMessagesToDiscard);
			
			for(; auxNode; auxNode = VirtualNode_getNext(auxNode))
			{
				if(delayedMessage == VirtualNode_getData(auxNode))
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

		node = VirtualList_begin(telegramsToDispatch);

		for(; node; node = VirtualNode_getNext(node))
		{
			DelayedMessage* delayedMessage = (DelayedMessage*)VirtualNode_getData(node);
			Telegram telegram = delayedMessage->telegram;

			VirtualList_removeElement(this->delayedMessages, delayedMessage);
			VirtualList_removeElement(this->delayedMessagesToDiscard, delayedMessage);

			__DELETE(telegram);
			__DELETE_BASIC(delayedMessage);
		}

		__DELETE(telegramsToDispatch);
	}
}

// discard delayed messages
void MessageDispatcher_discardAllDelayedMessages(MessageDispatcher this)
{
	ASSERT(this, "MessageDispatcher::discardDelayedMessages: null this");

	VirtualNode node = VirtualList_begin(this->delayedMessages);

	for(; node; node = VirtualNode_getNext(node))
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)VirtualNode_getData(node);
		Telegram telegram = delayedMessage->telegram;

		__DELETE(telegram);
		__DELETE_BASIC(delayedMessage);
	}

	VirtualList_clear(this->delayedMessages);
	VirtualList_clear(this->delayedMessagesToDiscard);
}

// discard delayed messages of an specific type
void MessageDispatcher_discardDelayedMessages(MessageDispatcher this, int message)
{
	ASSERT(this, "MessageDispatcher::discardDelayedMessages: null this");

	VirtualNode node = VirtualList_begin(this->delayedMessages);

	for(; node; node = VirtualNode_getNext(node))
	{
		DelayedMessage* delayedMessage = (DelayedMessage*)VirtualNode_getData(node);
		Telegram telegram = delayedMessage->telegram;

		if(Telegram_getMessage(telegram) == message)
		{
			VirtualList_pushBack(this->delayedMessagesToDiscard, delayedMessage);
		}
	}
}