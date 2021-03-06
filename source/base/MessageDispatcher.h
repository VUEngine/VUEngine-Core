/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

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
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base

singleton class MessageDispatcher : Object
{
	// Delayed messages
	VirtualList delayedMessages;
	// Delayed messages to discard
	VirtualList delayedMessagesToDiscard;
	// Delayed messages to dispatch
	VirtualList delayedMessagesToDispatch;

	/// @publicsection
	static MessageDispatcher getInstance();
	static bool dispatchMessage(u32 delay, Object sender, Object receiver, int message, void* extraInfo);
	void dispatchDelayedMessage(Clock clock, u32 delay, Object sender,
		Object receiver, int message, void* extraInfo);
 	u32 dispatchDelayedMessages();
	bool discardDelayedMessagesWithClock(Clock clock);
	bool discardDelayedMessagesFromSender(Object sender, int message);
	bool discardDelayedMessagesForReceiver(Object receiver, int message);
	bool discardAllDelayedMessagesFromSender(Object sender);
	bool discardAllDelayedMessagesForReceiver(Object receiver);
	void processDiscardedMessages();
	void print(int x, int y);
}


#endif
