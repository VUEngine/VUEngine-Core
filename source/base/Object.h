/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_H_
#define OBJECT_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Oop.h>
#include <Constants.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class Object;
class Telegram;
class Error;

typedef void (*EventListener)(Object, Object);
typedef Object (*AllocatorPointer)();

/**
 * An Event
 * @memberof Object
 */
typedef struct Event
{
	/// object to register event listener at
	Object listener;
	/// the method to execute on event
	EventListener method;
	/// the code of the event to listen to
	uint32 code;

} Event;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// Base class for all other classes in the engine, it derives from nothing but itself
/// @ingroup base
abstract class Object : Object
{
	// Pointer to the class's virtual table.
	void* vTable;
	// List of registered events.
	VirtualList events;

	/// @publicsection
	static Object getCast(void* object, ClassPointer targetClassGetClassMethod, ClassPointer baseClassGetClassMethod);
	void constructor();
	void addEventListener(Object listener, EventListener method, uint32 eventCode);
	void removeEventListener(Object listener, EventListener method, uint32 eventCode);
	void removeEventListeners(EventListener method, uint32 eventCode);
	void removeEventListenerScopes(Object listener, uint32 eventCode);
	void removeAllEventListeners();
	bool hasActiveEventListeners();
	void fireEvent(uint32 eventCode);
	void sendMessageTo(Object receiver, uint32 message, uint32 delay, uint32 randomDelay);
	void sendMessageToSelf(uint32 message, uint32 delay, uint32 randomDelay);
	void discardAllMessages();
	void discardMessages(uint32 message);
	bool evolveTo(const void* targetClass);
	const void* getVTable();

	virtual bool handleMessage(Telegram telegram);
}


#endif
