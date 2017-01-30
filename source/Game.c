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
#include <Utilities.h>
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
	kCleanAndSwapState,
	kPushState,
	kPopState
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
        /* current process flags */																		\
        u32 gameFrameDone;																			    \
        u32 updatingVisuals;																			\
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
static u32 Game_handleInput(Game this);
inline static u32 Game_dispatchDelayedMessages(Game this);
inline static void Game_updateVisuals(Game this);
inline static void Game_updateLogic(Game this);
inline static void Game_updatePhysics(Game this);
inline static void Game_updateTransformations(Game this);
inline static u32 Game_updateCollisions(Game this);
inline static void Game_stream(Game this);
inline static void Game_render(Game this);
inline static void Game_checkForNewState(Game this);
inline static void Game_checkFrameRate(Game this, u32 gameFrameDuration);
static void Game_update(Game this);
static void Game_autoPause(Game this);
#ifdef __LOW_BATTERY_INDICATOR
static void Game_checkLowBattery(Game this, u16 keyPressed);
static void Game_printLowBatteryIndicator(Game this, bool showIndicator);
#endif
#ifdef __PROFILE_GAME_DETAILED
static void Game_showProfiling(Game this, int x, int y);
#endif

#ifdef __PROFILE_GAME
static void Game_showLastGameFrameProfiling(Game this __attribute__ ((unused)), int x, int y);
#endif

#ifdef __PROFILE_STREAMING
void Stage_showStreamingProfiling(Stage this, int x, int y);
#endif

void MessageDispatcher_processDiscardedMessages(MessageDispatcher this);
u32 VIPManager_writeDRAM(VIPManager this);

#ifdef __PROFILE_GAME
static u32 updateVisualsTotalTime = 0;
static u32 updateLogicTotalTime = 0;
static u32 updatePhysicsTotalTime = 0;
static u32 updateTransformationsTotalTime = 0;
static u32 streamingTotalTime = 0;
static u32 handleInputTotalTime = 0;
static u32 dispatchDelayedMessageTotalTime = 0;
static u32 renderingTotalTime = 0;

static u32 updateVisualsHighestTime = 0;
static u32 updateLogicHighestTime = 0;
static u32 streamingHighestTime = 0;
static u32 updatePhysicsHighestTime = 0;
static u32 updateTransformationsHighestTime = 0;
static u32 handleInputHighestTime = 0;
static u32 dispatchDelayedMessageHighestTime = 0;
static u32 processCollisionsTotalTime = 0;
static u32 processCollisionsHighestTime = 0;
static u32 renderingHighestTime = 0;

static u32 gameFrameProcessTime = 0;
static u32 updateVisualsProcessTime = 0;
static u32 updateLogicProcessTime = 0;
static u32 streamingProcessTime = 0;
static u32 updatePhysicsProcessTime = 0;
static u32 updateTransformationsProcessTime = 0;
static u32 handleInputProcessTime = 0;
static u32 dispatchDelayedMessageProcessTime = 0;
static u32 processCollisionsProcessTime = 0;
static u32 renderingProcessTime = 0;

#endif


#ifdef __PROFILE_GAME_DETAILED
static u32 gameFrameHighestTime = 0;
#endif

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
	this->gameFrameDone = false;
	this->updatingVisuals = false;

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

#ifdef __ANIMATION_EDITOR
	AnimationEditorState_getInstance();
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

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
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

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
			this->lastProcessName = "state push";
#endif
			// setup new state
		    StateMachine_pushState(this->stateMachine, (State)state);
			break;

		case kPopState:

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
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
    TimerManager_resetMilliseconds(this->timerManager);
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
    Screen_setFocusInGameEntity(this->screen, NULL);
	BgmapTextureManager_reset(BgmapTextureManager_getInstance());
	CharSetManager_reset(CharSetManager_getInstance());
	ParamTableManager_reset(ParamTableManager_getInstance());
	SpriteManager_reset(SpriteManager_getInstance());
	MBackgroundManager_reset(MBackgroundManager_getInstance());
	AnimationCoordinatorFactory_reset(AnimationCoordinatorFactory_getInstance());
	Printing_reset(Printing_getInstance());

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
    u32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

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
		return true;
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
		return true;
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
		return true;
	}
