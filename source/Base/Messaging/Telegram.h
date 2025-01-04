/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TELEGRAM_H_
#define TELEGRAM_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Telegram
///
/// Inherits from Object
///
/// Wraps a message to be sent to an Object.
class Telegram : Object
{
	/// @protectedsection

	/// The message's code
	int32 message;
	
	/// Additional information that may accompany the message
	void* extraInfo;

	/// The object that sends the message
	void* sender;

	/// The object that receives the message
	void* receiver;

	/// @publicsection

	/// Class' constructor
	/// @param sender: The object that sends the message
	/// @param receiver: The object that sends the message
	/// @param message:	The message's code
	/// @param extraInfo: Additional information that may accompany the message
	void constructor(void* sender, void* receiver, int32 message, void* extraInfo);

	/// Retrieve the Telegram's sender.
	/// @return Pointer to the object that sent the message
	void* getSender();

	/// Retrieve the Telegram's receiver.
	/// @return Pointer to the object that receives the message
	void* getReceiver();

	/// Retrieve the Telegram's message code.
	/// @return The message's code
	int32 getMessage();

	/// Retrieve the Telegram's sender.
	/// @return Additional information that may accompany the message
	void* getExtraInfo();
}


#endif
