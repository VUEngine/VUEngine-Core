/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <AnimationCoordinatorFactory.h>
#include <AnimationInspectorState.h>
#include <BgmapTextureManager.h>
#include <Camera.h>
#include <CommunicationManager.h>
#include <CharSetManager.h>
#include <Clock.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <DebugConfig.h>
#include <DebugState.h>
#include <DirectDraw.h>
#include <CollisionManager.h>
#include <FrameRate.h>
#include <GameState.h>
#include <HardwareManager.h>
#include <KeypadManager.h>
#include <MemoryPool.h>
#include <MessageDispatcher.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <PhysicalWorld.h>
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
#include <Utilities.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "VUEngine.h"


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

VUEngine _vuEngine __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = NULL;

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
uint32 _dispatchCycle = 0;
#endif

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

static void VUEngine::init()
{
	_vuEngine = VUEngine::getInstance();
}

// class's constructor
static bool VUEngine::isConstructed()
{
	return 0 < _singletonConstructed;
}

// class's constructor
void VUEngine::constructor()
{
	// check memory map before anything else
	HardwareManager::checkMemoryMap();

	// construct base object
	Base::constructor();

	// make sure the memory pool is initialized now
	MemoryPool::getInstance();

	// force construction now
	this->clockManager = ClockManager::getInstance();

	// construct the general clock
	this->clock = new Clock();

	// construct the game's state machine
	this->stateMachine = new StateMachine(this);

	this->nextGameCycleStarted = false;
	this->currentGameCycleEnded = false;
	this->isPaused = false;
	this->isToolStateTransition = false;

	// make sure all managers are initialized now
	this->saveDataManager = NULL;
	this->camera = NULL;
	this->keypadManager = NULL;
	this->vipManager = NULL;
	this->timerManager = NULL;
	this->communicationManager = NULL;
	this->frameRate = NULL;
	this->soundManager = NULL;

	Utilities::setClock(this->clock);
	Utilities::setKeypadManager(this->keypadManager);

#ifdef __TOOLS
	DebugState::getInstance();
	StageEditorState::getInstance();
	AnimationInspectorState::getInstance();
	SoundTestState::getInstance();
#endif

	// to make debugging easier
	this->lastProcessName = PROCESS_NAME_START_UP;

	// setup engine parameters
	VUEngine::initialize(this);
}

// class's destructor
void VUEngine::destructor()
{
	// destroy the clocks
	Clock::destructor(this->clock);

	delete this->stateMachine;

	Base::destructor();
}

// setup engine parameters
void VUEngine::initialize()
{
	// make sure all managers are initialized now
	this->camera = Camera::getInstance();
	this->keypadManager = KeypadManager::getInstance();
	this->vipManager = VIPManager::getInstance();
	this->timerManager = TimerManager::getInstance();
	this->communicationManager = CommunicationManager::getInstance();
	this->frameRate = FrameRate::getInstance();
	this->soundManager = SoundManager::getInstance();
	this->wireframeManager = WireframeManager::getInstance();
	this->spriteManager = SpriteManager::getInstance();

	SpriteManager::reset(this->spriteManager);
	DirectDraw::reset(DirectDraw::getInstance());
	SRAMManager::reset(SRAMManager::getInstance());

	// setup vectorInterrupts
	HardwareManager::setInterruptVectors(HardwareManager::getInstance());

	// clear sprite memory
	HardwareManager::clearScreen(HardwareManager::getInstance());

	// make sure timer interrupts are enable
	HardwareManager::setupTimer(HardwareManager::getInstance(), __TIMER_100US, 10, kMS);

	// Reset sounds
	SoundManager::reset(this->soundManager);

	// Reset Rumble Pak
	RumbleManager::reset(RumbleManager::getInstance());

	// start the game's general clock
	Clock::start(this->clock);

	// Enable interrupts
	HardwareManager::enableInterrupts();

	// Enable communications
#ifdef __ENABLE_COMMUNICATIONS
#ifdef __RELEASE
	CommunicationManager::enableCommunications(this->communicationManager, NULL, NULL, 2000);
#else
	CommunicationManager::enableCommunications(this->communicationManager, NULL, NULL, 500);
#endif
#else
#ifdef __RELEASE
	VUEngine::wait(VUEngine::getInstance(), 4000);
#endif
#endif
}

