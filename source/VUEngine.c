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
#include <VUEngine.h>
#include <SRAMManager.h>
#include <HardwareManager.h>
#include <StopwatchManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <MiscStructs.h>
#include <FrameRate.h>
#include <Clock.h>
#include <BgmapTextureManager.h>
#include <WireframeManager.h>
#include <GameState.h>
#include <Utilities.h>
#include <MessageDispatcher.h>
#include <Stage.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <AnimationCoordinatorFactory.h>
#include <StateMachine.h>
#include <Camera.h>
#include <CameraMovementManager.h>
#include <KeypadManager.h>
#include <SoundManager.h>
#include <RumbleManager.h>
#include <TimerManager.h>
#include <VIPManager.h>
#include <Profiler.h>
#include <debugConfig.h>

#ifdef __DEBUG_TOOLS
#include <DebugState.h>
#endif
#ifdef __STAGE_EDITOR
#endif
#include <StageEditorState.h>
#ifdef __ANIMATION_INSPECTOR
#include <AnimationInspectorState.h>
#endif
#ifdef __SOUND_TEST
#include <SoundTestState.h>
#endif


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

enum StateOperations
{
	kSwapState = 0,
	kCleanAndSwapState,
	kPushState,
	kPopState
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

VUEngine _vuEngine = NULL;

uint32 _gameRandomSeed = 0;

#ifdef __REGISTER_PROCESS_NAME_DURING_FRAMESTART
static char* _processNameDuringGAMESTART = NULL;
#endif

#ifdef __REGISTER_PROCESS_NAME_DURING_XPEND
static char* _processNameDuringXPEND = NULL;
#endif

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

	this->randomSeed = 0;

	// force construction now
	this->clockManager = ClockManager::getInstance();

	// construct the general clock
	this->clock = new Clock();

	// construct the game's state machine
	this->stateMachine = new StateMachine(this);

	this->currentState = NULL;
	this->saveDataManager = NULL;
	this->nextState = NULL;
	this->nextGameCycleStarted = false;
	this->currentGameCycleEnded = false;
	this->isPaused = false;

	// make sure all managers are initialized now
	this->camera = NULL;
	this->keypadManager = NULL;
	this->vipManager = NULL;
	this->timerManager = NULL;
	this->communicationManager = NULL;
	this->frameRate = NULL;
	this->soundManager = NULL;

	Utilities::setClock(this->clock);
	Utilities::setKeypadManager(this->keypadManager);

#ifdef __DEBUG_TOOLS
	DebugState::getInstance();
#endif
#ifdef __STAGE_EDITOR
	StageEditorState::getInstance();
#endif
#ifdef __ANIMATION_INSPECTOR
	AnimationInspectorState::getInstance();
#endif
#ifdef __SOUND_TEST
	SoundTestState::getInstance();
#endif

	// to make debugging easier
	this->lastProcessName = PROCESS_NAME_START_UP;

	this->nextStateOperation = kSwapState;

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
	CommunicationManager::enableCommunications(this->communicationManager, NULL, NULL);
#else
#ifdef __RELEASE
	TimerManager::wait(TimerManager::getInstance(), 4000);
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
void VUEngine::start(GameState state)
{
	ASSERT(state, "VUEngine::start: initial state is NULL");

	// Initialize VPU and turn off the brightness
	HardwareManager::lowerBrightness(HardwareManager::getInstance());

	if(!StateMachine::getCurrentState(this->stateMachine))
	{
		// Set state
		VUEngine::setNextState(this, state);

		while(true)
		{
			VUEngine::checkForNewState(this);

			VUEngine::currentFrameStarted(this);

			VUEngine::run(this);

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

// Set game's state
void VUEngine::changeState(GameState state)
{
	// State changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kSwapState;
}

// Set game's state after cleaning the stack
void VUEngine::cleanAndChangeState(GameState state)
{
	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kCleanAndSwapState;
}

// Add a state to the game's state machine's stack
void VUEngine::addState(GameState state)
{
	// State changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kPushState;
}

// set game's state
void VUEngine::setNextState(GameState state)
{
	ASSERT(state, "VUEngine::setState: setting NULL state");

	HardwareManager::displayOff(HardwareManager::getInstance());
	HardwareManager::disableRendering(HardwareManager::getInstance());

	switch(this->nextStateOperation)
	{
		case kCleanAndSwapState:
			{
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

				StateMachine::popAllStates(this->stateMachine);
			}

			// Setup new state
			StateMachine::pushState(this->stateMachine, (State)state);
			break;

		case kSwapState:

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = PROCESS_NAME_STATE_SWAP;
#endif

			if(this->currentState)
			{
				// Discard delayed messages from the current state
				MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine))));
				MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
			}

			// Setup new state
			StateMachine::swapState(this->stateMachine, (State)state);
			break;

		case kPushState:

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = PROCESS_NAME_STATE_PUSH;
#endif
			// Setup new state
			StateMachine::pushState(this->stateMachine, (State)state);
			break;

		case kPopState:

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = PROCESS_NAME_STATE_POP;
#endif

			if(this->currentState)
			{
				// discard delayed messages from the current state
				MessageDispatcher::discardDelayedMessagesWithClock(MessageDispatcher::getInstance(), GameState::getMessagingClock(GameState::safeCast(StateMachine::getCurrentState(this->stateMachine))));
				MessageDispatcher::processDiscardedMessages(MessageDispatcher::getInstance());
			}

			// setup new state
			StateMachine::popState(this->stateMachine);
			break;
	}

