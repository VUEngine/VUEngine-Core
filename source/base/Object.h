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
	u32 code;

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
	void addEventListener(Object listener, EventListener method, u32 eventCode);
	void removeEventListener(Object listener, EventListener method, u32 eventCode);
	void removeEventListeners(Object listener, u32 eventCode);
	void removeAllEventListeners(u32 eventCode);
	bool hasActiveEventListeners();
	void fireEvent(u32 eventCode);
	void sendMessageTo(Object receiver, u32 message, u32 delay, u32 randomDelay);
	void sendMessageToSelf(u32 message, u32 delay, u32 randomDelay);
	void discardAllMessages();
	void discardMessages(u32 message);
	virtual bool handleMessage(Telegram telegram);
	const void* getVTable();
}


#endif