#endif

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(this) && ((pressedKey & K_LT) || (pressedKey & K_RT)))
	{
		KeypadManager_clear(this->keypadManager);
		return true;
	}
#endif

#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(this) && ((pressedKey & K_LT) || (pressedKey & K_RT)))
	{
		KeypadManager_clear(this->keypadManager);
		return true;
	}
#endif

#ifdef __ANIMATION_EDITOR
	if(!Game_isInSpecialMode(this) && ((pressedKey & K_LT) || (pressedKey & K_RT)))
	{
		KeypadManager_clear(this->keypadManager);
		return true;
	}
#endif

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
	this->lastProcessName = "input handling";
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

#ifdef __PROFILE_GAME
    handleInputProcessTime = TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
    handleInputHighestTime = handleInputProcessTime > handleInputHighestTime ? handleInputProcessTime : handleInputHighestTime;
    handleInputTotalTime += handleInputProcessTime;
#endif

    return pressedKey | releasedKey;
}

inline static u32 Game_dispatchDelayedMessages(Game this __attribute__ ((unused)))
{
#ifdef __PROFILE_GAME
    u32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
	this->lastProcessName = "dispatching messages";
#endif

#ifdef __RUN_DELAYED_MESSAGES_DISPATCHING_AT_HALF_FRAME_RATE
    static u32 dispatchCycle = 0;

    if(dispatchCycle++ % 2)
    {
#endif

#ifdef __PROFILE_GAME
        u32 dispatchedMessages = MessageDispatcher_dispatchDelayedMessages(MessageDispatcher_getInstance());

        dispatchDelayedMessageProcessTime = TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
        dispatchDelayedMessageHighestTime = dispatchDelayedMessageProcessTime > dispatchDelayedMessageHighestTime ? dispatchDelayedMessageProcessTime : dispatchDelayedMessageHighestTime;
        dispatchDelayedMessageTotalTime += dispatchDelayedMessageProcessTime;

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
    u32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(this))
	{
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(this))
	{
#endif
#ifdef __ANIMATION_EDITOR
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
#ifdef __ANIMATION_EDITOR
	}
#endif

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
	this->lastProcessName = "state machine update";
#endif

	// update the game's logic
	StateMachine_update(this->stateMachine);

#ifdef __PROFILE_GAME
    updateLogicProcessTime = TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
    updateLogicHighestTime = updateLogicProcessTime > updateLogicHighestTime ? updateLogicProcessTime : updateLogicHighestTime;
    updateLogicTotalTime += updateLogicProcessTime;
#endif
}

// update game's rendering subsystem
inline static void Game_updateVisuals(Game this __attribute__ ((unused)))
{
#ifdef __PROFILE_GAME
    u32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
	this->lastProcessName = "visuals update";
#endif

	this->updatingVisuals = true;

	// apply transformations to visuals
	GameState_updateVisuals(this->currentState);

	this->updatingVisuals = false;

#ifdef __PROFILE_GAME
    updateVisualsProcessTime = TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
    updateVisualsHighestTime = updateVisualsProcessTime > updateVisualsHighestTime ? updateVisualsProcessTime : updateVisualsHighestTime;
    updateVisualsTotalTime += updateVisualsProcessTime;
#endif
}

// update game's physics subsystem
inline static void Game_updatePhysics(Game this)
{
#ifdef __PROFILE_GAME
    u32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
	this->lastProcessName = "physics processing";
#endif

	// simulate physics
	GameState_updatePhysics(this->currentState);

#ifdef __PROFILE_GAME
    updatePhysicsProcessTime = TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
    updatePhysicsHighestTime = updatePhysicsProcessTime > updatePhysicsHighestTime ? updatePhysicsProcessTime : updatePhysicsHighestTime;
    updatePhysicsTotalTime += updatePhysicsProcessTime;
#endif
}

inline static void Game_updateTransformations(Game this)
{
#ifdef __PROFILE_GAME
    u32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
	this->lastProcessName = "screen focusing";
#endif
	// position the screen
	Screen_focus(this->screen, true);

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
	this->lastProcessName = "transformation";
#endif

	// apply world transformations
	GameState_transform(this->currentState);

#ifdef __PROFILE_GAME
    updateTransformationsProcessTime = TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
    updateTransformationsHighestTime = updateTransformationsProcessTime > updateTransformationsHighestTime ? updateTransformationsProcessTime : updateTransformationsHighestTime;
    updateTransformationsTotalTime += updateTransformationsProcessTime;
#endif
}

