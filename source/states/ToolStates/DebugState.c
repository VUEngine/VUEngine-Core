/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


#ifdef __TOOLS

//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Debug.h>

#include "DebugState.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void DebugState::constructor()
{
	Base::constructor();

	this->tool = Tool::safeCast(Debug::getInstance());
}
//---------------------------------------------------------------------------------------------------------
void DebugState::destructor()
{
	// destroy base
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
bool DebugState::isKeyCombination(const UserInput* userInput)
{
	return ((userInput->holdKey & K_A) && (userInput->holdKey & K_B) && (userInput->releasedKey & K_RU));
}
//---------------------------------------------------------------------------------------------------------

#endif
