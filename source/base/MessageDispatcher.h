/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MESSAGE_DISPATCHER_H_
#define MESSAGE_DISPATCHER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <ListenerObject.h>
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
	uint32 timeOfArrival;
	/// reference to clock
	Clock clock;
	/// Discarded flag
	bool discarded;

} DelayedMessage;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base

singleton class MessageDispatcher : Object
{
	// Delayed messages
	VirtualList delayedMessages;
	// Telegram used when there is no stacking of telegrams
	Telegram helperTelegram;
	bool helperTelegramIsInUse;

	/// @publicsection
	static MessageDispatcher getInstance();
	static bool dispatchMessage(uint32 delay, ListenerObject sender, ListenerObject receiver, int32 message, void* extraInfo);
	void dispatchDelayedMessage(Clock clock, uint32 delay, ListenerObject sender,
		ListenerObject receiver, int32 message, void* extraInfo);
 	uint32 dispatchDelayedMessages();
	bool discardDelayedMessagesWithClock(Clock clock);
	bool discardDelayedMessagesFromSender(ListenerObject sender, int32 message);
	bool discardDelayedMessagesForReceiver(ListenerObject receiver, int32 message);
	bool discardAllDelayedMessagesFromSender(ListenerObject sender);
	bool discardAllDelayedMessagesForReceiver(ListenerObject receiver);
	bool discardAllDelayedMessages(ListenerObject listenerObject);
	void processDiscardedMessages();
	void printAllDelayedMessagesFromSender(ListenerObject sender, int16 x, int16 y);
	void print(int32 x, int32 y);
}


#endif
