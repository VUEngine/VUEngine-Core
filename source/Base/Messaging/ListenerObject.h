/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef LISTENER_OBJECT_H_
#define LISTENER_OBJECT_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class ListenerObject;
class Telegram;
class VirtualList;


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

typedef bool (*EventListener)(ListenerObject, ListenerObject);

/// Event struct
typedef struct Event
{
	/// The object listening for the event
	ListenerObject listener;

	/// The callback on the listener object
	EventListener callback;

	/// The code of the event
	uint16 code;

	/// Internal flag to prevent race conditions when firing events
	bool remove;

} Event;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class ListenerObject
///
/// Inherits from Object
///
/// Interacts by means of events and messages.
abstract class ListenerObject : Object
{
	/// @protectedsection

	/// List of registered events
	VirtualList events;
	
	/// Counter that keeps track of the number of fired events to prevent race conditions in nested firings
	int8 eventFirings;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Class' destructor
	void destructor();

	/// Register an object that will listen for events.
	/// @param listener: ListenerObject that listen for the event
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to listen for
	void addEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Remove a specific listener object from the listening to a give code with the provided callback.
	/// @param listener: ListenerObject to remove from the list of listeners
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to stop listen for
	void removeEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Remove all listener objects for a specific callback and code from the listeners.
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to stop listen for
	void removeEventListeners(EventListener callback, uint16 eventCode);

	/// Remove a specific listener object from the listeners.
	/// @param listener: ListenerObject to remove from the list of listeners
	/// @param eventCode: Event's code to stop listen for
	void removeEventListenerScopes(ListenerObject listener, uint16 eventCode);

	/// Remove all listener objects.
	void removeAllEventListeners();

	/// Check if the object has active event listeners.
	/// @return True if the object's list of event listeners is not empty
	bool hasActiveEventListeners();

	/// Fire an event with the provided code
	/// @param eventCode: Code of the event to fire
	void fireEvent(uint16 eventCode);

	/// Send a message to another object.
	/// @param receiver: ListenerObject that is the target of the message
	/// @param message: The message's code
	/// @param delay: Milliseconds to wait before sending the message
	/// @param randomDelay: Range of a random delay in milliseconds to wait before sending the message
	void sendMessageTo(ListenerObject receiver, uint32 message, uint32 delay, uint32 randomDelay);

	/// Send a message to itself.
	/// @param message: The message's code
	/// @param delay: Milliseconds to wait before sending the message
	/// @param randomDelay: Range of a random delay in milliseconds to wait before sending the message
	void sendMessageToSelf(uint32 message, uint32 delay, uint32 randomDelay);

	/// Discard all messages, both to be sent and to be received.
	void discardAllMessages();

	/// Discard all messages with a specific code, both to be sent and to be received.
	/// @param message: The message's code to discard
	void discardMessages(uint32 message);

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	virtual bool handleMessage(Telegram telegram);
}


#endif