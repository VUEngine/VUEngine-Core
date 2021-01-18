/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>
#include <Game.h>
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
#include <TimerManager.h>
#include <VIPManager.h>
#include <Profiler.h>
#include <debugConfig.h>

#ifdef __DEBUG_TOOLS
#include <DebugState.h>
#endif
#ifdef __STAGE_EDITOR
#include <StageEditorState.h>
#endif
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

u32 _gameRandomSeed = 0;

#ifdef __REGISTER_PROCESS_NAME_DURING_FRAMESTART
static char* _processNameDuringFRAMESTART = NULL;
#endif

#ifdef __REGISTER_PROCESS_NAME_DURING_XPEND
static char* _processNameDuringXPEND = NULL;
#endif

#ifdef __SHOW_TORN_FRAMES_COUNT

s16 _tornGameFrameCount = 0;

#endif


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

static void Game::init()
{
	Game::getInstance();
}

// class's constructor
static bool Game::isConstructed()
{
	return 0 < _singletonConstructed;
}

// class's constructor
void Game::constructor()
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
	this->autoPauseState = NULL;
	this->saveDataManager = NULL;
	this->nextState = NULL;
	this->nextFrameStarted = false;
	this->currentFrameEnded = false;
	this->isPaused = false;

	// make sure all managers are initialized now
	this->camera = Camera::getInstance();
	this->keypadManager = KeypadManager::getInstance();
	this->vipManager = VIPManager::getInstance();
	this->timerManager = TimerManager::getInstance();
	this->communicationManager = CommunicationManager::getInstance();
	this->frameRate = FrameRate::getInstance();
	this->soundManager = SoundManager::getInstance();

	CharSetManager::getInstance();
	BgmapTextureManager::getInstance();
	HardwareManager::getInstance();
	SpriteManager::getInstance();
	DirectDraw::getInstance();
	ParamTableManager::getInstance();

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
	Game::initialize(this);
}

// class's destructor
void Game::destructor()
{
	// destroy the clocks
	Clock::destructor(this->clock);

	delete this->stateMachine;

	Base::destructor();
}

// setup engine parameters
void Game::initialize()
{
	// setup vectorInterrupts
	HardwareManager::setInterruptVectors(HardwareManager::getInstance());

	// clear sprite memory
	HardwareManager::clearScreen(HardwareManager::getInstance());

	// make sure timer interrupts are enable
	HardwareManager::setupTimer(HardwareManager::getInstance(), __TIMER_100US, 10, kMS);

	// Reset sounds
	SoundManager::reset(this->soundManager);

	// start the game's general clock
	Clock::start(this->clock);

	// Enable interrupts
	HardwareManager::enableInterrupts();

	// Enable communications
#ifdef __ENABLE_COMMUNICATIONS
	CommunicationManager::enableCommunications(this->communicationManager, NULL, NULL);
#endif
}

bool Game::updateSpecialProcesses()
{
	bool result = false;

	result |= SoundManager::playPCMSounds(this->soundManager);

	return result;
}

void Game::debug()
{
#ifdef __SHOW_WIREFRAME_MANAGER_STATUS
	WireframeManager::print(WireframeManager::getInstance(), 1, 1);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_END_FRAME;
#endif

#ifdef __SHOW_SOUND_STATUS
	SoundManager::print(this->soundManager);
#endif
}

