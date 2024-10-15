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


//=========================================================================================================
//												INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
//											FORWARD DECLARATIONS
//=========================================================================================================

class Clock;
class Telegram;


//=========================================================================================================
//												CLASS'S DATA
//=========================================================================================================

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


//=========================================================================================================
//											CLASS'S DECLARATION
//=========================================================================================================

class ListenerObject;
class StateMachine;
class Telegram;
class Clock;

///
/// Class MessageDispatcher
///
/// Inherits from Object
///
/// Implements a dispatcher central of message codes wrapped in a Telegram.
/// @ingroup base
singleton class MessageDispatcher : Object
{
	/// Linked list of queued messages to be dispatched
	VirtualList delayedMessages;

	/// Telegram used when there is no stacking of telegrams
	Telegram helperTelegram;

	/// Flag that indicates the usage state of the helper telegram
	bool helperTelegramIsInUse;

	/// @publicsection
	static MessageDispatcher getInstance();

	/// Dispatch a message
	/// @param delay: Milliseconds to wait before dispatching the message
	/// @param sender: Object that sends the message
	/// @param receiver: Object that receives the message
	/// @param message: Message's code
	/// @param extraInfo: Pointer to any extra data that must accompany the message
	/// @return	Boolean indicating the status of the processing of the message if immediately dispatched
	static bool dispatchMessage(uint32 delay, ListenerObject sender, ListenerObject receiver, int32 message, void* extraInfo);

	/// Dispatch delayed message
	/// @param delay: Milliseconds to wait before dispatching the message
	/// @param sender: Object that sends the message
	/// @param receiver: Object that receives the message
	/// @param message: Message's code
	/// @param extraInfo: Pointer to any extra data that must accompany the message
	void dispatchDelayedMessage(Clock clock, uint32 delay, ListenerObject sender,
		ListenerObject receiver, int32 message, void* extraInfo);
 	bool dispatchDelayedMessages();
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
