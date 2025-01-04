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

#include <SoundTest.h>

#include "SoundTestState.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTestState::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->tool = Tool::safeCast(SoundTest::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTestState::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTestState::enter(void* owner __attribute__ ((unused)))
{
	Base::enter(this, owner);

	this->stream = false;
	this->transform = false;
	this->updatePhysics = false;
	this->processCollisions = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundTestState::isKeyCombination(const UserInput* userInput)
{
	return ((userInput->holdKey & K_SEL) && (userInput->holdKey & K_RT) && (userInput->releasedKey & K_RD));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
