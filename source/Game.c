/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#include <Game.h>
#include <SRAMManager.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <MiscStructs.h>
#include <FrameRate.h>
#include <Clock.h>
#include <BgmapTextureManager.h>
#include <PolyhedronManager.h>
#include <GameState.h>
#include <Utilities.h>
#include <MessageDispatcher.h>
#include <Stage.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <RecyclableBgmapTextureManager.h>
#include <AnimationCoordinatorFactory.h>
#include <StateMachine.h>
#include <Screen.h>
#include <ScreenMovementManager.h>
#include <KeyPadManager.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <VIPManager.h>
#include <Printing.h>
#include <I18n.h>
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


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#ifdef __PROFILE_GAME
#ifndef __REGISTER_LAST_PROCESS_NAME
#define __REGISTER_LAST_PROCESS_NAME
#endif
#endif

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

#define Game_ATTRIBUTES																					\
		Object_ATTRIBUTES																				\
		/**
		 * @var StateMachine	stateMachine
		 * @brief				game's state machine
		 * @memberof			Game
		 */																								\
		StateMachine stateMachine;																		\
		/**
		 * @var GameState		currentState
		 * @brief				game's current state
		 * @memberof			Game
		 */																								\
		GameState currentState;																			\
		/**
		 * @var Clock			clock
		 * @brief				engine's global timer
		 * @memberof			Game
		 */																								\
		Clock clock;																					\
		/**
		 * @var ClockManager	clockManager
		 * @brief				managers
		 * @memberof			Game
		 */																								\
		ClockManager clockManager;																		\
		/**
		 * @var KeypadManager 	keypadManager
		 * @brief
		 * @memberof			Game
		 */																								\
		KeypadManager keypadManager;																	\
		/**
		 * @var VIPManager 		vipManager
		 * @brief
		 * @memberof			Game
		 */																								\
		VIPManager vipManager;																			\
		/**
		 * @var TimerManager 	timerManager
		 * @brief
		 * @memberof			Game
		 */																								\
		TimerManager timerManager;																		\
		/**
		 * @var Screen 			screen
		 * @brief
		 * @memberof			Game
		 */																								\
		Screen screen;																					\
		/**
		 * @var GameState		nextState
		 * @brief				game's next state
		 * @memberof			Game
		 */																								\
		GameState nextState;																			\
		/**
		 * @var int				nextStateOperation
		 * @brief				game's next state operation
		 * @memberof			Game
		 */																								\
		int nextStateOperation; 																		\
		/**
		 * @var char*			lastProcessName
		 * @brief				last process' name
		 * @memberof			Game
		 */																								\
		char* lastProcessName;																			\
		/**
		 * @var GameState		automaticPauseState
		 * @brief				auto pause state
		 * @memberof			Game
		 */																								\
		GameState automaticPauseState;																	\
		/**
		 * @var u32				lastAutoPauseCheckTime
		 * @brief				auto pause last checked time
		 * @memberof			Game
		 */																								\
		u32 lastAutoPauseCheckTime;																		\
		/**
		 * @var u32				gameFrameTotalTime
		 * @brief				elapsed time in current 50hz cycle
		 * @memberof			Game
		 */																								\
		u32 gameFrameTotalTime;																			\
		/**
		 * @var bool			isShowingLowBatteryIndicator
		 * @brief				low battery indicator showing flag
		 * @memberof			Game
		 */																								\
		bool isShowingLowBatteryIndicator;																\

/**
 * @class	Game
 * @extends Object
 * @ingroup base
 */
