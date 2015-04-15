/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Game.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <MiscStructs.h>
#include <FrameRate.h>
#include <Clock.h>
#include <TextureManager.h>
#include <GameState.h>
#include <MessageDispatcher.h>
#include <Stage.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <SoundManager.h>
#include <MBackgroundManager.h>
#include <StateMachine.h>
#include <Screen.h>
#include <VPUManager.h>
#include <Printing.h>
#include <I18n.h>
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
#undef __TARGET_FPS
#define __TARGET_FPS 50

#define __FPS_BASED_SECONDS		(int)(1000 / __TARGET_FPS) 

enum StateOperations
{
	kSwapState = 0,
	kPushState,
	kPopState
};


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define Game_ATTRIBUTES															\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* game's state machine */													\
	StateMachine stateMachine;													\
																				\
	/* engine's global timer */													\
	Clock clock;																\
																				\
	/* timer to use in game */													\
	Clock inGameClock;															\
																				\
	/* optic values used in projection values */								\
	Optical optical;															\
																				\
	/* managers */																\
	HardwareManager hardwareManager;											\
	FrameRate frameRate;														\
	TextureManager bgmapManager;												\
	CharSetManager charSetManager;												\
	SoundManager soundManager;													\
	ParamTableManager paramTableManager;										\
	SpriteManager spriteManager;												\
	CollisionManager collisionManager;											\
	PhysicalWorld physicalWorld;												\
	KeypadManager keypadManager;												\
	VPUManager vpuManager;														\
	DirectDraw directDraw;														\
	I18n i18n;																	\
	Screen screen;																\
																				\
	/* game's next state */														\
	GameState nextState;														\
																				\
	/* game's next state operation */											\
	int nextStateOperation; 													\
																				\
	/* last process' name */													\
	char* lastProcessName;														\
																				\
	/* auto pause state */											 			\
	GameState automaticPauseState;												\
																				\
	/* auto pause last checked time */											\
	u32 lastAutoPauseCheckTime;													\
																				\
	/* low battery indicator showing flag */									\
	bool isShowingLowBatteryIndicator;											\
	

__CLASS_DEFINITION(Game, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

Optical* _optical = NULL;
extern void MessageDispatcher_discardAllDelayedMessages(MessageDispatcher this);

static void Game_constructor(Game this);
static void Game_initialize(Game this);
static void Game_setOpticalGlobals(Game this);
static void Game_setNextState(Game this, GameState state);
static void Game_handleInput(Game this);
static void Game_update(Game this);
static void Game_updateLogic(Game this);
static void Game_updatePhysics(Game this);
static void Game_updateRendering(Game this);
static void Game_cleanUp(Game this);
#ifdef __LOW_BATTERY_INDICATOR
static void Game_checkLowBattery(Game this, u16 keyPressed);
static void Game_printLowBatteryIndicator(Game this, bool showIndicator);
#endif
static void Game_autoPause(Game this);


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
	ClockManager_getInstance();

	// construct the general clock
	this->clock = __NEW(Clock);

	// construct the in game clock
	this->inGameClock = __NEW(Clock);

	// construct the game's state machine
	this->stateMachine = __NEW(StateMachine, this);

	this->nextState = NULL;
	this->automaticPauseState = NULL;
	this->lastAutoPauseCheckTime = 0;
	this->isShowingLowBatteryIndicator = false;

	// make sure all managers are initialized now
	this->frameRate  = FrameRate_getInstance();
	this->hardwareManager = HardwareManager_getInstance();
	this->bgmapManager = TextureManager_getInstance();
	this->paramTableManager =  ParamTableManager_getInstance();
	this->charSetManager = CharSetManager_getInstance();
	this->soundManager = SoundManager_getInstance();
	this->spriteManager = SpriteManager_getInstance();
	this->collisionManager = CollisionManager_getInstance();
	this->physicalWorld = PhysicalWorld_getInstance();
	this->keypadManager = KeypadManager_getInstance();
	this->vpuManager = VPUManager_getInstance();
	this->directDraw = DirectDraw_getInstance();
	this->i18n = I18n_getInstance();
	this->screen = Screen_getInstance();

	// to make debugging easier
	this->lastProcessName = "starting up";

	// set optical value
	this->optical.distanceEyeScreen = 0;

	// maximum distance from the screen's position to the horizon
	this->optical.maximumViewDistance = 0;

	// distance from left to right eye (deep sensation)
	this->optical.baseDistance = 0;

	// screen's vertical view point center
	this->optical.verticalViewPointCenter = 0;

	// screen's horizontal view point center
	this->optical.horizontalViewPointCenter = 0;

	// setup global pointers
	// need globals to speed up critical processes
	_optical = &this->optical;
    
    this->nextStateOperation = kSwapState;

	// setup engine paramenters
	Game_initialize(this);
}