inline static u32 Game_updateCollisions(Game this)
{
#ifdef __PROFILE_GAME
    u32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

	// process the collisions after the transformations have taken place
#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
	this->lastProcessName = "collisions handling";
#endif

	// process collisions
#ifdef __PROFILE_GAME
	u32 processedCollisions = GameState_processCollisions(this->currentState);

    processCollisionsProcessTime = TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
    processCollisionsHighestTime = processCollisionsProcessTime > processCollisionsHighestTime ? processCollisionsProcessTime : processCollisionsHighestTime;
    processCollisionsTotalTime += processCollisionsProcessTime;

    return processedCollisions;
#else
    return GameState_processCollisions(this->currentState);
#endif
}

inline static void Game_stream(Game this)
{
#ifdef __PROFILE_GAME
    u32 timeBeforeProcess = TimerManager_getMillisecondsElapsed(this->timerManager);
#endif

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
    this->lastProcessName = "streaming";
#endif

    GameState_stream(this->currentState);

#ifdef __PROFILE_GAME
    streamingProcessTime = TimerManager_getMillisecondsElapsed(this->timerManager) - timeBeforeProcess;
    streamingHighestTime = streamingProcessTime > streamingHighestTime ? streamingProcessTime :  streamingHighestTime;
    streamingTotalTime += streamingProcessTime;
#endif
}

inline static void Game_render(Game this)
{
#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
    this->lastProcessName = "rendering";
#endif


#ifdef __PROFILE_GAME
    renderingProcessTime = VIPManager_writeDRAM(this->vipManager);
    renderingHighestTime = renderingProcessTime > renderingHighestTime ? renderingProcessTime : renderingHighestTime;
    renderingTotalTime += renderingProcessTime;
#else
    VIPManager_writeDRAM(this->vipManager);
#endif
}