void VUEngine::debug()
{
#ifdef __SHOW_WIREFRAME_MANAGER_STATUS
	WireframeManager::print(this->wireframeManager, 1, 1);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_END_FRAME;
#endif

#ifdef __SHOW_SOUND_STATUS
	SoundManager::print(this->soundManager);
#endif
}

// set game's initial state
void VUEngine::start(GameState currentGameState)
{
	ASSERT(state, "VUEngine::start: initial state is NULL");

	// Initialize VPU and turn off the brightness
	HardwareManager::lowerBrightness(HardwareManager::getInstance());

	if(NULL == StateMachine::getCurrentState(this->stateMachine))
	{
		// Set state
		VUEngine::setState(this, currentGameState);

		while(NULL != currentGameState)
		{
			VUEngine::currentFrameStarted(this);

#ifdef __ENABLE_PROFILER
			HardwareManager::disableInterrupts();
			Profiler::start(Profiler::getInstance());
#endif

			currentGameState = VUEngine::run(this, currentGameState);

#ifdef __ENABLE_PROFILER
			HardwareManager::enableInterrupts();
#endif

			VUEngine::currentGameCycleEnded(this);

#ifndef __RELEASE
			VUEngine::debug(this);
#endif

			while(!this->nextGameCycleStarted)
			{
				// This breaks PCM playback but reports torn frames more accurately
				if(!this->nextGameCycleStarted)
				{
					// Halting the CPU seems to only affect the profiling in Mednafen
					// But still haven't tested it on hardware
#if !defined(__ENABLE_PROFILER) && !defined(__PRINT_FRAMERATE)
					HardwareManager::halt();
#endif
				}
			}
		}
	}
	else
	{
		ASSERT(false, "VUEngine::start: already started");
	}
}

void VUEngine::setState(GameState gameState)
{
	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::cleaniningStatesStack, kEventStateMachineWillCleanStack);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, kEventStateMachineCleanedStack);
	StateMachine::prepareTransition(this->stateMachine, NULL != gameState ? State::safeCast(gameState) : NULL, kStateMachineCleanStack);
}

void VUEngine::changeState(GameState gameState)
{
	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::swappingState, kEventStateMachineWillSwapState);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, kEventStateMachineSwapedState);
	StateMachine::prepareTransition(this->stateMachine, State::safeCast(gameState), kStateMachineSwapState);
}

void VUEngine::addState(GameState gameState)
{
	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::pushingState, kEventStateMachineWillPushState);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, kEventStateMachinePushedState);
	StateMachine::prepareTransition(this->stateMachine, State::safeCast(gameState), kStateMachinePushState);
}

void VUEngine::removeState(GameState gameState)
{
	StateMachine::removeAllEventListeners(this->stateMachine);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::poppingState, kEventStateMachineWillPopState);
	StateMachine::addEventListener(this->stateMachine, ListenerObject::safeCast(this), (EventListener)VUEngine::changedState, kEventStateMachinePoppedState);
	StateMachine::prepareTransition(this->stateMachine, State::safeCast(gameState), kStateMachinePopState);
}

void VUEngine::cleaniningStatesStack(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::displayOff(HardwareManager::getInstance());
	HardwareManager::disableRendering(HardwareManager::getInstance());

	// Clean the game's stack
	// pop states until the stack is empty
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

void VUEngine::pushingState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::displayOff(HardwareManager::getInstance());
	HardwareManager::disableRendering(HardwareManager::getInstance());
}

void VUEngine::swappingState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::displayOff(HardwareManager::getInstance());
	HardwareManager::disableRendering(HardwareManager::getInstance());

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(currentGameState));
		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
	}
}

void VUEngine::poppingState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STATE_SWAP;
#endif

	HardwareManager::displayOff(HardwareManager::getInstance());
	HardwareManager::disableRendering(HardwareManager::getInstance());

	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(!isDeleted(currentGameState))
	{
		// Discard delayed messages from the current state
		MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(currentGameState));
		MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
	}
}