__CLASS_DEFINITION(Game, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Game_constructor(Game this);
static void Game_initialize(Game this);
static void Game_setNextState(Game this, GameState state);
static u32 Game_handleInput(Game this);
inline static u32 Game_dispatchDelayedMessages(Game this);
inline static void Game_updateLogic(Game this);
inline static void Game_updatePhysics(Game this);
inline static void Game_updateTransformations(Game this);
inline static u32 Game_updateCollisions(Game this);
void Game_updateVisuals(Game this);
bool Game_stream(Game this);
inline static void Game_checkForNewState(Game this);
inline static void Game_run(Game this);
void Game_checkFrameRate(Game thi);
static void Game_autoPause(Game this);
#ifdef __LOW_BATTERY_INDICATOR
static void Game_checkLowBattery(Game this, u16 keyPressed);
static void Game_printLowBatteryIndicator(Game this, bool showIndicator);
#endif

#ifdef __SHOW_GAME_PROFILING
void Game_showProfiling(Game this __attribute__ ((unused)), int x, int y);
#endif

void SpriteManager_sortLayersProgressively(SpriteManager this);
void MessageDispatcher_processDiscardedMessages(MessageDispatcher this);
void HardwareManager_checkMemoryMap();

u32 VIPManager_frameStarted(VIPManager this);
u32 VIPManager_gameStarted(VIPManager this);
void VIPManager_resetFrameStarted(VIPManager this);

#ifdef __PROFILE_GAME

bool _updateProfiling = false;

static char* _processNameDuringGAMESTART = NULL;
static char* _processNameDuringXPEND = NULL;

static s16 _gameFrameRealDuration = 0;
static s16 _gameFrameDurationAverage = 0;
static s16 _gameFrameEffectiveDuration = 0;
static s16 _gameFrameEffectiveDurationAverage = 0;
static s16 _gameFrameHighestEffectiveDuration = 0;
s16 _tornGameFrameCount = 0;

s16 _renderingTotalTime = 0;
static s16 _updateVisualsTotalTime = 0;
static s16 _updateLogicTotalTime = 0;
static s16 _updatePhysicsTotalTime = 0;
static s16 _updateTransformationsTotalTime = 0;
static s16 _streamingTotalTime = 0;
static s16 _handleInputTotalTime = 0;
static s16 _dispatchDelayedMessageTotalTime = 0;

s16 _renderingHighestTime = 0;
static s16 _updateVisualsHighestTime = 0;
static s16 _updateLogicHighestTime = 0;
static s16 _streamingHighestTime = 0;
static s16 _updatePhysicsHighestTime = 0;
static s16 _updateTransformationsHighestTime = 0;
static s16 _handleInputHighestTime = 0;
static s16 _dispatchDelayedMessageHighestTime = 0;
static s16 _processCollisionsTotalTime = 0;
static s16 _processCollisionsHighestTime = 0;

s16 _renderingProcessTimeHelper = 0;
s16 _renderingProcessTime = 0;
static s16 _updateVisualsProcessTime = 0;
static s16 _updateLogicProcessTime = 0;
static s16 _streamingProcessTime = 0;
static s16 _updatePhysicsProcessTime = 0;
static s16 _updateTransformationsProcessTime = 0;
static s16 _handleInputProcessTime = 0;
static s16 _dispatchDelayedMessageProcessTime = 0;
static s16 _processCollisionsProcessTime = 0;

static s16 _previousGameFrameRealDuration = 0;
static s16 _previousGameFrameDurationAverage = 0;
static s16 _previousGameFrameEffectiveDuration = 0;
static s16 _previousGameFrameEffectiveDurationAverage = 0;
static s16 _previousGameFrameHighestEffectiveDuration = 0;
static s16 _previousTornGameFrameCount = 0;

static s16 _previousRenderingTotalTime = 0;
static s16 _previousUpdateVisualsTotalTime = 0;
static s16 _previousUpdateLogicTotalTime = 0;
static s16 _previousUpdatePhysicsTotalTime = 0;
static s16 _previousUpdateTransformationsTotalTime = 0;
static s16 _previousStreamingTotalTime = 0;
static s16 _previousHandleInputTotalTime = 0;
static s16 _previousDispatchDelayedMessageTotalTime = 0;

static s16 _previousRenderingHighestTime = 0;
static s16 _previousUpdateVisualsHighestTime = 0;
static s16 _previousUpdateLogicHighestTime = 0;
static s16 _previousStreamingHighestTime = 0;
static s16 _previousUpdatePhysicsHighestTime = 0;
static s16 _previousUpdateTransformationsHighestTime = 0;
static s16 _previousHandleInputHighestTime = 0;
static s16 _previousDispatchDelayedMessageHighestTime = 0;
static s16 _previousProcessCollisionsTotalTime = 0;
static s16 _previousProcessCollisionsHighestTime = 0;

static s16 _previousGameFrameTotalTime = 0;

#endif


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(Game);

// class's constructor
bool Game_isConstructed()
{
	return 0 < _singletonConstructed;
}

// class's constructor
static void __attribute__ ((noinline)) Game_constructor(Game this)
{
	ASSERT(this, "Game::constructor: null this");

	// check memory map before anything else
	HardwareManager_checkMemoryMap();

	// construct base object
	__CONSTRUCT_BASE(Object);

	// make sure the memory pool is initialized now
	MemoryPool_getInstance();

	this->gameFrameTotalTime = 0;

	// force construction now
	this->clockManager = ClockManager_getInstance();

	// construct the general clock
	this->clock = __NEW(Clock);
	Utilities_setClock(this->clock);

	// construct the game's state machine
	this->stateMachine = __NEW(StateMachine, this);

	this->currentState = NULL;
	this->nextState = NULL;
	this->automaticPauseState = NULL;
	this->lastAutoPauseCheckTime = 0;
	this->isShowingLowBatteryIndicator = false;

	// make sure all managers are initialized now
	this->screen = Screen_getInstance();
	this->keypadManager = KeypadManager_getInstance();
	this->vipManager = VIPManager_getInstance();
	this->timerManager = TimerManager_getInstance();

	SoundManager_getInstance();
	CharSetManager_getInstance();
	BgmapTextureManager_getInstance();
	FrameRate_getInstance();
	HardwareManager_getInstance();
	SpriteManager_getInstance();
	DirectDraw_getInstance();
	I18n_getInstance();
	ParamTableManager_getInstance();

#ifdef __DEBUG_TOOLS
	DebugState_getInstance();
#endif

#ifdef __STAGE_EDITOR
	StageEditorState_getInstance();
#endif

#ifdef __ANIMATION_INSPECTOR
	AnimationInspectorState_getInstance();
#endif

	// to make debugging easier
	this->lastProcessName = "start up";

	this->nextStateOperation = kSwapState;

	// setup engine parameters
	Game_initialize(this);
}

// class's destructor
void Game_destructor(Game this)
{
	ASSERT(this, "Game::destructor: null this");

	// destroy the clocks
	Clock_destructor(this->clock);

	__DELETE(this->stateMachine);

	__SINGLETON_DESTROY;
}

// setup engine parameters
void Game_initialize(Game this)
{
	ASSERT(this, "Game::initialize: null this");

	// setup vectorInterrupts
	HardwareManager_setInterruptVectors(HardwareManager_getInstance());

	// set waveform data
	SoundManager_setWaveForm(SoundManager_getInstance());

	// clear sprite memory
	HardwareManager_clearScreen(HardwareManager_getInstance());

	// make sure timer interrupts are enable
	HardwareManager_initializeTimer(HardwareManager_getInstance());

	// start the game's general clock
	Clock_start(this->clock);
}

// set game's initial state
void Game_start(Game this, GameState state)
{
	ASSERT(this, "Game::start: null this");
	ASSERT(state, "Game::start: initial state is NULL");

	// initialize SRAM
	SRAMManager_getInstance();

	// initialize VPU and turn off the brightness
	HardwareManager_displayOn(HardwareManager_getInstance());
	HardwareManager_lowerBrightness(HardwareManager_getInstance());

	if(!StateMachine_getCurrentState(this->stateMachine))
	{
		// register start time for auto pause check
		this->lastAutoPauseCheckTime = Clock_getTime(this->clock);

		// set state
		Game_setNextState(this, state);

		while(true)
		{
#ifdef __PRINT_PROFILING_INFO
			Game_checkFrameRate(this);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "start frame";
#endif

#ifdef __PROFILE_GAME
			if(_updateProfiling)
			{
				static u32 cycleCount = 0;
				static u32 totalGameFrameDuration = 0;
				u32 elapsedTime = TimerManager_getMillisecondsElapsed(this->timerManager);
				totalGameFrameDuration += elapsedTime;

				_gameFrameEffectiveDuration = elapsedTime;
				_gameFrameEffectiveDurationAverage = totalGameFrameDuration / ++cycleCount;
				_gameFrameHighestEffectiveDuration = elapsedTime > _gameFrameHighestEffectiveDuration ? elapsedTime : _gameFrameHighestEffectiveDuration;
			}
#endif

			while(!VIPManager_frameStarted(this->vipManager));
			VIPManager_resetFrameStarted(this->vipManager);

			TimerManager_resetMilliseconds(this->timerManager);
			Game_updateVisuals(this);
			Game_run(this);

#ifdef __PROFILE_GAME
			if(_updateProfiling)
			{
				static u32 cycleCount = 0;
				s16 gameFrameDuration = _renderingProcessTime +
									_updateVisualsProcessTime +
									_updateLogicProcessTime +
									_streamingProcessTime +
									_updatePhysicsProcessTime +
									_updateTransformationsProcessTime +
									_handleInputProcessTime +
									_dispatchDelayedMessageProcessTime +
									_processCollisionsProcessTime;

				static s16 totalGameFrameRealDuration = 0;
				totalGameFrameRealDuration += gameFrameDuration;

				_gameFrameRealDuration = gameFrameDuration > _gameFrameRealDuration ? gameFrameDuration : _gameFrameRealDuration;
				_gameFrameDurationAverage = totalGameFrameRealDuration / ++cycleCount;
			}
#endif

#ifdef __PROFILE_GAME
			_updateProfiling = !Game_isInSpecialMode(this);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "frame done";
#endif

		}
	}
	else
	{
		ASSERT(false, "Game::start: already started");
	}
}

// set game's state
void Game_changeState(Game this, GameState state)
{
	ASSERT(this, "Game::changeState: null this");

	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kSwapState;
}

// set game's state after cleaning the stack
void Game_cleanAndChangeState(Game this, GameState state)
{
	ASSERT(this, "Game::changeState: null this");

	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kCleanAndSwapState;
}

// add a state to the game's state machine's stack
void Game_addState(Game this, GameState state)
{
	ASSERT(this, "Game::changeState: null this");

	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
	this->nextStateOperation = kPushState;
}

// set game's state
static void Game_setNextState(Game this, GameState state)
{
	ASSERT(this, "Game::setState: null this");
	ASSERT(state, "Game::setState: setting NULL state");

	// disable rendering
	HardwareManager_disableRendering(HardwareManager_getInstance());

	// set waveform data
	SoundManager_setWaveForm(SoundManager_getInstance());

	switch(this->nextStateOperation)
	{
		case kCleanAndSwapState:

			// clean the game's stack
			// pop states until the stack is empty
			while(StateMachine_getStackSize(this->stateMachine) > 0)
			{
				State stateMachineCurrentState = StateMachine_getCurrentState(this->stateMachine);
				if(stateMachineCurrentState)
				{
					// discard delayed messages from the current state
					MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher_getInstance(), GameState_getMessagingClock(__SAFE_CAST(GameState, stateMachineCurrentState)));
					MessageDispatcher_processDiscardedMessages(MessageDispatcher_getInstance());
				}

				StateMachine_popState(this->stateMachine);
			}

			// setup new state
			StateMachine_pushState(this->stateMachine, (State)state);
			break;

		case kSwapState:

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "state swap";
#endif

			if(this->currentState)
			{
				// discard delayed messages from the current state
				MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher_getInstance(), GameState_getMessagingClock(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine))));
				MessageDispatcher_processDiscardedMessages(MessageDispatcher_getInstance());
			}

			// setup new state
			StateMachine_swapState(this->stateMachine, (State)state);
			break;

		case kPushState:

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "state push";
#endif
			// setup new state
			StateMachine_pushState(this->stateMachine, (State)state);
			break;

		case kPopState:

