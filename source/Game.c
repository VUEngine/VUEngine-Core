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
#include <SoundManager.h>
#include <MBackgroundManager.h>
#include <AnimationCoordinatorFactory.h>
#include <StateMachine.h>
#include <Screen.h>
#include <ScreenMovementManager.h>
#include <VPUManager.h>
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
	kPushState,
	kPopState
};


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Game_ATTRIBUTES																					\
																										\
	/* super's attributes */																			\
	Object_ATTRIBUTES;																					\
																										\
	/* game's state machine */																			\
	StateMachine stateMachine;																			\
																										\
	/* engine's global timer */																			\
	Clock clock;																						\
																										\
	/* timer to use for animations */																	\
	Clock animationsClock;																				\
																										\
	/* timer to use for physics */																		\
	Clock physicsClock;																					\
																										\
	/* managers */																						\
	ClockManager clockManager;																			\
	HardwareManager hardwareManager;																	\
	FrameRate frameRate;																				\
	BgmapTextureManager bgmapTextureManager;															\
	CharSetManager charSetManager;																		\
	SoundManager soundManager;																			\
	ParamTableManager paramTableManager;																\
	SpriteManager spriteManager;																		\
	CollisionManager collisionManager;																	\
	PhysicalWorld physicalWorld;																		\
	KeypadManager keypadManager;																		\
	VPUManager vpuManager;																				\
	DirectDraw directDraw;																				\
	I18n i18n;																							\
	Screen screen;																						\
																										\
	/* game's next state */																				\
	GameState nextState;																				\
																										\
	/* game's next state operation */																	\
	int nextStateOperation; 																			\
																										\
	/* last process' name */																			\
	char* lastProcessName;																				\
																										\
	/* auto pause state */											 									\
	GameState automaticPauseState;																		\
																										\
	/* auto pause last checked time */																	\
	u32 lastAutoPauseCheckTime;																			\
																										\
	/* low battery indicator showing flag */															\
	bool isShowingLowBatteryIndicator;																	\