// set game's initial state
void Game::start(GameState state)
{
	ASSERT(state, "Game::start: initial state is NULL");

	// Initialize SRAM
	SRAMManager::getInstance();

	// Initialize VPU and turn off the brightness
	HardwareManager::lowerBrightness(HardwareManager::getInstance());

	if(!StateMachine::getCurrentState(this->stateMachine))
	{
		// Set state
		Game::setNextState(this, state);

		while(true)
		{
			Game::checkForNewState(this);

			Game::currentFrameStarted(this);

			Game::run(this);

			Game::debug(this);

			Game::currentFrameEnded(this);

			while(!this->nextFrameStarted)
			{
				// This breaks PCM playback but reports torn frames more accurately
				if(!Game::updateSpecialProcesses(this) && !this->nextFrameStarted)
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
		ASSERT(false, "Game::start: already started");
	}
}

// Set game's state
void Game::changeState(GameState state)
{
	// State changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kSwapState;
}

// Set game's state after cleaning the stack
void Game::cleanAndChangeState(GameState state)
{
	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kCleanAndSwapState;
}

// Add a state to the game's state machine's stack
void Game::addState(GameState state)
{
	// State changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kPushState;
}

// set game's state
void Game::setNextState(GameState state)
{
	ASSERT(state, "Game::setState: setting NULL state");

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

				for(; node; node = VirtualNode::getNext(node))
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
	this->currentFrameEnded = true;
	this->nextFrameStarted = false;

	// Save current state
	this->currentState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));

	if(GameState::isVersusMode(this->currentState))
	{
		CommunicationManager::startSyncCycle(this->communicationManager);
	}

	HardwareManager::enableRendering(HardwareManager::getInstance());
	HardwareManager::displayOn(HardwareManager::getInstance());

	// Fire event
	Object::fireEvent(this, kEventNextStateSet);

	StopwatchManager::reset(StopwatchManager::getInstance());
	FrameRate::reset(this->frameRate);

#ifdef __SHOW_TORN_FRAMES_COUNT
	_tornGameFrameCount = 0;
#endif
}

// erase engine's current status
void Game::reset()
{
#ifdef	__MEMORY_POOL_CLEAN_UP
	MemoryPool::cleanUp(MemoryPool::getInstance());
#endif

#ifdef __ENABLE_PROFILER
	Profiler::reset(Profiler::getInstance());
#endif

	HardwareManager::disableInterrupts();

	// Disable timer
	TimerManager::enable(this->timerManager, false);
	TimerManager::reset(this->timerManager);

	// disable rendering
	HardwareManager::lowerBrightness(HardwareManager::getInstance());
	HardwareManager::clearScreen(HardwareManager::getInstance());
	HardwareManager::setupColumnTable(HardwareManager::getInstance(), NULL);
	VIPManager::removePostProcessingEffects(this->vipManager);

	// reset managers
	WireframeManager::reset(WireframeManager::getInstance());
	SoundManager::reset(this->soundManager);
	TimerManager::resetMilliseconds(this->timerManager);
	KeypadManager::reset(this->keypadManager);
	CommunicationManager::cancelCommunications(this->communicationManager);
	StopwatchManager::reset(StopwatchManager::getInstance());
	FrameRate::reset(this->frameRate);

	// the order of reset for the graphics managers must not be changed!
	VIPManager::reset(this->vipManager);
	SpriteManager::reset(SpriteManager::getInstance());
	BgmapTextureManager::reset(BgmapTextureManager::getInstance());
	CharSetManager::reset(CharSetManager::getInstance());
	ParamTableManager::reset(ParamTableManager::getInstance());
	AnimationCoordinatorFactory::reset(AnimationCoordinatorFactory::getInstance());
	Printing::reset(Printing::getInstance());

	// Enable timer
	TimerManager::enable(this->timerManager, true);

	HardwareManager::enableInterrupts();
}

