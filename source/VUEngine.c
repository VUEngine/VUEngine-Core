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

#include <AnimationCoordinatorFactory.h>
#include <AnimationInspectorState.h>
#include <BgmapTextureManager.h>
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
#include <BodyManager.h>
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
VUEngine _vuEngine __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
uint32 _dispatchCycle = 0;
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VUEngine::init()
{
	_vuEngine = VUEngine::getInstance();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 main(void)
{
	return __GAME_ENTRY_POINT();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::handleMessage(Telegram telegram)
{
	ASSERT(this->stateMachine, "VUEngine::handleMessage: NULL stateMachine");

	return StateMachine::handleMessage(this->stateMachine, telegram);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::reset(bool resetSounds)
{
#ifdef __ENABLE_PROFILER
	Profiler::reset(Profiler::getInstance());
#endif

	HardwareManager::disableInterrupts();

	// Disable timer

	// Disable rendering
	VIPManager::lowerBrightness(this->vipManager);
	VIPManager::removePostProcessingEffects(this->vipManager);

	// Reset managers
	WireframeManager::reset();

	if(resetSounds)
	{
		SoundManager::reset(this->soundManager);
	}

	TimerManager::reset(this->timerManager);
	KeypadManager::reset();
	CommunicationManager::reset(this->communicationManager);
	StopwatchManager::reset();
	FrameRate::reset();

	// The order of reset for the graphics managers must not be changed!
	VIPManager::reset(this->vipManager);
	SpriteManager::reset();
	DirectDraw::reset();
	AnimationCoordinatorFactory::reset();

	HardwareManager::enableInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::resetClock()
{
	Clock::reset(this->clock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 VUEngine::start(GameState currentGameState)
{
	ASSERT(currentGameState, "VUEngine::start: currentGameState is NULL");

	// Initialize VPU and turn off the brightness
	VIPManager::lowerBrightness(this->vipManager);

	if(NULL == StateMachine::getCurrentState(this->stateMachine))
	{
		VUEngine::run(this, currentGameState);
	}
	else
	{
		ASSERT(false, "VUEngine::start: already started");
	}

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::pause(GameState pauseState)
{
	ASSERT(pauseState, "VUEngine::pause: null pauseState");

	if(pauseState)
	{
		VUEngine::addState(this, pauseState);
		this->isPaused = true;
		// VUEngine::fireEvent(this, kEventGamePaused);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::unpause(GameState pauseState)
{
	ASSERT(pauseState, "VUEngine::unpause: null pauseState");

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	ASSERT(pauseState == currentGameState, "VUEngine::unpause: pauseState sent is not the current one");

	if(NULL != pauseState && currentGameState == pauseState)
	{
		VUEngine::removeState(this, pauseState);
		this->isPaused = false;
		// VUEngine::fireEvent(this, kEventGameUnpaused);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::setState(GameState gameState)
{
#ifdef __TOOLS
	this->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener
	(
		this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::cleaniningStatesStack, 
		kEventStateMachineWillCleanStack
	);

	StateMachine::addEventListener
	(
		this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, 
		kEventStateMachineCleanedStack
	);

	StateMachine::transitionTo
	(
		this->stateMachine, NULL != gameState ? State::safeCast(gameState) : NULL, kStateMachineCleanStack
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::addState(GameState gameState)
{
#ifdef __TOOLS
	this->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener
	(
		this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::pushingState, 
		kEventStateMachineWillPushState
	);

	StateMachine::addEventListener
	(
		this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, 
		kEventStateMachinePushedState
	);

	StateMachine::transitionTo(this->stateMachine, State::safeCast(gameState), kStateMachinePushState);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::changeState(GameState gameState)
{
#ifdef __TOOLS
	this->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener
	(
		this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::swappingState, 
		kEventStateMachineWillSwapState
	);

	StateMachine::addEventListener
	(
		this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, 
		kEventStateMachineSwapedState
	);

	StateMachine::transitionTo(this->stateMachine, State::safeCast(gameState), kStateMachineSwapState);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::isInToolState()
{
	int32 isInToolState = false;

#ifdef __TOOLS
	isInToolState |= VUEngine::isInDebugMode(this);
	isInToolState |= VUEngine::isInStageEditor(this);
	isInToolState |= VUEngine::isInAnimationInspector(this);
	isInToolState |= VUEngine::isInSoundTest(this);
#endif

	return isInToolState;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::isInToolStateTransition()
{
	return this->isInToolStateTransition;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

GameState VUEngine::getCurrentState()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::safeCast(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

GameState VUEngine::getPreviousState()
{
	State state = StateMachine::getPreviousState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::safeCast(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

UIContainer VUEngine::getUIContainer()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		return GameState::getUIContainer(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getUIContainer(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Stage VUEngine::getStage()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		return GameState::getStage(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getStage(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

BodyManager VUEngine::getBodyManager()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getBodyManager(state);
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getBodyManager(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ColliderManager VUEngine::getColliderManager()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getColliderManager(state);
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getColliderManager(state);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

StateMachine VUEngine::getStateMachine()
{
	return this->stateMachine;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Clock VUEngine::getClock()
{
	return this->clock;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Clock VUEngine::getLogicsClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getLogicsClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Clock VUEngine::getMessagingClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getMessagingClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Clock VUEngine::getPhysicsClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicsClock(GameState::safeCast(state));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

char* VUEngine::getProcessName()
{
	return this->processName;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 VUEngine::getGameFrameDuration()
{
	return VIPManager::getGameFrameDuration(this->vipManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::setGameFrameRate(uint16 gameFrameRate)
{
	if(__MAXIMUM_FPS < gameFrameRate)
	{
		gameFrameRate = __MAXIMUM_FPS;
	}

	FrameRate::setTarget(gameFrameRate);
	VIPManager::setFrameCycle(this->vipManager, __MAXIMUM_FPS / gameFrameRate - 1);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::lockFrameRate()
{
	this->syncToVIP = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::unlockFrameRate()
{
	this->syncToVIP = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::enableKeypad()
{
	KeypadManager::enable();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::disableKeypad()
{
	KeypadManager::disable();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::setSaveDataManager(ListenerObject saveDataManager)
{
	this->saveDataManager = saveDataManager;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ListenerObject VUEngine::getSaveDataManager()
{
	return this->saveDataManager;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	VIPManager::pushFrontPostProcessingEffect(this->vipManager, postProcessingEffect, entity);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	VIPManager::pushBackPostProcessingEffect(this->vipManager, postProcessingEffect, entity);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::removePostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity)
{
	VIPManager::removePostProcessingEffect(this->vipManager, postProcessingEffect, entity);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::frameStarted(uint16 gameFrameDuration)
{
	static uint16 totalTime = 0;

	totalTime += gameFrameDuration;

	TimerManager::frameStarted(this->timerManager, gameFrameDuration * __MICROSECONDS_PER_MILLISECOND);

	if(__MILLISECONDS_PER_SECOND <= totalTime)
	{
		if(NULL != this->events)
		{
			VUEngine::fireEvent(this, kEventVUEngineNextSecondStarted);
		}
#ifdef __SHOW_TIMER_MANAGER_STATUS
		TimerManager::nextSecondStarted(this->timerManager);
#endif

		totalTime = 0;

#ifdef __SHOW_STREAMING_PROFILING

		if(!VUEngine::isInToolState(this))
		{
			Printing::resetCoordinates();
			Stage::print(VUEngine::getStage(this), 1, 1);
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
		if(!VUEngine::isInToolState(this))
		{
#ifdef __SHOW_DETAILED_MEMORY_POOL_STATUS
			MemoryPool::printDetailedUsage(30, 1);
#else
			MemoryPool::printResumedUsage(35, 1);
#endif
		}
#endif

#ifdef __SHOW_STACK_OVERFLOW_ALERT
		if(!VUEngine::isInToolState(this))
		{
			Printing::resetCoordinates();
			HardwareManager::printStackStatus((__SCREEN_WIDTH_IN_CHARS) - 25, 0, false);
		}
#endif
	}

#ifdef __TOOLS
	if(VUEngine::isInSoundTest(this))
	{
		SoundManager::printPlaybackTime(this->soundManager, 1, 6);
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::gameFrameStarted(uint16 gameFrameDuration)
{
	this->gameFrameStarted = true;

	VUEngine::focusCamera(this);

	GameState gameState = VUEngine::getCurrentState(this);
	
	if(!isDeleted(gameState))
	{
		GameState::transformUI(gameState);
	}

	ClockManager::update(gameFrameDuration);

#ifdef __PRINT_FRAMERATE
	bool printFPS = true;
#else
	bool printFPS = !this->syncToVIP;
#endif

	FrameRate::gameFrameStarted(this->currentGameCycleEnded, printFPS);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::isPaused()
{
	return this->isPaused;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::wait(uint32 milliSeconds)
{
	TimerManager::wait(this->timerManager, milliSeconds);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::prepareGraphics()
{
	SpriteManager::prepareAll();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __ENABLE_PROFILER
void VUEngine::startProfiling()
{
	Profiler::initialize(Profiler::getInstance());
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Make sure the memory pool is initialized now
	MemoryPool::getInstance();

	// Construct the general clock
	this->clock = new Clock();

	// Construct the game's state machine
	this->stateMachine = new StateMachine(this);

	this->gameFrameStarted = false;
	this->currentGameCycleEnded = false;
	this->isPaused = false;
	this->isInToolStateTransition = false;
	this->syncToVIP = true;

	// Make sure all managers are initialized now
	this->saveDataManager = NULL;
	this->vipManager = NULL;
	this->timerManager = NULL;
	this->communicationManager = NULL;
	this->soundManager = NULL;

#ifdef __TOOLS
	DebugState::getInstance();
	StageEditorState::getInstance();
	AnimationInspectorState::getInstance();
	SoundTestState::getInstance();
#endif

	// To make debugging easier
	this->processName = PROCESS_NAME_START_UP;

	// Setup engine parameters
	VUEngine::initialize(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::destructor()
{
	// Destroy the clocks
	Clock::destructor(this->clock);

	delete this->stateMachine;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::initialize()
{
	// Make sure all managers are initialized now
	this->vipManager = VIPManager::getInstance();
	this->timerManager = TimerManager::getInstance();
	this->communicationManager = CommunicationManager::getInstance();
	this->soundManager = SoundManager::getInstance();

	SpriteManager::reset();
	DirectDraw::reset();
	SRAMManager::reset(SRAMManager::getInstance());

	// Initialize hardware registries
	HardwareManager::initialize();

	// Make sure timer interrupts are enable
	TimerManager::configure(this->timerManager, __TIMER_100US, 10, kMS);

	// Reset sounds
	SoundManager::reset(this->soundManager);

	// Reset Rumble Pak
	RumbleManager::reset(RumbleManager::getInstance());

	// Start the game's general clock
	Clock::start(this->clock);

	// Enable interrupts
	HardwareManager::enableInterrupts();

	// Enable communications
#ifdef __ENABLE_COMMUNICATIONS
	CommunicationManager::enableCommunications(this->communicationManager, NULL, NULL);
#else
#ifdef __RELEASE
	VUEngine::wait(VUEngine::getInstance(), 4000);
#endif
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::removeState(GameState gameState)
{
#ifdef __TOOLS
	this->isInToolStateTransition = false;
#endif

	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener
	(
		this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::poppingState, kEventStateMachineWillPopState
	);

	StateMachine::addEventListener
	(
		this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, kEventStateMachinePoppedState
	);

	StateMachine::transitionTo(this->stateMachine, State::safeCast(gameState), kStateMachinePopState);
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

	VIPManager::stopDisplaying(this->vipManager);

	VIPManager::stopDrawing(this->vipManager);

	// Clean the game's stack
	// Pop states until the stack is empty
	VirtualList stateMachineStack = StateMachine::getStateStack(this->stateMachine);

	// Cancel all messages
	VirtualNode node = VirtualList::begin(stateMachineStack);

	for(; NULL != node; node = VirtualNode::getNext(node))
	{
		GameState gameState = GameState::safeCast(VirtualNode::getData(node));

		MessageDispatcher::discardDelayedMessagesWithClock(GameState::getMessagingClock(gameState));
		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
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

	VIPManager::stopDisplaying(this->vipManager);
	VIPManager::stopDrawing(this->vipManager);

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

	VIPManager::stopDisplaying(this->vipManager);
	VIPManager::stopDrawing(this->vipManager);

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock
		(
			GameState::getMessagingClock(currentGameState)
		);

		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
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

	VIPManager::stopDisplaying(this->vipManager);
	VIPManager::stopDrawing(this->vipManager);

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock
		(
			GameState::getMessagingClock(currentGameState)
		);

		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
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
		CommunicationManager::startSyncCycle(this->communicationManager);
	}

	// Make sure everything is properly rendered
	VUEngine::prepareGraphics(this);

	VIPManager::startDrawing(this->vipManager);
	VIPManager::startDisplaying(this->vipManager);

	// Fire event
	VUEngine::fireEvent(this, kEventNextStateSet);

	StopwatchManager::reset();
	FrameRate::reset();

	return false;
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
		if(VUEngine::isInToolState(this))
		{
			VUEngine::removeState(this, GameState::safeCast(toolState));
		}

		VUEngine::addState(this, GameState::safeCast(toolState));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::checkIfToggleTool(const UserInput* userInput)
{
	ToolState engineToolStates[] =
	{
#ifdef __TOOLS
		ToolState::safeCast(DebugState::getInstance()),
		ToolState::safeCast(StageEditorState::getInstance()),
		ToolState::safeCast(AnimationInspectorState::getInstance()),
		ToolState::safeCast(SoundTestState::getInstance()),
#endif
		NULL
	};

	int32 i = 0;

	for(; engineToolStates[i]; i++)
	{
		// Check code to access special feature
		if(ToolState::isKeyCombination(engineToolStates[i], userInput))
		{
			VUEngine::toggleTool(this, engineToolStates[i]);
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
			VUEngine::toggleTool(this, _userToolStates[i]);
			return true;
		}
	}

	return false;
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::updateFrameRate()
{
#ifdef __TOOLS
	if(VUEngine::isInToolState(this))
	{
		return;
	}
#endif

	FrameRate::update();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::processUserInput(GameState currentGameState)
{
	if(!KeypadManager::isEnabled())
	{
#ifdef __ENABLE_PROFILER
		Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
		return;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_INPUT;
#endif

	UserInput userInput = KeypadManager::readUserInput(this->syncToVIP);
	
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
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
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
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_PHYSICS);
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
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_TRANSFORMS);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::processCollisions(GameState gameState)
{
	// Process the collisions after the transformations have taken place
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->processName = PROCESS_NAME_COLLISIONS;
#endif

	// Process collisions
	GameState::processCollisions(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_COLLISIONS);
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
		Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_MESSAGES);
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
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_LOGIC);
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

	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_STREAMING);
#endif

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VUEngine::run(GameState currentGameState)
{
	// Set state
	VUEngine::setState(this, currentGameState);

	while(NULL != currentGameState)
	{
#ifdef __SHOW_VSU_MANAGER_STATUS
		VSUManager::print(VSUManager::getInstance(), 1, 1);
#endif
		this->gameFrameStarted = false;
		this->currentGameCycleEnded = false;

		VUEngine::updateFrameRate(this);

#ifdef __ENABLE_PROFILER
		HardwareManager::disableInterrupts();
		Profiler::start(Profiler::getInstance());
#endif

		// Generate random seed
		_gameRandomSeed = Math::randomSeed();

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
					this->currentGameCycleEnded = true;
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
	if(!VUEngine::isInToolState(this))
	{
#endif
		// Position the camera
		Camera::focus();
#ifdef __TOOLS
	}
#endif

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_CAMERA);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __TOOLS
bool VUEngine::isInState(GameState gameState)
{
	return StateMachine::getCurrentState(this->stateMachine) == State::safeCast(gameState);
}
bool VUEngine::isInDebugMode()
{
	return VUEngine::isInState(this, GameState::safeCast(DebugState::getInstance()));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::isInStageEditor()
{
	return VUEngine::isInState(this, GameState::safeCast(StageEditorState::getInstance()));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::isInAnimationInspector()
{
	return VUEngine::isInState(this, GameState::safeCast(AnimationInspectorState::getInstance()));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VUEngine::isInSoundTest()
{
	return VUEngine::isInState(this, GameState::safeCast(SoundTestState::getInstance()));
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