__CLASS_DEFINITION(Game, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Game_constructor(Game this);
static void Game_initialize(Game this);
static void Game_setNextState(Game this, GameState state);
static void Game_handleInput(Game this);
static void Game_update(Game this);
static void Game_updateLogic(Game this);
static void Game_updatePhysics(Game this);
static void Game_updateTransformations(Game this);
static void Game_checkForNewState(Game this);
static void Game_cleanUp(Game this);
static void Game_autoPause(Game this);
#ifdef __LOW_BATTERY_INDICATOR
static void Game_checkLowBattery(Game this, u16 keyPressed);
static void Game_printLowBatteryIndicator(Game this, bool showIndicator);
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
static void Game_constructor(Game this)
{
	ASSERT(this, "Game::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE();

	// make sure the memory pool is initialized now
	MemoryPool_getInstance();

	// force construction now
	this->clockManager = ClockManager_getInstance();

	// construct the general clock
	this->clock = __NEW(Clock);

	// construct the clocks
	this->animationsClock = __NEW(Clock);
	this->physicsClock = __NEW(Clock);

	// construct the game's state machine
	this->stateMachine = __NEW(StateMachine, this);

	this->nextState = NULL;
	this->automaticPauseState = NULL;
	this->lastAutoPauseCheckTime = 0;
	this->isShowingLowBatteryIndicator = false;

	// make sure all managers are initialized now
	this->frameRate  = FrameRate_getInstance();
	this->hardwareManager = HardwareManager_getInstance();
	this->bgmapTextureManager = BgmapTextureManager_getInstance();
	this->paramTableManager =  ParamTableManager_getInstance();
	this->charSetManager = CharSetManager_getInstance();
	this->screen = Screen_getInstance();
	this->soundManager = SoundManager_getInstance();
	this->spriteManager = SpriteManager_getInstance();
	this->collisionManager = CollisionManager_getInstance();
	this->physicalWorld = PhysicalWorld_getInstance();
	this->keypadManager = KeypadManager_getInstance();
	this->vpuManager = VPUManager_getInstance();
	this->directDraw = DirectDraw_getInstance();
	this->i18n = I18n_getInstance();
	
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
	Clock_destructor(this->animationsClock);
	Clock_destructor(this->physicsClock);

	__DELETE(this->stateMachine);

	__SINGLETON_DESTROY;
}

// setup engine parameters
void Game_initialize(Game this)
{
	ASSERT(this, "Game::initialize: null this");

	// setup vectorInterrupts
	HardwareManager_setInterruptVectors(this->hardwareManager);

	// make sure timer interrupts are enable
	HardwareManager_initializeTimer(this->hardwareManager);

    // set waveform data
    SoundManager_setWaveForm(this->soundManager);

    // reset collision manager
    CollisionManager_reset(this->collisionManager);

	// clear sprite memory
    HardwareManager_clearScreen(this->hardwareManager);
    
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

	HardwareManager_displayOn(this->hardwareManager);
    HardwareManager_lowerBrightness(this->hardwareManager);

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

// add a state to the game's state machine's stack
void Game_removeState(Game this, GameState state)
{
	ASSERT(this, "Game::changeState: null this");

	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextStateOperation = kPopState;
}

// set game's state
static void Game_setNextState(Game this, GameState state)
{
	ASSERT(this, "Game::setState: null this");
    ASSERT(state, "Game::setState: setting NULL state");

	// disable rendering
	HardwareManager_disableRendering(HardwareManager_getInstance());

	// set waveform data
    SoundManager_setWaveForm(this->soundManager);

    switch(this->nextStateOperation)
    {
		case kSwapState:

#ifdef __DEBUG
			this->lastProcessName = "kSwapState";
#endif		
	
			if(Game_getCurrentState(this))
			{
				// discard delayed messages from the current state
				MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher_getInstance(), GameState_getInGameClock(Game_getCurrentState(this)));
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

			if(Game_getCurrentState(this))
			{
				// discard delayed messages from the current state
			    MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher_getInstance(), GameState_getInGameClock(Game_getCurrentState(this)));
				MessageDispatcher_processDiscardedMessages(MessageDispatcher_getInstance());
			}

			// setup new state
		    StateMachine_popState(this->stateMachine);
			break;
    }

    // TODO: crashes on Mednafen
    // enable hardware pad read
    //HardwareManager_enableKeypad(this->hardwareManager);

	// load chars into graphic memory
	Printing_loadFonts(Printing_getInstance());

	// disable rendering
	HardwareManager_enableRendering(this->hardwareManager);

	// if automatic pause function is in place
	if(this->automaticPauseState)
	{
		int automaticPauseCheckDelay = __AUTO_PAUSE_DELAY - (Clock_getTime(this->clock) - this->lastAutoPauseCheckTime);
		automaticPauseCheckDelay = 0 > automaticPauseCheckDelay? automaticPauseCheckDelay: automaticPauseCheckDelay;
		
		MessageDispatcher_dispatchMessage((u32)automaticPauseCheckDelay, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kAutoPause, NULL);
		this->lastAutoPauseCheckTime = Clock_getTime(this->clock);
	}

	// no next state now
	this->nextState = NULL;
}

// disable interrupts
void Game_disableHardwareInterrupts(Game this)
{
	ASSERT(this, "Game::disableHardwareInterrupts: null this");

	// disable rendering
	HardwareManager_disableRendering(HardwareManager_getInstance());
}

// enable interrupts
void Game_enableHardwareInterrupts(Game this)
{
	ASSERT(this, "Game::enableHardwareInterrupts: null this");

	// enable rendering
	HardwareManager_enableRendering(this->hardwareManager);
}

// erase engine's current status
void Game_reset(Game this)
{
	ASSERT(this, "Game::reset: null this");

#ifdef	__MEMORY_POOL_CLEAN_UP
	MemoryPool_cleanUp(MemoryPool_getInstance());
#endif
	
	// setup the display
    HardwareManager_clearScreen(this->hardwareManager);
	HardwareManager_setupColumnTable(this->hardwareManager);
    HardwareManager_displayOn(this->hardwareManager);
    HardwareManager_lowerBrightness(this->hardwareManager);

	// reset managers
    Screen_setFocusInGameEntity(this->screen, NULL);
	BgmapTextureManager_reset(this->bgmapTextureManager);
	CharSetManager_reset(this->charSetManager);
	ParamTableManager_reset(this->paramTableManager);
	SpriteManager_reset(this->spriteManager);
	MBackgroundManager_reset(MBackgroundManager_getInstance());
	AnimationCoordinatorFactory_reset(AnimationCoordinatorFactory_getInstance());

	// load chars into graphic memory
	Printing_loadFonts(Printing_getInstance());

#ifdef __DEBUG_NO_FADE
    HardwareManager_displayOn(HardwareManager_getInstance());

	// make sure the brightness is ok
    HardwareManager_upBrightness(HardwareManager_getInstance());
#endif

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

	u16 pressedKey = KeypadManager_getPressedKey(this->keypadManager);
	u16 releasedKey = KeypadManager_getReleasedKey(this->keypadManager);
	u16 holdKey = KeypadManager_getHoldKey(this->keypadManager);
	
#ifdef __DEBUG_TOOLS 
	u16 previousKey = KeypadManager_getPreviousKey(this->keypadManager);
#else
	
#ifdef __STAGE_EDITOR 
	u16 previousKey = KeypadManager_getPreviousKey(this->keypadManager);
#else
	
#ifdef __ANIMATION_EDITOR 
	u16 previousKey = KeypadManager_getPreviousKey(this->keypadManager);
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
	if((previousKey & K_LT) && (previousKey & K_RT) && (pressedKey & K_RR))
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

	// check for a new key pressed
	if(pressedKey)
	{
		// inform the game about the pressed key
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->stateMachine), kKeyPressed, &pressedKey);
	}

	if(releasedKey)
	{
		// inform the game about the released key
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->stateMachine), kKeyReleased, &releasedKey);
	}

	if(holdKey)
	{
		// inform the game about the hold key
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->stateMachine), kKeyHold, &holdKey);
	}

	KeypadManager_clear(this->keypadManager);
	
