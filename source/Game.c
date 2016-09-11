/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
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
#include <GameState.h>
#include <MessageDispatcher.h>
#include <Stage.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <MBackgroundManager.h>
#include <AnimationCoordinatorFactory.h>
#include <StateMachine.h>
#include <Screen.h>
#include <ScreenMovementManager.h>
#include <KeypadManager.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <VIPManager.h>
#include <Printing.h>
#include <I18n.h>
#include <ParticleRemover.h>
#include <debugConfig.h>

#ifdef __DEBUG_TOOLS
#include <DebugState.h>
#endif
#ifdef __STAGE_EDITOR
#include <StageEditorState.h>
#endif
#ifdef __ANIMATION_EDITOR
#include <AnimationEditorState.h>
#endif


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

enum StateOperations
{
	kSwapState = 0,
	kPushState,
	kPopState
};

enum GameCurrentProcess
{
	kGameStartingUp = 0,
	kGameUpdatingVisuals,
	kGameUpdatingVisualsDone,
	kGameTransforming,
	kGameTransformingDone,
	kGameCheckingCollisions,
	kGameCheckingCollisionsDone,
	kGameHandlingUserInput,
	kGameHandlingUserInputDone,
	kGameDispatchingDelayedMessages,
	kGameDispatchingDelayedMessagesDone,
	kGameDeletingParticles,
	kGameDeletingParticlesDone,
	kGameUpdatingStageMachine,
	kGameUpdatingStageMachineDone,
	kGameUpdatingPhysics,
	kGameUpdatingPhysicsDone,
	kGameCheckingForNewState,
	kGameCheckingForNewStateDone,
};

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Game_ATTRIBUTES																					\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* game's state machine */																		\
        StateMachine stateMachine;																		\
        /* game's state machine */																		\
        GameState currentState;																			\
        /* engine's global timer */																		\
        Clock clock;																					\
        /* managers */																					\
        ClockManager clockManager;																		\
        KeypadManager keypadManager;																	\
        VIPManager vipManager;																			\
        TimerManager timerManager;																		\
        Screen screen;																					\
        ParticleRemover particleRemover;                                                                \
        /* game's next state */																			\
        GameState nextState;																			\
        /* game's next state operation */																\
        int nextStateOperation; 																		\
        /* last process' name */																		\
        char* lastProcessName;																			\
        /* auto pause state */											 								\
        GameState automaticPauseState;																	\
        /* auto pause last checked time */																\
        u32 lastAutoPauseCheckTime;																		\
        /* current process enum */																		\
        u32 currentProcess;																				\
        /* elapsed time in current 50hz cycle */														\
        u32 gameFrameTotalTime;																            \
        /* low battery indicator showing flag */														\
        bool isShowingLowBatteryIndicator;																\

__CLASS_DEFINITION(Game, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Game_constructor(Game this);
static void Game_initialize(Game this);
static void Game_setNextState(Game this, GameState state);
static void Game_handleInput(Game this);
static void Game_update(Game this);
inline static void Game_updateVisuals(Game this);
inline static void Game_updateLogic(Game this);
inline static void Game_updatePhysics(Game this);
inline static void Game_updateTransformations(Game this);
inline static void Game_checkForNewState(Game this);
static void Game_autoPause(Game this);
#ifdef __LOW_BATTERY_INDICATOR
static void Game_checkLowBattery(Game this, u16 keyPressed);
static void Game_printLowBatteryIndicator(Game this, bool showIndicator);
#endif
#ifdef __PROFILING
static void Game_showProfiling(Game this);
#endif

void MessageDispatcher_processDiscardedMessages(MessageDispatcher this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
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

	// construct base object
	__CONSTRUCT_BASE(Object);

	// make sure the memory pool is initialized now
	MemoryPool_getInstance();

	// current process
	this->currentProcess = kGameStartingUp;

	this->gameFrameTotalTime = 0;

	// force construction now
	this->clockManager = ClockManager_getInstance();

	// construct the general clock
	this->clock = __NEW(Clock);

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
    this->particleRemover = ParticleRemover_getInstance();

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

#ifdef __ANIMATION_EDITOR
	AnimationEditorState_getInstance();
#endif

	// to make debugging easier
#ifndef __DEBUG
	this->lastProcessName = "not available";
#else
	this->lastProcessName = "starting up";
#endif
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

	// intialize SRAM
	SRAMManager_getInstance();

    // initialize VPU and turn of the brightness
	HardwareManager_displayOn(HardwareManager_getInstance());
    HardwareManager_lowerBrightness(HardwareManager_getInstance());

	if(!StateMachine_getCurrentState(this->stateMachine))
	{
		// register start time for auto pause check
		this->lastAutoPauseCheckTime = Clock_getTime(this->clock);

		// set state
		Game_setNextState(this, state);

		// start game's cycle
		Game_update(this);
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
		case kSwapState:

#ifdef __DEBUG
			this->lastProcessName = "kSwapState";
#endif

			if(this->currentState)
			{
				// discard delayed messages from the current state
				MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher_getInstance(), GameState_getInGameClock(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine))));
				MessageDispatcher_processDiscardedMessages(MessageDispatcher_getInstance());
			}

			// setup new state
		    StateMachine_swapState(this->stateMachine, (State)state);
			break;

		case kPushState:

