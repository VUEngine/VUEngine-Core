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

static void VUEngine::resetClock()
{
	// This is strictly not necessary, since the singleton instance could be
	// directly accessed by setting a static global in the constructor that points to it.
	// But this looks subjectively better and is inlined by the compiler anyway.
	VUEngine vuEngine = VUEngine::getInstance();

	Clock::reset(vuEngine->clock);
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
	isInToolState = NULL != __GET_CAST(ToolState, VUEngine::getCurrentState());
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

static GameState VUEngine::getCurrentState()
{
	VUEngine vuEngine = VUEngine::getInstance();

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::safeCast(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static GameState VUEngine::getPreviousState()
{
	VUEngine vuEngine = VUEngine::getInstance();

	State state = StateMachine::getPreviousState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::safeCast(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Stage VUEngine::getStage()
{
	VUEngine vuEngine = VUEngine::getInstance();

#ifdef __TOOLS
	if(VUEngine::isInToolState())
	{
		return GameState::getStage(GameState::safeCast(StateMachine::getPreviousState(vuEngine->stateMachine)));
	}
#endif

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getStage(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Clock VUEngine::getClock()
{
	VUEngine vuEngine = VUEngine::getInstance();

	return vuEngine->clock;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Clock VUEngine::getLogicsClock()
{
	VUEngine vuEngine = VUEngine::getInstance();

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getLogicsClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Clock VUEngine::getMessagingClock()
{
	VUEngine vuEngine = VUEngine::getInstance();

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getMessagingClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Clock VUEngine::getPhysicsClock()
{
	VUEngine vuEngine = VUEngine::getInstance();

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicsClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static char* VUEngine::getProcessName()
{
	VUEngine vuEngine = VUEngine::getInstance();

	return vuEngine->processName;
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

	vuEngine->syncToVIP = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::unlockFrameRate()
{
	VUEngine vuEngine = VUEngine::getInstance();

	vuEngine->syncToVIP = false;
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
#ifdef __SHOW_PROCESS_NAME_DURING_FRAMESTART
			PRINT_TEXT("F START:            ", 0, 27);
			PRINT_TEXT(vuEngine->processName, 9, 27);
#endif

			VUEngine::frameStarted(this, __MILLISECONDS_PER_SECOND / __MAXIMUM_FPS);

			return true;
		}

		case kEventVIPManagerGAMESTART:
		{
#ifdef __SHOW_PROCESS_NAME_DURING_GAMESTART
			PRINT_TEXT("G START:           ", 0, 26);
			PRINT_TEXT(vuEngine->processName, 9, 26);
#endif

			VUEngine::gameFrameStarted(this, VIPManager::getGameFrameDuration(eventFirer));

			return true;
		}

		case kEventVIPManagerXPEND:
		{
#ifdef __SHOW_PROCESS_NAME_DURING_XPEND
			PRINT_TEXT("XPEND:            ", 0, 27);
			PRINT_TEXT(vuEngine->processName, 9, 26);
#endif

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
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Construct the general clock
	this->clock = new Clock();
	Clock::start(this->clock);

	// Construct the game's state machine
	this->stateMachine = new StateMachine(this);

	this->gameFrameStarted = false;
	this->currentGameCycleEnded = false;
	this->isPaused = false;
	this->isInToolStateTransition = false;
	this->syncToVIP = true;

	// Make sure all managers are initialized now
	this->saveDataManager = NULL;

	// To make debugging easier
	this->processName = PROCESS_NAME_START_UP;

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

	// Destroy the clocks
	Clock::destructor(this->clock);

	delete this->stateMachine;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//—————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure int32 VUEngine::start(GameState currentGameState)
{
	if(isDeleted(currentGameState))
	{
		NM_ASSERT(false, "VUEngine::start: currentGameState is invalid");
		return 0;
	}

	if(NULL == StateMachine::getCurrentState(this->stateMachine))
	{
		VUEngine::run(this, currentGameState);
	}
	else
	{
		NM_ASSERT(false, "VUEngine::start: already started");
	}

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::cleaniningStatesStack()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::disableInterrupts();

	VIPManager::stopDisplaying(VIPManager::getInstance());
	VIPManager::stopDrawing(VIPManager::getInstance());

	// Clean the game's stack
	// Pop states until the stack is empty
	VirtualList stateMachineStack = StateMachine::getStateStack(this->stateMachine);

	// Cancel all messages
	VirtualNode node = VirtualList::begin(stateMachineStack);

	for(; NULL != node; node = VirtualNode::getNext(node))
	{
		GameState gameState = GameState::safeCast(VirtualNode::getData(node));

		MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(gameState));
		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::pushingState()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_STATE_SWAP;
#endif

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
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::disableInterrupts();
	
	VIPManager::stopDisplaying(VIPManager::getInstance());
	VIPManager::stopDrawing(VIPManager::getInstance());

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock
		(
			MessageDispatcher::getInstance(), GameState::getMessagingClock(currentGameState)
		);

		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::poppingState()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::disableInterrupts();

	VIPManager::stopDisplaying(VIPManager::getInstance());
	VIPManager::stopDrawing(VIPManager::getInstance());

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock
		(
			MessageDispatcher::getInstance(), GameState::getMessagingClock(currentGameState)
		);

		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
	}

#ifdef __TOOLS
	this->isInToolStateTransition = NULL != __GET_CAST(ToolState, currentGameState);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::changedState()
{
	StateMachine::removeAllEventListeners(this->stateMachine);

	// Reset flags
	this->currentGameCycleEnded = true;
	this->gameFrameStarted = true;

	// Save current state
	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(GameState::isVersusMode(currentGameState))
	{
		CommunicationManager::startSyncCycle(CommunicationManager::getInstance());
	}

	VIPManager::startDrawing(VIPManager::getInstance());
	VIPManager::startDisplaying(VIPManager::getInstance());

	// Fire event
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

	// Update random seed
	_gameRandomSeed = Math::randomSeed();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::gameFrameStarted(uint16 gameFrameDuration)
{
	this->gameFrameStarted = true;

	VUEngine::focusCamera(this);

	GameState gameState = VUEngine::getCurrentState();
	
	if(!isDeleted(gameState))
	{
		GameState::transformUI(gameState);
	}

	ClockManager::update(ClockManager::getInstance(), gameFrameDuration);

#ifdef __PRINT_FRAMERATE
	bool printFPS = true;
#else
	bool printFPS = !this->syncToVIP;
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

void VUEngine::processUserInput(GameState currentGameState)
{
	if(!KeypadManager::isEnabled(KeypadManager::getInstance()))
	{
#ifdef __ENABLE_PROFILER
		Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
		return;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_INPUT;
#endif

	UserInput userInput = KeypadManager::readUserInput(KeypadManager::getInstance(), this->syncToVIP);
	
#ifdef __TOOLS
	if(VUEngine::checkIfToggleTool(this, &userInput))
	{
		return;
	}
#endif

	if(0 != (userInput.dummyKey | userInput.pressedKey | userInput.holdKey | userInput.releasedKey))
	{
		GameState::processUserInput(currentGameState, &userInput);
	}

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::simulatePhysics(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_PHYSICS;
#endif

	// Simulate physics
	GameState::simulatePhysics(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_PHYSICS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::processTransformations(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_TRANSFORMS;
#endif

	// Apply world transformations
	GameState::transform(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_TRANSFORMS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::processCollisions(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_COLLISIONS;
#endif

	// Process collisions
	GameState::processCollisions(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_COLLISIONS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::dispatchDelayedMessages()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_MESSAGES;
#endif

#ifndef __ENABLE_PROFILER
#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	if(_dispatchCycle++ & 1)
#endif
#endif
	{
		MessageDispatcher::dispatchDelayedMessages(MessageDispatcher::getInstance());

#ifdef __ENABLE_PROFILER
		Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_MESSAGES);
#endif
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

GameState VUEngine::updateLogic(GameState currentGameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_LOGIC;
#endif

	// Update the game's logic
	currentGameState = GameState::safeCast(StateMachine::update(this->stateMachine));

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_LOGIC);
#endif

	return currentGameState;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::stream(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_STREAMING;
#endif

#ifndef __ENABLE_PROFILER
	bool result = GameState::stream(gameState);
#else
	bool result = false;

	// While we wait for the next game start
	while(!this->gameFrameStarted)
	{
		// Stream the heck out of the pending actors
		result = GameState::stream(gameState);
	}

	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_STREAMING);
#endif

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::run(GameState currentGameState)
{
	VUEngine::setState(currentGameState);

	while(NULL != currentGameState)
	{
		this->gameFrameStarted = false;
		this->currentGameCycleEnded = false;

		VUEngine::updateFrameRate(this);

#ifdef __ENABLE_PROFILER
		HardwareManager::disableInterrupts();
		Profiler::start();
#endif

		// Process user's input
		VUEngine::processUserInput(this, currentGameState);

		// Simulate physics
		VUEngine::simulatePhysics(this, currentGameState);

		// Apply transformations
		VUEngine::processTransformations(this, currentGameState);

		// Process collisions
		VUEngine::processCollisions(this, currentGameState);

		// Dispatch delayed messages
		VUEngine::dispatchDelayedMessages(this);

		currentGameState = VUEngine::updateLogic(this, currentGameState);

#ifdef __ENABLE_PROFILER
		HardwareManager::enableInterrupts();

		// Stream actors
		VUEngine::stream(this, currentGameState);
#else
		if(!this->syncToVIP)
		{
			while(VUEngine::stream(this, currentGameState));
		}
		else
		{
			do
			{
				// Stream the heck out of the pending actors
				if(!VUEngine::stream(this, currentGameState))
				{
#ifndef __ENABLE_PROFILER
					this->currentGameCycleEnded = true;
#ifndef __DEBUG
					if(!this->gameFrameStarted)
					{
						// Don't spin cycle the CPU
						HardwareManager::halt();
					}
#endif
#endif
				}
			}
			// While we wait for the next game start
			while(!this->gameFrameStarted);			
		}
#endif

		this->currentGameCycleEnded = true;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::focusCamera()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_CAMERA;
#endif

#ifdef __TOOLS
	if(!VUEngine::isInToolState())
	{
#endif
		// Position the camera
		Camera::focus(Camera::getInstance());
#ifdef __TOOLS
	}
#endif

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_CAMERA);
#endif
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

#ifndef __RELEASE
	// Restrict singleton access
	Singleton::secure();
#endif

	// Initialize hardware related stuff
	HardwareManager::initialize();

	// Start the game
	return VUEngine::start(VUEngine::getInstance(), __GAME_ENTRY_POINT());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