void VUEngine::changedState(ListenerObject eventFirer)
{
	if(StateMachine::safeCast(eventFirer) != this->stateMachine)
	{
		return;
	}

	StateMachine::removeAllEventListeners(this->stateMachine);

	// Reset flags
	this->currentGameCycleEnded = true;
	this->nextGameCycleStarted = false;

	// Save current state
	GameState currentGameState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(GameState::isVersusMode(currentGameState))
	{
		CommunicationManager::startSyncCycle(this->communicationManager);
	}

	// Make sure everything is properly rendered
	SpriteManager::prepareAll(this->spriteManager);

	HardwareManager::enableRendering(HardwareManager::getInstance());
	HardwareManager::displayOn(HardwareManager::getInstance());

	// Fire event
	VUEngine::fireEvent(this, kEventNextStateSet);

	StopwatchManager::reset(StopwatchManager::getInstance());
	FrameRate::reset(this->frameRate);
}

// erase engine's current status
void VUEngine::reset(bool resetSounds)
{
#ifdef	__MEMORY_POOL_CLEAN_UP
	MemoryPool::cleanUp(MemoryPool::getInstance());
#endif

#ifdef __ENABLE_PROFILER
	Profiler::reset(Profiler::getInstance());
#endif

	HardwareManager::disableInterrupts();

	// Disable timer

	// disable rendering
	HardwareManager::lowerBrightness(HardwareManager::getInstance());
	HardwareManager::clearScreen(HardwareManager::getInstance());
	HardwareManager::setupColumnTable(HardwareManager::getInstance(), NULL);
	VIPManager::removePostProcessingEffects(this->vipManager);

	// reset managers
	WireframeManager::reset(this->wireframeManager);

	if(resetSounds)
	{
		SoundManager::reset(this->soundManager);
	}

	TimerManager::resetMilliseconds(this->timerManager);
	KeypadManager::reset(this->keypadManager);
	CommunicationManager::reset(this->communicationManager);
	StopwatchManager::reset(StopwatchManager::getInstance());
	FrameRate::reset(this->frameRate);

	// the order of reset for the graphics managers must not be changed!
	VIPManager::reset(this->vipManager);
	SpriteManager::reset(this->spriteManager);
	DirectDraw::reset(DirectDraw::getInstance());
	AnimationCoordinatorFactory::reset(AnimationCoordinatorFactory::getInstance());

	HardwareManager::enableInterrupts();
}

#ifdef __TOOLS
void VUEngine::openTool(ToolState toolState)
{
	if(VUEngine::isInToolState(this, toolState))
	{
		this->isToolStateTransition = true;
		StateMachine::popState(this->stateMachine);
		this->isToolStateTransition = false;
	}
	else
	{
		if(VUEngine::isInSpecialMode(this))
		{
			this->isToolStateTransition = true;
			StateMachine::popState(this->stateMachine);
			this->isToolStateTransition = false;
		}

		this->isToolStateTransition = true;
		StateMachine::pushState(this->stateMachine, State::safeCast(toolState));
		this->isToolStateTransition = false;
	}
}

bool VUEngine::checkIfOpenTool(const UserInput* userInput)
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
		// check code to access special feature
		if(ToolState::isKeyCombination(engineToolStates[i], userInput))
		{
			VUEngine::openTool(this, engineToolStates[i]);
			return true;
		}
	}

	extern const ToolState _userToolStates[];

	i = 0;

	for(; _userToolStates[i]; i++)
	{
		// check code to access special feature
		if(ToolState::isKeyCombination(_userToolStates[i], userInput))
		{
			VUEngine::openTool(this, _userToolStates[i]);
			return true;
		}
	}

	return false;
}

#endif