#ifdef __REGISTER_LAST_PROCESS_NAME
			this->lastProcessName = "state pop";
#endif

			if(this->currentState)
			{
				// discard delayed messages from the current state
				MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher_getInstance(), GameState_getMessagingClock(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine))));
				MessageDispatcher_processDiscardedMessages(MessageDispatcher_getInstance());
			}

			// setup new state
			StateMachine_popState(this->stateMachine);
			break;
	}

	// TODO: crashes on Mednafen
	// enable hardware pad read
	//HardwareManager_enableKeypad(HardwareManager_getInstance());

	// if automatic pause function is in place
	if(this->automaticPauseState)
	{
		int automaticPauseCheckDelay = __AUTO_PAUSE_DELAY - (Clock_getTime(this->clock) - this->lastAutoPauseCheckTime);
		automaticPauseCheckDelay = 0 > automaticPauseCheckDelay? automaticPauseCheckDelay: automaticPauseCheckDelay;

		MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher_getInstance(), __SAFE_CAST(Object, this), kAutoPause);
		MessageDispatcher_dispatchMessage((u32)automaticPauseCheckDelay, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kAutoPause, NULL);
		this->lastAutoPauseCheckTime = Clock_getTime(this->clock);
	}

	// no next state now
	this->nextState = NULL;

	// save current state
	this->currentState = __SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine));

	// reset timer
	TimerManager_resetMilliseconds(this->timerManager);

	// reset profiling
	Game_resetProfiling(this);

	// disable rendering
	HardwareManager_enableRendering(HardwareManager_getInstance());
}

// disable interrupts
void Game_disableHardwareInterrupts(Game this __attribute__ ((unused)))
{
	ASSERT(this, "Game::disableHardwareInterrupts: null this");

	// disable rendering
	HardwareManager_disableRendering(HardwareManager_getInstance());
}

// enable interrupts
void Game_enableHardwareInterrupts(Game this __attribute__ ((unused)))
{
	ASSERT(this, "Game::enableHardwareInterrupts: null this");
}

// erase engine's current status
void Game_reset(Game this)
{
	ASSERT(this, "Game::reset: null this");

#ifdef	__MEMORY_POOL_CLEAN_UP
	MemoryPool_cleanUp(MemoryPool_getInstance());
#endif

	// setup the display
	HardwareManager_clearScreen(HardwareManager_getInstance());
	HardwareManager_setupColumnTable(HardwareManager_getInstance(), NULL);
	HardwareManager_displayOn(HardwareManager_getInstance());
	HardwareManager_lowerBrightness(HardwareManager_getInstance());
	VIPManager_removePostProcessingEffects(this->vipManager);

	// reset managers
	PolyhedronManager_reset(PolyhedronManager_getInstance());
	Screen_setFocusInGameEntity(this->screen, NULL);
	BgmapTextureManager_reset(BgmapTextureManager_getInstance());
	CharSetManager_reset(CharSetManager_getInstance());
	ParamTableManager_reset(ParamTableManager_getInstance());
	SpriteManager_reset(SpriteManager_getInstance());
	RecyclableBgmapTextureManager_reset(RecyclableBgmapTextureManager_getInstance());
	AnimationCoordinatorFactory_reset(AnimationCoordinatorFactory_getInstance());
	Printing_reset(Printing_getInstance());
	Screen_resetCameraFrustum(Screen_getInstance());

	// TODO
	//SoundManager_getInstance();
}

// process input data according to the actual game status
static u32 Game_handleInput(Game this)
{
	ASSERT(this, "Game::handleInput: null this");

	if(!KeypadManager_isEnabled(this->keypadManager))
	{
		return false;
	}

#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "input handling";
#endif

	// poll the user's input
	UserInput userInput = KeypadManager_read(this->keypadManager);

#ifdef __DEBUG_TOOLS

	// check code to access special feature
	if((userInput.previousKey & K_LT) && (userInput.previousKey & K_RT) && (userInput.pressedKey & K_RL))
	{
		if(Game_isInDebugMode(this))
		{
			this->nextState = __SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine));
			StateMachine_popState(this->stateMachine);
			this->nextState = NULL;
		}
		else
		{
			if(Game_isInSpecialMode(this))
			{
				this->nextState = __SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine));
				StateMachine_popState(this->stateMachine);
				this->nextState = NULL;
			}

			this->nextState = __SAFE_CAST(GameState, DebugState_getInstance());
			StateMachine_pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		return true;
	}
#endif

#ifdef __STAGE_EDITOR

	// check code to access special feature
	if((userInput.previousKey & K_LT) && (userInput.previousKey & K_RT) && (userInput.pressedKey & K_RD))
	{
		if(Game_isInStageEditor(this))
		{
			this->nextState = __SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine));
			StateMachine_popState(this->stateMachine);
			this->nextState = NULL;
		}
		else
		{
			if(Game_isInSpecialMode(this))
			{
				this->nextState = __SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine));
				StateMachine_popState(this->stateMachine);
				this->nextState = NULL;
			}

			this->nextState = __SAFE_CAST(GameState, StageEditorState_getInstance());
			StateMachine_pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		return true;
	}
