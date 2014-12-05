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
#include <StateMachine.h>
#include <Screen.h>
#include <VPUManager.h>
#include <Printing.h>

#ifdef __DEBUG_TOOLS
#include <DebugState.h>
#endif


#ifdef __STAGE_EDITOR
#include <StageEditorState.h>
#endif

#ifdef __ANIMATION_EDITOR
#include <AnimationEditorState.h>
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
	
	kFirst = 0,
	kLogic,
	kRender,
	kPhysics,
	kLast
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
	u32 lastTime[kLast];							\
													\
	/* game's next state */							\
	State nextState;								\
													\
	/* last process' name */						\
	char* lastProcessName;							\
													\
	/* high fps flag */								\
	u8 highFPS;										\
	

	

__CLASS_DEFINITION(Game);
 

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// global
Optical* _optical = NULL;

// class's constructor
static void Game_constructor(Game this);

// setup engine paramenters
static void Game_initialize(Game this);

// initialize optic paramenters
static void Game_setOpticalGlobals(Game this);

// set game's state
static void Game_setState(Game this, State state);

// process input data according to the actual game status
static void Game_handleInput(Game this);

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
	
	// force construction now
	ClockManager_getInstance();
	
	// construct the general clock
	this->clock = __NEW(Clock);
	
	// construct the in game clock
	this->inGameClock = __NEW(Clock);

	// construct the game's state machine
	this->stateMachine = __NEW(StateMachine, __ARGUMENTS(this));
	
	this->nextState = NULL;
	
	// make sure all managers are initialized now
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
	
	// to make debugging easier
	this->lastProcessName = "starting up";
	
	// set optical value
	this->optical.distanceEyeScreen = 0;	
	
	// maximun distance from the screen's position to the horizon
	this->optical.maximunViewDistance = 0;
	
	// distance from left to right eye (deep sensation)
	this->optical.baseDistance = 0;
	
	// screen's vertical view point center
	this->optical.verticalViewPointCenter = 0;
	
	// screen's horizontal view point center
	this->optical.horizontalViewPointCenter = 0;
	
	this->highFPS = false;
	
	// setup global pointers
	// need them to speed up critical processes
	_optical = &this->optical;	

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
	
	// initialize optical paramenters
	Game_setOpticalGlobals(this);
	
    // set waveform data
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

	if(!StateMachine_getCurrentState(this->stateMachine)) {
		
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

	// state changing must be done when no other process
	// may be affecting the game's general state
	this->nextState = state;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set game's state
static void Game_setState(Game this, State state){

	ASSERT(this, "Game::setState: null this");
    ASSERT(state, "Game::setState: setting NULL state");

	// disable rendering
	HardwareManager_disableRendering(HardwareManager_getInstance());

	// set waveform data
    SoundManager_setWaveForm(this->soundManager);

    // setup new state 
    StateMachine_swapState(this->stateMachine, state);

    // enable hardware pad read
    HardwareManager_enableKeypad(this->hardwareManager);

	// load chars into graphic memory
	Printing_loadFont();
	
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

	// enable rendering
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
	Printing_loadFont();

	// TODO
	//SoundManager_getInstance();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// backup engine's current status 
void Game_saveState(Game this){

	ASSERT(this, "Game::saveState: null this");

	// TODO
	// save gameworld's object's current state
	//Stage_copy(this->auxStage, this->stage);
	
	// save engine's state
	// this->previousLogic = this->currentLogic;
	//his->previousState=this->currentState;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reload engine's current status
void Game_recoverState(Game this){
	
	ASSERT(this, "Game::recoverState: null this");

	// TODO
	
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

	// accounts for the phisical (real) space between the eyes and
	// the VB's screens, whose virtual representation is the Screen instance
	this->optical.distanceEyeScreen = ITOFIX19_13(__DISTANCE_EYE_SCREEN);
	
	// maximun distance from the _SC to the infinite	
	this->optical.maximunViewDistance = ITOFIX19_13(__MAX_VIEW_DISTANCE);
	
	// distance from left to right eye (deep sensation)	
	this->optical.baseDistance = ITOFIX19_13(__BASE_FACTOR);
	
	// horizontal view point center
	this->optical.horizontalViewPointCenter = ITOFIX19_13(__HVPC);
	
	// vertical view point center  
	this->optical.verticalViewPointCenter = ITOFIX19_13(__VVPC);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process input data according to the actual game status
static void Game_handleInput(Game this){
	
	ASSERT(this, "Game::handleInput: null this");

	KeypadManager keypadManager = KeypadManager_getInstance();
	KeypadManager_read(keypadManager);
	u16 pressedKey = KeypadManager_getPressedKey(keypadManager);
	u16 releasedKey = KeypadManager_getReleasedKey(keypadManager);
	u16 holdKey = KeypadManager_getHoldKey(keypadManager);
	u16 previousKey = KeypadManager_getPreviousKey(keypadManager);

#ifdef __DEBUG_TOOLS

	// check code to access special feature
	if((previousKey & K_SEL) && (pressedKey & K_STA)){

		if(Game_isInDebugMode(this)){
			
			StateMachine_popState(this->stateMachine);
		}
		else{ 
			
			if(Game_isInSpecialMode(this)){
			
				StateMachine_popState(this->stateMachine);
			}
			
			StateMachine_pushState(this->stateMachine, (State)DebugState_getInstance());
		}

		return;
	}
#endif

#ifdef __STAGE_EDITOR

	// check code to access special feature
	if((previousKey & K_STA) && (pressedKey & K_SEL)){

		if(Game_isInStageEditor(this)){
			
			StateMachine_popState(this->stateMachine);
		}
		else {
			
			if(Game_isInSpecialMode(this)){
			
				StateMachine_popState(this->stateMachine);
			}
		
			StateMachine_pushState(this->stateMachine, (State)StageEditorState_getInstance());
		}

		return;
	}
	
	if(pressedKey & K_STA){
		
		return;
	}
	
#endif
	
#ifdef __ANIMATION_EDITOR
	
	// check code to access special feature
	if((previousKey & K_LT) && (pressedKey & K_RT)){

		if(Game_isInAnimationEditor(this)){
			
			StateMachine_popState(this->stateMachine);
		}
		else {
			
			if(Game_isInSpecialMode(this)){
			
				StateMachine_popState(this->stateMachine);
			}
		
			StateMachine_pushState(this->stateMachine, (State)AnimationEditorState_getInstance());
		}

		return;
	}
	
#endif
	
#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(this) && (pressedKey & K_SEL)){
		
		return;
	}
#endif

#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(this) && (pressedKey & K_STA)){
		
		return;
	}
#endif

#ifdef __ANIMATION_EDITOR
	if(Game_isInAnimationEditor(this) && (pressedKey & K_LT)){
		
		return;
	}	
#endif

	// check for a new key pressed
	if(pressedKey){

		// inform the game about the pressed key 		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyPressed, &pressedKey);
	}
	
	if(releasedKey){

		// inform the game about the released key 		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyUp, &releasedKey);
	}
	
	if(holdKey){

		// inform the game about the hold key 		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyHold, &holdKey);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render game
void Game_render(Game this) {
	
	ASSERT(this, "Game::render: null this");

	// sort sprites
	SpriteManager_sortLayers(this->spriteManager, true);

	// increase the frame rate
	FrameRate_increaseRenderFPS(this->frameRate);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update engine's world's state
void Game_update(Game this){

	ASSERT(this, "Game::update: null this");

	u32 currentTime = 0;

	// don't allow all game's systems to be updated on the same cycle,
	// this makes obligatory to have a unified target frame rate for all
	// of them
	while(true){

		static int cycle = kLogic;

		currentTime = __CAP_FPS? Clock_getTime(this->clock): this->lastTime[kLogic] + 1001;
		
		if(kLogic == cycle) {
			
			if(currentTime - this->lastTime[kLogic] > 1000 / __TARGET_FPS){
	
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
	
				// increase the frame rate
				FrameRate_increaseLogicFPS(this->frameRate);
	
				// record time
			    this->lastTime[kLogic] = currentTime;
#ifdef __DEBUG
				this->lastProcessName = "kLogic ended";
#endif
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
				// simulate collisions and set streaming flag
				GameState_setCanStream((GameState)StateMachine_getCurrentState(this->stateMachine), !CollisionManager_update(this->collisionManager));

				// save time
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
#ifdef __STAGE_EDITOR
				if(!Game_isInSpecialMode(this))
#endif
#ifdef __ANIMATION_EDITOR
				if(!Game_isInSpecialMode(this))
#endif
				// apply world transformations
				GameState_transform((GameState)StateMachine_getCurrentState(this->stateMachine));
#ifdef __DEBUG
				this->lastProcessName = "render";
#endif
				// render sprites
				SpriteManager_render(this->spriteManager);

				// increase the frame rate
				FrameRate_increasePhysicsFPS(this->frameRate);
				
				// save time
				this->lastTime[kRender] = currentTime;
			}
		}

		// increase cycle
		if(kLast <= ++cycle) {
			
			cycle = kFirst + 1;
		}
		
		// check if new state available
		if(this->nextState){
			
			Game_setState(this, this->nextState);
			this->nextState = NULL;
		}
		
		// do some intensive tasks whenever fps are high
		if(this->highFPS) {
			
			CharSetManager_defragmentProgressively(this->charSetManager);

			this->highFPS = false;
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
			
			// since performance is good, do some cleaning up to 
			// char memory
			this->highFPS = true;
			break;
			
			// TODO: bgmap memory defragmentation
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
		
	ASSERT(this, "Game::isInDebugMode: null this");

	return StateMachine_getCurrentState(this->stateMachine) == (State)DebugState_getInstance();
}
#endif

#ifdef __STAGE_EDITOR
int Game_isInStageEditor(Game this){
		
	ASSERT(this, "Game::isInGameStateEditor: null this");

	return StateMachine_getCurrentState(this->stateMachine) == (State)StageEditorState_getInstance();
}
#endif

#ifdef __ANIMATION_EDITOR
int Game_isInAnimationEditor(Game this){
		
	ASSERT(this, "Game::isInAnimationEditor: null this");
	
	return StateMachine_getCurrentState(this->stateMachine) == (State)AnimationEditorState_getInstance();
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// whether if a special mode is active
int Game_isInSpecialMode(Game this) {

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve state machine, use with caution!!!
StateMachine Game_getStateMachine(Game this) {

	ASSERT(this, "Game::getStateMachine: null this");

	return this->stateMachine;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve the current level's stage
Stage Game_getStage(Game this){

	ASSERT(this, "Game::getStage: null this");

	return GameState_getStage((GameState)StateMachine_getCurrentState(this->stateMachine));
}
