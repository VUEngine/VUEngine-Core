/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TELEGRAM_H_
#define TELEGRAM_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
class Telegram : ListenerObject
{
	// The message itself. These are all enumerated in a file.
	int32 message;
	// Any additional information that may accompany the message
	void* extraInfo;
	// Who sent this telegram
	void* sender;
	// Who is to receive this telegram
	void* receiver;

	/// @publicsection
	void constructor(void* sender, void* receiver, int32 message, void* extraInfo);
	void* getSender();
	void* getReceiver();
	int32 getMessage();
	void* getExtraInfo();
}


#endif
