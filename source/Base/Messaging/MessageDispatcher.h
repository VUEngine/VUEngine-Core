/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MESSAGE_DISPATCHER_H_
#define MESSAGE_DISPATCHER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Clock;
class ListenerObject;
class Telegram;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// @memberof MessageDispatcher
typedef struct DelayedMessage
{
	/// Telegram to dispatch
	Telegram telegram;

	/// Telegram's time of arrival
	uint32 timeOfArrival;

	/// Reference to clock for the time of arrival
	Clock clock;

	/// Discarded flag
	bool discarded;

} DelayedMessage;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class MessageDispatcher
///
/// Inherits from Object
///
/// Implements a dispatcher central of message codes wrapped in a Telegram.
singleton class MessageDispatcher : Object
{
	/// @protectedsection

	/// Linked list of queued messages to be dispatched
	VirtualList delayedMessages;

	/// Telegram used when there is no stacking of telegrams
	Telegram helperTelegram;

	/// Flag that indicates the usage state of the helper telegram
	bool helperTelegramIsInUse;

	/// @publicsection

	/// Dispatch a message
	/// @param delay: Milliseconds to wait before dispatching the message
	/// @param sender: Object that sends the message
	/// @param receiver: Object that receives the message
	/// @param message: Message's code
	/// @param extraInfo: Pointer to any extra data that must accompany the message
	/// @return	Boolean indicating the status of the processing of the message if immediately dispatched
	static bool dispatchMessage
	(
		uint32 delay, ListenerObject sender, ListenerObject receiver, int32 message, void* extraInfo
	);

	/// Dispatch delayed message
	/// @param clock: Clock on which to check for the delay timeout
	/// @param delay: Milliseconds to wait before dispatching the message
	/// @param sender: Object that sends the message
	/// @param receiver: Object that receives the message
	/// @param message: Message's code
	/// @param extraInfo: Pointer to any extra data that must accompany the message
	static void dispatchDelayedMessage
	(
		Clock clock, uint32 delay, ListenerObject sender, ListenerObject receiver, int32 message,
		void* extraInfo
	);

	/// Dispatch the delayed messages whose delay has expired.
	static bool dispatchDelayedMessages();

	/// Force the destruction of discarded messages.
	static void processDiscardedMessages();

	/// Discarde delayed messages associated to the given clock.
	/// @param clock: Clock to search in delayed messages to discard
	/// @return True if any messages is discarded
	static bool discardDelayedMessagesWithClock(Clock clock);

	/// Discard delayed messages sent by an object.
	/// @param sender: Object that originally sent the message
	/// @param message: Message's code
	/// @return True if any messages is discarded
	static bool discardDelayedMessagesFromSender(ListenerObject sender, int32 message);

	/// Discard delayed messages sent to an object.
	/// @param receiver: Object that was the target of the message
	/// @param message: Message's code
	/// @return True if any messages is discarded
	static bool discardDelayedMessagesForReceiver(ListenerObject receiver, int32 message);

	/// Discard all delayed messages sent by an object.
	/// @param sender: Object that was the target of the message
	/// @return True if any messages is discarded
	static bool discardAllDelayedMessagesFromSender(ListenerObject sender);

	/// Discard all delayed messages sent to an object.
	/// @param receiver: Object that was the target of the message
	/// @return True if any messages is discarded
	static bool discardAllDelayedMessagesForReceiver(ListenerObject receiver);

	/// Discard all delayed messages sent to an object.
	/// @param listenerObject: Object that the messages were originally sent to or sent by
	static bool discardAllDelayedMessages(ListenerObject listenerObject);

	/// Print all delayed messages sent by an object.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int32 x, int32 y);

	/// Print all delayed messages sent by an object.
	/// @param sender: Object that originally sent the message
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void printAllDelayedMessagesFromSender(ListenerObject sender, int16 x, int16 y);
}

#endif
