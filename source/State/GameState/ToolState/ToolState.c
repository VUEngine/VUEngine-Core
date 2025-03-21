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

#include <AnimationInspectorState.h>
#include <DebugState.h>
#include <KeypadManager.h>
#include <StageEditor.h>
#include <StageEditorState.h>
#include <StateMachine.h>
#include <SoundTestState.h>
#include <Tool.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "ToolState.h"

friend class GameState;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static ToolState ToolState::get(const UserInput* userInput __attribute__((unused)))
{
#ifdef __TOOLS
	ToolState engineToolStates[] =
	{
		ToolState::safeCast(DebugState::getInstance()),
		ToolState::safeCast(StageEditorState::getInstance()),
		ToolState::safeCast(AnimationInspectorState::getInstance()),
		ToolState::safeCast(SoundTestState::getInstance()),
		NULL
	};

	int32 i = 0;

	for(; engineToolStates[i]; i++)
	{
		// Check code to access special feature
		if(ToolState::isKeyCombination(engineToolStates[i], userInput))
		{
			return engineToolStates[i];
		}
	}

	extern const ToolState _userToolStates[];

	i = 0;

	for(; _userToolStates[i]; i++)
	{
		// Check code to access special feature
		if(ToolState::isKeyCombination(_userToolStates[i], userInput))
		{
			return _userToolStates[i];
		}
	}
#endif

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->stream = false;
	this->transform = false;
	this->updatePhysics = false;
	this->processCollisions = false;

	this->tool = NULL;
	this->currentGameState = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::destructor()
{
	this->tool = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::start(void* owner __attribute__ ((unused)))
{
	Base::enter(this, owner);

	if(NULL != this->currentGameState)
	{
		GameState::pauseClocks(this->currentGameState);
		GameState::unpauseClock(this->currentGameState, kGameStateAnimationsClock);

		GameState::startClocks(this);

		if(!isDeleted(this->tool))
		{
			Tool::setToolState(this->tool, this);
			Tool::show(this->tool);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::update(void* owner __attribute__ ((unused)))
{
	KeypadManager::readUserInput(KeypadManager::getInstance(), this->lockFrameRate);

	UserInput userInput = KeypadManager::getUserInput();

	if(0 != (userInput.dummyKey | userInput.pressedKey | userInput.holdKey | userInput.releasedKey))
	{
		ToolState::processUserInput(this, &userInput);
	}

	if(!isDeleted(this->tool))
	{
		Tool::update(this->tool);
	}

	if(NULL != this->currentGameState)
	{
		SpriteManager::render(this->currentGameState->componentManagers[kSpriteComponent]);
		
		WireframeManager::render(this->currentGameState->componentManagers[kWireframeComponent]);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ToolState::stop(void* owner __attribute__ ((unused)))
{
	if(!isDeleted(this->tool))
	{
		Tool::hide(this->tool);
	}

	if(NULL != this->currentGameState)
	{
		GameState::unpauseClocks(GameState::safeCast(this->currentGameState));
	}

	this->currentGameState = NULL;

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

void ToolState::configure(GameState currentGameState, Stage currentStage)
{
	this->currentGameState = currentGameState;
	this->currentStage = currentStage;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

GameState ToolState::getCurrentGameState()
{
	return this->currentGameState;	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Stage ToolState::getCurrentStage()
{
	return this->currentStage;	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ComponentManager ToolState::getComponentManager(uint32 componentType)
{
	if(kComponentTypes <= componentType || NULL == this->currentGameState)
	{
		return NULL;
	}
	
	return this->currentGameState->componentManagers[componentType];	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