#ifdef __LOW_BATTERY_INDICATOR
    Game_checkLowBattery(this, holdKey);
#endif
}

// update game's logic subsystem
static void Game_updateLogic(Game this)
{
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
    MessageDispatcher_dispatchDelayedMessages(MessageDispatcher_getInstance());
	
#ifdef __DEBUG
	this->lastProcessName = "handle input";
#endif
	// process user's input
	Game_handleInput(this);
#ifdef __DEBUG
	this->lastProcessName = "update state machines";
#endif
	// it is the update cycle
	ASSERT(this->stateMachine, "Game::update: no state machine");

	// update the game's logic
	StateMachine_update(this->stateMachine);

#ifdef __DEBUG
	this->lastProcessName = "logic ended";
#endif
}

// update game's physics subsystem
static void Game_updatePhysics(Game this)
{
#ifdef __DEBUG
	this->lastProcessName = "update physics";
#endif
	
	fix19_13 elapsedTime = ITOFIX19_13(Clock_getElapsedTime(this->physicsClock));

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(this))
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(this))
#endif
#ifdef __ANIMATION_EDITOR
	if(!Game_isInSpecialMode(this))
#endif
	// simulate physics
	PhysicalWorld_update(this->physicalWorld, elapsedTime);
	
	// process the collisions after the transformations have taken place
#ifdef __DEBUG
	this->lastProcessName = "process collisions";
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
	// process collisions
	CollisionManager_update(this->collisionManager, elapsedTime);
	
#ifdef __DEBUG
	this->lastProcessName = "physics ended";
#endif
}

