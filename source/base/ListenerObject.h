/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef LISTENER_OBJECT_H_
#define LISTENER_OBJECT_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class ListenerObject;
class Telegram;
class VirtualList;

typedef bool (*EventListener)(ListenerObject, ListenerObject);

/**
 * An Event
 * @memberof ListenerObject
 */
typedef struct Event
{
	/// object to register event listener at
	ListenerObject listener;
	/// the method to execute on event
	EventListener method;
	/// the code of the event to listen to
	uint16 code;
	/// flag to prevent race conditions when firing events
	bool remove;

} Event;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

abstract class ListenerObject : Object
{
	// List of registered events.
	VirtualList events;
	// Counter user to prevent race conditions in nested event firings
	int8 eventFirings;

	/// @publicsection
	void constructor();
	void addEventListener(ListenerObject listener, EventListener method, uint16 eventCode);
	void removeEventListener(ListenerObject listener, EventListener method, uint16 eventCode);
	void removeEventListeners(EventListener method, uint16 eventCode);
	void removeEventListenerScopes(ListenerObject listener, uint16 eventCode);
	void removeAllEventListeners();
	bool hasActiveEventListeners();
	void fireEvent(uint16 eventCode);
	void sendMessageTo(ListenerObject receiver, uint32 message, uint32 delay, uint32 randomDelay);
	void sendMessageToSelf(uint32 message, uint32 delay, uint32 randomDelay);
	void discardAllMessages();
	void discardMessages(uint32 message);

	virtual bool handleMessage(Telegram telegram);
}


#endif