// class's destructor
void Game_destructor(Game this)
{
	ASSERT(this, "Game::destructor: null this");

	// destroy the clocks
	Clock_destructor(this->clock);
	Clock_destructor(this->inGameClock);

	__DELETE(this->stateMachine);

	__SINGLETON_DESTROY;
}

// setup engine paramenters
void Game_initialize(Game this)
{
	ASSERT(this, "Game::initialize: null this");

	// setup vectorInterrupts
	HardwareManager_setInterruptVectors(this->hardwareManager);

	// make sure timer interrupts are enable
	HardwareManager_initializeTimer(this->hardwareManager);

	// initialize optical paramenters
	Game_setOpticalGlobals(this);

    // set waveform data
    SoundManager_setWaveForm(this->soundManager);

    // reset collision manager
    CollisionManager_reset(this->collisionManager);

	// clear sprite memory
    HardwareManager_clearScreen(this->hardwareManager);
}

// set game's initial state
void Game_start(Game this, GameState state)
{
	ASSERT(this, "Game::start: null this");
	ASSERT(state, "Game::start: initial state is NULL");

	HardwareManager_displayOn(this->hardwareManager);

	if (!StateMachine_getCurrentState(this->stateMachine))
	{
		// start the game's general clock
		Clock_start(this->clock);
		
		// register start time for auto pause check
		this->lastAutoPauseCheckTime = Clock_getTime(this->clock);

		// set state
		Game_setNextState(this, state);

		// start game's cycle
		Game_update(this);
	}
	else
	{
		ASSERT(false, "Game: already started");
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

// set game's state
static void Game_setNextState(Game this, GameState state)
{
	ASSERT(this, "Game::setState: null this");
    ASSERT(state, "Game::setState: setting NULL state");

    // discard delayed messages
    MessageDispatcher_discardAllDelayedMessages(MessageDispatcher_getInstance());

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
			// setup new state
		    StateMachine_popState(this->stateMachine);
			break;
    }

    // TODO: crashes on Mednafen
    // enable hardware pad read
    //HardwareManager_enableKeypad(this->hardwareManager);

	// load chars into graphic memory
	Printing_loadFonts(Printing_getInstance());

	// make sure printing layer is set
	SpriteManager_setLastLayer(this->spriteManager);

	// start physical simulation again
	PhysicalWorld_start(this->physicalWorld);

	// disable rendering
	HardwareManager_enableRendering(this->hardwareManager);

	// if automatic pause function is in place
	if (this->automaticPauseState)
	{
		int automaticPauseCheckDelay = __AUTO_PAUSE_DELAY - (Clock_getTime(this->clock) - this->lastAutoPauseCheckTime);
		automaticPauseCheckDelay = 0 > automaticPauseCheckDelay? automaticPauseCheckDelay: automaticPauseCheckDelay;
		
		MessageDispatcher_dispatchMessage((u32)automaticPauseCheckDelay, __UPCAST(Object, this), __UPCAST(Object, this), kAutoPause, NULL);
		this->lastAutoPauseCheckTime = Clock_getTime(this->clock);
	}

	// no next state now
	this->nextState = NULL;
}

// disable interrutps
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

	MessageDispatcher_discardAllDelayedMessages(MessageDispatcher_getInstance());
	
	// setup the display
    HardwareManager_clearScreen(this->hardwareManager);
	HardwareManager_setupColumnTable(this->hardwareManager);
    HardwareManager_displayOn(this->hardwareManager);

	// reset managers
	CharSetManager_reset(this->charSetManager);
	TextureManager_reset(this->bgmapManager);
	ParamTableManager_reset(this->paramTableManager);
	SpriteManager_reset(this->spriteManager);
	MBackgroundManager_reset(MBackgroundManager_getInstance());
	
	// load chars into graphic memory
	Printing_loadFonts(Printing_getInstance());

	// TODO
	//SoundManager_getInstance();
}

// initialize optic paramenters
static void Game_setOpticalGlobals(Game this)
{
	ASSERT(this, "Game::setOpticalGlobals: null this");

	// accounts for the physical (real) space between the eyes and
	// the VB's screens, whose virtual representation is the Screen instance
	this->optical.distanceEyeScreen = ITOFIX19_13(__DISTANCE_EYE_SCREEN);

	// maximum distance from the _SC to the infinite
	this->optical.maximumViewDistance = ITOFIX19_13(__MAX_VIEW_DISTANCE);

	// distance from left to right eye (deep sensation)
	this->optical.baseDistance = ITOFIX19_13(__BASE_FACTOR);

	// horizontal view point center
	this->optical.horizontalViewPointCenter = ITOFIX19_13(__HVPC);

	// vertical view point center
	this->optical.verticalViewPointCenter = ITOFIX19_13(__VVPC);
}

