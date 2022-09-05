/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STATE_H_
#define STATE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
abstract class State : ListenerObject
{
	/// @publicsection
	void constructor();
	virtual void enter(void* owner);
	virtual void execute(void* owner);
	virtual void exit(void* owner);
	virtual void suspend(void* owner);
	virtual void resume(void* owner);
	virtual bool processMessage(void* owner, Telegram telegram);
}


#endif
