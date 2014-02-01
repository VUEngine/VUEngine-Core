
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

#include <Printing.h>


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
	/* change state delay */						\
	int fadeDelay;									\

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
static void Game_setState(Game this, State state, int fadeDelay);

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
static void Game_constructor(Game this){

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
	this->fadeDelay = 0;
	
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
	_clock = this->clock;
	_inGameClock = this->inGameClock;
	
	// initialize this with your own first game world number
	this->actualStage = 0;
	
	int i = 0; 
	for (; i < kLogic + 1; i++) {
		
		this->lastTime[i] = 0;
	}
	
	// setup engine paramenters
	Game_initialize(this);
	
	// start the game's general clock
	Clock_start(this->clock);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Game_destructor(Game this){
	
	// destroy the clocks
	Clock_destructor(this->clock);
	Clock_destructor(this->inGameClock);
	
	__DELETE(this->stateMachine);	

	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup engine paramenters
void Game_initialize(Game this){
	
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
void Game_start(Game this, State state, int fadeDelay){

	ASSERT(state, "Game: initial state is NULL");

	if(!this->started) {
		
		this->started = true;
		
		// set state
		Game_setState(this, state, fadeDelay);
		
		// start game's cycle
		Game_update(this);
	}
	else {
		
		ASSERT(false, "Game: already started");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set game's state
void Game_changeState(Game this, State state, int fadeDelay){

	this->nextState = state;
	this->fadeDelay = fadeDelay;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set game's state
static void Game_setState(Game this, State state, int fadeDelay){

    ASSERT(state, "Game: setting NULL state");

	// make a fade out
	Screen_FXFadeOut(Screen_getInstance(), fadeDelay);
	
	// turn off the display
    HardwareManager_displayOff(this->hardwareManager);
    
	// disable rendering
	HardwareManager_disableRendering(HardwareManager_getInstance());

	//set waveform data
    SoundManager_setWaveForm(this->soundManager);

    //setup state 
    StateMachine_swapState(this->stateMachine, state);

    //enable hardware pad read
    HardwareManager_enableKeypad(this->hardwareManager);
	
	// load chars into graphic memory
	Printing_writeAscii(TextureManager_getFreeBgmap(this->bgmapManager));
	
	// enable rendering
	HardwareManager_enableRendering(this->hardwareManager);

	// make a fade in
	Screen_FXFadeIn(Screen_getInstance(), fadeDelay);
	
	// start physical simulation again
	PhysicalWorld_start(this->physicalWorld);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// recover graphics memory
void Game_recoverGraphicMemory(Game this){
	
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

	//initialize managers
//	CharSetManager_destructor(this->charSetManager);
//	TextureManager_destructor(this->bgmapManager);
//	ParamTableManager_destructor(this->paramTableManager);
		
	//clear char and bgmap memory
    HardwareManager_clearScreen(this->hardwareManager);

	CharSetManager_reset(this->charSetManager);
	TextureManager_reset(this->bgmapManager);
	ParamTableManager_reset(this->paramTableManager);
	SpriteManager_reset(this->spriteManager);
	CollisionManager_reset(this->collisionManager);
	PhysicalWorld_reset(this->physicalWorld);
	
	// load chars into graphic memory
	Printing_writeAscii(TextureManager_getFreeBgmap(this->bgmapManager));

	// call get instance in singletons to make sure their constructors
	// are called now
	//SoundManager_getInstance();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// backup engine's current status 
void Game_saveState(Game this){

	//save gameworld's object's current state
	//Stage_copy(this->auxStage, this->stage);
	
	//save engine's state
	//this->previousLogic = this->currentLogic;
	//his->previousState=this->currentState;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reload engine's current status
void Game_recoverState(Game this){
	
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
	
	this->optical.distanceEyeScreen = ITOFIX19_13(__DISTANCEEYESCREEN);
	
	//maximun distance from the _SC to the infinite	
	this->optical.maximunViewDistance = ITOFIX19_13(__MAXVIEWDISTANCE);
	
	//distance from left to right eye (128) (deep sensation)	
	this->optical.baseDistance = ITOFIX19_13(__BASEFACTOR);
	
	//horizontal View point center (192)
	this->optical.horizontalViewPointCenter = ITOFIX19_13(__HVPC);
	
	//vertical View point center (112)  
	this->optical.verticalViewPointCenter = ITOFIX19_13(__VVPC);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process input data according to the actual game status
void Game_handleInput(Game this, int currentKey){
	
	static int pressedKey = 0;
	static int previousKeyPressed = 0;
	
	int newKey = currentKey & ~previousKeyPressed;
	
	// read key pad
	pressedKey = currentKey;

	// check for a new key pressed
	if(newKey){

		// inform the game about the key pressed		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyPressed, &newKey);
	
		previousKeyPressed |= newKey;
	}
	
	if(!(pressedKey & previousKeyPressed)){

		// inform the game about the key pressed		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyUp, &previousKeyPressed);
		
		previousKeyPressed = pressedKey;
	}
	else if(pressedKey & previousKeyPressed){

		// inform the game about the key pressed		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyHold, &pressedKey);
		
		previousKeyPressed = pressedKey;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render game
void Game_render(Game this) {
	
	u32 currentTime = __CAP_FPS? Clock_getTime(_clock): this->lastTime[kLogic] + 1001;
	
	if(currentTime - this->lastTime[kRender] > 1000 / __RENDER_FPS){

		// save current time
		this->lastTime[kRender] = currentTime;
	
		// render sprites as fast as possible
		SpriteManager_render(SpriteManager_getInstance());

#ifdef __DEBUG
		// increase the frame rate
		FrameRate_increaseRenderFPS(this->frameRate);
#endif
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update engine's world's state
void Game_update(Game this){

	enum UpdateSubsystems{
		
		kRender = 0,
		kPhysics,
		kLogic
	};

	while(true){

		u32 currentTime = __CAP_FPS? Clock_getTime(_clock): this->lastTime[kLogic] + 1001;
			
		if(currentTime - this->lastTime[kLogic] > 1000 / __LOGIC_FPS){

			if(this->nextState){
				
				Game_setState(this, this->nextState, this->fadeDelay);
				this->nextState = NULL;
			}

			this->lastTime[kLogic] = currentTime;

			// it is the update cycle
			ASSERT(this->stateMachine, "Game: no state machine");

			// process user's input 
			Game_handleInput(this, HardwareManager_readKeypad(this->hardwareManager));

		    // update the game's logic
			StateMachine_update(this->stateMachine);

#ifdef __DEBUG
			// increase the frame rate
			FrameRate_increaseLogicFPS(this->frameRate);
#endif
		}
		
		if(currentTime - this->lastTime[kPhysics] > 1000 / __PHYSICS_FPS){
			
			this->lastTime[kPhysics] = currentTime;

			// simulate physics
			PhysicalWorld_update(this->physicalWorld);

			// simulate collisions
			CollisionManager_update(this->collisionManager);

			// update entities' position
			Level_transform((Level)StateMachine_getCurrentState(this->stateMachine));

			// check sprite layers
			SpriteManager_sortLayersProgressively(this->spriteManager);

#ifdef __DEBUG
			// increase the frame rate
			FrameRate_increasePhysicsFPS(this->frameRate);
#endif
		}
		
#ifdef __DEBUG
		FrameRate_increaseRawFPS(this->frameRate);
#endif		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process a telegram
int Game_handleMessage(Game this, Telegram telegram){
	
	ASSERT(this->stateMachine, "Game::handleMessage: NULL stateMachine");
	
	return StateMachine_handleMessage(this->stateMachine, telegram);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set rest flag
void Game_setRestFlag(Game this, int flag){
	
	this->restFlag = flag;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve clock
Clock Game_getClock(Game this){
	
	return this->clock;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve in game clock
Clock Game_getInGameClock(Game this){
	
	return this->inGameClock;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Game_printClassSizes(int x, int y){

	Printing_text("CLASS				SIZE (B)", x, y);
	Printing_text("AnimatedSprite", x, ++y);
	Printing_int(AnimatedSprite_getObjectSize(), x + 27, y);
	Printing_text("Background", x, ++y);
	Printing_int(Background_getObjectSize(), x + 27, y);
	Printing_text("Character", x, ++y);
	Printing_int(Actor_getObjectSize(), x + 27, y);
	Printing_text("CharGroup", x, ++y);
	Printing_int(CharGroup_getObjectSize(), x + 27, y);
	Printing_text("Clock", x, ++y);
	Printing_int(Clock_getObjectSize(), x + 27, y);
	Printing_text("Entity", x, ++y);
	Printing_int(Entity_getObjectSize(), x + 27, y);
	Printing_text("Image", x, ++y);
	Printing_int(Image_getObjectSize(), x + 27, y);
	Printing_text("InGameEntity", x, ++y);
	Printing_int(InGameEntity_getObjectSize(), x + 27, y);
//	Printing_text("Rect", x, ++y);
//	Printing_int(Rect_getObjectSize(), x + 27, y);
	Printing_text("Shape", x, ++y);
	Printing_int(Shape_getObjectSize(), x + 27, y);
	Printing_text("Sprite", x, ++y);
	Printing_int(Sprite_getObjectSize(), x + 27, y);
	Printing_text("State", x, ++y);
	Printing_int(State_getObjectSize(), x + 27, y);
	Printing_text("StateMachine", x, ++y);
	Printing_int(StateMachine_getObjectSize(), x + 27, y);
	//vbjPrintText("Scroll", x, ++y);
	//vbjPrintInt(Scroll_getObjectSize(), x + 27, y);
	Printing_text("Telegram", x, ++y);
	Printing_int(Telegram_getObjectSize(), x + 27, y);;
	Printing_text("Texture", x, ++y);
	Printing_int(Texture_getObjectSize(), x + 27, y);
	Printing_text("VirtualList", x, ++y);
	Printing_int(VirtualList_getObjectSize(), x + 27, y);
	Printing_text("VirtualNode", x, ++y);
	Printing_int(VirtualNode_getObjectSize(), x + 27, y);
}