// process input data according to the actual game status
static void Game_handleInput(Game this)
{
	ASSERT(this, "Game::handleInput: null this");

#ifdef __POLL_USER_INPUT_ONLY_ON_LOGIC_CYCLE
	KeypadManager_read(this->keypadManager);
#endif
	u16 pressedKey = KeypadManager_getPressedKey(this->keypadManager);
	u16 releasedKey = KeypadManager_getReleasedKey(this->keypadManager);
	u16 holdKey = KeypadManager_getHoldKey(this->keypadManager);
	u16 previousKey = KeypadManager_getPreviousKey(this->keypadManager);

#ifdef __DEBUG_TOOLS

	// check code to access special feature
	if ((previousKey & K_LT) && (previousKey & K_RT) && (pressedKey & K_RU))
	{
		if (Game_isInDebugMode(this))
		{
			this->nextState = __UPCAST(GameState, StateMachine_getCurrentState(this->stateMachine));
			StateMachine_popState(this->stateMachine);
			this->nextState = NULL;
		}
		else
		{
			if (Game_isInSpecialMode(this))
			{
				this->nextState = __UPCAST(GameState, StateMachine_getCurrentState(this->stateMachine));
				StateMachine_popState(this->stateMachine);
				this->nextState = NULL;
			}

			this->nextState = __UPCAST(GameState, DebugState_getInstance());
			StateMachine_pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

#ifdef __STAGE_EDITOR

	// check code to access special feature
	if ((previousKey & K_LT) && (previousKey & K_RT) && (pressedKey & K_RD))
	{
		if (Game_isInStageEditor(this))
		{
			this->nextState = __UPCAST(GameState, StateMachine_getCurrentState(this->stateMachine));
			StateMachine_popState(this->stateMachine);
			this->nextState = NULL;
		}
		else
		{
			if (Game_isInSpecialMode(this))
			{
				this->nextState = __UPCAST(GameState, StateMachine_getCurrentState(this->stateMachine));
				StateMachine_popState(this->stateMachine);
				this->nextState = NULL;
			}

			this->nextState = __UPCAST(GameState, StageEditorState_getInstance());
			StateMachine_pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

#ifdef __ANIMATION_EDITOR

	// check code to access special feature
	if ((previousKey & K_LT) && (previousKey & K_RT) && (pressedKey & K_RR))
	{
		if (Game_isInAnimationEditor(this))
		{
			this->nextState = __UPCAST(GameState, StateMachine_getCurrentState(this->stateMachine));
			StateMachine_popState(this->stateMachine);
			this->nextState = NULL;
		}
		else
		{
			if (Game_isInSpecialMode(this))
			{
				this->nextState = __UPCAST(GameState, StateMachine_getCurrentState(this->stateMachine));
				StateMachine_popState(this->stateMachine);
				this->nextState = NULL;
			}

			this->nextState = __UPCAST(GameState, AnimationEditorState_getInstance());
			StateMachine_pushState(this->stateMachine, (State)this->nextState);
			this->nextState = NULL;
		}

		KeypadManager_clear(this->keypadManager);
		return;
	}

#endif

#ifdef __DEBUG_TOOLS
	if (!Game_isInSpecialMode(this) && ((pressedKey & K_LT) || (pressedKey & K_RT)))
	{
		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

#ifdef __STAGE_EDITOR
	if (!Game_isInSpecialMode(this) && ((pressedKey & K_LT) || (pressedKey & K_RT)))
	{
		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

#ifdef __ANIMATION_EDITOR
	if (!Game_isInSpecialMode(this) && ((pressedKey & K_LT) || (pressedKey & K_RT)))
	{
		KeypadManager_clear(this->keypadManager);
		return;
	}
#endif

	// check for a new key pressed
	if (pressedKey)
	{
		// inform the game about the pressed key
		MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this->stateMachine), kKeyPressed, &pressedKey);
	}

	if (releasedKey)
	{
		// inform the game about the released key
		MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this->stateMachine), kKeyReleased, &releasedKey);
	}

	if (holdKey)
	{
		// inform the game about the hold key
		MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this->stateMachine), kKeyHold, &holdKey);
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
	this->lastProcessName = "dispatch delayed messages";
#endif
#ifdef __DEBUG_TOOLS
	if (!Game_isInSpecialMode(this))
#endif
#ifdef __STAGE_EDITOR
	if (!Game_isInSpecialMode(this))
#endif
#ifdef __ANIMATION_EDITOR
	if (!Game_isInSpecialMode(this))
#endif
	// dispatch queued messages
    MessageDispatcher_dispatchDelayedMessages(MessageDispatcher_getInstance());

	// increase the frame rate
	FrameRate_increaseLogicFPS(this->frameRate);

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
	// simulate physics
	PhysicalWorld_update(this->physicalWorld);
#ifdef __DEBUG
	this->lastProcessName = "process collisions";
#endif

#ifdef __DEBUG_TOOLS
	if (!Game_isInSpecialMode(this))
#endif
#ifdef __STAGE_EDITOR
	if (!Game_isInSpecialMode(this))
#endif
#ifdef __ANIMATION_EDITOR
	if (!Game_isInSpecialMode(this))
#endif
	// process collisions
	CollisionManager_update(this->collisionManager);

	// increase the frame rate
	FrameRate_increasePhysicsFPS(this->frameRate);
#ifdef __DEBUG
	this->lastProcessName = "physics ended";
#endif
}

// update game's rendering subsystem
static void Game_updateRendering(Game this)
{
#ifdef __DEBUG
	this->lastProcessName = "move screen";
#endif

	// position the screen
	Screen_positione(this->screen, true);

#ifdef __DEBUG
	this->lastProcessName = "apply transformations";
#endif
#ifdef __DEBUG_TOOLS
	if (!Game_isInSpecialMode(this))
#endif
#ifdef __STAGE_EDITOR
	if (!Game_isInSpecialMode(this))
#endif
#ifdef __ANIMATION_EDITOR
	if (!Game_isInSpecialMode(this))
#endif
	// apply world transformations
	GameState_transform(__UPCAST(GameState, StateMachine_getCurrentState(this->stateMachine)));

#ifdef __DEBUG
	this->lastProcessName = "render";
#endif
	// render sprites
	SpriteManager_render(this->spriteManager);

	// increase the frame rate
	FrameRate_increaseRenderFPS(this->frameRate);
#ifdef __DEBUG
	this->lastProcessName = "render done";
#endif
}

// do defragmentation, memory recovy, etc
static void Game_cleanUp(Game this)
{
	
#ifdef __DEBUG
	this->lastProcessName = "update param table";
#endif

	if (!ParamTableManager_processRemovedSprites(this->paramTableManager))
	{
#ifdef __DEBUG
		this->lastProcessName = "defragmenting";
#endif
		CharSetManager_defragmentProgressively(this->charSetManager);
		
		// TODO: bgmap memory defragmentation
	}
}

// update game's subsystems
static void Game_update(Game this)
{
	ASSERT(this, "Game::update: null this");

	u32 currentTime = 0;
	u32 mainLogicTime = 0;
	u32 cleanUpTime = 0;
	

#ifdef __DEBUG
	char* previousLastProcessName = NULL;
#endif

	while (true)
	{
#ifdef __DEBUG
		currentTime = __CAP_FPS ? Clock_getTime(this->clock) : mainLogicTime + 1000 + __TIMER_RESOLUTION;
		previousLastProcessName = this->lastProcessName;
#else
		currentTime = Clock_getTime(this->clock);
#endif		
		if (currentTime - mainLogicTime >= __FPS_BASED_SECONDS)
		{
			// check if new state available
			if (this->nextState)
			{
#ifdef __DEBUG
				this->lastProcessName = "setting next state";
#endif		
				Game_setNextState(this, this->nextState);
#ifdef __DEBUG
				this->lastProcessName = "setting next state done";
#endif		
			}

			// update each subsystem
			Game_updateLogic(this);
			Game_updatePhysics(this);
			Game_updateRendering(this);
			
			// record time
			mainLogicTime = currentTime;
		}
		// do some clean up at the half of the second, to don't interfere
		// with the game' normal flow
		else if (currentTime - cleanUpTime >= __FPS_BASED_SECONDS * 3 / 2 && FrameRate_isFPSHigh(this->frameRate))
		{
			Game_cleanUp(this);

			// record time
			cleanUpTime = currentTime;
		}
		
		// increase the frame rate
		FrameRate_increaseRawFPS(this->frameRate);

#ifndef __POLL_USER_INPUT_ONLY_ON_LOGIC_CYCLE
		// accumulate user's input until next logic cycle
		KeypadManager_read(this->keypadManager);
#endif
		
#ifdef __DEBUG
		if (previousLastProcessName != this->lastProcessName)
		{
			Printing_text(Printing_getInstance(), ":                              ", 10, 0, NULL);
			Printing_text(Printing_getInstance(), this->lastProcessName, 12, 0, NULL);
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

		case kLowBatteryIndicator:

			Game_printLowBatteryIndicator(this, ((bool)Telegram_getExtraInfo(telegram)));
			return true;
			break;
	}
	
	return StateMachine_handleMessage(this->stateMachine, telegram);
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

	return this->inGameClock;
}

// retrieve last process' name
char* Game_getLastProcessName(Game this)
{
	ASSERT(this, "Game::getLastProcessName: null this");

	return this->lastProcessName;
}

// retrieve optical config structure
Optical Game_getOptical(Game this)
{
	ASSERT(this, "Game::getOptical: null this");

	return this->optical;
}

// set optical config structure
void Game_setOptical(Game this, Optical optical)
{
	ASSERT(this, "Game::setOptical: null this");

	this->optical = optical;
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
	isEnteringSpecialMode |= __UPCAST(GameState, DebugState_getInstance()) == this->nextState;
#endif
#ifdef __STAGE_EDITOR
	isEnteringSpecialMode |= __UPCAST(GameState, StageEditorState_getInstance()) == this->nextState;
#endif
#ifdef __ANIMATION_EDITOR
	isEnteringSpecialMode |= __UPCAST(GameState, AnimationEditorState_getInstance()) == this->nextState;
#endif

	return isEnteringSpecialMode;
}

// whether if a special mode is being started
bool Game_isExitingSpecialMode(Game this)
{
	ASSERT(this, "Game::isInSpecialMode: null this");

	int isEnteringSpecialMode = false;
#ifdef __DEBUG_TOOLS
	isEnteringSpecialMode |= __UPCAST(GameState, DebugState_getInstance()) == this->nextState;
#endif
#ifdef __STAGE_EDITOR
	isEnteringSpecialMode |= __UPCAST(GameState, StageEditorState_getInstance()) == this->nextState;
#endif
#ifdef __ANIMATION_EDITOR
	isEnteringSpecialMode |= __UPCAST(GameState, AnimationEditorState_getInstance()) == this->nextState;
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

	return GameState_getStage(__UPCAST(GameState, StateMachine_getCurrentState(this->stateMachine)));
}

// retrieve current state
GameState Game_getCurrentState(Game this)
{
	ASSERT(this, "Game::getCurrentState: null this");

	return __UPCAST(GameState, StateMachine_getCurrentState(this->stateMachine));
}

#ifdef __LOW_BATTERY_INDICATOR
// low battery indicator check
static void Game_checkLowBattery(Game this, u16 keypad)
{
	ASSERT(this, "Game::checkLowBatteryIndicator: null this");

    if (keypad & K_PWR)
    {
        if (!this->isShowingLowBatteryIndicator) {
            MessageDispatcher_dispatchMessage(__LOW_BATTERY_INDICATOR_INITIAL_DELAY, __UPCAST(Object, this), __UPCAST(Object, this), kLowBatteryIndicator, (bool*)true);
            this->isShowingLowBatteryIndicator = true;
        }
    }
    else
    {
         if (this->isShowingLowBatteryIndicator) {
            MessageDispatcher_discardDelayedMessages(MessageDispatcher_getInstance(), kLowBatteryIndicator);
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
    MessageDispatcher_dispatchMessage(__LOW_BATTERY_INDICATOR_BLINK_DELAY, __UPCAST(Object, this), __UPCAST(Object, this), kLowBatteryIndicator, (bool*)(!showIndicator));
}
#endif

// pause
void Game_pause(Game this, GameState pauseState)
{
	ASSERT(this, "Game::pause: null this");
	ASSERT(pauseState, "Game::pause: null pauseState");

	if (pauseState)
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

	if (pauseState && Game_getCurrentState(this) == pauseState)
	{
		this->nextState = pauseState;
		this->nextStateOperation = kPopState;
		
		if (Game_getCurrentState(this) == this->automaticPauseState)
		{
			MessageDispatcher_dispatchMessage(__AUTO_PAUSE_DELAY, __UPCAST(Object, this), __UPCAST(Object, this), kAutoPause, NULL);
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

	if (this->automaticPauseState)
	{
		// only pause if no more than one state is active
		if (1 == StateMachine_getStackSize(this->stateMachine))
		{
			Game_pause(this, this->automaticPauseState);
		}
		else 
		{
			// otherwise just wait a minute to check again
			MessageDispatcher_dispatchMessage(__AUTO_PAUSE_RECHECK_DELAY, __UPCAST(Object, this), __UPCAST(Object, this), kAutoPause, NULL);
		}
	}
}
