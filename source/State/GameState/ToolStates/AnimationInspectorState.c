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

#include <AnimationInspector.h>

#include "AnimationInspectorState.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspectorState::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->tool = Tool::safeCast(AnimationInspector::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void AnimationInspectorState::destructor()
{
	// destroy base
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool AnimationInspectorState::isKeyCombination(const UserInput* userInput)
{
	return ((userInput->holdKey & K_SEL) && (userInput->holdKey & K_RT) && (userInput->releasedKey & K_RR));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


#endif