inline static void Game_checkForNewState(Game this)
{
	ASSERT(this, "Game::checkForNewState: null this");

    if(this->nextState)
	{
#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
		this->lastProcessName = "next state setting up";
#endif
		Game_setNextState(this, this->nextState);

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

inline static void Game_checkFrameRate(Game this, u32 gameFrameDuration)
{
    FrameRate frameRate = FrameRate_getInstance();

    this->gameFrameTotalTime += gameFrameDuration;

    // increase the fps counter
    FrameRate_increaseFps(frameRate);

    if(this->gameFrameTotalTime >= __MILLISECONDS_IN_SECOND)
    {
#ifdef __PROFILE_GAME_DETAILED
        Game_showProfiling(this, 0, 3);
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
}

// update game's subsystems
static void Game_update(Game this)
{
	ASSERT(this, "Game::update: null this");

#if __FRAME_CYCLE == 1
	bool cycle = true;
#endif

	while(true)
	{
    	this->gameFrameDone = true;

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
        this->lastProcessName = "game cycle done";
#endif

#ifdef __PROFILE_GAME_DETAILED
        u32 gameFrameTotalTime = TimerManager_getMillisecondsElapsed(this->timerManager);

        gameFrameHighestTime = gameFrameHighestTime < gameFrameTotalTime ?  gameFrameTotalTime : gameFrameHighestTime;
#endif

		// update each subsystem
		// wait to sync with the game start to render
		// this wait actually controls the frame rate
		while(VIPManager_waitForFrameStart(this->vipManager));
		VIPManager_resetGameFrameStarted(this->vipManager);

    	this->gameFrameDone = false;

	    u32 gameFrameDuration = TimerManager_getMillisecondsElapsed(this->timerManager);

	    TimerManager_resetMilliseconds(this->timerManager);

#ifdef __PROFILE_GAME

        static u32 cycleCount = 0;
        static u32 totalGameFrameDuration = 0;
        totalGameFrameDuration += gameFrameDuration;

        Printing_text(Printing_getInstance(), "GFT(ms):   ", 7, 0, NULL);
        Printing_int(Printing_getInstance(), gameFrameDuration, 15, 0, NULL);
        Printing_text(Printing_getInstance(), "AVG:", 18, 0, NULL);
        Printing_int(Printing_getInstance(), totalGameFrameDuration / ++cycleCount, 22, 0, NULL);

        if(gameFrameDuration > __GAME_FRAME_DURATION)
        {
            static u32 tornFrames = 0;

            Printing_text(Printing_getInstance(), "HI:   ", 25, 0, NULL);
            Printing_int(Printing_getInstance(), gameFrameDuration, 28, 0, NULL);
            Printing_text(Printing_getInstance(), "(   )", 30, 0, NULL);
            Printing_int(Printing_getInstance(), ++tornFrames, 31, 0, NULL);
        }
#endif

        gameFrameDuration = gameFrameDuration < __GAME_FRAME_DURATION? __GAME_FRAME_DURATION : gameFrameDuration;

	    // the engine's game logic must be free of racing
	    // conditions against the VIP
	    Game_updateVisuals(this);

        // increase game frame total time
    	Game_checkFrameRate(this, gameFrameDuration);

        // update the clocks
        ClockManager_update(this->clockManager, gameFrameDuration);

	    // register the frame buffer in use by the VPU's drawing process
	    VIPManager_registerCurrentDrawingFrameBufferSet(this->vipManager);

		// update each subsystem
#if __FRAME_CYCLE == 1
	    if(cycle)
	    {
#endif
		// this is the moment to check if the game's state
		// needs to be changed
		Game_checkForNewState(this);

        // process user's input
        u32 suspendNonCriticalProcesses = Game_handleInput(this);

        // dispatch delayed messages
        if(!suspendNonCriticalProcesses)
        {
            suspendNonCriticalProcesses = Game_dispatchDelayedMessages(this);
        }

        // update game's logic
        Game_updateLogic(this);

#if __FRAME_CYCLE == 1
		cycle = false;
	    }
	    else
	    {
#endif

		// physics' update takes place after game's logic
		// has been done
		Game_updatePhysics(this);

	    // apply transformations
	    Game_updateTransformations(this);

        // process collisions
        suspendNonCriticalProcesses |= Game_updateCollisions(this);

#ifdef __PROFILE_GAME
        streamingProcessTime = 0;
#endif
        if(!suspendNonCriticalProcesses)
        {
            Game_stream(this);
        }

//        Game_render(this);

#ifdef __PROFILE_GAME
        {
            static u32 cycleCount = 0;
            u32 gameFrameRealDuration = gameFrameProcessTime +
                                        updateVisualsProcessTime +
                                        updateLogicProcessTime +
                                        streamingProcessTime +
                                        updatePhysicsProcessTime +
                                        updateTransformationsProcessTime +
                                        handleInputProcessTime +
                                        dispatchDelayedMessageProcessTime +
                                        processCollisionsProcessTime +
                                        renderingProcessTime;
            static u32 totalGameFrameRealDuration = 0;
            totalGameFrameRealDuration += gameFrameRealDuration;


            Printing_text(Printing_getInstance(), "GFT(ms):   ", 7, 1, NULL);
            Printing_int(Printing_getInstance(), gameFrameRealDuration, 15, 1, NULL);
            Printing_text(Printing_getInstance(), "AVG:", 18, 1, NULL);
            Printing_int(Printing_getInstance(), totalGameFrameRealDuration / ++cycleCount, 22, 1, NULL);

            if(gameFrameRealDuration > __GAME_FRAME_DURATION)
            {
                static u32 tornFrames = 0;

                Printing_text(Printing_getInstance(), "HI:   ", 25, 1, NULL);
                Printing_int(Printing_getInstance(), gameFrameRealDuration, 28, 1, NULL);
                Printing_text(Printing_getInstance(), "(   )", 30, 1, NULL);
                Printing_int(Printing_getInstance(), ++tornFrames, 31, 1, NULL);

                Game_showLastGameFrameProfiling(this, 24, 3);
            }

        }
#endif

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

			Game_printLowBatteryIndicator(this, (Telegram_getExtraInfo(telegram)? true: false));
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

void Game_addPostProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject)
{
	ASSERT(this, "Game::addPostProcessingEffect: null this");

    VIPManager_addPostProcessingEffect(this->vipManager, postProcessingEffect, spatialObject);
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

#ifdef __PROFILE_GAME_STATE_DURING_VIP_INTERRUPT
bool Game_isGameFrameDone(Game this)
{
	ASSERT(this, "Game::isGameFrameDone: null this");

	return this->gameFrameDone;
}
#endif

#ifdef __PROFILE_GAME_DETAILED
static void Game_showProfiling(Game this __attribute__ ((unused)), int x, int y)
{
    ASSERT(this, "Game::showProfiling: this null");

    int xDisplacement = 6;

    Printing_text(Printing_getInstance(), "GAME PROFILING", x, y++, NULL);

    Printing_text(Printing_getInstance(), "Time:    ", x, y, NULL);
    Printing_int(Printing_getInstance(), this->gameFrameTotalTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Rend.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), renderingTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), renderingHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Visl.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateVisualsTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), updateVisualsHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Inpt.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), handleInputTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), handleInputHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Mess.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), dispatchDelayedMessageTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), dispatchDelayedMessageHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Logic:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateLogicTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), updateLogicHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Strm.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), streamingTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), streamingHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Phys.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), updatePhysicsTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), updatePhysicsHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Trnf.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateTransformationsTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), updateTransformationsHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "Coll.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), processCollisionsTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), processCollisionsHighestTime, x + xDisplacement + 4, y++, NULL);

    Printing_text(Printing_getInstance(), "TOTAL:      ", x, y, NULL);
    Printing_int(Printing_getInstance(), renderingTotalTime + updateVisualsTotalTime + handleInputTotalTime + dispatchDelayedMessageTotalTime + updateLogicTotalTime + streamingTotalTime + updatePhysicsTotalTime + updateTransformationsTotalTime + processCollisionsTotalTime, x + xDisplacement, y, NULL);
    Printing_int(Printing_getInstance(), gameFrameHighestTime, x + xDisplacement + 4, y++, NULL);

    updateVisualsTotalTime = 0;
    updateLogicTotalTime = 0;
    updatePhysicsTotalTime = 0;
    updateTransformationsTotalTime = 0;
    streamingTotalTime = 0;
    handleInputTotalTime = 0;
    dispatchDelayedMessageTotalTime = 0;
    processCollisionsTotalTime = 0;
    renderingTotalTime = 0;

    gameFrameHighestTime = 0;
    updateVisualsHighestTime = 0;
    updateLogicHighestTime = 0;
    updatePhysicsHighestTime = 0;
    updateTransformationsHighestTime = 0;
    streamingHighestTime = 0;
    handleInputHighestTime = 0;
    dispatchDelayedMessageHighestTime = 0;
    processCollisionsHighestTime = 0;
    renderingHighestTime = 0;

