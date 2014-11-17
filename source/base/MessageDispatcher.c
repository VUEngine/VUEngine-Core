/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <MessageDispatcher.h>
#include <Game.h>
#include <VirtualList.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void MessageDispatcher_constructor(MessageDispatcher this);

// class's destructor
static void MessageDispatcher_destructor(MessageDispatcher this);

// dispatch delayed messages
static void MessageDispatcher_dispatchDelayedMessage(MessageDispatcher this, u32 delay, Object sender, 
		Object receiver, int message, void* extraInfo);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// text box based on bgmaps
#define MessageDispatcher_ATTRIBUTES			\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* delayed messages */						\
	VirtualList delayedMessages;


__CLASS_DEFINITION(MessageDispatcher);


typedef struct DelayedMessage {
	
	// time of arrival
	u32 timeOfArrival;
	
	// pointer to the telegram to dispatch
	Telegram telegram;
	
}DelayedMessage;
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// a singleton
__SINGLETON(MessageDispatcher);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void MessageDispatcher_constructor(MessageDispatcher this){
	
	__CONSTRUCT_BASE(Object);
	
	this->delayedMessages = __NEW(VirtualList);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void MessageDispatcher_destructor(MessageDispatcher this){

	ASSERT(this, "MessageDispatcher::destructor: null this");

	__DELETE(this->delayedMessages);

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dispatch a telegram
/*
static int MessageDispatcher_discharge(StateMachine receiver, Telegram telegram){

	return StateMachine_handleMessage(receiver, telegram);
}
*/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MessageDispatcher_dispatchMessage(u32 delay, Object sender, 
									Object receiver, int message, void* extraInfo){

	//make sure the receiver is valid
	ASSERT(sender, "MessageDispatcher::dispatchMessage: null sender");
	ASSERT(receiver, "MessageDispatcher::dispatchMessage: null receiver");
  
	if(0 >= delay){
		
		//create the telegram
		Telegram telegram = __NEW(Telegram, __ARGUMENTS(0, sender, receiver, message, extraInfo));

		//send the telegram to the recipient
		int result = __VIRTUAL_CALL(int, Object, handleMessage, receiver, __ARGUMENTS(telegram));
		
		__DELETE(telegram);
		return result;
	}
	else {

		MessageDispatcher_dispatchDelayedMessage(MessageDispatcher_getInstance(), delay, sender, receiver, message, extraInfo);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dispatch delayed messages
void MessageDispatcher_dispatchDelayedMessage(MessageDispatcher this, u32 delay, Object sender, 
		Object receiver, int message, void* extraInfo){

	ASSERT(this, "MessageDispatcher::dispatchDelayedMessage: null this");

	//create the telegram
	Telegram telegram = __NEW(Telegram, __ARGUMENTS(delay, sender, receiver, message, extraInfo));

	DelayedMessage* delayMessage = __NEW_BASIC(DelayedMessage);
	
	delayMessage->telegram = telegram;
	delayMessage->timeOfArrival = Clock_getTime(Game_getClock(Game_getInstance())); 

	VirtualList_pushFront(this->delayedMessages, delayMessage);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dispatch delayed messages
void MessageDispatcher_dispatchDelayedMessages(MessageDispatcher this){
	
	ASSERT(this, "MessageDispatcher::dispatchDelayedMessages: null this");
	ASSERT(this->delayedMessages, "MessageDispatcher::reset: null delayedMessages");

	if(0 < VirtualList_getSize(this->delayedMessages)){

		VirtualList telegramsToDispatch = __NEW(VirtualList);

		VirtualNode node = VirtualList_begin(this->delayedMessages);

		for(; node; node = VirtualNode_getNext(node)){
			
			DelayedMessage* delayedMessage = (DelayedMessage*)VirtualNode_getData(node);
			Telegram telegram = delayedMessage->telegram;
			
			ASSERT(__GET_CAST(Telegram, telegram), "MessageDispatcher::dispatchDelayedMessages: no telegram in queue")
			
			if(Clock_getTime(Game_getClock(Game_getInstance())) > delayedMessage->timeOfArrival + Telegram_getDelay(telegram)){

				VirtualList_pushFront(telegramsToDispatch, delayedMessage);
			}
		}
		
		node = VirtualList_begin(telegramsToDispatch);
		
		for(; node; node = VirtualNode_getNext(node)){
			
			DelayedMessage* delayedMessage = (DelayedMessage*)VirtualNode_getData(node);
			Telegram telegram = delayedMessage->telegram;
			
			__VIRTUAL_CALL(int, Object, handleMessage, Telegram_getReceiver(telegram), __ARGUMENTS(telegram));
		}

		node = VirtualList_begin(telegramsToDispatch);

		for(; node; node = VirtualNode_getNext(node)){
			
			DelayedMessage* delayedMessage = (DelayedMessage*)VirtualNode_getData(node);
			Telegram telegram = delayedMessage->telegram;
			
			VirtualList_removeElement(this->delayedMessages, delayedMessage);
			
			__DELETE(telegram);
			__DELETE_BASIC(delayedMessage);
		}

		__DELETE(telegramsToDispatch);
	}
}