// process input data according to the actual game status
void VUEngine::processUserInput(GameState currentGameState)
{
	if(!KeypadManager::isEnabled(this->keypadManager))
	{
#ifdef __ENABLE_PROFILER
		Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
		return;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_INPUT;
#endif

	// poll the user's input
	UserInput userInput = KeypadManager::captureUserInput(this->keypadManager);
	
#ifdef __TOOLS
	if(VUEngine::checkIfOpenTool(this, &userInput))
	{
		return;
	}
#endif

	if(0 != (userInput.pressedKey + userInput.holdKey + userInput.releasedKey) || GameState::processUserInputRegardlessOfInput(currentGameState))
	{
		GameState::processUserInput(currentGameState, &userInput);
	}

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
}

void VUEngine::dispatchDelayedMessages()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_MESSAGES;
#endif

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	if(_dispatchCycle++ & 1)
	{
#endif

		MessageDispatcher::dispatchDelayedMessages(MessageDispatcher::getInstance());

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_MESSAGES);
#endif

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	}
#endif

}

// update game's logic subsystem
GameState VUEngine::updateLogic(GameState currentGameState)
{
#ifdef __TOOLS
	if(!VUEngine::isInSpecialMode(this))
	{
#endif
	// it is the update cycle
	ASSERT(this->stateMachine, "VUEngine::update: no state machine");
#ifdef __TOOLS
	}
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_LOGIC;
#endif

	// update the game's logic
	currentGameState = GameState::safeCast(StateMachine::update(this->stateMachine));

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_LOGIC);
#endif

	return currentGameState;
}

	// Update sound related logic
void VUEngine::updateSound()
{
	SoundManager::update(this->soundManager);

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_SOUND_SETUP;
#endif

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_SOUND_SETUP);
#endif
}

// update game's physics subsystem
void VUEngine::updatePhysics(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_PHYSICS;
#endif

	// simulate physics
	GameState::updatePhysics(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_PHYSICS);
#endif
}

void VUEngine::focusCamera()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_CAMERA;
#endif

#ifdef __TOOLS
	if(!VUEngine::isInSpecialMode(this))
	{
#endif
		// position the camera
		Camera::focus(this->camera, true);
#ifdef __TOOLS
	}
#endif

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_CAMERA);
#endif

}

void VUEngine::updateTransformations(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_TRANSFORMS;
#endif

	// apply world transformations
	GameState::transform(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_TRANSFORMS);
#endif
}

void VUEngine::updateCollisions(GameState gameState)
{
	// process the collisions after the transformations have taken place
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_COLLISIONS;
#endif

	// process collisions
	GameState::processCollisions(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_COLLISIONS);
#endif
}

bool VUEngine::stream(GameState gameState)
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STREAMING;
#endif

	bool result = GameState::stream(gameState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_STREAMING);
#endif

	return result;
}

void VUEngine::updateFrameRate()
{
#ifdef __TOOLS
	if(VUEngine::isInSpecialMode(this))
	{
		return;
	}
#endif
	FrameRate::update(this->frameRate);
}

void VUEngine::nextGameCycleStarted(uint16 gameFrameDuration)
{
	this->nextGameCycleStarted = true;

	ClockManager::update(this->clockManager, gameFrameDuration);

	FrameRate::gameFrameStarted(this->frameRate, this->currentGameCycleEnded);
}