	// No next state now
	this->nextState = NULL;

	// Reset flags
	this->currentGameCycleEnded = true;
	this->nextGameCycleStarted = false;

	// Save current state
	this->currentState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(GameState::isVersusMode(this->currentState))
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
		this->nextState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
		StateMachine::popState(this->stateMachine);
		this->nextState = NULL;
	}
	else
	{
		if(VUEngine::isInSpecialMode(this))
		{
			this->nextState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
			StateMachine::popState(this->stateMachine);
			this->nextState = NULL;
		}

		this->nextState = GameState::safeCast(toolState);
		StateMachine::pushState(this->stateMachine, (State)this->nextState);
		this->nextState = NULL;
	}

	this->currentState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
}

bool VUEngine::checkIfOpenTool(UserInput userInput)
{
	ToolState engineToolStates[] =
	{
#ifdef __DEBUG_TOOLS
		ToolState::safeCast(DebugState::getInstance()),
#endif
#ifdef __STAGE_EDITOR
		ToolState::safeCast(StageEditorState::getInstance()),
#endif
#ifdef __ANIMATION_INSPECTOR
		ToolState::safeCast(AnimationInspectorState::getInstance()),
#endif
#ifdef __SOUND_TEST
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
uint32 VUEngine::processUserInput()
{
	if(!KeypadManager::isEnabled(this->keypadManager))
	{
#ifdef __ENABLE_PROFILER
		Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif
		return false;
	}

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_INPUT;
#endif

	// poll the user's input
	KeypadManager::captureUserInput(this->keypadManager);
	UserInput userInput = KeypadManager::getUserInput(this->keypadManager);

#ifdef __TOOLS
	if(VUEngine::checkIfOpenTool(this, userInput))
	{
		return true;
	}
#endif

	if(GameState::processUserInputRegardlessOfInput(VUEngine::getCurrentState(this)) || (userInput.pressedKey | userInput.releasedKey | userInput.holdKey))
	{
		GameState::processUserInput(VUEngine::getCurrentState(this), userInput);
	}

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif

	return userInput.pressedKey | userInput.releasedKey;
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

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	}

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_MESSAGES);
#endif

#endif
}

// update game's logic subsystem
void VUEngine::updateLogic()
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
	StateMachine::update(this->stateMachine);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_LOGIC);
#endif
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

// update game's rendering subsystem
void VUEngine::synchronizeGraphics()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_GRAPHICS;
#endif

#ifdef __TOOLS
	if(VUEngine::isInSoundTest(this))
	{
		return;
	}
#endif

	// apply transformations to graphics
	GameState::synchronizeGraphics(this->currentState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_GRAPHICS);
#endif
}

// update game's physics subsystem
void VUEngine::updatePhysics()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_PHYSICS;
#endif

	// simulate physics
	GameState::updatePhysics(this->currentState);

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

void VUEngine::updateTransformations()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_TRANSFORMS;
#endif

	// apply world transformations
	GameState::transform(this->currentState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_TRANSFORMS);
#endif
}

void VUEngine::updateCollisions()
{
	// process the collisions after the transformations have taken place
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_COLLISIONS;
#endif

	// process collisions
	GameState::processCollisions(this->currentState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_COLLISIONS);
#endif
}

void VUEngine::stream()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STREAMING;
#endif

	GameState::stream(this->currentState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_STREAMING);
#endif
}