#ifdef __DEBUG
			this->lastProcessName = "kPushState";
#endif
			// setup new state
		    StateMachine_pushState(this->stateMachine, (State)state);
			break;

		case kPopState:

#ifdef __DEBUG
			this->lastProcessName = "kPopState";
#endif

			if(this->currentState)
			{
				// discard delayed messages from the current state
			    MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher_getInstance(), GameState_getInGameClock(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine))));
				MessageDispatcher_processDiscardedMessages(MessageDispatcher_getInstance());
			}

			// setup new state
		    StateMachine_popState(this->stateMachine);
			break;
    }

    // TODO: crashes on Mednafen
    // enable hardware pad read
    //HardwareManager_enableKeypad(HardwareManager_getInstance());

	// load chars into graphic memory
	Printing_loadFonts(Printing_getInstance());

	// disable rendering
	HardwareManager_enableRendering(HardwareManager_getInstance());

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

    // reset timer's ticks
    TimerManager_getAndResetTicks(this->timerManager);
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

	// enable rendering
	HardwareManager_enableRendering(HardwareManager_getInstance());
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
    Screen_setFocusInGameEntity(this->screen, NULL);
    ParticleRemover_reset(this->particleRemover);
	BgmapTextureManager_reset(BgmapTextureManager_getInstance());
	CharSetManager_reset(CharSetManager_getInstance());
	ParamTableManager_reset(ParamTableManager_getInstance());
	SpriteManager_reset(SpriteManager_getInstance());
	MBackgroundManager_reset(MBackgroundManager_getInstance());
	AnimationCoordinatorFactory_reset(AnimationCoordinatorFactory_getInstance());

	// load chars into graphic memory
	Printing_loadFonts(Printing_getInstance());

	// TODO
	//SoundManager_getInstance();
}

// process input data according to the actual game status
static void Game_handleInput(Game this)
{
	ASSERT(this, "Game::handleInput: null this");

	if(!KeypadManager_isEnabled(this->keypadManager))
	{
		return;
	}

	// poll the user's input
	KeypadManager_read(this->keypadManager);

	u32 pressedKey = KeypadManager_getPressedKey(this->keypadManager);
	u32 releasedKey = KeypadManager_getReleasedKey(this->keypadManager);
	u32 holdKey = KeypadManager_getHoldKey(this->keypadManager);

#ifdef __DEBUG_TOOLS
	u32 previousKey = KeypadManager_getPreviousKey(this->keypadManager);
#else

#ifdef __STAGE_EDITOR
	u32 previousKey = KeypadManager_getPreviousKey(this->keypadManager);
#else

#ifdef __ANIMATION_EDITOR
	u32 previousKey = KeypadManager_getPreviousKey(this->keypadManager);
#endif

#endif

#endif

#ifdef __DEBUG_TOOLS

	// check code to access special feature
	if((previousKey & K_LT) && (previousKey & K_RT) && (pressedKey & K_RU))
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

		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

#ifdef __STAGE_EDITOR

	// check code to access special feature
	if((previousKey & K_LT) && (previousKey & K_RT) && (pressedKey & K_RD))
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

		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

#ifdef __ANIMATION_EDITOR

	// check code to access special feature
	if((previousKey & K_LT) && (previousKey & K_RT) && (releasedKey & K_RR))
	{

		if(Game_isInAnimationEditor(this))
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

			this->nextState = __SAFE_CAST(GameState, AnimationEditorState_getInstance());
			StateMachine_pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(this) && ((pressedKey & K_LT) || (pressedKey & K_RT)))
	{
		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(this) && ((pressedKey & K_LT) || (pressedKey & K_RT)))
	{
		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

#ifdef __ANIMATION_EDITOR
	if(!Game_isInSpecialMode(this) && ((pressedKey & K_LT) || (pressedKey & K_RT)))
	{
		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

	this->currentProcess = kGameHandlingUserInput;

#ifdef __DEBUG
	this->lastProcessName = "handle input";
#endif

	// check for a new key pressed
	if(pressedKey)
	{
		// inform the game about the pressed key
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this->stateMachine), __SAFE_CAST(Object, this->stateMachine), kKeyPressed, &pressedKey);
	}

	if(releasedKey)
	{
		// inform the game about the released key
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this->stateMachine), __SAFE_CAST(Object, this->stateMachine), kKeyReleased, &releasedKey);
	}

	if(holdKey)
	{
		// inform the game about the hold key
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this->stateMachine), __SAFE_CAST(Object, this->stateMachine), kKeyHold, &holdKey);
	}

	KeypadManager_clear(this->keypadManager);