void VUEngine::nextFrameStarted(uint16 gameFrameDuration)
{
	static uint16 totalTime = 0;

	totalTime += gameFrameDuration;

	TimerManager::nextFrameStarted(this->timerManager, gameFrameDuration * __MICROSECONDS_PER_MILLISECOND);

	if(__MILLISECONDS_PER_SECOND <= totalTime)
	{
		if(NULL != this->events)
		{
			VUEngine::fireEvent(this, kEventVUEngineNextSecondStarted);
		}
#ifdef __SHOW_TIMER_MANAGER_STATUS
		TimerManager::printStatus(this->timerManager, 1, 10);
		TimerManager::nextSecondStarted(this->timerManager);
#endif
		SoundManager::updateFrameRate(this->soundManager);

		totalTime = 0;

#ifdef __SHOW_STREAMING_PROFILING

		if(!VUEngine::isInSpecialMode(this))
		{
			Printing::resetCoordinates(Printing::getInstance());
			Stage::showStreamingProfiling(VUEngine::getStage(this), 1, 1);
		}
#endif

#ifdef __DEBUG
#ifdef __PRINT_DEBUG_ALERT
		Printing::text(Printing::getInstance(), "DEBUG MODE", 0, (__SCREEN_HEIGHT_IN_CHARS) - 1, NULL);
#endif
#endif

#ifdef __SHOW_CHAR_MEMORY_STATUS
		CharSetManager::print(CharSetManager::getInstance(), 1, 5);
#endif

#ifdef __SHOW_BGMAP_MEMORY_STATUS
		BgmapTextureManager::print(BgmapTextureManager::getInstance(), 1, 5);
		ParamTableManager::print(ParamTableManager::getInstance(), 1 + 27, 5);
#endif

#ifdef __SHOW_MEMORY_POOL_STATUS
		if(!VUEngine::isInSpecialMode(this))
		{
			Printing::resetCoordinates(Printing::getInstance());

#ifdef __SHOW_DETAILED_MEMORY_POOL_STATUS
			MemoryPool::printDetailedUsage(MemoryPool::getInstance(), 30, 1);
#else
			MemoryPool::printResumedUsage(MemoryPool::getInstance(), 35, 1);
#endif
		}
#endif

#ifdef __SHOW_STACK_OVERFLOW_ALERT
		if(!VUEngine::isInSpecialMode(this))
		{
			Printing::resetCoordinates(Printing::getInstance());
			HardwareManager::printStackStatus((__SCREEN_WIDTH_IN_CHARS) - 25, 0, false);
		}
#endif
	}

#ifdef __TOOLS
	if(VUEngine::isInSoundTest(this))
	{
		SoundManager::printPlaybackTime(this->soundManager);
	}
#endif
}

void VUEngine::currentFrameStarted()
{
	this->nextGameCycleStarted = false;
	this->currentGameCycleEnded = false;

	VUEngine::updateFrameRate(this);
}

void VUEngine::currentGameCycleEnded()
{
	this->currentGameCycleEnded = true;
}

bool VUEngine::hasCurrentFrameEnded()
{
	// raise flag to allow the next frame to start
	return this->currentGameCycleEnded;
}

GameState VUEngine::run(GameState currentGameState)
{
	// Generate random seed
	_gameRandomSeed = Utilities::randomSeed();

	// process user's input
	VUEngine::processUserInput(this, currentGameState);

	// simulate physics
	VUEngine::updatePhysics(this, currentGameState);

	// apply transformations
	VUEngine::updateTransformations(this, currentGameState);

	// process collisions
	VUEngine::updateCollisions(this, currentGameState);

	// focus the camera once collisions are resolved
	VUEngine::focusCamera(this);

	// dispatch delayed messages
	VUEngine::dispatchDelayedMessages(this);

	// stream after the logic to avoid having a very heady frame
	if(!VUEngine::stream(this, currentGameState))
	{
		// Update sound related logic
		VUEngine::updateSound(this);
	}

	// update game's logic
	return VUEngine::updateLogic(this, currentGameState);
}

#ifdef __REGISTER_LAST_PROCESS_NAME
void VUEngine::setLastProcessName(char* processName)
{
	this->lastProcessName = processName;
}
#endif

// process a telegram
bool VUEngine::handleMessage(Telegram telegram)
{
	ASSERT(this->stateMachine, "VUEngine::handleMessage: NULL stateMachine");

	return StateMachine::handleMessage(this->stateMachine, telegram);
}

// retrieve time
uint32 VUEngine::getTime()
{
	return Clock::getTime(this->clock);
}

// retrieve clock
Clock VUEngine::getClock()
{
	return this->clock;
}

void VUEngine::resetClock()
{
	Clock::reset(this->clock);
}

// retrieve in game clock
Clock VUEngine::getMessagingClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getMessagingClock(GameState::safeCast(state));
}

// retrieve animations' clock
Clock VUEngine::getUpdateClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getUpdateClock(GameState::safeCast(state));
}

// retrieve in physics' clock
Clock VUEngine::getPhysicsClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicsClock(GameState::safeCast(state));
}