#ifdef __TOOLS
void Game::openTool(ToolState toolState)
{
	if(Game::isInToolState(this, toolState))
	{
		this->nextState = GameState::safeCast(StateMachine::getCurrentState(this->stateMachine));
		StateMachine::popState(this->stateMachine);
		this->nextState = NULL;
	}
	else
	{
		if(Game::isInSpecialMode(this))
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

bool Game::checkIfOpenTool(UserInput userInput)
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

	int i = 0;

	for(; engineToolStates[i]; i++)
	{
		// check code to access special feature
		if(ToolState::isKeyCombination(engineToolStates[i], userInput))
		{
			Game::openTool(this, engineToolStates[i]);
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
			Game::openTool(this, _userToolStates[i]);
			return true;
		}
	}

	return false;
}

#endif


// process input data according to the actual game status
u32 Game::processUserInput()
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
	if(Game::checkIfOpenTool(this, userInput))
	{
		return true;
	}
#endif

	if(GameState::processUserInputRegardlessOfInput(Game::getCurrentState(this)) || (userInput.pressedKey | userInput.releasedKey | userInput.holdKey))
	{
		GameState::processUserInput(Game::getCurrentState(this), userInput);
	}

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_INPUT);
#endif

	return userInput.pressedKey | userInput.releasedKey;
}

void Game::dispatchDelayedMessages()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_MESSAGES;
#endif

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	static u32 dispatchCycle = 0;

	if(dispatchCycle++ & 1)
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
void Game::updateLogic()
{
#ifdef __TOOLS
	if(!Game::isInSpecialMode(this))
	{
#endif
	// it is the update cycle
	ASSERT(this->stateMachine, "Game::update: no state machine");
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
void Game::updateSound()
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
void Game::synchronizeGraphics()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_GRAPHICS;
#endif

#ifdef __TOOLS
	if(Game::isInSoundTest(this))
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
void Game::updatePhysics()
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

void Game::focusCamera()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_CAMERA;
#endif

#ifdef __TOOLS
	if(!Game::isInSpecialMode(this))
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

void Game::updateTransformations()
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

void Game::updateCollisions()
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

void Game::stream()
{
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = PROCESS_NAME_STREAMING;
#endif

	GameState::stream(this->currentState);

#ifdef __ENABLE_PROFILER
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeNormalProcess, PROCESS_NAME_STREAMING);
#endif
}

void Game::checkForNewState()
{
	if(this->nextState)
	{
#ifdef __REGISTER_LAST_PROCESS_NAME
		this->lastProcessName = PROCESS_NAME_NEW_STATE;
#endif
		Game::setNextState(this, this->nextState);

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

void Game::updateFrameRate()
{
	SoundManager::updateFrameRate(this->soundManager);

	if(Game::isInSpecialMode(this))
	{
		return;
	}

#ifdef __PRINT_FRAMERATE
	FrameRate::update(this->frameRate);
#endif
}

void Game::nextFrameStarted()
{
	this->nextFrameStarted = true;

	static u16 totalTime = 0;

	totalTime += __GAME_FRAME_DURATION;

	if(__MILLISECONDS_PER_SECOND < totalTime)
	{
		totalTime = 0;

#ifdef __SHOW_TORN_FRAMES_COUNT
		static int previousTornGameFrameCount = 0;
		if(_tornGameFrameCount != previousTornGameFrameCount)
		{
			previousTornGameFrameCount = _tornGameFrameCount;
			PRINT_TEXT("Torn Frames:    ", 1, 27);
			PRINT_INT(_tornGameFrameCount, 13, 27);
		}
#endif

#ifdef __SHOW_STREAMING_PROFILING

		if(!Game::isInSpecialMode(this))
		{
			Printing::resetCoordinates(Printing::getInstance());
			Stage::showStreamingProfiling(Game::getStage(this), 1, 1);
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

#ifdef __SHOW_MEMORY_POOL_STATUS
		if(!Game::isInSpecialMode(this))
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
		if(!Game::isInSpecialMode(this))
		{
			Printing::resetCoordinates(Printing::getInstance());
			HardwareManager::printStackStatus((__SCREEN_WIDTH_IN_CHARS) - 10, 0, true);
		}
#endif
	}

#ifdef __ENABLE_PROFILER
	if(this->currentFrameEnded)
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

void Game::currentFrameStarted()
{
	this->nextFrameStarted = false;
	this->currentFrameEnded = false;
}

void Game::currentFrameEnded()
{
	this->currentFrameEnded = true;

	Game::updateFrameRate(this);

#ifdef __SHOW_TORN_FRAMES_COUNT
	if(this->nextFrameStarted)
	{
		++_tornGameFrameCount;
	}
#endif
}

bool Game::hasCurrentFrameEnded()
{
	// raise flag to allow the next frame to start
	return this->currentFrameEnded;
}

void Game::run()
{
	// sync entities with their sprites
	Game::synchronizeGraphics(this);

	// reset timer
	TimerManager::resetMilliseconds(this->timerManager);

	// Generate random seed
	_gameRandomSeed = this->randomSeed = Utilities::randomSeed();

	// process user's input
	Game::processUserInput(this);

	// simulate physics
	Game::updatePhysics(this);

	// apply transformations
	Game::updateTransformations(this);

	// process collisions
	Game::updateCollisions(this);

	// focus the camera once collisions are resolved
	Game::focusCamera(this);

	// dispatch delayed messages
	Game::dispatchDelayedMessages(this);

	// stream
	Game::stream(this);

	// update game's logic
	Game::updateLogic(this);

	// Update sound related logic
	Game::updateSound(this);
}

#ifdef __REGISTER_LAST_PROCESS_NAME
void Game::setLastProcessName(char* processName)
{
	this->lastProcessName = processName;
}
#endif

// process a telegram
bool Game::handleMessage(Telegram telegram)
{
	ASSERT(this->stateMachine, "Game::handleMessage: NULL stateMachine");

	return StateMachine::handleMessage(this->stateMachine, telegram);
}

// retrieve time
u32 Game::getTime()
{
	return Clock::getTime(this->clock);
}

// retrieve clock
Clock Game::getClock()
{
	return this->clock;
}

// retrieve in game clock
Clock Game::getMessagingClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getMessagingClock(GameState::safeCast(state));
}

// retrieve animations' clock
Clock Game::getUpdateClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getUpdateClock(GameState::safeCast(state));
}

// retrieve in physics' clock
Clock Game::getPhysicsClock()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicsClock(GameState::safeCast(state));
}

// retrieve last process' name
char* Game::getLastProcessName()
{
	return this->lastProcessName;
}

#ifdef __TOOLS
bool Game::isInToolState(ToolState toolState)
{
	return StateMachine::getCurrentState(this->stateMachine) == State::safeCast(toolState);
}
#endif

#ifdef __DEBUG_TOOLS
bool Game::isInDebugMode()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)DebugState::getInstance();
}
#endif