#ifdef __LOW_BATTERY_INDICATOR
    Game_checkLowBattery(this, holdKey);
#endif
}

// update game's logic subsystem
inline static void Game_updateLogic(Game this)
{
	this->currentProcess = kGameDispatchingDelayedMessages;

#ifdef __DEBUG
	this->lastProcessName = "dispatch delayed messages";
#endif
#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(this))
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(this))
#endif
#ifdef __ANIMATION_EDITOR
	if(!Game_isInSpecialMode(this))
#endif
	// dispatch queued messages
//    MessageDispatcher_dispatchDelayedMessages(MessageDispatcher_getInstance());

	// it is the update cycle
	ASSERT(this->stateMachine, "Game::update: no state machine");

	this->currentProcess = kGameUpdatingStageMachine;

#ifdef __DEBUG
	this->lastProcessName = "updating state machine";
#endif

	// update the game's logic
	StateMachine_update(this->stateMachine);

#ifdef __DEBUG
	this->lastProcessName = "deleting particles";
#endif

	this->currentProcess = kGameDeletingParticles;

    ParticleRemover_update(this->particleRemover);

	this->currentProcess = kGameDeletingParticlesDone;

#ifdef __DEBUG
	this->lastProcessName = "logic ended";
#endif
}

// update game's rendering subsystem
inline static void Game_updateVisuals(Game this __attribute__ ((unused)))
{
#ifdef __DEBUG
	this->lastProcessName = "update visuals";
#endif

	this->currentProcess = kGameUpdatingVisuals;

#ifdef __FORCE_VIP_SYNC
	// disable rendering until collisions have been checked
	VIPManager_disableInterrupt(this->vipManager);
#endif

#ifdef __DEBUG
	this->lastProcessName = "update visuals";
#endif

	// apply transformations to visuals
	GameState_updateVisuals(this->currentState);

	this->currentProcess = kGameUpdatingVisualsDone;

#ifdef __FORCE_VIP_SYNC
	// allow rendering
	VIPManager_enableInterrupt(this->vipManager);
#endif

#ifdef __DEBUG
	this->lastProcessName = "update visuals ended";
#endif
}

// update game's physics subsystem
inline static void Game_updatePhysics(Game this)
{
#ifdef __DEBUG
	this->lastProcessName = "update physics";
#endif

	this->currentProcess = kGameUpdatingPhysics;

	// simulate physics
	GameState_updatePhysics(this->currentState);

#ifdef __DEBUG
	this->lastProcessName = "physics ended";
#endif
}

// update game's rendering subsystem
inline static void Game_updateTransformations(Game this)
{
#ifdef __DEBUG
	this->lastProcessName = "move screen";
#endif

	// position the screen
	Screen_focus(this->screen, true);

	this->currentProcess = kGameTransforming;

#ifdef __DEBUG
	this->lastProcessName = "apply transformations";
#endif

	// apply world transformations
	GameState_transform(this->currentState);

	this->currentProcess = kGameCheckingCollisions;

	// process the collisions after the transformations have taken place
#ifdef __DEBUG
	this->lastProcessName = "process collisions";
#endif

	// process collisions
	GameState_processCollisions(this->currentState);

	this->currentProcess = kGameCheckingCollisionsDone;

#ifdef __DEBUG
	this->lastProcessName = "transformations ended";
#endif
}

