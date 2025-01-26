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

#include <string.h>

#include <Camera.h>
#include <CommunicationManager.h>
#include <Clock.h>
#include <ClockManager.h>
#include <DebugConfig.h>
#include <FrameRate.h>
#include <GameState.h>
#include <HardwareManager.h>
#include <KeypadManager.h>
#include <MessageDispatcher.h>
#include <Profiler.h>
#include <Singleton.h>
#include <Stage.h>
#include <StateMachine.h>
#include <SoundTestState.h>
#include <Telegram.h>
#include <ToolState.h>
#include <TimerManager.h>
#include <VirtualList.h>
#include <VIPManager.h>

#include "VUEngine.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

GameState __GAME_ENTRY_POINT();

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
uint32 _dispatchCycle = 0;
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::hasGameFrameStarted()
{
	VUEngine vuEngine = VUEngine::getInstance();

	return vuEngine->gameFrameStarted;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::pause(GameState pauseState)
{
	VUEngine vuEngine = VUEngine::getInstance();

	ASSERT(!isDeleted(pauseState), "VUEngine::pause: null pauseState");

	if(!isDeleted(pauseState))
	{
		VUEngine::addState(pauseState);
		vuEngine->isPaused = true;
		// VUEngine::fireEvent(vuEngine, kEventGamePaused);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::unpause(GameState pauseState)
{
	VUEngine vuEngine = VUEngine::getInstance();

	ASSERT(!isDeleted(pauseState), "VUEngine::unpause: null pauseState");

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(vuEngine->stateMachine));

	ASSERT(pauseState == currentGameState, "VUEngine::unpause: pauseState sent is not the current one");

	if(NULL != pauseState && currentGameState == pauseState)
	{
		VUEngine::removeState(vuEngine, pauseState);
		vuEngine->isPaused = false;
		VUEngine::fireEvent(vuEngine, kEventGameUnpaused);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::setState(GameState gameState)
{
	VUEngine vuEngine = VUEngine::getInstance();

#ifdef __TOOLS
	vuEngine->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(vuEngine->stateMachine);
	StateMachine::addEventListener(vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), kEventStateMachineWillCleanStack);
	StateMachine::addEventListener(vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), kEventStateMachineCleanedStack);

	StateMachine::transitionTo
	(
		vuEngine->stateMachine, NULL != gameState ? State::safeCast(gameState) : NULL, kStateMachineCleanStack
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::addState(GameState gameState)
{
	VUEngine vuEngine = VUEngine::getInstance();

#ifdef __TOOLS
	vuEngine->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(vuEngine->stateMachine);
	StateMachine::addEventListener(vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), kEventStateMachineWillPushState);
	StateMachine::addEventListener(vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), kEventStateMachinePushedState);

	StateMachine::transitionTo(vuEngine->stateMachine, State::safeCast(gameState), kStateMachinePushState);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::changeState(GameState gameState)
{
	VUEngine vuEngine = VUEngine::getInstance();

#ifdef __TOOLS
	vuEngine->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(vuEngine->stateMachine);
	StateMachine::addEventListener(vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), kEventStateMachineWillSwapState);
	StateMachine::addEventListener(vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), kEventStateMachineSwapedState);

	StateMachine::transitionTo(vuEngine->stateMachine, State::safeCast(gameState), kStateMachineSwapState);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::isInToolState()
{
	int32 isInToolState = false;

#ifdef __TOOLS
	VUEngine vuEngine = VUEngine::getInstance();
	isInToolState = NULL != __GET_CAST(ToolState, vuEngine->currentGameState);
#endif

	return isInToolState;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::isInToolStateTransition()
{
	VUEngine vuEngine = VUEngine::getInstance();

	return vuEngine->isInToolStateTransition;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Clock VUEngine::getMessagingClock()
{
	VUEngine vuEngine = VUEngine::getInstance();

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getMessagingClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 VUEngine::getGameFrameDuration()
{
	return VIPManager::getGameFrameDuration(VIPManager::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::setGameFrameRate(uint16 gameFrameRate)
{
	if(__MAXIMUM_FPS < gameFrameRate)
	{
		gameFrameRate = __MAXIMUM_FPS;
	}

	FrameRate::setTarget(FrameRate::getInstance(), gameFrameRate);
	VIPManager::setFrameCycle(__MAXIMUM_FPS / gameFrameRate - 1);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::lockFrameRate()
{
	VUEngine vuEngine = VUEngine::getInstance();

	vuEngine->lockFrameRate = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::unlockFrameRate()
{
	VUEngine vuEngine = VUEngine::getInstance();

	vuEngine->lockFrameRate = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::setSaveDataManager(ListenerObject saveDataManager)
{
	VUEngine vuEngine = VUEngine::getInstance();

	vuEngine->saveDataManager = saveDataManager;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static ListenerObject VUEngine::getSaveDataManager()
{
	VUEngine vuEngine = VUEngine::getInstance();

	return vuEngine->saveDataManager;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::isPaused()
{
	VUEngine vuEngine = VUEngine::getInstance();

	return vuEngine->isPaused;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::wait(uint32 milliSeconds)
{
	TimerManager::wait(milliSeconds);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __ENABLE_PROFILER
static void VUEngine::startProfiling()
{
	Profiler::initialize();
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::onEvent(ListenerObject eventFirer, uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventVIPManagerFRAMESTART:
		{
			VUEngine::frameStarted(this, __MILLISECONDS_PER_SECOND / __MAXIMUM_FPS);

			return true;
		}

		case kEventVIPManagerGAMESTART:
		{
			VUEngine::gameFrameStarted(this, VIPManager::getGameFrameDuration(eventFirer));

			return true;
		}

		case kEventVIPManagerXPEND:
		{
			VUEngine::drawingStarted(this);
			
			return true;
		}

		case kEventStateMachineWillCleanStack:
		{
			if(StateMachine::safeCast(eventFirer) != this->stateMachine)
			{
				return false;
			}

			VUEngine::cleaniningStatesStack(this);

			return false;
		}
		case kEventStateMachineWillPushState:
		{
			if(StateMachine::safeCast(eventFirer) != this->stateMachine)
			{
				return false;
			}

			VUEngine::pushingState(this);	

			return false;
		}

		case kEventStateMachineWillSwapState:
		{
			if(StateMachine::safeCast(eventFirer) != this->stateMachine)
			{
				return false;
			}

			VUEngine::swappingState(this);	

			return false;
		}

		case kEventStateMachineWillPopState:
		{
			if(StateMachine::safeCast(eventFirer) != this->stateMachine)
			{
				return false;
			}

			VUEngine::poppingState(this);	

			return false;
		}

		case kEventStateMachineCleanedStack:
		case kEventStateMachinePushedState:
		case kEventStateMachineSwapedState:
		case kEventStateMachinePoppedState:
		{
			if(StateMachine::safeCast(eventFirer) != this->stateMachine)
			{
				return false;
			}

			VUEngine::changedState(this);	

			return false;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::handleMessage(Telegram telegram)
{
	ASSERT(!isDeleted(this->stateMachine), "VUEngine::handleMessage: NULL stateMachine");

	return StateMachine::handleMessage(this->stateMachine, telegram);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::constructor()
{
#ifndef __RELEASE
	// Restrict singleton access
	Singleton::secure();
#endif

	// Initialize hardware related stuff
	HardwareManager::initialize();

	// Always explicitly call the base's constructor 
	Base::constructor();

	this->stateMachine = new StateMachine(this);
	this->currentGameState = NULL;

	this->gameFrameStarted = false;
	this->currentGameCycleEnded = false;
	this->isPaused = false;
	this->isInToolStateTransition = false;
	this->lockFrameRate = true;

	this->saveDataManager = NULL;

	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerFRAMESTART);
	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerGAMESTART);
#ifdef __SHOW_PROCESS_NAME_DURING_XPEND
	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerXPEND);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::destructor()
{
	VUEngine::cleanUp(this);

	delete this->stateMachine;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::cleaniningStatesStack()
{
	HardwareManager::disableInterrupts();

	VIPManager::stopDisplaying(VIPManager::getInstance());
	VIPManager::stopDrawing(VIPManager::getInstance());

	VirtualList stateMachineStack = StateMachine::getStateStack(this->stateMachine);

	for(VirtualNode node = VirtualList::begin(stateMachineStack); NULL != node; node = VirtualNode::getNext(node))
	{
		GameState gameState = GameState::safeCast(VirtualNode::getData(node));

		MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(gameState));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::pushingState()
{
#ifdef __TOOLS
	this->isInToolStateTransition = NULL != __GET_CAST(ToolState, StateMachine::getNextState(this->stateMachine));
#endif

	HardwareManager::disableInterrupts();

	VIPManager::stopDisplaying(VIPManager::getInstance());
	VIPManager::stopDrawing(VIPManager::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::swappingState()
{
	HardwareManager::disableInterrupts();
	
	VIPManager::stopDisplaying(VIPManager::getInstance());
	VIPManager::stopDrawing(VIPManager::getInstance());

	if(!isDeleted(this->currentGameState))
	{
		MessageDispatcher::discardDelayedMessagesWithClock
		(
			MessageDispatcher::getInstance(), GameState::getMessagingClock(this->currentGameState)
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::poppingState()
{
	HardwareManager::disableInterrupts();

	VIPManager::stopDisplaying(VIPManager::getInstance());
	VIPManager::stopDrawing(VIPManager::getInstance());

	if(!isDeleted(this->currentGameState))
	{
		MessageDispatcher::discardDelayedMessagesWithClock
		(
			MessageDispatcher::getInstance(), GameState::getMessagingClock(this->currentGameState)
		);
	}

#ifdef __TOOLS
	this->isInToolStateTransition = NULL != __GET_CAST(ToolState, this->currentGameState);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::changedState()
{
	StateMachine::removeAllEventListeners(this->stateMachine);

	this->currentGameCycleEnded = true;
	this->gameFrameStarted = true;

	this->currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(GameState::isVersusMode(this->currentGameState))
	{
		CommunicationManager::startSyncCycle(CommunicationManager::getInstance());
	}

	VIPManager::startDrawing(VIPManager::getInstance());
	VIPManager::startDisplaying(VIPManager::getInstance());

	VUEngine::fireEvent(this, kEventNextStateSet);

	HardwareManager::enableInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::frameStarted(uint16 gameFrameDuration)
{
	static uint16 totalTime = 0;

	totalTime += gameFrameDuration;

	TimerManager::frameStarted(TimerManager::getInstance(), gameFrameDuration * __MICROSECONDS_PER_MILLISECOND);

	if(__MILLISECONDS_PER_SECOND <= totalTime)
	{
		if(NULL != this->events)
		{
			VUEngine::fireEvent(this, kEventVUEngineNextSecondStarted);
		}

		totalTime = 0;
	}

	_gameRandomSeed = Math::randomSeed();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::gameFrameStarted(uint16 gameFrameDuration)
{
	this->gameFrameStarted = true;

	ClockManager::update(ClockManager::getInstance(), gameFrameDuration);

#ifdef __PRINT_FRAMERATE
	bool printFPS = true;
#else
	bool printFPS = !this->lockFrameRate;
#endif

#ifdef __TOOLS
	printFPS = !VUEngine::isInToolState();
#endif

	FrameRate::gameFrameStarted(FrameRate::getInstance(), this->currentGameCycleEnded, printFPS);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::drawingStarted()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::removeState(GameState gameState)
{
#ifdef __TOOLS
	this->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), kEventStateMachineWillPopState);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), kEventStateMachinePoppedState);

	StateMachine::transitionTo(this->stateMachine, State::safeCast(gameState), kStateMachinePopState);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __TOOLS
void VUEngine::toggleTool(ToolState toolState)
{
	if(VUEngine::isInState(this, GameState::safeCast(toolState)))
	{
		VUEngine::removeState(this, GameState::safeCast(toolState));
	}
	else
	{
		if(VUEngine::isInToolState())
		{
			VUEngine::removeState(this, GameState::safeCast(toolState));
		}

		ToolState::setCurrentGameState(toolState, this->currentGameState);

		VUEngine::addState(GameState::safeCast(toolState));
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::checkIfToggleTool(const UserInput* userInput __attribute__((unused)))
{
#ifdef __TOOLS
	ToolState toolState = ToolState::get(userInput);

	if(NULL != toolState)
	{
		VUEngine::toggleTool(this, toolState);
		return true;
	}
#endif

	return false;

}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::updateFrameRate()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState())
	{
		return;
	}
#endif

	FrameRate::update(FrameRate::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void VUEngine::run(GameState currentGameState)
{
	if(isDeleted(currentGameState))
	{
		NM_ASSERT(false, "VUEngine::run: currentGameState is invalid");
		return;
	}

	VUEngine::setState(currentGameState);

	while(NULL != this->currentGameState)
	{
		this->gameFrameStarted = false;
		this->currentGameCycleEnded = false;

		VUEngine::updateFrameRate(this);

#ifdef __ENABLE_PROFILER
		Profiler::start();
#endif

		KeypadManager::readUserInput(KeypadManager::getInstance(), this->lockFrameRate);

#ifdef __TOOLS
		UserInput userInput = KeypadManager::getUserInput();

		if(VUEngine::checkIfToggleTool(this, &userInput))
		{
			continue;
		}
#endif

		StateMachine::update(this->stateMachine);

		this->currentGameCycleEnded = true;

		if(this->lockFrameRate)
		{
			//  Wait for the next game start
			while(!this->gameFrameStarted);			
		}
	}

	// Being a program running in an embedded system, there is no point in trying to 
	// shut down things, I'm the operative system!
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::isInState(GameState gameState)
{
	return StateMachine::getCurrentState(this->stateMachine) == State::safeCast(gameState);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::cleanUp()
{
	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerFRAMESTART);
	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerGAMESTART);
	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerXPEND);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// GLOBAL FUNCTIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 main(void)
{
	// This function is created by the transpiler
	extern void setupClasses();

	// Setup the classes' virtual tables
	setupClasses();

	// Make sure that everything is properly intialized before giving control to the game
	volatile VUEngine vuEngine = VUEngine::getInstance();

	// Run the game with the GameState returned by its entry point
	VUEngine::run(vuEngine, __GAME_ENTRY_POINT());

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