// update game's rendering subsystem
static void Game_updateTransformations(Game this)
{
#ifdef __DEBUG
	this->lastProcessName = "move screen";
#endif

	// position the screen
	Screen_position(this->screen, true);

#ifdef __DEBUG
	this->lastProcessName = "apply transformations";
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
	// apply world transformations
	GameState_transform(__SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

// do defragmentation, memory recovery, etc
static void Game_cleanUp(Game this)
{
	
#ifdef __DEBUG
	this->lastProcessName = "update param table";
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
	if(!ParamTableManager_processRemovedSprites(this->paramTableManager))
	{
#ifdef __DEBUG
		this->lastProcessName = "defragmenting";
#endif
		CharSetManager_defragmentProgressively(this->charSetManager);
		
		// TODO: bgmap memory defragmentation
	}
}

static void Game_checkForNewState(Game this)
{
	ASSERT(this, "Game::checkForNewState: null this");

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
}

// update game's subsystems
static void Game_update(Game this)
{
	ASSERT(this, "Game::update: null this");

#ifdef __DEBUG
	char* previousLastProcessName = NULL;
#endif

	while (true)
	{
		// update each subsystem
		// wait to sync with the game start to render
		// this wait actually controls the frame rate
	    while(!(VIP_REGS[INTPND] & GAMESTART)); 
	    VIP_REGS[INTCLR]= GAMESTART;

		// update each subsystem
		
		// apply transformations from the previous frame
		// while the VPU is busy writing the frame buffers
	    Game_updateTransformations(this);
	    
	    // the engine's game logic is free of racing 
	    // conditions against the VPU
		Game_updateLogic(this);
		
		// physics' update takes place after game's logic
		// has been done
		Game_updatePhysics(this);

		// this is the point were the main game's subsystems
		// have done all their work
		// at this point save the current time on each 
		// clock so they can properly calculate the elapsed
		// time afterwards
		ClockManager_saveCurrentTime(this->clockManager);
		
		// this is the moment to check if the game's state
		// needs to be changed
		Game_checkForNewState(this);

		// if performance was good enough in the 
		// the previous second do some defragmenting
		if(!FrameRate_getFPS(this->frameRate) && FrameRate_isFPSHigh(this->frameRate))
		{
			Game_cleanUp(this);
		}
		
#ifdef __PRINT_FRAMERATE
		FrameRate_increaseFPS(this->frameRate);
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

		case kLowBatteryIndicator:

			Game_printLowBatteryIndicator(this, ((bool)Telegram_getExtraInfo(telegram)));
			return true;
			break;
	}
	
	return StateMachine_handleMessage(this->stateMachine, telegram);
}

// retrieve clock
const Clock Game_getClock(Game this)
{
	ASSERT(this, "Game::getClock: null this");

	return this->clock;
}

// retrieve clock
const Clock Game_getInGameClock(Game this)
{
	ASSERT(this, "Game::getInGameClock: null this");

	return GameState_getInGameClock(Game_getCurrentState(this));
}

// retrieve in game clock
const Clock Game_getAnimationsClock(Game this)
{
	ASSERT(this, "Game::getAnimationsClock: null this");

	return this->animationsClock;
}

const Clock Game_getPhysicsClock(Game this)
{
	ASSERT(this, "Game::getPhysicsClock: null this");

	return this->physicsClock;
}

void Game_startClocks(Game this)
{
	ASSERT(this, "Game::pauseClocks: null this");
	
	Clock_start(Game_getInGameClock(this));
	Clock_start(this->animationsClock);
	Clock_start(this->physicsClock);
}

void Game_pauseClocks(Game this)
{
	ASSERT(this, "Game::pauseClocks: null this");

	Clock_pause(Game_getInGameClock(this), true);
	Clock_pause(this->animationsClock, true);
	Clock_pause(this->physicsClock, true);
}

void Game_resumeClocks(Game this)
{
	ASSERT(this, "Game::resumeClocks: null this");

	Clock_pause(Game_getInGameClock(this), false);
	Clock_pause(this->animationsClock, false);
	Clock_pause(this->physicsClock, false);
}

void Game_startInGameClock(Game this)
{
	ASSERT(this, "Game::startInGameClock: null this");

	Clock_start(Game_getInGameClock(this));
}

void Game_startAnimations(Game this)
{
	ASSERT(this, "Game::startAnimations: null this");

	Clock_start(this->animationsClock);
}

void Game_startPhysics(Game this)
{
	ASSERT(this, "Game::startPhysics: null this");

	Clock_start(this->physicsClock);
}

void Game_pauseInGameClock(Game this, bool pause)
{
	ASSERT(this, "Game::pauseInGameClock: null this");

	Clock_pause(GameState_getInGameClock(Game_getCurrentState(this)), pause);
}

void Game_pauseAnimations(Game this, bool pause)
{
	ASSERT(this, "Game::pauseAnimations: null this");

	Clock_pause(this->animationsClock, pause);
}

void Game_pausePhysics(Game this, bool pause)
{
	ASSERT(this, "Game::pausePhysics: null this");
	
	Clock_pause(this->physicsClock, pause);
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
bool Game_isInSpecialMode(Game this)
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
bool Game_isEnteringSpecialMode(Game this)
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
bool Game_isExitingSpecialMode(Game this)
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

	return StateMachine_getCurrentState(this->stateMachine)? __SAFE_CAST(GameState, StateMachine_getCurrentState(this->stateMachine)): NULL;
}

#ifdef __LOW_BATTERY_INDICATOR
// low battery indicator check
static void Game_checkLowBattery(Game this, u16 keypad)
{
	ASSERT(this, "Game::checkLowBatteryIndicator: null this");

    if(keypad & K_PWR)
    {
        if(!this->isShowingLowBatteryIndicator) {
            MessageDispatcher_dispatchMessage(__LOW_BATTERY_INDICATOR_INITIAL_DELAY, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kLowBatteryIndicator, (bool*)true);
            this->isShowingLowBatteryIndicator = true;
        }
    }
    else
    {
         if(this->isShowingLowBatteryIndicator) {
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
	ASSERT(pauseState == Game_getCurrentState(this), "Game::unpause: null pauseState sent is not the current one");

	if(pauseState && Game_getCurrentState(this) == pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPopState;
		
		if(Game_getCurrentState(this) == this->automaticPauseState)
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
