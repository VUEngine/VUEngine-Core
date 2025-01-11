/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with vuEngine source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <AnimationCoordinatorFactory.h>
#include <AnimationInspectorState.h>
#include <BgmapTextureManager.h>
#include <BodyManager.h>
#include <Camera.h>
#include <CommunicationManager.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <ClockManager.h>
#include <ColliderManager.h>
#include <DebugConfig.h>
#include <DebugState.h>
#include <DirectDraw.h>
#include <ColliderManager.h>
#include <FrameRate.h>
#include <GameState.h>
#include <HardwareManager.h>
#include <KeypadManager.h>
#include <MemoryPool.h>
#include <MessageDispatcher.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <Profiler.h>
#include <RumbleManager.h>
#include <SoundManager.h>
#include <SpriteManager.h>
#include <SRAMManager.h>
#include <Stage.h>
#include <StageEditor.h>
#include <StageEditorState.h>
#include <State.h>
#include <StateMachine.h>
#include <StopwatchManager.h>
#include <SoundTestState.h>
#include <Telegram.h>
#include <ToolState.h>
#include <TimerManager.h>
#include <UIContainer.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "VUEngine.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 __GAME_ENTRY_POINT(void);

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
uint32 _dispatchCycle = 0;
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// GLOBAL FUNCTIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 main(void)
{
	return __GAME_ENTRY_POINT();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::init()
{
	VUEngine::getInstance(NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::registerEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	VUEngine::addEventListener(vuEngine, listener, callback, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::unregisterEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	VUEngine::removeEventListener(vuEngine, listener, callback, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::receieveMessage(uint32 delay, ListenerObject sender, int32 message, void* extraInfo)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	return MessageDispatcher::dispatchMessage(delay, sender, ListenerObject::safeCast(vuEngine), message, extraInfo);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::reset(bool resetSounds)
{
#ifdef __ENABLE_PROFILER
	Profiler::reset();
#endif

	HardwareManager::disableInterrupts();

	HardwareManager::reset();
	KeypadManager::reset();
	StopwatchManager::reset();
	FrameRate::reset();
	VIPManager::reset();
	DirectDraw::reset();
	SpriteManager::reset();
	WireframeManager::reset();
	SRAMManager::reset();
	TimerManager::reset();
	RumbleManager::reset();
	CommunicationManager::reset();
	AnimationCoordinatorFactory::reset();

	if(resetSounds)
	{
		SoundManager::reset();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::resetClock()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	Clock::reset(vuEngine->clock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int32 VUEngine::start(GameState currentGameState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	ASSERT(currentGameState, "VUEngine::start: currentGameState is NULL");

	// Initialize VPU and turn off the brightness
	VIPManager::lowerBrightness();

	if(NULL == StateMachine::getCurrentState(vuEngine->stateMachine))
	{
		VUEngine::run(currentGameState);
	}
	else
	{
		ASSERT(false, "VUEngine::start: already started");
	}

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::pause(GameState pauseState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

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
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	ASSERT(!isDeleted(pauseState), "VUEngine::unpause: null pauseState");

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(vuEngine->stateMachine));

	ASSERT(pauseState == currentGameState, "VUEngine::unpause: pauseState sent is not the current one");

	if(NULL != pauseState && currentGameState == pauseState)
	{
		VUEngine::removeState(pauseState);
		vuEngine->isPaused = false;
		VUEngine::fireEvent(vuEngine, kEventGameUnpaused);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::setState(GameState gameState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

#ifdef __TOOLS
	vuEngine->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(vuEngine->stateMachine);
	StateMachine::addEventListener
	(
		vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::cleaniningStatesStack, 
		kEventStateMachineWillCleanStack
	);

	StateMachine::addEventListener
	(
		vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::changedState, 
		kEventStateMachineCleanedStack
	);

	StateMachine::transitionTo
	(
		vuEngine->stateMachine, NULL != gameState ? State::safeCast(gameState) : NULL, kStateMachineCleanStack
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::addState(GameState gameState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

#ifdef __TOOLS
	vuEngine->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(vuEngine->stateMachine);
	StateMachine::addEventListener
	(
		vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::pushingState, 
		kEventStateMachineWillPushState
	);

	StateMachine::addEventListener
	(
		vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::changedState, 
		kEventStateMachinePushedState
	);

	StateMachine::transitionTo(vuEngine->stateMachine, State::safeCast(gameState), kStateMachinePushState);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::changeState(GameState gameState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

#ifdef __TOOLS
	vuEngine->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(vuEngine->stateMachine);
	StateMachine::addEventListener
	(
		vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::swappingState, 
		kEventStateMachineWillSwapState
	);

	StateMachine::addEventListener
	(
		vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::changedState, 
		kEventStateMachineSwapedState
	);

	StateMachine::transitionTo(vuEngine->stateMachine, State::safeCast(gameState), kStateMachineSwapState);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::isInToolState()
{
	int32 isInToolState = false;

#ifdef __TOOLS
	isInToolState |= VUEngine::isInDebugMode();
	isInToolState |= VUEngine::isInStageEditor();
	isInToolState |= VUEngine::isInAnimationInspector();
	isInToolState |= VUEngine::isInSoundTest();
#endif

	return isInToolState;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::isInToolStateTransition()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	return vuEngine->isInToolStateTransition;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static GameState VUEngine::getCurrentState()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::safeCast(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static GameState VUEngine::getPreviousState()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	State state = StateMachine::getPreviousState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::safeCast(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static UIContainer VUEngine::getUIContainer()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

#ifdef __TOOLS
	if(VUEngine::isInToolState())
	{
		return GameState::getUIContainer(GameState::safeCast(StateMachine::getPreviousState(vuEngine->stateMachine)));
	}
#endif

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getUIContainer(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Stage VUEngine::getStage()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

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

static BodyManager VUEngine::getBodyManager()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

#ifdef __TOOLS
	if(VUEngine::isInToolState())
	{
		State state = StateMachine::getPreviousState(vuEngine->stateMachine);
		return isDeleted(state) ? NULL : GameState::getBodyManager(state);
	}
#endif

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getBodyManager(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static ColliderManager VUEngine::getColliderManager()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

#ifdef __TOOLS
	if(VUEngine::isInToolState())
	{
		State state = StateMachine::getPreviousState(vuEngine->stateMachine);
		return isDeleted(state) ? NULL : GameState::getColliderManager(state);
	}
#endif

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getColliderManager(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static StateMachine VUEngine::getStateMachine()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	return vuEngine->stateMachine;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Clock VUEngine::getClock()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	return vuEngine->clock;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Clock VUEngine::getLogicsClock()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getLogicsClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Clock VUEngine::getMessagingClock()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getMessagingClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Clock VUEngine::getPhysicsClock()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	State state = StateMachine::getCurrentState(vuEngine->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicsClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static char* VUEngine::getProcessName()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	return vuEngine->processName;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 VUEngine::getGameFrameDuration()
{
	return VIPManager::getGameFrameDuration();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::setGameFrameRate(uint16 gameFrameRate)
{
	if(__MAXIMUM_FPS < gameFrameRate)
	{
		gameFrameRate = __MAXIMUM_FPS;
	}

	FrameRate::setTarget(gameFrameRate);
	VIPManager::setFrameCycle(__MAXIMUM_FPS / gameFrameRate - 1);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::lockFrameRate()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	vuEngine->syncToVIP = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::unlockFrameRate()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	vuEngine->syncToVIP = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::enableKeypad()
{
	KeypadManager::enable();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::disableKeypad()
{
	KeypadManager::disable();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::setSaveDataManager(ListenerObject saveDataManager)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	vuEngine->saveDataManager = saveDataManager;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static ListenerObject VUEngine::getSaveDataManager()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	return vuEngine->saveDataManager;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	VIPManager::pushFrontPostProcessingEffect(postProcessingEffect, entity);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	VIPManager::pushBackPostProcessingEffect(postProcessingEffect, entity);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::removePostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	VIPManager::removePostProcessingEffect(postProcessingEffect, entity);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::frameStarted(uint16 gameFrameDuration)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	static uint16 totalTime = 0;

	totalTime += gameFrameDuration;

	TimerManager::frameStarted(gameFrameDuration * __MICROSECONDS_PER_MILLISECOND);

	if(__MILLISECONDS_PER_SECOND <= totalTime)
	{
		if(NULL != vuEngine->events)
		{
			VUEngine::fireEvent(vuEngine, kEventVUEngineNextSecondStarted);
		}

		totalTime = 0;

#ifndef __RELEASE
		VUEngine::printDebug();
#endif
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::gameFrameStarted(uint16 gameFrameDuration)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	vuEngine->gameFrameStarted = true;

	VUEngine::focusCamera();

	GameState gameState = VUEngine::getCurrentState();
	
	if(!isDeleted(gameState))
	{
		GameState::transformUI(gameState);
	}

	ClockManager::update(gameFrameDuration);

#ifdef __PRINT_FRAMERATE
	bool printFPS = true;
#else
	bool printFPS = !vuEngine->syncToVIP;
#endif

#ifdef __TOOLS
	printFPS = !VUEngine::isInToolState();
#endif

	FrameRate::gameFrameStarted(vuEngine->currentGameCycleEnded, printFPS);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::drawingStarted()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::isPaused()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

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

bool VUEngine::handleMessage(Telegram telegram)
{
	ASSERT(!isDeleted(this->stateMachine), "VUEngine::handleMessage: NULL stateMachine");

	return StateMachine::handleMessage(this->stateMachine, telegram);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::removeState(GameState gameState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

#ifdef __TOOLS
	vuEngine->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(vuEngine->stateMachine);
	StateMachine::addEventListener
	(
		vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::poppingState, kEventStateMachineWillPopState
	);

	StateMachine::addEventListener
	(
		vuEngine->stateMachine, ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::changedState, kEventStateMachinePoppedState
	);

	StateMachine::transitionTo(vuEngine->stateMachine, State::safeCast(gameState), kStateMachinePopState);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __TOOLS
static void VUEngine::toggleTool(ToolState toolState)
{
	if(VUEngine::isInState(GameState::safeCast(toolState)))
	{
		VUEngine::removeState(GameState::safeCast(toolState));
	}
	else
	{
		if(VUEngine::isInToolState())
		{
			VUEngine::removeState(GameState::safeCast(toolState));
		}

		VUEngine::addState(GameState::safeCast(toolState));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::checkIfToggleTool(const UserInput* userInput)
{
	ToolState engineToolStates[] =
	{
#ifdef __TOOLS
		ToolState::safeCast(DebugState::getInstance(NULL)),
		ToolState::safeCast(StageEditorState::getInstance(NULL)),
		ToolState::safeCast(AnimationInspectorState::getInstance(NULL)),
		ToolState::safeCast(SoundTestState::getInstance(NULL)),
#endif
		NULL
	};

	int32 i = 0;

	for(; engineToolStates[i]; i++)
	{
		// Check code to access special feature
		if(ToolState::isKeyCombination(engineToolStates[i], userInput))
		{
			VUEngine::toggleTool(engineToolStates[i]);
			return true;
		}
	}

	extern const ToolState _userToolStates[];

	i = 0;

	for(; _userToolStates[i]; i++)
	{
		// Check code to access special feature
		if(ToolState::isKeyCombination(_userToolStates[i], userInput))
		{
			VUEngine::toggleTool(_userToolStates[i]);
			return true;
		}
	}

	return false;
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::updateFrameRate()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState())
	{
		return;
	}
#endif

	FrameRate::update();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::processUserInput(GameState currentGameState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	if(!KeypadManager::isEnabled())
	{
#ifdef __ENABLE_PROFILER
		Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
		return;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	vuEngine->processName = PROCESS_NAME_INPUT;
#endif

	UserInput userInput = KeypadManager::readUserInput(vuEngine->syncToVIP);
	
#ifdef __TOOLS
	if(VUEngine::checkIfToggleTool(&userInput))
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

static void VUEngine::simulatePhysics(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	VUEngine vuEngine = VUEngine::getInstance(NULL);
	vuEngine->processName = PROCESS_NAME_PHYSICS;
#endif

	// Simulate physics
	GameState::simulatePhysics(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_PHYSICS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::processTransformations(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	VUEngine vuEngine = VUEngine::getInstance(NULL);
	vuEngine->processName = PROCESS_NAME_TRANSFORMS;
#endif

	// Apply world transformations
	GameState::transform(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_TRANSFORMS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::processCollisions(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	VUEngine vuEngine = VUEngine::getInstance(NULL);
	vuEngine->processName = PROCESS_NAME_COLLISIONS;
#endif

	// Process collisions
	GameState::processCollisions(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_COLLISIONS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::dispatchDelayedMessages()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	VUEngine vuEngine = VUEngine::getInstance(NULL);
	vuEngine->processName = PROCESS_NAME_MESSAGES;
#endif

#ifndef __ENABLE_PROFILER
#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	if(_dispatchCycle++ & 1)
#endif
#endif
	{
		MessageDispatcher::dispatchDelayedMessages();

#ifdef __ENABLE_PROFILER
		Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_MESSAGES);
#endif
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static GameState VUEngine::updateLogic(GameState currentGameState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

#ifdef __REGISTER_LAST_PROCESS_NAME
	vuEngine->processName = PROCESS_NAME_LOGIC;
#endif

	// Update the game's logic
	currentGameState = GameState::safeCast(StateMachine::update(vuEngine->stateMachine));

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_LOGIC);
#endif

	return currentGameState;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::stream(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	VUEngine vuEngine = VUEngine::getInstance(NULL);
	vuEngine->processName = PROCESS_NAME_STREAMING;
#endif

#ifndef __ENABLE_PROFILER
	bool result = GameState::stream(gameState);
#else
	bool result = false;

	// While we wait for the next game start
	while(!vuEngine->gameFrameStarted)
	{
		// Stream the heck out of the pending actors
		result = GameState::stream(gameState);
	}

	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_STREAMING);
#endif

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::run(GameState currentGameState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	// Set state
	VUEngine::setState(currentGameState);

	while(NULL != currentGameState)
	{
#ifdef __SHOW_VSU_MANAGER_STATUS
		VSUManager::print(1, 1);
#endif
		vuEngine->gameFrameStarted = false;
		vuEngine->currentGameCycleEnded = false;

		VUEngine::updateFrameRate();

#ifdef __ENABLE_PROFILER
		HardwareManager::disableInterrupts();
		Profiler::start();
#endif

		// Generate random seed
		_gameRandomSeed = Math::randomSeed();

		// Process user's input
		VUEngine::processUserInput(currentGameState);

		// Simulate physics
		VUEngine::simulatePhysics(currentGameState);

		// Apply transformations
		VUEngine::processTransformations(currentGameState);

		// Process collisions
		VUEngine::processCollisions(currentGameState);

		// Dispatch delayed messages
		VUEngine::dispatchDelayedMessages();

		currentGameState = VUEngine::updateLogic(currentGameState);

#ifdef __ENABLE_PROFILER
		HardwareManager::enableInterrupts();

		// Stream actors
		VUEngine::stream(currentGameState);
#else
		if(!vuEngine->syncToVIP)
		{
			while(VUEngine::stream(currentGameState));
		}
		else
		{
			do
			{
				// Stream the heck out of the pending actors
				if(!VUEngine::stream(currentGameState))
				{
					vuEngine->currentGameCycleEnded = true;
				}
			}
			// While we wait for the next game start
			while(!vuEngine->gameFrameStarted);			
		}
#endif

		vuEngine->currentGameCycleEnded = true;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::focusCamera()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	VUEngine vuEngine = VUEngine::getInstance(NULL);
	vuEngine->processName = PROCESS_NAME_CAMERA;
#endif

#ifdef __TOOLS
	if(!VUEngine::isInToolState())
	{
#endif
		// Position the camera
		Camera::focus();
#ifdef __TOOLS
	}
#endif

#ifdef __ENABLE_PROFILER
	Profiler::lap(kProfilerLapTypeNormalProcess, PROCESS_NAME_CAMERA);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __TOOLS
static bool VUEngine::isInState(GameState gameState)
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	return StateMachine::getCurrentState(vuEngine->stateMachine) == State::safeCast(gameState);
}
static bool VUEngine::isInDebugMode()
{
	return VUEngine::isInState(GameState::safeCast(DebugState::getInstance(NULL)));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::isInStageEditor()
{
	return VUEngine::isInState(GameState::safeCast(StageEditorState::getInstance(NULL)));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::isInAnimationInspector()
{
	return VUEngine::isInState(GameState::safeCast(AnimationInspectorState::getInstance(NULL)));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VUEngine::isInSoundTest()
{
	return VUEngine::isInState(GameState::safeCast(SoundTestState::getInstance(NULL)));
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::cleanUp()
{
	VUEngine vuEngine = VUEngine::getInstance(NULL);

	VIPManager::unregisterEventListener
	(
		ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::onVIPFRAMESTART, 
		kEventVIPManagerFRAMESTART
	);

	VIPManager::unregisterEventListener
	(
		ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::onVIPGAMESTART, 
		kEventVIPManagerGAMESTART
	);

	VIPManager::unregisterEventListener
	(
		ListenerObject::safeCast(vuEngine), (EventListener)VUEngine::onVIPXPEND, 
		kEventVIPManagerXPEND
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::printDebug()
{
#ifdef __SHOW_TIMER_MANAGER_STATUS
	TimerManager::nextSecondStarted();
#endif

#ifdef __SHOW_STREAMING_PROFILING

	if(!VUEngine::isInToolState())
	{
		Printing::resetCoordinates();
		Stage::print(VUEngine::getStage(), 1, 1);
	}
#endif

#ifdef __DEBUG
#ifdef __PRINT_DEBUG_ALERT
	Printing::text(EN_HEIGHT_IN_CHARS) - 1, NULL);
#endif
#endif

#ifdef __SHOW_CHAR_MEMORY_STATUS
	CharSetManager::print(1, 5);
#endif

#ifdef __SHOW_BGMAP_MEMORY_STATUS
	BgmapTextureManager::print(1, 5);
	ParamTableManager::print(1 + 27, 5);
#endif

#ifdef __SHOW_MEMORY_POOL_STATUS
	if(!VUEngine::isInToolState())
	{
#ifdef __SHOW_DETAILED_MEMORY_POOL_STATUS
		MemoryPool::printDetailedUsage(30, 1);
#else
		MemoryPool::printResumedUsage(35, 1);
#endif
	}
#endif

#ifdef __SHOW_STACK_OVERFLOW_ALERT
	if(!VUEngine::isInToolState())
	{
		Printing::resetCoordinates();
		HardwareManager::printStackStatus((__SCREEN_WIDTH_IN_CHARS) - 25, 0, false);
	}
#endif

#ifdef __TOOLS
	if(VUEngine::isInSoundTest())
	{
		SoundManager::printPlaybackTime(1, 6);
	}
#endif
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

	VIPManager::registerEventListener
	(
		ListenerObject::safeCast(this), (EventListener)VUEngine::onVIPFRAMESTART, 
		kEventVIPManagerFRAMESTART
	);

	VIPManager::registerEventListener
	(
		ListenerObject::safeCast(this), (EventListener)VUEngine::onVIPGAMESTART,
		kEventVIPManagerGAMESTART
	);

#ifdef __SHOW_PROCESS_NAME_DURING_XPEND
	VIPManager::registerEventListener
	(
		ListenerObject::safeCast(this), (EventListener)VUEngine::onVIPXPEND, 
		kEventVIPManagerXPEND
	);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::destructor()
{
	VUEngine::cleanUp();

	// Destroy the clocks
	Clock::destructor(this->clock);

	delete this->stateMachine;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::cleaniningStatesStack(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_STATE_SWAP;
#endif

	VIPManager::stopDisplaying();

	VIPManager::stopDrawing();

	// Clean the game's stack
	// Pop states until the stack is empty
	VirtualList stateMachineStack = StateMachine::getStateStack(this->stateMachine);

	// Cancel all messages
	VirtualNode node = VirtualList::begin(stateMachineStack);

	for(; NULL != node; node = VirtualNode::getNext(node))
	{
		GameState gameState = GameState::safeCast(VirtualNode::getData(node));

		MessageDispatcher::discardDelayedMessagesWithClock(GameState::getMessagingClock(gameState));
		MessageDispatcher::processDiscardedMessages();
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::pushingState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_STATE_SWAP;
#endif

#ifdef __TOOLS
	this->isInToolStateTransition = NULL != __GET_CAST(ToolState, StateMachine::getNextState(this->stateMachine));
#endif

	VIPManager::stopDisplaying();
	VIPManager::stopDrawing();

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::swappingState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_STATE_SWAP;
#endif

	VIPManager::stopDisplaying();
	VIPManager::stopDrawing();

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock
		(
			GameState::getMessagingClock(currentGameState)
		);

		MessageDispatcher::processDiscardedMessages();
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::poppingState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_STATE_SWAP;
#endif

	VIPManager::stopDisplaying();
	VIPManager::stopDrawing();

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock
		(
			GameState::getMessagingClock(currentGameState)
		);

		MessageDispatcher::processDiscardedMessages();
	}

#ifdef __TOOLS
	this->isInToolStateTransition = NULL != __GET_CAST(ToolState, currentGameState);
#endif

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::changedState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return false;
	}

	StateMachine::removeAllEventListeners(this->stateMachine);

	// Reset flags
	this->currentGameCycleEnded = true;
	this->gameFrameStarted = true;

	// Save current state
	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(GameState::isVersusMode(currentGameState))
	{
		CommunicationManager::startSyncCycle();
	}

	VIPManager::startDrawing();
	VIPManager::startDisplaying();
	HardwareManager::enableInterrupts();

	// Fire event
	VUEngine::fireEvent(this, kEventNextStateSet);

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::onVIPFRAMESTART(ListenerObject eventFirer __attribute__ ((unused)))
{
#ifdef __SHOW_PROCESS_NAME_DURING_FRAMESTART
	PRINT_TEXT("F START:            ", 0, 27);
	PRINT_TEXT(vuEngine->processName, 9, 27);
#endif

	VUEngine::frameStarted(__MILLISECONDS_PER_SECOND / __MAXIMUM_FPS);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::onVIPGAMESTART(ListenerObject eventFirer __attribute__ ((unused)))
{
#ifdef __SHOW_PROCESS_NAME_DURING_GAMESTART
	PRINT_TEXT("G START:           ", 0, 26);
	PRINT_TEXT(vuEngine->processName, 9, 26);
#endif

	VUEngine::gameFrameStarted(VIPManager::getGameFrameDuration());

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::onVIPXPEND(ListenerObject eventFirer __attribute__ ((unused)))
{
#ifdef __SHOW_PROCESS_NAME_DURING_XPEND
	PRINT_TEXT("XPEND:            ", 0, 27);
	PRINT_TEXT(vuEngine->processName, 9, 26);
#endif

	VUEngine::drawingStarted();
	
	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