#endif

#ifdef __ANIMATION_INSPECTOR

	// check code to access special feature
	if((userInput.previousKey & K_LT) && (userInput.previousKey & K_RT) && (userInput.pressedKey & K_RR))
	{

		if(Game_isInAnimationInspector(this))
		{
			this->nextState = __SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine));
			StateMachine_popState(this->stateMachine);
			this->nextState = NULL;
		}
		else
		{
			if(Game_isInSpecialMode(this))
			{
				this->nextState = __SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine));
				StateMachine_popState(this->stateMachine);
				this->nextState = NULL;
			}

			this->nextState = __SAFE_CAST(GameState, AnimationInspectorState_getInstance());
			StateMachine_pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		return true;
	}
#endif

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(this) && ((userInput.pressedKey & K_LT) || (userInput.pressedKey & K_RT)))
	{
		return true;
	}
#endif

#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(this) && ((userInput.pressedKey & K_LT) || (userInput.pressedKey & K_RT)))
	{
		return true;
	}
#endif

#ifdef __ANIMATION_INSPECTOR
	if(!Game_isInSpecialMode(this) && ((userInput.pressedKey & K_LT) || (userInput.pressedKey & K_RT)))
	{
		return true;
	}
#endif

#ifdef __LOW_BATTERY_INDICATOR
	Game_checkLowBattery(this, userInput.powerFlag);
#endif

	if(userInput.pressedKey | userInput.releasedKey | userInput.holdKey)
	{
		__VIRTUAL_CALL(GameState, processUserInput, Game_getCurrentState(this), userInput);
	}

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_handleInputProcessTime =  -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_handleInputTotalTime += _handleInputProcessTime;
	}
#endif

	return userInput.pressedKey | userInput.releasedKey;
}

inline static u32 Game_dispatchDelayedMessages(Game this __attribute__ ((unused)))
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "messaging";
#endif

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	static u32 dispatchCycle = 0;

	if(dispatchCycle++ & 1)
	{
#endif

#ifdef __PROFILE_GAME
		u32 dispatchedMessages = MessageDispatcher_dispatchDelayedMessages(MessageDispatcher_getInstance());

		if(_updateProfiling)
		{
			_dispatchDelayedMessageProcessTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
			_dispatchDelayedMessageTotalTime += _dispatchDelayedMessageProcessTime;
		}

		return dispatchedMessages;
#else
		return MessageDispatcher_dispatchDelayedMessages(MessageDispatcher_getInstance());
#endif

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
	}

	return false;
#endif
}

// update game's logic subsystem
inline static void Game_updateLogic(Game this)
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(this))
	{
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(this))
	{
#endif
#ifdef __ANIMATION_INSPECTOR
	if(!Game_isInSpecialMode(this))
	{
#endif
	// it is the update cycle
	ASSERT(this->stateMachine, "Game::update: no state machine");
#ifdef __DEBUG_TOOLS
	}
#endif
#ifdef __STAGE_EDITOR
	}
#endif
#ifdef __ANIMATION_INSPECTOR
	}
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "state machine";
#endif

	// update the game's logic
	StateMachine_update(this->stateMachine);

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_updateLogicProcessTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_updateLogicTotalTime += _updateLogicProcessTime;
	}
#endif
}

// update game's rendering subsystem
void Game_updateVisuals(Game this __attribute__ ((unused)))
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "visuals update";
#endif

	// apply transformations to visuals
	GameState_updateVisuals(this->currentState);

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_updateVisualsProcessTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_updateVisualsTotalTime += _updateVisualsProcessTime;
	}
#endif
}

// update game's physics subsystem
inline static void Game_updatePhysics(Game this)
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "physics";
#endif

	// simulate physics
	GameState_updatePhysics(this->currentState);

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_updatePhysicsProcessTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_updatePhysicsTotalTime += _updatePhysicsProcessTime;
	}
#endif
}

inline static void Game_updateTransformations(Game this)
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "transformation";
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "focus screen";
#endif
	// position the screen
	Screen_focus(this->screen, true);

	// apply world transformations
	GameState_transform(this->currentState);

#ifdef __PROFILE_GAME
	if(_updateProfiling)
	{
		_updateTransformationsProcessTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_updateTransformationsTotalTime += _updateTransformationsProcessTime;
	}
#endif
}

inline static u32 Game_updateCollisions(Game this)
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

	// process the collisions after the transformations have taken place
#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "collisions";
#endif

	// process collisions
#ifdef __PROFILE_GAME
	u32 processedCollisions = GameState_processCollisions(this->currentState);

	if(_updateProfiling)
	{
		_processCollisionsProcessTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_processCollisionsTotalTime += _processCollisionsProcessTime;
	}

	return processedCollisions;
#else
	return GameState_processCollisions(this->currentState);
#endif
}

bool Game_stream(Game this)
{
#ifdef __PROFILE_GAME
	_renderingProcessTimeHelper = 0;
	s32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __REGISTER_LAST_PROCESS_NAME
	this->lastProcessName = "streaming";
#endif

#ifdef __PROFILE_GAME
	u32 streamProcessed = GameState_stream(this->currentState);

	if(_updateProfiling)
	{
		_streamingProcessTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
		_streamingTotalTime += _streamingProcessTime;
	}

	return streamProcessed;
#else
	return GameState_stream(this->currentState);
#endif
}

