/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STATE_H_
#define STATE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Telegram;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class State
///
/// Inherits from ListenerObject
///
/// Implements a behavioral model to be used in a finite state machine.
abstract class State : ListenerObject
{
	/// @publicsection

	/// Class's constructor
	void constructor();

	/// Class's destructor
	void destructor();

	/// Prepares the object to enter this state.
	/// @param owner: Object that is entering in this state
	virtual void enter(void* owner);

	/// Updates the object in this state.
	/// @param owner: Object that is in this state
	virtual void execute(void* owner);
	
	/// Prepares the object to exit this state.
	/// @param owner: Object that is exiting this state
	virtual void exit(void* owner);

	/// Prepares the object to become inactive in this state.
	/// @param owner: Object that is in this state
	virtual void suspend(void* owner);

	/// Prepares the object to become active in this state.
	/// @param owner: Object that is in this state
	virtual void resume(void* owner);

	/// Process a Telegram sent to an object that is in this state.
	/// @param owner: Object that is in this state
	/// @param telegram: Telegram to process
	virtual bool processMessage(void* owner, Telegram telegram);
}

#endif