inline static void Game_checkForNewState(Game this)
{
	ASSERT(this, "Game::checkForNewState: null this");

	this->currentProcess = kGameCheckingForNewState;
    if(this->nextState)
	{
#ifdef __DEBUG
		this->lastProcessName = "setting next state";
#endif
		Game_setNextState(this, this->nextState);
#ifdef __DEBUG
		this->lastProcessName = "setting next state done";
#endif
	}

	this->currentProcess = kGameCheckingForNewStateDone;
}

#ifdef __PROFILING
static u32 updateVisualsTotalTime = 0;
static u32 updateLogicTotalTime = 0;
static u32 updatePhysicsTotalTime = 0;
static u32 updateTransformationsTotalTime = 0;
static u32 streamingTotalTime = 0;
static u32 handleInputTotalTime = 0;
static u32 dispatchDelayedMessageTotalTime = 0;

static u32 gameFrameHighestTime = 0;
static u32 updateVisualsHighestTime = 0;
static u32 updateLogicHighestTime = 0;
static u32 streamingHighestTime = 0;
static u32 updatePhysicsHighestTime = 0;
static u32 updateTransformationsHighestTime = 0;
static u32 handleInputHighestTime = 0;
static u32 dispatchDelayedMessageHighestTime = 0;
#endif