inline static void Game_checkForNewState(Game this)
{
	ASSERT(this, "Game::checkForNewState: null this");

	if(this->nextState)
	{
#ifdef __REGISTER_LAST_PROCESS_NAME
		this->lastProcessName = "set next state";
#endif
		Game_setNextState(this, this->nextState);

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

void Game_increaseGameFrameDuration(Game this, u32 gameFrameDuration)
{
	this->gameFrameTotalTime += gameFrameDuration;
}

void Game_checkFrameRate(Game this)
{
	if(Game_isInSpecialMode(this))
	{
		return;
	}

	// this method "doesn't" exist
	TimerManager_enable(this->timerManager, false);

	FrameRate frameRate = FrameRate_getInstance();

	// increase the fps counter
	FrameRate_increaseFps(frameRate);

	if(this->gameFrameTotalTime >= __MILLISECONDS_IN_SECOND)
	{
#ifdef __SHOW_GAME_PROFILING
		if(_updateProfiling)
		{
			Game_showProfiling(this, 1, 0);
			Game_resetProfiling(this);
		}
#else
#ifdef __PROFILE_GAME
		if(_updateProfiling)
		{
			Game_resetProfiling(this);
		}
#endif
#ifdef __SHOW_STREAMING_PROFILING

		if(!Game_isInSpecialMode(Game_getInstance()))
		{
			Printing_resetWorldCoordinates(Printing_getInstance());
			Stage_showStreamingProfiling(Game_getStage(this), 1, 1);
		}
#endif

#endif
		this->gameFrameTotalTime = 0;

#ifdef __DEBUG
		Printing_text(Printing_getInstance(), "DEBUG MODE", 0, (__SCREEN_HEIGHT >> 3) - 1, NULL);
#endif

#ifdef __PRINT_FRAMERATE
		if(!Game_isInSpecialMode(this))
		{
			FrameRate_print(frameRate, 21, 27);
		}
#endif

#ifdef __PRINT_MEMORY_POOL_STATUS
		if(!Game_isInSpecialMode(this))
		{
#ifdef __PRINT_DETAILED_MEMORY_POOL_STATUS
			MemoryPool_printDetailedUsage(MemoryPool_getInstance(), 30, 1);
#else
			MemoryPool_printResumedUsage(MemoryPool_getInstance(), 40, 1);
#endif
		}
#endif

#ifdef __ALERT_STACK_OVERFLOW
		if(!Game_isInSpecialMode(this))
		{
			HardwareManager_printStackStatus(HardwareManager_getInstance(), (__SCREEN_WIDTH >> 3) - 10, 0, true);
		}
#endif
		//reset frame rate counters
		FrameRate_reset(frameRate);
	}

	// enable timer
	TimerManager_enable(this->timerManager, true);
}

inline static void Game_run(Game this)
{
	// process user"s input
	bool skipNonCriticalProcesses = Game_handleInput(this);

	// physics" update takes place after game"s logic
	// has been done
	Game_updatePhysics(this);

	// apply transformations
	Game_updateTransformations(this);

	// process collisions
	skipNonCriticalProcesses |= Game_updateCollisions(this);

	// skip the rest of the cycle if already late
	if(VIPManager_frameStarted(this->vipManager))
	{
#ifdef __PROFILE_GAME
		_tornGameFrameCount++;
#endif
		return;
	}

	// this is the moment to check if the game"s state
	// needs to be changed
	Game_checkForNewState(this);

	// update game"s logic
	Game_updateLogic(this);

	// skip the rest of the cycle if already late
	if(VIPManager_frameStarted(this->vipManager))
	{
#ifdef __PROFILE_GAME
		_tornGameFrameCount++;
#endif
		return;
	}

	// stream
	if(!skipNonCriticalProcesses)
	{
		skipNonCriticalProcesses |= Game_stream(this);
	}

	// skip the rest of the cycle if already late
	if(VIPManager_frameStarted(this->vipManager))
	{
#ifdef __PROFILE_GAME
		_tornGameFrameCount++;
#endif
		return;
	}

	// dispatch delayed messages
	if(!skipNonCriticalProcesses)
	{
		Game_dispatchDelayedMessages(this);
	}
}

#ifdef __REGISTER_LAST_PROCESS_NAME
void Game_setLastProcessName(Game this, char* processName)
{
	ASSERT(this, "Game::setLastProcessName: null this");
	this->lastProcessName = processName;
}
#endif

// process a telegram
bool Game_handleMessage(Game this, Telegram telegram)
{
	ASSERT(this, "Game::handleMessage: null this");
	ASSERT(this->stateMachine, "Game::handleMessage: NULL stateMachine");

	switch(Telegram_getMessage(telegram))
	{
		case kAutoPause:

			Game_autoPause(this);
			return true;
			break;

#ifdef __LOW_BATTERY_INDICATOR
		case kLowBatteryIndicator:

			Game_printLowBatteryIndicator(this, Telegram_getExtraInfo(telegram) ? true : false);
			return true;
			break;
#endif
	}

	return StateMachine_handleMessage(this->stateMachine, telegram);
}

// retrieve time
u32 Game_getTime(Game this)
{
	ASSERT(this, "Game::getTime: null this");

	return Clock_getTime(this->clock);
}

// retrieve clock
Clock Game_getClock(Game this)
{
	ASSERT(this, "Game::getClock: null this");

	return this->clock;
}

// retrieve in game clock
Clock Game_getMessagingClock(Game this)
{
	ASSERT(this, "Game::getMessagingClock: null this");

	return GameState_getMessagingClock(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

// retrieve animations' clock
Clock Game_getUpdateClock(Game this)
{
	ASSERT(this, "Game::getUpdateClock: null this");

	return GameState_getUpdateClock(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

// retrieve in physics' clock
Clock Game_getPhysicsClock(Game this)
{
	ASSERT(this, "Game::getPhysicsClock: null this");

	return GameState_getPhysicsClock(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

// retrieve last process' name
char* Game_getLastProcessName(Game this)
{
	ASSERT(this, "Game::getLastProcessName: null this");

	return this->lastProcessName;
}

#ifdef __DEBUG_TOOLS
bool Game_isInDebugMode(Game this)
{
	ASSERT(this, "Game::isInDebugMode: null this");

	return StateMachine_getCurrentState(this->stateMachine) == (State)DebugState_getInstance();
}
#endif

#ifdef __STAGE_EDITOR
bool Game_isInStageEditor(Game this)
{
	ASSERT(this, "Game::isInGameStateEditor: null this");

	return StateMachine_getCurrentState(this->stateMachine) == (State)StageEditorState_getInstance();
}
#endif

#ifdef __ANIMATION_INSPECTOR
bool Game_isInAnimationInspector(Game this)
{
	ASSERT(this, "Game::isInAnimationInspector: null this");

	return StateMachine_getCurrentState(this->stateMachine) == (State)AnimationInspectorState_getInstance();
}
#endif

// whether if a special mode is active
bool Game_isInSpecialMode(Game this __attribute__ ((unused)))
{
	ASSERT(this, "Game::isInSpecialMode: null this");

	int isInSpecialMode = false;

#ifdef __DEBUG_TOOLS
	isInSpecialMode |= Game_isInDebugMode(this);
#endif
#ifdef __STAGE_EDITOR
	isInSpecialMode |= Game_isInStageEditor(this);
#endif
#ifdef __ANIMATION_INSPECTOR
	isInSpecialMode |= Game_isInAnimationInspector(this);
#endif

	return isInSpecialMode;
}

// whether if a special mode is being started
bool Game_isEnteringSpecialMode(Game this __attribute__ ((unused)))
{
	ASSERT(this, "Game::isInSpecialMode: null this");

	int isEnteringSpecialMode = false;
#ifdef __DEBUG_TOOLS
	isEnteringSpecialMode |= __SAFE_CAST(GameState, DebugState_getInstance()) == this->nextState;
#endif
#ifdef __STAGE_EDITOR
	isEnteringSpecialMode |= __SAFE_CAST(GameState, StageEditorState_getInstance()) == this->nextState;
#endif
#ifdef __ANIMATION_INSPECTOR
	isEnteringSpecialMode |= __SAFE_CAST(GameState, AnimationInspectorState_getInstance()) == this->nextState;
#endif

	return isEnteringSpecialMode;
}

// whether if a special mode is being started
bool Game_isExitingSpecialMode(Game this __attribute__ ((unused)))
{
	ASSERT(this, "Game::isInSpecialMode: null this");

	int isEnteringSpecialMode = false;
#ifdef __DEBUG_TOOLS
	isEnteringSpecialMode |= __SAFE_CAST(GameState, DebugState_getInstance()) == this->nextState;
#endif
#ifdef __STAGE_EDITOR
	isEnteringSpecialMode |= __SAFE_CAST(GameState, StageEditorState_getInstance()) == this->nextState;
#endif
#ifdef __ANIMATION_INSPECTOR
	isEnteringSpecialMode |= __SAFE_CAST(GameState, AnimationInspectorState_getInstance()) == this->nextState;
#endif

	return isEnteringSpecialMode;
}

// retrieve state machine, use with caution!!!
StateMachine Game_getStateMachine(Game this)
{
	ASSERT(this, "Game::getStateMachine: null this");

	return this->stateMachine;
}

// retrieve the current level's stage
Stage Game_getStage(Game this)
{
	ASSERT(this, "Game::getStage: null this");

	return GameState_getStage(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

// retrieve current state
GameState Game_getCurrentState(Game this)
{
	ASSERT(this, "Game::getCurrentState: null this");

	return __SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine));
}

PhysicalWorld Game_getPhysicalWorld(Game this)
{
	ASSERT(this, "Game::PhysicalWorld: null this");

	if(Game_isInSpecialMode(this))
	{
		return GameState_getPhysicalWorld(__SAFE_CAST(GameState, StateMachine_getPreviousState(this->stateMachine)));
	}

	return GameState_getPhysicalWorld(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

CollisionManager Game_getCollisionManager(Game this)
{
	ASSERT(this, "Game::getCollisionManager: null this");

	if(Game_isInSpecialMode(this))
	{
		return GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getPreviousState(this->stateMachine)));
	}

	return GameState_getCollisionManager(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

#ifdef __LOW_BATTERY_INDICATOR
// low battery indicator check
static void Game_checkLowBattery(Game this, u16 keypad)
{
	ASSERT(this, "Game::checkLowBatteryIndicator: null this");

	if(keypad & K_PWR)
	{
		if(!this->isShowingLowBatteryIndicator)
		{
			MessageDispatcher_dispatchMessage(__LOW_BATTERY_INDICATOR_INITIAL_DELAY, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kLowBatteryIndicator, (bool*)true);
			this->isShowingLowBatteryIndicator = true;
		}
	}
	else
	{
		if(this->isShowingLowBatteryIndicator)
		{
			MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher_getInstance(), __SAFE_CAST(Object, this), kLowBatteryIndicator);
			Printing_text(Printing_getInstance(), "  ", __LOW_BATTERY_INDICATOR_POS_X, __LOW_BATTERY_INDICATOR_POS_Y, NULL);
			this->isShowingLowBatteryIndicator = false;
		}
	}
}

// print low battery indicator
static void Game_printLowBatteryIndicator(Game this, bool showIndicator)
{
	ASSERT(this, "Game::printLowBatteryIndicator: null this");

	Printing_text(Printing_getInstance(), (showIndicator) ? __CHAR_BATTERY : "  ", __LOW_BATTERY_INDICATOR_POS_X, __LOW_BATTERY_INDICATOR_POS_Y, NULL);
	MessageDispatcher_dispatchMessage(__LOW_BATTERY_INDICATOR_BLINK_DELAY, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kLowBatteryIndicator, (bool*)(!showIndicator));
}
#endif

// pause
void Game_pause(Game this, GameState pauseState)
{
	ASSERT(this, "Game::pause: null this");
	ASSERT(pauseState, "Game::pause: null pauseState");

	if(pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPushState;
	}
}

// resume game
void Game_unpause(Game this, GameState pauseState)
{
	ASSERT(this, "Game::unpause: null this");
	ASSERT(pauseState, "Game::unpause: null pauseState");
	ASSERT(pauseState == this->currentState, "Game::unpause: null pauseState sent is not the current one");

	if(pauseState && this->currentState == pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPopState;

		if(this->currentState == this->automaticPauseState)
		{
			MessageDispatcher_dispatchMessage(__AUTO_PAUSE_DELAY, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kAutoPause, NULL);
			this->lastAutoPauseCheckTime = Clock_getTime(this->clock);
		}
	}
}

// set auto pause state
void Game_setAutomaticPauseState(Game this, GameState automaticPauseState)
{
	ASSERT(this, "Game::setAutomaticPauseState: null this");
	this->automaticPauseState = automaticPauseState;
}

// get auto pause state
GameState Game_getAutomaticPauseState(Game this)
{
	ASSERT(this, "Game::getAutomaticPauseState: null this");

	return this->automaticPauseState;
}

// show auto pause screen
static void Game_autoPause(Game this)
{
	ASSERT(this, "Game::autoPause: null this");

	if(this->automaticPauseState)
	{
		// only pause if no more than one state is active
		if(1 == StateMachine_getStackSize(this->stateMachine))
		{
			Game_pause(this, this->automaticPauseState);
		}
		else
		{
			// otherwise just wait a minute to check again
			MessageDispatcher_dispatchMessage(__AUTO_PAUSE_RECHECK_DELAY, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kAutoPause, NULL);
		}
	}
}

void Game_disableKeypad(Game this)
{
	ASSERT(this, "Game::disableKeyPad: null this");

	KeypadManager_disable(this->keypadManager);
}

void Game_enableKeypad(Game this)
{
	ASSERT(this, "Game::enableKeypad: null this");

	KeypadManager_enable(this->keypadManager);
}


void Game_pushFrontProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	ASSERT(this, "Game::pushFrontPostProcessingEffect: null this");

	VIPManager_pushFrontPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void Game_pushBackProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	ASSERT(this, "Game::pushBackPostProcessingEffect: null this");

	VIPManager_pushBackPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void Game_removePostProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	ASSERT(this, "Game::removePostProcessingEffect: null this");

	VIPManager_removePostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
}

void Game_wait(Game this, u32 milliSeconds)
{
	ASSERT(this, "Game::wait: this null");

	TimerManager_wait(this->timerManager, milliSeconds);
}

#ifdef __PROFILE_GAME
void Game_saveProcessNameDuringGAMESTART(Game this __attribute__ ((unused)))
{
	ASSERT(this, "Game::saveProcessNameDuringGAMESTART: this null");

	_processNameDuringGAMESTART = this->lastProcessName;
}

void Game_saveProcessNameDuringXPEND(Game this __attribute__ ((unused)))
{
	ASSERT(this, "Game::saveProcessNameDuringXPEND: this null");

	_processNameDuringXPEND = this->lastProcessName;
}
#endif
#ifdef __SHOW_GAME_PROFILING
void Game_showProfiling(Game this __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	ASSERT(this, "Game::showProfiling: this null");

	int xDisplacement = 32;

	Printing printing = Printing_getInstance();

	Printing_resetWorldCoordinates(printing);

	Printing_text(printing, "PROFILING", x, y++, NULL);

	Printing_text(printing, "Last game frame's info        (ms)", x, ++y, NULL);

	Printing_text(printing, "Real duration:", x, ++y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameRealDuration, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Average real duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameDurationAverage, x + xDisplacement, y++, NULL);

	Printing_text(printing, "Effective duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Average effective duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameEffectiveDurationAverage, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Highest effective duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameHighestEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Torn frames count:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _tornGameFrameCount, x + xDisplacement, y++, NULL);
	Printing_text(printing, "                                                ", x, y, NULL);
	if(_processNameDuringGAMESTART)
	{
		Printing_text(printing, "Process during GAMESTART:", x, y, NULL);
		Printing_text(printing, _processNameDuringGAMESTART, x + xDisplacement, y++, NULL);
	}
	Printing_text(printing, "                                                ", x, y, NULL);
	if(_processNameDuringXPEND)
	{
		Printing_text(printing, "Process during XPEND:", x, y, NULL);
		Printing_text(printing, _processNameDuringXPEND, x + xDisplacement, y++, NULL);
	}

	int xDisplacement2 = 7;

#ifdef __SHOW_GAME_DETAILED_PROFILING

	Printing_text(printing, "Processes' duration (ms/sec)", x, ++y, NULL);

	Printing_text(printing, "                              total highest", x, ++y, NULL);

	int processNumber = 1;
	Printing_text(printing, "  Rendering:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _renderingTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _renderingHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "  Updating visuals:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _updateVisualsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _updateVisualsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "  Handling input:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _handleInputTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _handleInputHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "  Processing messages:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _dispatchDelayedMessageTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _dispatchDelayedMessageHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "  Updating logic:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _updateLogicTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _updateLogicHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "  Streaming:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _streamingTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _streamingHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "  Updating physics:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _updatePhysicsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _updatePhysicsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "  Transforming:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _updateTransformationsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _updateTransformationsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "  Processing collisions:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _processCollisionsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _processCollisionsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);
#endif

	Printing_text(printing, "Last second processing (ms)", x, ++y, NULL);
	Printing_text(printing, "Real processing time:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _renderingTotalTime + _updateVisualsTotalTime + _handleInputTotalTime + _dispatchDelayedMessageTotalTime + _updateLogicTotalTime + _streamingTotalTime + _updatePhysicsTotalTime + _updateTransformationsTotalTime + _processCollisionsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _renderingHighestTime + _updateVisualsHighestTime + _updateLogicHighestTime + _streamingHighestTime + _updatePhysicsHighestTime + _updateTransformationsHighestTime + _handleInputHighestTime + _dispatchDelayedMessageHighestTime + _processCollisionsHighestTime, x + xDisplacement + xDisplacement2, y, NULL);

	Printing_text(printing, "Effective processing time:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, this->gameFrameTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameHighestEffectiveDuration, x + xDisplacement + xDisplacement2, y, NULL);
}
#endif

#ifdef __PROFILE_GAME
void Game_showCurrentGameFrameProfiling(Game this __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	ASSERT(this, "Game::showCurrentGameFrameProfiling: this null");

	int xDisplacement = 32;

	Printing printing = Printing_getInstance();

	Printing_resetWorldCoordinates(printing);

	Printing_text(printing, "PROFILING", x, y++, NULL);

	Printing_text(printing, "Last game frame's info        (ms)", x, ++y, NULL);

	Printing_text(printing, "Real duration:", x, ++y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameRealDuration, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Average real duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameDurationAverage, x + xDisplacement, y++, NULL);

	Printing_text(printing, "Effective duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Average effective duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameEffectiveDurationAverage, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Highest effective duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _gameFrameHighestEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Torn frames count:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _tornGameFrameCount, x + xDisplacement, y++, NULL);
	Printing_text(printing, "                                                ", x, y, NULL);
	if(_processNameDuringGAMESTART)
	{
		Printing_text(printing, "Process during GAMESTART:", x, y, NULL);
		Printing_text(printing, _processNameDuringGAMESTART, x + xDisplacement, y++, NULL);
	}
	Printing_text(printing, "                                                ", x, y, NULL);
	if(_processNameDuringXPEND)
	{
		Printing_text(printing, "Process during XPEND:", x, y, NULL);
		Printing_text(printing, _processNameDuringXPEND, x + xDisplacement, y++, NULL);
	}

	int xDisplacement2 = 7;

	Printing_text(printing, "Processes' duration (ms/sec)", x, ++y, NULL);

	Printing_text(printing, "                              total highest", x, ++y, NULL);

	int processNumber = 1;
	Printing_text(printing, "  Rendering:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _renderingProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "  Updating visuals:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _updateVisualsProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "  Handling input:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _handleInputProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "  Processing messages:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _dispatchDelayedMessageProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "  Updating logic:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _updateLogicProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "  Streaming:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _streamingProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "  Updating physics:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _updatePhysicsProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "  Transforming:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _updateTransformationsProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "  Processing collisions:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, processNumber, x, y, NULL);
	Printing_int(printing, processNumber++, x + xDisplacement - 2, y, NULL);
	Printing_int(printing, _processCollisionsProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "Last second processing (ms)", x, ++y, NULL);
	Printing_text(printing, "Real processing time:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _renderingProcessTime + _updateVisualsProcessTime + _handleInputProcessTime + _dispatchDelayedMessageProcessTime + _updateLogicProcessTime + _streamingProcessTime + _updatePhysicsProcessTime + _updateTransformationsProcessTime + _processCollisionsProcessTime, x + xDisplacement, y, NULL);

	Printing_text(printing, "Time elapsed:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, TimerManager_getMillisecondsElapsed(this->timerManager), x + xDisplacement, y, NULL);
}
#endif

void Game_showLastGameFrameProfiling(Game this __attribute__ ((unused)), int x __attribute__ ((unused)), int y __attribute__ ((unused)))
{
	ASSERT(this, "Game::showLastGameFrameProfiling: this null");

#ifdef __PROFILE_GAME

	int xDisplacement = 32;

	Printing printing = Printing_getInstance();

	Printing_text(printing, "PROFILING", x, y++, NULL);

	Printing_text(printing, "Last game frame's info (ms)", x, ++y, NULL);

	Printing_text(printing, "Real duration:", x, ++y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousGameFrameRealDuration, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Average real duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousGameFrameDurationAverage, x + xDisplacement, y++, NULL);

	Printing_text(printing, "Effective duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousGameFrameEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Average effective duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousGameFrameEffectiveDurationAverage, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Highest effective duration:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousGameFrameHighestEffectiveDuration, x + xDisplacement, y++, NULL);
	Printing_text(printing, "Torn frames count:", x, y, NULL);
	Printing_text(printing, "   ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousTornGameFrameCount, x + xDisplacement, y++, NULL);

	Printing_text(printing, "Processes' duration (ms in sec)", x, ++y, NULL);

	int xDisplacement2 = 7;

	Printing_text(printing, "                              total highest", x, ++y, NULL);

	Printing_text(printing, "Rendering:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousRenderingTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousRenderingHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "Updating visuals:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousUpdateVisualsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousUpdateVisualsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "Handling input:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousHandleInputTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousHandleInputHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "Processing messages:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousDispatchDelayedMessageTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousDispatchDelayedMessageHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "Updating logic:     ", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousUpdateLogicTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousUpdateLogicHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "Streaming:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousStreamingTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousStreamingHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "Updating physics:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousUpdatePhysicsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousUpdatePhysicsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "Transforming:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousUpdateTransformationsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousUpdateTransformationsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);

	Printing_text(printing, "Processing collisions:", x, y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousProcessCollisionsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousProcessCollisionsHighestTime, x + xDisplacement + xDisplacement2, y++, NULL);


	Printing_text(printing, "Last second processing (ms)", x, ++y, NULL);
	Printing_text(printing, "Real processing time:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousRenderingTotalTime + _previousUpdateVisualsTotalTime + _previousHandleInputTotalTime + _previousDispatchDelayedMessageTotalTime + _previousUpdateLogicTotalTime + _previousStreamingTotalTime + _previousUpdatePhysicsTotalTime + _previousUpdateTransformationsTotalTime + _previousProcessCollisionsTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousRenderingHighestTime + _previousUpdateVisualsHighestTime + _previousHandleInputHighestTime + _previousDispatchDelayedMessageHighestTime + _previousUpdateLogicHighestTime + _previousStreamingHighestTime + _previousUpdatePhysicsHighestTime + _previousUpdateTransformationsHighestTime + _previousProcessCollisionsHighestTime, x + xDisplacement + xDisplacement2, y, NULL);

	Printing_text(printing, "Effective processing time:", x, ++y, NULL);
	Printing_text(printing, "          ", x + xDisplacement, y, NULL);
	Printing_int(printing, _previousGameFrameTotalTime, x + xDisplacement, y, NULL);
	Printing_int(printing, _previousGameFrameHighestEffectiveDuration, x + xDisplacement + xDisplacement2, y, NULL);
#endif
}

void Game_resetCurrentFrameProfiling(Game this __attribute__ ((unused)), s32 gameFrameDuration __attribute__ ((unused)))
{
	ASSERT(this, "Game::showProfiling: this null");

#ifdef __PROFILE_GAME

	if(_gameFrameHighestEffectiveDuration < gameFrameDuration)
	{
		_dispatchDelayedMessageHighestTime = _dispatchDelayedMessageProcessTime;
		_handleInputHighestTime = _handleInputProcessTime;
		_updateLogicHighestTime = _updateLogicProcessTime;
		_updateVisualsHighestTime = _updateVisualsProcessTime;
		_updatePhysicsHighestTime = _updatePhysicsProcessTime;
		_updateTransformationsHighestTime = _updateTransformationsProcessTime;
		_processCollisionsHighestTime = _processCollisionsProcessTime;
		_streamingHighestTime = _streamingProcessTime;
	}

	_renderingProcessTimeHelper = 0;
	_renderingProcessTime = 0;
	_updateVisualsProcessTime = 0;
	_updateLogicProcessTime = 0;
	_streamingProcessTime = 0;
	_updatePhysicsProcessTime = 0;
	_updateTransformationsProcessTime = 0;
	_handleInputProcessTime = 0;
	_dispatchDelayedMessageProcessTime = 0;
	_processCollisionsProcessTime = 0;

#endif
}

void Game_resetProfiling(Game this __attribute__ ((unused)))
{
	ASSERT(this, "Game::resetProfiling: this null");

#ifdef __PROFILE_GAME

	_previousGameFrameTotalTime = this->gameFrameTotalTime;
	_previousGameFrameHighestEffectiveDuration = _gameFrameHighestEffectiveDuration;

	_previousGameFrameRealDuration = _gameFrameRealDuration;
	_previousGameFrameDurationAverage = _gameFrameDurationAverage;
	_previousGameFrameEffectiveDuration = _gameFrameEffectiveDuration;
	_previousGameFrameEffectiveDurationAverage = _gameFrameEffectiveDurationAverage;
	_previousTornGameFrameCount = _tornGameFrameCount;

	_previousRenderingTotalTime = _renderingTotalTime;
	_previousUpdateVisualsTotalTime = _updateVisualsTotalTime;
	_previousUpdateLogicTotalTime = _updateLogicTotalTime;
	_previousUpdatePhysicsTotalTime = _updatePhysicsTotalTime;
	_previousUpdateTransformationsTotalTime = _updateTransformationsTotalTime;
	_previousStreamingTotalTime = _streamingTotalTime;
	_previousHandleInputTotalTime = _handleInputTotalTime;
	_previousDispatchDelayedMessageTotalTime = _dispatchDelayedMessageTotalTime;
	_previousProcessCollisionsTotalTime = _processCollisionsTotalTime;

	_previousRenderingHighestTime = _renderingHighestTime;
	_previousUpdateVisualsHighestTime = _updateVisualsHighestTime;
	_previousUpdateLogicHighestTime = _updateLogicHighestTime;
	_previousUpdatePhysicsHighestTime = _updatePhysicsHighestTime;
	_previousUpdateTransformationsHighestTime = _updateTransformationsHighestTime;
	_previousStreamingHighestTime = _streamingHighestTime;
	_previousHandleInputHighestTime = _handleInputHighestTime;
	_previousDispatchDelayedMessageHighestTime = _dispatchDelayedMessageHighestTime;
	_previousProcessCollisionsHighestTime = _processCollisionsHighestTime;

	_gameFrameRealDuration = 0;
	_gameFrameDurationAverage = 0;
	_gameFrameEffectiveDuration = 0;
	_gameFrameEffectiveDurationAverage = 0;
	_tornGameFrameCount = 0;

	_renderingTotalTime = 0;
	_updateVisualsTotalTime = 0;
	_updateLogicTotalTime = 0;
	_updatePhysicsTotalTime = 0;
	_updateTransformationsTotalTime = 0;
	_streamingTotalTime = 0;
	_handleInputTotalTime = 0;
	_dispatchDelayedMessageTotalTime = 0;
	_processCollisionsTotalTime = 0;

	_renderingHighestTime = 0;
	_updateVisualsHighestTime = 0;
	_updateLogicHighestTime = 0;
	_updatePhysicsHighestTime = 0;
	_updateTransformationsHighestTime = 0;
	_streamingHighestTime = 0;
	_handleInputHighestTime = 0;
	_dispatchDelayedMessageHighestTime = 0;
	_processCollisionsHighestTime = 0;
	_renderingHighestTime = 0;

	_renderingProcessTime = 0;

	_gameFrameHighestEffectiveDuration = 0;

	_processNameDuringGAMESTART = NULL;
	_processNameDuringXPEND = NULL;
#endif
}
