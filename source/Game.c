/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Game.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <MiscStructs.h>
#include <Globals.h>
#include <FrameRate.h>
#include <Clock.h>
#include <TextureManager.h>
#include <Level.h>
#include <MessageDispatcher.h>
#include <Stage.h>
#include <ParamTableManager.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <SoundManager.h>
#include <StateMachine.h>
#include <Screen.h>
#include <Background.h>
#include <Image.h>
#include <VPUManager.h>
#include <Printing.h>

#ifdef __DEBUG_TOOLS
#include <DebugScreen.h>
#endif


#ifdef __LEVEL_EDITOR
#include <LevelEditorScreen.h>
#endif

#ifdef __ANIMATION_EDITOR
#include <AnimationEditorScreen.h>
#endif


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

enum UpdateSubsystems{
	
	kRender = 0,
	kPhysics,
	kLogic
};


#define Game_ATTRIBUTES								\
													\
	/* super's attributes */						\
	Object_ATTRIBUTES;								\
													\
	/* game's state machine */						\
	StateMachine stateMachine;						\
													\
	/* engine's global timer */						\
	Clock clock;									\
													\
	/* timer to use in game */						\
	Clock inGameClock;								\
													\
	/* optic values used in projection values */	\
	Optical optical;								\
													\
	/* actual gameplay gameworld */					\
	int actualStage;								\
													\
	/* flag to autopause or not the game*/ 			\
	/* after 15 minutes of play */ 					\
	int restFlag: 1;								\
													\
	/* managers */									\
	HardwareManager hardwareManager;				\
	FrameRate frameRate;							\
	TextureManager bgmapManager;					\
	CharSetManager charSetManager;					\
	SoundManager soundManager;						\
	ParamTableManager paramTableManager;			\
	SpriteManager spriteManager;					\
	CollisionManager collisionManager;				\
	PhysicalWorld physicalWorld;					\
	VPUManager vpuManager;							\
	DirectDraw directDraw;							\
													\
	/* update time registry */						\
	u32 lastTime[kLogic + 1];						\
													\
	/* game's start state */						\
	u8 started;										\
													\
	/* game's next state */							\
	State nextState;								\
													\
	/* game's next state */							\
	Level currentLevel;								\
													\
	/* last process' name */						\
	char* lastProcessName;							\
	

__CLASS_DEFINITION(Game);
 

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void Game_constructor(Game this);

// setup engine paramenters
static void Game_initialize(Game this);

// initialize optic paramenters
static void Game_setOpticalGlobals(Game this);