#ifdef __STAGE_EDITOR
bool Game::isInStageEditor()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)StageEditorState::getInstance();
}
#endif

#ifdef __ANIMATION_INSPECTOR
bool Game::isInAnimationInspector()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)AnimationInspectorState::getInstance();
}
#endif

#ifdef __SOUND_TEST
bool Game::isInSoundTest()
{
	return StateMachine::getCurrentState(this->stateMachine) == (State)SoundTestState::getInstance();
}
#endif


// whether if a special mode is active
bool Game::isInSpecialMode()
{
	int isInSpecialMode = false;

#ifdef __DEBUG_TOOLS
	isInSpecialMode |= Game::isInDebugMode(this);
#endif
#ifdef __STAGE_EDITOR
	isInSpecialMode |= Game::isInStageEditor(this);
#endif
#ifdef __ANIMATION_INSPECTOR
	isInSpecialMode |= Game::isInAnimationInspector(this);
#endif
#ifdef __SOUND_TEST
	isInSpecialMode |= Game::isInSoundTest(this);
#endif

	return isInSpecialMode;
}

// whether if a special mode is being started
bool Game::isEnteringSpecialMode()
{
	int isEnteringSpecialMode = false;
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
bool Game::isExitingSpecialMode()
{
	int isExitingSpecialMode = false;
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
StateMachine Game::getStateMachine()
{
	return this->stateMachine;
}

// retrieve the current level's stage
Stage Game::getStage()
{
	if(Game::isInSpecialMode(this))
	{
		return GameState::getStage(GameState::safeCast(StateMachine::getPreviousState(this->stateMachine)));
	}

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getStage(GameState::safeCast(state));
}

// retrieve current state
GameState Game::getCurrentState()
{
	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::safeCast(state);
}

PhysicalWorld Game::getPhysicalWorld()
{
	if(Game::isInSpecialMode(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getPhysicalWorld(state);
	}

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getPhysicalWorld(state);
}

CollisionManager Game::getCollisionManager()
{
	if(Game::isInSpecialMode(this))
	{
		State state = StateMachine::getPreviousState(this->stateMachine);
		return isDeleted(state) ? NULL : GameState::getCollisionManager(state);
	}

	State state = StateMachine::getCurrentState(this->stateMachine);
	return isDeleted(state) ? NULL : GameState::getCollisionManager(state);
}

// pause
void Game::pause(GameState pauseState)
{
	ASSERT(pauseState, "Game::pause: null pauseState");

	if(pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPushState;
		this->isPaused = true;
		Object::fireEvent(this, kEventGamePaused);
	}
}

// resume game
void Game::unpause(GameState pauseState)
{
	ASSERT(pauseState, "Game::unpause: null pauseState");
	ASSERT(pauseState == this->currentState, "Game::unpause: pauseState sent is not the current one");

	if(pauseState && this->currentState == pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPopState;
		this->isPaused = false;
		Object::fireEvent(this, kEventGameUnpaused);
	}
}

// is game currently paused?
bool Game::isPaused()
{
	return this->isPaused;
}

void Game::disableKeypad()
{
	KeypadManager::disable(this->keypadManager);
}

void Game::enableKeypad()
{
	KeypadManager::enable(this->keypadManager);
}

void Game::pushFrontProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::pushFrontPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void Game::pushBackProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::pushBackPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void Game::removePostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	VIPManager::removePostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void Game::wait(u32 milliSeconds)
{
	ASSERT(this, "Game::wait: this null");

	TimerManager::wait(this->timerManager, milliSeconds);
}

#ifdef __REGISTER_PROCESS_NAME_DURING_FRAMESTART
void Game::saveProcessNameDuringFRAMESTART()
{
	ASSERT(this, "Game::saveProcessNameDuringFRAMESTART: this null");

	_processNameDuringFRAMESTART = this->lastProcessName;

#ifdef __SHOW_PROCESS_NAME_DURING_FRAMESTART
	PRINT_TEXT("F START:           ", 0, 26);
	PRINT_TEXT(_processNameDuringFRAMESTART, 9, 26);

	if(strcmp("end frame", _processNameDuringFRAMESTART))
	{
		PRINT_TEXT(_processNameDuringFRAMESTART, 25, 26);
		PRINT_TEXT("    ", 44, 26);
		PRINT_INT(TimerManager::getMillisecondsElapsed(this->timerManager), 44, 26);
	}
#endif
}
#endif

#ifdef __REGISTER_PROCESS_NAME_DURING_XPEND
void Game::saveProcessNameDuringXPEND()
{
	ASSERT(this, "Game::saveProcessNameDuringXPEND: this null");

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
 * Register the current auto pause state. Use NULL if no state.
 *
 * @param autoPauseState
 */
void Game::registerAutoPauseState(GameState autoPauseState)
{
	this->autoPauseState = autoPauseState;
}

GameState Game::getAutoPauseState()
{
	return this->autoPauseState;
}

/**
 * Register the current save data manager. Use NULL if none.
 *
 * @param saveDataManager
 */
void Game::registerSaveDataManager(Object saveDataManager)
{
	this->saveDataManager = saveDataManager;
}

Object Game::getSaveDataManager()
{
	return this->saveDataManager;
}

long Game::getRandomSeed()
{
	return this->randomSeed;
}

#ifdef __ENABLE_PROFILER
void Game::startProfiling()
{
	Profiler::initialize(Profiler::getInstance());
}
#endif