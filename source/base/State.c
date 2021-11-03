/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <State.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void State::constructor()
{
	// construct base object
	Base::constructor();
}

/**
 * Class destructor
 */
void State::destructor()
{
	// free processor's memory
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Method called when the StateMachine enters to this state
 *
 * @param owner		StateMachine's owner
 */
void State::enter(void* owner __attribute__ ((unused)))
{}

/**
 * Method called when by the StateMachine's update method
 *
 * @param owner		StateMachine's owner
 */
void State::execute(void* owner __attribute__ ((unused)))
{}

/**
 * Method called when the StateMachine exits from this state
 *
 * @param owner		StateMachine's owner
 */
void State::exit(void* owner __attribute__ ((unused)))
{}

/**
 * Method called when the StateMachine enters another state without exiting this one
 *
 * @param owner		StateMachine's owner
 */
void State::suspend(void* owner __attribute__ ((unused)))
{}

/**
 * Method called when the StateMachine returns to this state from another
 *
 * @param owner		StateMachine's owner
 */
void State::resume(void* owner __attribute__ ((unused)))
{}

/**
 * Since C doesn't support function overloading, need a new method to receive the owner of the state
 *
 * @param owner		StateMachine's owner
 */
bool State::processMessage(void* owner __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	return true;
}