uint16 VUEngine::getGameFrameDuration()
{
	return VIPManager::getGameFrameDuration(this->vipManager);
}

// retrieve last process' name
char* VUEngine::getLastProcessName()
{
	return this->lastProcessName;
}

void VUEngine::setGameFrameRate(uint16 gameFrameRate)
{
	if(__MAXIMUM_FPS < gameFrameRate)
	{
		gameFrameRate = __MAXIMUM_FPS;
	}

	FrameRate::setTarget(this->frameRate, gameFrameRate);
	VIPManager::setFrameCycle(this->vipManager, __MAXIMUM_FPS / gameFrameRate - 1);
}

#ifdef __TOOLS
bool VUEngine::isInToolState(ToolState toolState)
{
	return StateMachine::getCurrentState(this->stateMachine) == State::safeCast(toolState);
}

bool VUEngine::isInDebugMode()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)DebugState::getInstance();
}

bool VUEngine::isInStageEditor()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)StageEditorState::getInstance();
}

bool VUEngine::isInAnimationInspector()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)AnimationInspectorState::getInstance();
}

bool VUEngine::isInSoundTest()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)SoundTestState::getInstance();
}
#endif


// whether if a special mode is active
bool VUEngine::isInSpecialMode()
{
	int32 isInSpecialMode = false;

#ifdef __TOOLS
	isInSpecialMode |= VUEngine::isInDebugMode(this);
	isInSpecialMode |= VUEngine::isInStageEditor(this);
	isInSpecialMode |= VUEngine::isInAnimationInspector(this);
	isInSpecialMode |= VUEngine::isInSoundTest(this);
#endif

	return isInSpecialMode;
}

// whether if a special mode is being started
bool VUEngine::isEnteringToolState()
{
	return this->isToolStateTransition;
}

// whether if a special mode is being started
bool VUEngine::isExitingToolState()
{
	return this->isToolStateTransition;
}

// retrieve state machine, use with caution!!!
StateMachine VUEngine::getStateMachine()
{
	return this->stateMachine;
}

// retrieve the current level's stage
Stage VUEngine::getStage()
{
#ifdef __TOOLS
	if(VUEngine::isInSpecialMode(this))
	{
		return GameState::getStage(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getStage(GameState::safeCast(state));
}

// retrieve current state
GameState VUEngine::getCurrentState()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::safeCast(state);
}

PhysicalWorld VUEngine::getPhysicalWorld()
{
#ifdef __TOOLS
	if(VUEngine::isInSpecialMode(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getPhysicalWorld(state);
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicalWorld(state);
}

CollisionManager VUEngine::getCollisionManager()
{
#ifdef __TOOLS
	if(VUEngine::isInSpecialMode(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getCollisionManager(state);
	}
#endif

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getCollisionManager(state);
}

// pause
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

// resume game
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

// is game currently paused?
bool VUEngine::isPaused()
{
	return this->isPaused;
}

void VUEngine::disableKeypad()
{
	KeypadManager::disable(this->keypadManager);
}

void VUEngine::enableKeypad()
{
	KeypadManager::enable(this->keypadManager);
}

void VUEngine::pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::pushFrontPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void VUEngine::pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::pushBackPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void VUEngine::removePostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::removePostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void VUEngine::wait(uint32 milliSeconds)
{
	ASSERT(this, "VUEngine::wait: this null");

	TimerManager::wait(this->timerManager, milliSeconds);
}

/**
 * Register the current save data manager. Use NULL if none.
 *
 * @param saveDataManager
 */
void VUEngine::setSaveDataManager(ListenerObject saveDataManager)
{
	this->saveDataManager = saveDataManager;
}

ListenerObject VUEngine::getSaveDataManager()
{
	return this->saveDataManager;
}

#ifdef __ENABLE_PROFILER
void VUEngine::startProfiling()
{
	Profiler::initialize(Profiler::getInstance());
}
#endif

int32 __GAME_ENTRY_POINT(void);
int32 main(void)
{
	return __GAME_ENTRY_POINT();
}