void VUEngine::checkForNewState()
{
	if(this->nextState)
	{
#ifdef __REGISTER_LAST_PROCESS_NAME
		this->lastProcessName = PROCESS_NAME_NEW_STATE;
#endif
		VUEngine::setNextState(this, this->nextState);

#undef __DIMM_FOR_PROFILING
#ifdef __DIMM_FOR_PROFILING

		_vipRegisters[__GPLT0] = 0x50;
		_vipRegisters[__GPLT1] = 0x50;
		_vipRegisters[__GPLT2] = 0x54;
		_vipRegisters[__GPLT3] = 0x54;
		_vipRegisters[__JPLT0] = 0x54;
		_vipRegisters[__JPLT1] = 0x54;
		_vipRegisters[__JPLT2] = 0x54;
		_vipRegisters[__JPLT3] = 0x54;

		_vipRegisters[0x30 | __PRINTING_PALETTE] = 0xE4;
#endif
	}
}

void VUEngine::updateFrameRate()
{
	if(VUEngine::isInSpecialMode(this))
	{
		return;
	}

	FrameRate::update(this->frameRate);
}

void VUEngine::nextGameCycleStarted(uint16 gameFrameDuration)
{
	this->nextGameCycleStarted = true;

	ClockManager::update(this->clockManager, gameFrameDuration);

	FrameRate::gameFrameStarted(this->frameRate, this->currentGameCycleEnded);

	// Graphics synchronization involves moving the camera for the UI
	// which can mess rendering if the VIP's XPEND interrupt happens when the camera is
	// modified
	VUEngine::synchronizeGraphics(this);

	SpriteManager::render(this->spriteManager);
	WireframeManager::render(this->wireframeManager);
}

void VUEngine::nextFrameStarted(uint16 gameFrameDuration)
{
	static uint16 totalTime = 0;

	totalTime += gameFrameDuration;

	TimerManager::nextFrameStarted(this->timerManager, gameFrameDuration * __MICROSECONDS_PER_MILLISECOND);

	if(__MILLISECONDS_PER_SECOND <= totalTime)
	{
#ifdef __SHOW_TIMER_MANAGER_STATUS
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

#ifdef __ENABLE_PROFILER
	if(this->currentGameCycleEnded)
	{
		Profiler::end(Profiler::getInstance());
		Profiler::start(Profiler::getInstance());
	}
	else
	{
		Profiler::end(Profiler::getInstance());
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

void VUEngine::run()
{
	// Generate random seed
	_gameRandomSeed = this->randomSeed = Utilities::randomSeed();

	// process user's input
	VUEngine::processUserInput(this);

	// simulate physics
	VUEngine::updatePhysics(this);

	// apply transformations
	VUEngine::updateTransformations(this);

	// process collisions
	VUEngine::updateCollisions(this);

	// focus the camera once collisions are resolved
	VUEngine::focusCamera(this);

	// dispatch delayed messages
	VUEngine::dispatchDelayedMessages(this);

	// stream
	VUEngine::stream(this);

	// update game's logic
	VUEngine::updateLogic(this);

	// Update sound related logic
	VUEngine::updateSound(this);
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
#endif

#ifdef __DEBUG_TOOLS
bool VUEngine::isInDebugMode()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)DebugState::getInstance();
}
#endif

#ifdef __STAGE_EDITOR
bool VUEngine::isInStageEditor()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)StageEditorState::getInstance();
}
#endif

#ifdef __ANIMATION_INSPECTOR
bool VUEngine::isInAnimationInspector()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)AnimationInspectorState::getInstance();
}
#endif

#ifdef __SOUND_TEST
bool VUEngine::isInSoundTest()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)SoundTestState::getInstance();
}
#endif


// whether if a special mode is active
bool VUEngine::isInSpecialMode()
{
	int32 isInSpecialMode = false;

#ifdef __DEBUG_TOOLS
	isInSpecialMode |= VUEngine::isInDebugMode(this);
#endif
#ifdef __STAGE_EDITOR
	isInSpecialMode |= VUEngine::isInStageEditor(this);
#endif
#ifdef __ANIMATION_INSPECTOR
	isInSpecialMode |= VUEngine::isInAnimationInspector(this);
#endif
#ifdef __SOUND_TEST
	isInSpecialMode |= VUEngine::isInSoundTest(this);
#endif

	return isInSpecialMode;
}

// whether if a special mode is being started
bool VUEngine::isEnteringSpecialMode()
{
	int32 isEnteringSpecialMode = false;
#ifdef __DEBUG_TOOLS
	isEnteringSpecialMode |= GameState::safeCast(DebugState::getInstance()) == this->nextState;
#endif
#ifdef __STAGE_EDITOR
	isEnteringSpecialMode |= GameState::safeCast(StageEditorState::getInstance()) == this->nextState;
#endif
#ifdef __ANIMATION_INSPECTOR
	isEnteringSpecialMode |= GameState::safeCast(AnimationInspectorState::getInstance()) == this->nextState;
#endif
#ifdef __SOUND_TEST
	isEnteringSpecialMode |= GameState::safeCast(SoundTestState::getInstance()) == this->nextState;
#endif

	return isEnteringSpecialMode;
}

