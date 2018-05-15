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

#ifndef MESSAGE_DISPATCHER_H_
#define MESSAGE_DISPATCHER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <StateMachine.h>
#include <Telegram.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

singleton class MessageDispatcher : Object
{
	/**
	 * @var VirtualList	delayedMessages
	 * @brief			Delayed messages
	 * @memberof		MessageDispatcher
	 */
	VirtualList delayedMessages;
	/**
	 * @var VirtualList	delayedMessagesToDiscard
	 * @brief			Delayed messages to discard
	 * @memberof		MessageDispatcher
	 */
	VirtualList delayedMessagesToDiscard;
	/**
	 * @var VirtualList	delayedMessagesToDispatch
	 * @brief			Delayed messages to dispatch
	 * @memberof		MessageDispatcher
	 */
	VirtualList delayedMessagesToDispatch;

	static MessageDispatcher getInstance();
	static bool dispatchMessage(u32 delay, Object sender, Object receiver, int message, void* extraInfo);
	u32 dispatchDelayedMessages(MessageDispatcher this);
	void discardDelayedMessagesWithClock(MessageDispatcher this, Clock clock);
	void discardDelayedMessagesFromSender(MessageDispatcher this, Object sender, int message);
	void discardDelayedMessagesForReceiver(MessageDispatcher this, Object receiver, int message);
	void discardAllDelayedMessagesFromSender(MessageDispatcher this, Object sender);
	void discardAllDelayedMessagesForReceiver(MessageDispatcher this, Object receiver);
	void print(MessageDispatcher this, int x, int y);
}


#endif
