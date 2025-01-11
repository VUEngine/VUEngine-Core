/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __TOOLS

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Debug.h>

#include "DebugState.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void DebugState::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->tool = Tool::safeCast(Debug::getInstance(NULL));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void DebugState::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool DebugState::isKeyCombination(const UserInput* userInput)
{
	return ((userInput->holdKey & K_SEL) && (userInput->holdKey & K_RT) && (userInput->releasedKey & K_RU));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