// whether if a special mode is being started
bool VUEngine::isExitingSpecialMode()
{
	int32 isExitingSpecialMode = false;
#ifdef __DEBUG_TOOLS
	isExitingSpecialMode |= GameState::safeCast(DebugState::getInstance()) == this->nextState;
#endif
#ifdef __STAGE_EDITOR
	isExitingSpecialMode |= GameState::safeCast(StageEditorState::getInstance()) == this->nextState;
#endif
#ifdef __ANIMATION_INSPECTOR
	isExitingSpecialMode |= GameState::safeCast(AnimationInspectorState::getInstance()) == this->nextState;
#endif
#ifdef __SOUND_TEST
	isExitingSpecialMode |= GameState::safeCast(SoundTestState::getInstance()) == this->nextState;
#endif

	return isExitingSpecialMode;
}

// retrieve state machine, use with caution!!!
StateMachine VUEngine::getStateMachine()
{
	return this->stateMachine;
}

// retrieve the current level's stage
Stage VUEngine::getStage()
{
	if(VUEngine::isInSpecialMode(this))
	{
		return GameState::getStage(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}

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
	if(VUEngine::isInSpecialMode(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getPhysicalWorld(state);
	}

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicalWorld(state);
}

CollisionManager VUEngine::getCollisionManager()
{
	if(VUEngine::isInSpecialMode(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getCollisionManager(state);
	}

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getCollisionManager(state);
}

// pause
void VUEngine::pause(GameState pauseState)
{
	ASSERT(pauseState, "VUEngine::pause: null pauseState");

	if(pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPushState;
		this->isPaused = true;
		VUEngine::fireEvent(this, kEventGamePaused);
	}
}

// resume game
void VUEngine::unpause(GameState pauseState)
{
	ASSERT(pauseState, "VUEngine::unpause: null pauseState");
	ASSERT(pauseState == this->currentState, "VUEngine::unpause: pauseState sent is not the current one");

	if(pauseState && this->currentState == pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPopState;
		this->isPaused = false;
		VUEngine::fireEvent(this, kEventGameUnpaused);
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

#ifdef __REGISTER_PROCESS_NAME_DURING_FRAMESTART
void VUEngine::saveProcessNameDuringGAMESTART()
{
	ASSERT(this, "VUEngine::saveProcessNameDuringGAMESTART: this null");

	_processNameDuringGAMESTART = this->lastProcessName;

#ifdef __SHOW_PROCESS_NAME_DURING_FRAMESTART
	PRINT_TEXT("F START:           ", 0, 26);
	PRINT_TEXT(_processNameDuringGAMESTART, 9, 26);

	if(strcmp("end frame", _processNameDuringGAMESTART))
	{
		PRINT_TEXT(_processNameDuringGAMESTART, 25, 26);
		PRINT_TEXT("    ", 44, 26);
		PRINT_INT(TimerManager::getMillisecondsElapsed(this->timerManager), 44, 26);
	}
#endif
}
#endif

#ifdef __REGISTER_PROCESS_NAME_DURING_XPEND
void VUEngine::saveProcessNameDuringXPEND()
{
	ASSERT(this, "VUEngine::saveProcessNameDuringXPEND: this null");

	_processNameDuringXPEND = this->lastProcessName;

#ifdef __SHOW_PROCESS_NAME_DURING_XPEND
	PRINT_TEXT("XPEND:            ", 0, 27);
	PRINT_TEXT(_processNameDuringXPEND, 9, 27);

	if(strcmp("end frame", _processNameDuringXPEND))
	{
		PRINT_TEXT(_processNameDuringXPEND, 25, 27);
		PRINT_TEXT("    ", 44, 27);
		PRINT_INT(TimerManager::getMillisecondsElapsed(this->timerManager), 44, 27);
	}
#endif
}
#endif

/**
 * Register the current save data manager. Use NULL if none.
 *
 * @param saveDataManager
 */
void VUEngine::registerSaveDataManager(ListenerObject saveDataManager)
{
	this->saveDataManager = saveDataManager;
}

ListenerObject VUEngine::getSaveDataManager()
{
	return this->saveDataManager;
}

long VUEngine::getRandomSeed()
{
	return this->randomSeed;
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