// update game's subsystems
static void Game_update(Game this)
{
	ASSERT(this, "Game::update: null this");

	FrameRate frameRate = FrameRate_getInstance();

#ifdef __PROFILING
    u32 timeBeforeProcess = 0;
    u32 processTime = 0;
#endif

#if __FRAME_CYCLE == 1
	bool cycle = true;
#endif

	while(true)
	{
		// update each subsystem
		// wait to sync with the game start to render
		// this wait actually controls the frame rate
		if(TimerManager_getTicks(this->timerManager) <=  __MILLISECONDS_IN_SECOND / __TARGET_FPS)
		{
	        while(!(_vipRegisters[__INTPND] & __GAMESTART));
	    }

	    _vipRegisters[__INTCLR]= __GAMESTART;

#ifdef __PROFILING
        u32 gameFrameTotalTime = 0;

	    timeBeforeProcess = TimerManager_getTicks(this->timerManager);
#endif

	    // the engine's game logic must be free of racing
	    // conditions against the VIP
	    Game_updateVisuals(this);

#ifdef __PROFILING
        processTime = TimerManager_getTicks(this->timerManager) - timeBeforeProcess;
        updateVisualsHighestTime = processTime > updateVisualsHighestTime? processTime: updateVisualsHighestTime;
	    updateVisualsTotalTime += processTime;
	    gameFrameTotalTime += processTime;
#endif

#ifdef __CAP_FRAMERATE
	    // cap framerate
	    volatile u32 gameFrameTime = 0;

#if __FRAME_CYCLE != 1
	    do
	    {
#endif
            gameFrameTime = TimerManager_getTicks(this->timerManager);
#if __FRAME_CYCLE != 1
	    }
	    while(gameFrameTime < __MILLISECONDS_IN_SECOND / __TARGET_FPS);
#endif

        TimerManager_getAndResetTicks(this->timerManager);
#else
	    u32 gameFrameTime = TimerManager_getAndResetTicks(this->timerManager);
#endif

        // inclease game frame total time
    	this->gameFrameTotalTime += gameFrameTime;

        if(this->gameFrameTotalTime >= __MILLISECONDS_IN_SECOND)
        {
#ifdef __PROFILING
            Game_showProfiling(this);
#endif

            this->gameFrameTotalTime = 0;

#ifdef __DEBUG
        	Printing_text(Printing_getInstance(), "DEBUG MODE", 0, (__SCREEN_HEIGHT >> 3) - 1, NULL);
#endif

#ifdef __PRINT_FRAMERATE
            if(!Game_isInSpecialMode(this))
            {
                FrameRate_print(frameRate, 0, 0);
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

        // update the clocks
        ClockManager_update(this->clockManager, gameFrameTime);

	    // register the frame buffer in use by the VPU's drawing process
	    VIPManager_registerCurrentDrawingframeBufferSet(this->vipManager);

		// update each subsystem
#if __FRAME_CYCLE == 1
	    if(cycle)
	    {
#endif

		// this is the moment to check if the game's state
		// needs to be changed
		Game_checkForNewState(this);

#ifdef __PROFILING
	    timeBeforeProcess = TimerManager_getTicks(this->timerManager);
#endif
        // process user's input
        Game_handleInput(this);
#ifdef __PROFILING
        processTime = TimerManager_getTicks(this->timerManager) - timeBeforeProcess;
        handleInputHighestTime = processTime > handleInputHighestTime? processTime: handleInputHighestTime;
	    handleInputTotalTime += processTime;
	    gameFrameTotalTime += processTime;
#endif

#ifdef __PROFILING
	    timeBeforeProcess = TimerManager_getTicks(this->timerManager);
#endif
        MessageDispatcher_dispatchDelayedMessages(MessageDispatcher_getInstance());
#ifdef __PROFILING
        processTime = TimerManager_getTicks(this->timerManager) - timeBeforeProcess;
        dispatchDelayedMessageHighestTime = processTime > dispatchDelayedMessageHighestTime? processTime: dispatchDelayedMessageHighestTime;
	    dispatchDelayedMessageTotalTime += processTime;
	    gameFrameTotalTime += processTime;
#endif

#ifdef __PROFILING
	    timeBeforeProcess = TimerManager_getTicks(this->timerManager);
#endif
	    // update game's logic
	    Game_updateLogic(this);
#ifdef __PROFILING
        processTime = TimerManager_getTicks(this->timerManager) - timeBeforeProcess;
        updateLogicHighestTime = processTime > updateLogicHighestTime? processTime: updateLogicHighestTime;
	    updateLogicTotalTime += processTime;
	    gameFrameTotalTime += processTime;
#endif

#ifdef __PROFILING
	    timeBeforeProcess = TimerManager_getTicks(this->timerManager);
#endif
        GameState_stream(this->currentState);
#ifdef __PROFILING
        processTime = TimerManager_getTicks(this->timerManager) - timeBeforeProcess;
        streamingHighestTime = processTime > streamingTotalTime? processTime: streamingHighestTime;
	    streamingTotalTime += processTime;
	    gameFrameTotalTime += processTime;
#endif

#if __FRAME_CYCLE == 1
		cycle = false;
	    }
	    else
	    {
#endif

#ifdef __PROFILING
	    timeBeforeProcess = TimerManager_getTicks(this->timerManager);
#endif
		// physics' update takes place after game's logic
		// has been done
		Game_updatePhysics(this);
#ifdef __PROFILING
        processTime = TimerManager_getTicks(this->timerManager) - timeBeforeProcess;
        updatePhysicsHighestTime = processTime > updatePhysicsHighestTime? processTime: updatePhysicsHighestTime;
	    updatePhysicsTotalTime += processTime;
	    gameFrameTotalTime += processTime;
#endif

#ifdef __PROFILING
	    timeBeforeProcess = TimerManager_getTicks(this->timerManager);
#endif
	    // apply transformations
	    Game_updateTransformations(this);
#ifdef __PROFILING
        processTime = TimerManager_getTicks(this->timerManager) - timeBeforeProcess;
        updateTransformationsHighestTime = processTime > updateTransformationsHighestTime? processTime: updateTransformationsHighestTime;
	    updateTransformationsTotalTime += processTime;
	    gameFrameTotalTime += processTime;

        gameFrameHighestTime = gameFrameHighestTime < gameFrameTotalTime?  gameFrameTotalTime: gameFrameHighestTime;
#endif

	    // increase the FPS counter
		FrameRate_increaseFPS(frameRate);

#if __FRAME_CYCLE == 1
		cycle = true;
	    }
#endif
	}
}

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

			Game_printLowBatteryIndicator(this, ((bool)Telegram_getExtraInfo(telegram)));
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
Clock Game_getInGameClock(Game this)
{
	ASSERT(this, "Game::getInGameClock: null this");

	return GameState_getInGameClock(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

// retrieve animations' clock
Clock Game_getAnimationsClock(Game this)
{
	ASSERT(this, "Game::getAnimationsClock: null this");

	return GameState_getAnimationsClock(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
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

#ifdef __ANIMATION_EDITOR
bool Game_isInAnimationEditor(Game this)
{
	ASSERT(this, "Game::isInAnimationEditor: null this");

	return StateMachine_getCurrentState(this->stateMachine) == (State)AnimationEditorState_getInstance();
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
#ifdef __ANIMATION_EDITOR
	isInSpecialMode |= Game_isInAnimationEditor(this);
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
#ifdef __ANIMATION_EDITOR
	isEnteringSpecialMode |= __SAFE_CAST(GameState, AnimationEditorState_getInstance()) == this->nextState;
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
#ifdef __ANIMATION_EDITOR
	isEnteringSpecialMode |= __SAFE_CAST(GameState, AnimationEditorState_getInstance()) == this->nextState;
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

	return GameState_getPhysicalWorld(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

CollisionManager Game_getCollisionManager(Game this)
{
	ASSERT(this, "Game::getCollisionManager: null this");

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

    Printing_text(Printing_getInstance(), (showIndicator) ? "\x01\x02" : "  ", __LOW_BATTERY_INDICATOR_POS_X, __LOW_BATTERY_INDICATOR_POS_Y, NULL);
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

// set auto pause flag
void Game_setAutomaticPauseState(Game this, GameState automaticPauseState)
{
	ASSERT(this, "Game::setAutoPause: null this");
	this->automaticPauseState = automaticPauseState;
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

void Game_addPostProcessingEffect(Game this, void (*postProcessingEffect) (u32))
{
	ASSERT(this, "Game::addPostProcessingEffect: null this");

    VIPManager_addPostProcessingEffect(this->vipManager, postProcessingEffect);
}

void Game_removePostProcessingEffect(Game this, void (*postProcessingEffect) (u32))
{
	ASSERT(this, "Game::removePostProcessingEffect: null this");

    VIPManager_removePostProcessingEffect(this->vipManager, postProcessingEffect);
}

void Game_wait(Game this, u32 milliSeconds)
{
    ASSERT(this, "Game::wait: this null");

    TimerManager_wait(this->timerManager, milliSeconds);
}

#ifndef	__FORCE_VIP_SYNC
#ifdef __ALERT_TRANSFORMATIONS_NOT_IN_SYNC_WITH_VIP
bool Game_doneDRAMPrecalculations(Game this)
{
	ASSERT(this, "Game::doneDRAMPrecalculations: null this");
	return this->currentProcess != kGameUpdatingVisuals;
}

const char* Game_getDRAMPrecalculationsStep(Game this)
{
	switch(this->currentProcess)
	{
		case kGameUpdatingVisuals:

			return "Step: updating visuals";
			break;
	}

	return NULL;
}
#endif
#endif

#ifdef __PROFILING
static void Game_showProfiling(Game this __attribute__ ((unused)))
{
    ASSERT(this, "Game::showProfiling: this null");

    int x = 0;
    int xDisplacement = 11;
    int y = 2;

    Printing_text(Printing_getInstance(), "GAME PROFILING", x, y++, NULL);

    Printing_text(Printing_getInstance(), "Total time:      ", x, y, NULL);
    Printing_int(Printing_getInstance(), this->gameFrameTotalTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Visuals:            ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateVisualsTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), updateVisualsHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Input:              ", x, y, NULL);
    Printing_int(Printing_getInstance(), handleInputTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), handleInputHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Messaging:              ", x, y, NULL);
    Printing_int(Printing_getInstance(), dispatchDelayedMessageTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), dispatchDelayedMessageHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Logic:              ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateLogicTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), updateLogicHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Streaming:          ", x, y, NULL);
    Printing_int(Printing_getInstance(), streamingTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), streamingHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Physics:            ", x, y, NULL);
    Printing_int(Printing_getInstance(), updatePhysicsTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), updatePhysicsHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Transf.:            ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateTransformationsTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), updateTransformationsHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "TOTAL:              ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateVisualsTotalTime + handleInputTotalTime + dispatchDelayedMessageTotalTime + updateLogicTotalTime + streamingTotalTime + updatePhysicsTotalTime + updateTransformationsTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), gameFrameHighestTime, x + xDisplacement + 4, y++, NULL);

    updateVisualsTotalTime = 0;
    updateLogicTotalTime = 0;
    updatePhysicsTotalTime = 0;
    updateTransformationsTotalTime = 0;
    streamingTotalTime = 0;
    handleInputTotalTime = 0;
    dispatchDelayedMessageTotalTime = 0;

    gameFrameHighestTime = 0;
    updateVisualsHighestTime = 0;
    updateLogicHighestTime = 0;
    updatePhysicsHighestTime = 0;
    updateTransformationsHighestTime = 0;
    streamingHighestTime = 0;
    handleInputHighestTime = 0;
    dispatchDelayedMessageHighestTime = 0;

#ifdef __STREAMING_PROFILING
    Stage_showProfiling(Game_getStage(this));
#endif
}
#endif