// set game's state
static void Game_setState(Game this, State state);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// a singleton
__SINGLETON(Game);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
int Game_isConstructed(){
	
	return 0 < _singletonConstructed;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Game_constructor(Game this){

	ASSERT(this, "Game::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	// make sure the memory pool is initialized now
	MemoryPool_getInstance();
	
	this->started = false;

	// construct the general clock
	this->clock = __NEW(Clock);
	
	// construc the in game clock
	this->inGameClock = __NEW(Clock);

	// construct the game's state machine
	this->stateMachine = __NEW(StateMachine, __ARGUMENTS(this));
	
	this->nextState = NULL;
	this->currentLevel = NULL;
	
	// call get instance in singletons to make sure their constructors
	// are called now
	this->hardwareManager = HardwareManager_getInstance();
	this->bgmapManager = TextureManager_getInstance();
	this->frameRate  = FrameRate_getInstance();	
	this->paramTableManager =  ParamTableManager_getInstance();
	this->charSetManager = CharSetManager_getInstance();
	this->soundManager = SoundManager_getInstance();
	this->spriteManager = SpriteManager_getInstance();
	this->collisionManager = CollisionManager_getInstance();
	this->physicalWorld = PhysicalWorld_getInstance();
	this->vpuManager = VPUManager_getInstance();
	this->directDraw = DirectDraw_getInstance();
	
	this->lastProcessName = "starting up";
	
	//OPTIC VALUES
	this->optical.distanceEyeScreen = 0;	
	
	//maximun distance from the _SC to the infinite
	this->optical.maximunViewDistance = 0;
	
	//distance from left to right eye (deep sensation)
	this->optical.baseDistance = 0;
	
	//vertical View point center
	this->optical.verticalViewPointCenter = 0;
	
	//horizontal View point center
	this->optical.horizontalViewPointCenter = 0;
	
	// setup global pointers	
	_optical = &this->optical;	
	
	// initialize this with your own first game world number
	this->actualStage = 0;
	
	int i = 0; 
	for (; i < kLogic + 1; i++) {
		
		this->lastTime[i] = 0;
	}
	
	// setup engine paramenters
	Game_initialize(this);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Game_destructor(Game this){
	
	ASSERT(this, "Game::destructor: null this");

	// destroy the clocks
	Clock_destructor(this->clock);
	Clock_destructor(this->inGameClock);
	
	__DELETE(this->stateMachine);	

	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup engine paramenters
void Game_initialize(Game this){
	
	ASSERT(this, "Game::initialize: null this");

	// setup vectorInterrupts
	HardwareManager_setInterruptVectors(this->hardwareManager);

	// make sure timer interrupts are enable
	HardwareManager_initializeTimer(this->hardwareManager);
	
	//initialize optic paramenters
	Game_setOpticalGlobals(this);
	
    //set waveform data
    SoundManager_setWaveForm(this->soundManager);
    
    // reset collision manager
    CollisionManager_reset(this->collisionManager);
    
	// clear sprite memory
    HardwareManager_clearScreen(this->hardwareManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set game's initial state
void Game_start(Game this, State state){

	ASSERT(this, "Game::start: null this");
	ASSERT(state, "Game::start: initial state is NULL");

	HardwareManager_displayOn(this->hardwareManager);

	if(!this->started) {
		
		this->started = true;

		// start the game's general clock
		Clock_start(this->clock);
		
		// set state
		Game_setState(this, state);
		
		// start game's cycle
		Game_update(this);
	}
	else {
		
		ASSERT(false, "Game: already started");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set game's state
void Game_changeState(Game this, State state){

	ASSERT(this, "Game::changeState: null this");

	this->nextState = state;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set game's state
static void Game_setState(Game this, State state){

	ASSERT(this, "Game::setState: null this");
    ASSERT(state, "Game::setState: setting NULL state");

	// disable rendering
	HardwareManager_disableRendering(HardwareManager_getInstance());

	//set waveform data
    SoundManager_setWaveForm(this->soundManager);

    //setup state 
    StateMachine_swapState(this->stateMachine, state);

	// save current level
	this->currentLevel = (Level)state;

    //enable hardware pad read
    HardwareManager_enableKeypad(this->hardwareManager);

	// load chars into graphic memory
	Printing_writeAscii();
	
	// start physical simulation again
	PhysicalWorld_start(this->physicalWorld);
	
	// disable rendering
	HardwareManager_enableRendering(this->hardwareManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// disable interrutps
void Game_disableHardwareInterrupts(Game this) {

	ASSERT(this, "Game::disableHardwareInterrupts: null this");

	// disable rendering
	HardwareManager_disableRendering(HardwareManager_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enable interrupts
void Game_enableHardwareInterrupts(Game this){

	ASSERT(this, "Game::enableHardwareInterrupts: null this");

	// disable rendering
	HardwareManager_enableRendering(this->hardwareManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// recover graphics memory
void Game_recoverGraphicMemory(Game this){
	
	ASSERT(this, "Game::recoverGraphicMemory: null this");

	//initialize graphic class managers
	CharSetManager_destructor(this->charSetManager);
	this->charSetManager = CharSetManager_getInstance();
	
	TextureManager_destructor(this->bgmapManager);
	this->bgmapManager = TextureManager_getInstance();
	
	ParamTableManager_destructor(this->paramTableManager);
	this->paramTableManager = ParamTableManager_getInstance();
	
	//allocate and write map characters
	//Stage_writeEntities(this->stage);
	
	HardwareManager_setupColumnTable(this->hardwareManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// erase engine's current status
void Game_reset(Game this){

	ASSERT(this, "Game::reset: null this");

	//clear char and bgmap memory
    HardwareManager_clearScreen(this->hardwareManager);

	// reset managers
	CharSetManager_reset(this->charSetManager);
	TextureManager_reset(this->bgmapManager);
	ParamTableManager_reset(this->paramTableManager);
	SpriteManager_reset(this->spriteManager);
	CollisionManager_reset(this->collisionManager);
	PhysicalWorld_reset(this->physicalWorld);
	
	// load chars into graphic memory
	Printing_writeAscii();

	// call get instance in singletons to make sure their constructors
	// are called now
	//SoundManager_getInstance();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// backup engine's current status 
void Game_saveState(Game this){

	ASSERT(this, "Game::saveState: null this");

	//save gameworld's object's current state
	//Stage_copy(this->auxStage, this->stage);
	
	//save engine's state
	//this->previousLogic = this->currentLogic;
	//his->previousState=this->currentState;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reload engine's current status
void Game_recoverState(Game this){
	
	ASSERT(this, "Game::recoverState: null this");

	//reload gameworld's object's current state
	//Stage_reset(this->stage);
	
	//Stage_copy(this->stage, this->auxStage);
	
	
	//reload graphics
	Game_recoverGraphicMemory(this);	
	
	//recover background music
	//SoundManager_loadBGM(this->soundManager, Stage_getBGM(this->stage));
	
	//recover engine's state
	//this->currentLogic = this->previousLogic;
	//this->currentState = this->previousState;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initialize optic paramenters
static void Game_setOpticalGlobals(Game this){
	
	ASSERT(this, "Game::setOpticalGlobals: null this");

	this->optical.distanceEyeScreen = ITOFIX19_13(__DISTANCE_EYE_SCREEN);
	
	//maximun distance from the _SC to the infinite	
	this->optical.maximunViewDistance = ITOFIX19_13(__MAX_VIEW_DISTANCE);
	
	//distance from left to right eye (128) (deep sensation)	
	this->optical.baseDistance = ITOFIX19_13(__BASE_FACTOR);
	
	//horizontal View point center (192)
	this->optical.horizontalViewPointCenter = ITOFIX19_13(__HVPC);
	
	//vertical View point center (112)  
	this->optical.verticalViewPointCenter = ITOFIX19_13(__VVPC);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process input data according to the actual game status
void Game_handleInput(Game this, int currentKey){
	
	ASSERT(this, "Game::handleInput: null this");

	static u32 previousKey = 0;
	
	u32 newKey = currentKey & ~previousKey;

#ifdef __DEBUG_TOOLS

	// check for a new key pressed
	if((previousKey & K_SEL) && (newKey & K_STA)){

		if(Game_isInDebugMode(this)){
			
			StateMachine_popState(this->stateMachine);
		}
		else{ 
			
			if(Game_isInSpecialMode(this)){
			
				StateMachine_popState(this->stateMachine);
			}
			
			StateMachine_pushState(this->stateMachine, (State)DebugScreen_getInstance());
		}

		previousKey = currentKey;
		return;
	}
#endif

#ifdef __LEVEL_EDITOR

	// check for a new key pressed
	if((previousKey & K_STA) && (newKey & K_SEL)){

		if(Game_isInLevelEditor(this)){
			
			StateMachine_popState(this->stateMachine);
		}
		else {
			
			if(Game_isInSpecialMode(this)){
			
				StateMachine_popState(this->stateMachine);
			}
		
			StateMachine_pushState(this->stateMachine, (State)LevelEditorScreen_getInstance());
		}

		previousKey = currentKey;
		return;
	}
	
	if(newKey & K_STA){
		
		previousKey = currentKey;
		return;
	}
	
#endif
	
#ifdef __ANIMATION_EDITOR
	
	// check for a new key pressed
	if((previousKey & K_LT) && (newKey & K_RT)){

		if(Game_isInAnimationEditor(this)){
			
			StateMachine_popState(this->stateMachine);
		}
		else {
			
			if(Game_isInSpecialMode(this)){
			
				StateMachine_popState(this->stateMachine);
			}
		
			StateMachine_pushState(this->stateMachine, (State)AnimationEditorScreen_getInstance());
		}

		previousKey = currentKey;
		return;
	}
	
#endif
	
#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(this) && (newKey & K_SEL)){
		
		previousKey = currentKey;
		return;
	}
#endif

#ifdef __LEVEL_EDITOR
	if(!Game_isInSpecialMode(this) && (newKey & K_STA)){
		
		previousKey = currentKey;
		return;
	}
#endif

#ifdef __ANIMATION_EDITOR
	if(Game_isInAnimationEditor(this) && (newKey & K_LT)){
		
		previousKey = currentKey;
		return;
	}	
#endif


	// check for a new key pressed
	if(newKey){

		// inform the game about the key pressed		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyPressed, &newKey);
	}
	
	if(currentKey != previousKey){

		u32 releasedKey = (previousKey & ~currentKey);

		// inform the game about the key pressed		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyUp, &releasedKey);
	}
	
	if(currentKey & previousKey){

		u32 holdKey = currentKey & previousKey;

		// inform the game about the key pressed		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyHold, &holdKey);
	}
	
	previousKey = currentKey;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render game
void Game_render(Game this) {
	
	ASSERT(this, "Game::render: null this");

	// sort sprites
	SpriteManager_sortLayersProgressively(this->spriteManager);

	// increase the frame rate
	FrameRate_increaseRenderFPS(this->frameRate);
}

#undef __CAP_FPS
#define __CAP_FPS true

#undef __TARGET_FPS
#define __TARGET_FPS 	60

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update engine's world's state
void Game_update(Game this){

	ASSERT(this, "Game::update: null this");

	enum UpdateSubsystems{
		
		kFirst = 0,
		kLogic,
		kRender,
		kPhysics,
		kLast
		
	};
	
	u32 currentTime = 0;

	while(true){

		static int cycle = kLogic;

		currentTime = __CAP_FPS? Clock_getTime(this->clock): this->lastTime[kLogic] + 1001;
		
		if(kLogic == cycle) {
			
			if(currentTime - this->lastTime[kLogic] > 1000 / __TARGET_FPS){
	
				if(this->nextState){
					
					Game_setState(this, this->nextState);
					this->nextState = NULL;
				}
	#ifdef __DEBUG
				this->lastProcessName = "handle input";
	#endif
	
				// process user's input 
				Game_handleInput(this, HardwareManager_readKeypad(this->hardwareManager));
	
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
			if(!Game_isInSpecialMode(this))
#endif
#ifdef __LEVEL_EDITOR
			if(!Game_isInSpecialMode(this))
#endif
				
#ifdef __ANIMATION_EDITOR
			if(!Game_isInSpecialMode(this))
#endif
				// dispatch queued messages
			    MessageDispatcher_dispatchDelayedMessages(MessageDispatcher_getInstance());
	
				// increase the frame rate
				FrameRate_increaseLogicFPS(this->frameRate);
	
			    this->lastTime[kLogic] = currentTime;
			    
				this->lastProcessName = "kLogic ended";

			}
		}
		else if(kPhysics == cycle) {

			if(currentTime - this->lastTime[kPhysics] > 1000 / __TARGET_FPS){
	
	#ifdef __DEBUG
				this->lastProcessName = "update physics";
	#endif

				
				// simulate physics
				PhysicalWorld_update(this->physicalWorld);
	
#ifdef __DEBUG
				this->lastProcessName = "process collisions";
#endif
				// simulate collisions
				Level_setCanStream((Level)StateMachine_getCurrentState(this->stateMachine), !CollisionManager_update(this->collisionManager));
				
				this->lastTime[kPhysics] = currentTime;
			}
		}
		else if(kRender == cycle) {
			
			if(currentTime - this->lastTime[kRender] > 1000 / __TARGET_FPS){
	
#ifdef __DEBUG
				this->lastProcessName = "apply transformations";
#endif
	
#ifdef __DEBUG_TOOLS
			if(!Game_isInSpecialMode(this))
#endif
				
#ifdef __LEVEL_EDITOR
			if(!Game_isInSpecialMode(this))
#endif

#ifdef __ANIMATION_EDITOR
			if(!Game_isInSpecialMode(this))
#endif
				// apply world transformations
				Level_transform((Level)StateMachine_getCurrentState(this->stateMachine));
	
#ifdef __DEBUG
				this->lastProcessName = "render";
#endif
				// render sprites
				SpriteManager_render(this->spriteManager);
	
				// increase the frame rate
				FrameRate_increasePhysicsFPS(this->frameRate);
				
				this->lastTime[kRender] = currentTime;
			}
		}

		if(kLast <= ++cycle) {
			
			cycle = kFirst + 1;
		}
		
		FrameRate_increaseRawFPS(this->frameRate);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process a telegram
int Game_handleMessage(Game this, Telegram telegram){
	
	ASSERT(this, "Game::handleMessage: null this");
	ASSERT(this->stateMachine, "Game::handleMessage: NULL stateMachine");

	switch(Telegram_getMessage(telegram)) {
	
		case kFRSareHigh:
			
			CharSetManager_defragmentProgressively(this->charSetManager);
			break;
	}
	
	return StateMachine_handleMessage(this->stateMachine, telegram);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set rest flag
void Game_setRestFlag(Game this, int flag){
	
	ASSERT(this, "Game::setRestFlag: null this");
	this->restFlag = flag;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve clock
Clock Game_getClock(Game this){
	
	ASSERT(this, "Game::getClock: null this");

	return this->clock;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve in game clock
Clock Game_getInGameClock(Game this){
	
	ASSERT(this, "Game::getInGameClock: null this");

	return this->inGameClock;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve last process' name
char* Game_getLastProcessName(Game this) {
	
	ASSERT(this, "Game::getLastProcessName: null this");

	return this->lastProcessName;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve optical config structure
Optical Game_getOptical(Game this) {
	
	ASSERT(this, "Game::getOptical: null this");

	return this->optical;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set optical config structure
void Game_setOptical(Game this, Optical optical) {
	
	ASSERT(this, "Game::setOptical: null this");

	this->optical = optical;
}

#ifdef __DEBUG_TOOLS
int Game_isInDebugMode(Game this){
		
	return StateMachine_getCurrentState(this->stateMachine) == (State)DebugScreen_getInstance();
}
#endif

#ifdef __LEVEL_EDITOR
int Game_isInLevelEditor(Game this){
		
	return StateMachine_getCurrentState(this->stateMachine) == (State)LevelEditorScreen_getInstance();
}
#endif

#ifdef __ANIMATION_EDITOR
int Game_isInAnimationEditor(Game this){
		
	return StateMachine_getCurrentState(this->stateMachine) == (State)AnimationEditorScreen_getInstance();
}
#endif

#ifdef __LEVEL_EDITOR
Level Game_getLevel(Game this){
		
	return this->currentLevel;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// whether an special mode is active
int Game_isInSpecialMode(Game this) {

	int isInSpecialMode = false;
	
#ifdef __DEBUG_TOOLS
	isInSpecialMode |= Game_isInDebugMode(this);
#endif
	
#ifdef __LEVEL_EDITOR
	isInSpecialMode |= Game_isInLevelEditor(this);
#endif	

#ifdef __ANIMATION_EDITOR
	isInSpecialMode |= Game_isInAnimationEditor(this);
#endif	
	
	return isInSpecialMode;
}
