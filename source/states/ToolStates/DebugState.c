/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __TOOLS


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <DebugState.h>
#include <Debug.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			DebugState::getInstance()
 * @memberof	DebugState
 * @public
 * @return		DebugState instance
 */


/**
 * Class constructor
 *
 * @private
 */
void DebugState::constructor()
{
	Base::constructor();

	this->tool = Tool::safeCast(Debug::getInstance());
}

/**
 * Class destructor
 *
 * @private
 */
void DebugState::destructor()
{
	// destroy base
	Base::destructor();
}

/**
 * Check if key combinations invokes me
 *
 * @public
 * @param userInput		UserInput
 *
 * @return bool
 */
bool DebugState::isKeyCombination(UserInput userInput)
{
	return ((userInput.holdKey & K_LT) && (userInput.holdKey & K_RT) && (userInput.releasedKey & K_RU));
}

#endif