#ifdef __PROFILE_STREAMING
    Stage_showStreamingProfiling(Game_getStage(this), x, ++y);
#endif
}
#endif

#ifdef __PROFILE_GAME
static void Game_showLastGameFrameProfiling(Game this __attribute__ ((unused)), int x, int y)
{
    ASSERT(this, "Game::showProfiling: this null");

    int xDisplacement = 6;

    Printing_text(Printing_getInstance(), "GAME PROFILING", x, y++, NULL);

    Printing_text(Printing_getInstance(), "Rend.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), renderingProcessTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Visl.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateVisualsProcessTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Inpt.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), handleInputProcessTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Mess.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), dispatchDelayedMessageProcessTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Logic:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateLogicProcessTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Strm.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), streamingProcessTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Phys.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), updatePhysicsProcessTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Trnf.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), updateTransformationsProcessTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "Coll.:     ", x, y, NULL);
    Printing_int(Printing_getInstance(), processCollisionsProcessTime, x + xDisplacement, y++, NULL);

    Printing_text(Printing_getInstance(), "TOTAL:      ", x, y, NULL);
    Printing_int(Printing_getInstance(), renderingProcessTime + updateVisualsProcessTime + handleInputProcessTime + dispatchDelayedMessageProcessTime + updateLogicProcessTime + streamingProcessTime + updatePhysicsProcessTime + updateTransformationsProcessTime + processCollisionsProcessTime, x + xDisplacement, y++, NULL);
}
#endif
