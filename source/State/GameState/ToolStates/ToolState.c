/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <StateMachine.h>
#include <Tool.h>
#include <VUEngine.h>

#include "ToolState.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->tool = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::destructor()
{
	this->tool = NULL;

	// destroy base
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::enter(void* owner __attribute__ ((unused)))
{
	Base::enter(this, owner);
	GameState::pauseClocks(GameState::safeCast(VUEngine::getPreviousState(VUEngine::getInstance())));
	GameState::startClocks(this);

	this->stream = false;
	this->transform = false;
	this->updatePhysics = false;
	this->processCollisions = false;

	if(!isDeleted(this->tool))
	{
		Tool::setStage(this->tool, GameState::getStage(GameState::safeCast(VUEngine::getPreviousState(VUEngine::getInstance()))));
		Tool::show(this->tool);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::execute(void* owner __attribute__ ((unused)))
{
	if(!isDeleted(this->tool))
	{
		Tool::update(this->tool);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::exit(void* owner __attribute__ ((unused)))
{
	if(!isDeleted(this->tool))
	{
		Tool::hide(this->tool);
	}

	GameState::unpauseClocks(GameState::safeCast(VUEngine::getPreviousState(VUEngine::getInstance())));
	Base::exit(this, owner);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::processUserInput(const UserInput* userInput)
{
	if(!isDeleted(this->tool))
	{
		Tool::processUserInput(this->tool, userInput->releasedKey);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ToolState::stream()
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

