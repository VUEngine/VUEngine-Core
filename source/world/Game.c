
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
#include <Rect.h>
#include <SpriteManager.h>
#include <CharSetManager.h>
#include <SoundManager.h>
#include <StateMachine.h>
#include <Screen.h>

#include <Printing.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

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
	/* time elapsed to show rest screen */			\
	float restTimer;								\
													\
	/* last registered time */						\
	float lastTime;									\
													\
	/* flag to autopause or not the game*/ 			\
	/* after 15 minutes of play */ 					\
	int restFlag: 1;								\
													\
	/* managers */									\
	FrameRate frameRate;							\
	TextureManager bgmapManager;					\
	CharSetManager charSetManager;					\
	SoundManager soundManager;						\
	ParamTableManager paramTableManager;			\
	SpriteManager spriteManager;					\
	CollisionManager collisionManager;				\
	PhysicalWorld physicalWorld;
	

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
	
	// construct the general clock
	this->clock = __NEW(Clock);
	
	// construc the in game clock
	this->inGameClock = __NEW(Clock);

	// construct the game's state machine
	this->stateMachine = __NEW(StateMachine, __ARGUMENTS(this));
	
	// call get instance in singletons to make sure their constructors
	// are called now
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
	
	//initialize resttimer
	this->restTimer=0;
	
	//initialize rest flag
	this->restFlag=true;
	
	// reset delays
	this->lastTime = 0;

	// setup global pointers	
	_optical = &this->optical;	
	_clock = this->clock;
	_inGameClock = this->inGameClock;
	
	// initialize this with your own first game world number
	this->actualStage = 0;
	
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
	
	
	StateMachine_destructor(this->stateMachine);	

	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup engine paramenters
void Game_initialize(Game this){
	
	// set ROM wainting to 1 cycle
	HW_REGS[WCR] |= 0x0001;	
	
	// setup vectorInterrupts
	setInterruptsVectors();	

	// make sure timer interrupts are enable
	timerInitialize();
	
	//initialize optic paramenters
	Game_setOpticalGlobals(this);
	
    //set waveform data
    SoundManager_setWaveForm(this->soundManager);
    
    // reset collision manager
    CollisionManager_reset(this->collisionManager);
    
	// clear sprite memory
	vbClearScreen();

	// turn on the display
	vbDisplayOn(); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set game's state
void Game_setState(Game this, State state, int fadeDelay){

	// make a fade out
	Screen_FXFadeOut(Screen_getInstance(), fadeDelay);
	
	// turn back the background
	VIP_REGS[BKCOL] = 0x00;

	// these shouldn't be here, but there is a bug in the engine which makes the world's table 
	// be corrupted, so don't remove the next call

	//disable hardware pad read
	vbDisableReadPad();
    
	//set waveform data
    SoundManager_setWaveForm(this->soundManager);

    //flush key buffer
    vbKeyFlush();
    
    ASSERT(state, Game: setting NULL state);
    
    //setup state 
    StateMachine_swapState(this->stateMachine, state);

    //enable hardware pad read
	vbEnableReadPad();
	
	// load chars into graphic memory
	vbjSetPrintingMemory(TextureManager_getFreeBgmap(this->bgmapManager));
	
	// make a fade in
	Screen_FXFadeIn(Screen_getInstance(), fadeDelay);
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
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// erase engine's current status
void Game_reset(Game this){

	//initialize managers
//	CharSetManager_destructor(this->charSetManager);
//	TextureManager_destructor(this->bgmapManager);
//	ParamTableManager_destructor(this->paramTableManager);
	
	//clear char and bgmap memory
	vbClearScreen();

	CharSetManager_reset(this->charSetManager);
	TextureManager_reset(this->bgmapManager);
	ParamTableManager_reset(this->paramTableManager);
	SpriteManager_reset(this->spriteManager);
	CollisionManager_reset(this->collisionManager);
	PhysicalWorld_reset(this->physicalWorld);
	
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
	
	vbSetColTable();
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
void Game_handleInput(Game this){
	
	static int pressedKey = 0;
	static int previousKeyPressed = 0;
	
	// read key pad
	pressedKey = vbReadPad();

	// check for a new key pressed
	if(pressedKey && pressedKey != previousKeyPressed){

		// inform the game about the key pressed		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyPressed, &pressedKey);
	}
	else if(!pressedKey && previousKeyPressed){

		// inform the game about the key pressed		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyUp, &pressedKey);
	}
	else if(pressedKey){

		// inform the game about the key pressed		
		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->stateMachine, kKeyHold, &pressedKey);
	}
	
	// save actual key pressed
	previousKeyPressed = pressedKey;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update engine's world's state
void Game_update1(Game this){

	enum UpdateSubsystems{
		
		kRender = 0,
		kPhysics,
		kLogic
	};

	u32 currentTime = 0;
	u32 lastTime[kLogic + 1] = {0, 0, 0};
	
	while(true){

		currentTime = Clock_getTime(_clock);

		lastTime[kLogic] = currentTime;
		
		// process user's input 
		Game_handleInput(this);

		// it is the update cycle
		ASSERT(this->stateMachine, Game: no state machine);

		// update the game's logic
		StateMachine_update(this->stateMachine);
		
		// check sprite layers
		SpriteManager_checkLayers(this->spriteManager);

		// increase the frame rate
		FrameRate_increaseFPS(this->frameRate);

		// draw the current game level
		Level_render((Level)StateMachine_getCurrentState(this->stateMachine));
		
		// render sprites as fast as possible
		SpriteManager_render(SpriteManager_getInstance());

		// simulate physics
			PhysicalWorld_update(this->physicalWorld);

			// process removed bodies
			PhysicalWorld_processRemovedBodies(this->physicalWorld);

			// simulate collisions
			CollisionManager_update(this->collisionManager);
			
			// process removed shapes
			CollisionManager_processRemovedShapes(this->collisionManager);
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

	u32 currentTime = 0;
	u32 lastTime[kLogic + 1] = {0, 0, 0};
	
	while(true){

		currentTime = Clock_getTime(_clock);

		if(currentTime - lastTime[kLogic] > 1000 / __LOGIC_FPS){

			lastTime[kLogic] = currentTime;
			
			// process user's input 
			Game_handleInput(this);

			// it is the update cycle
			ASSERT(this->stateMachine, Game: no state machine);

			// update the game's logic
			StateMachine_update(this->stateMachine);
			
			// check sprite layers
			SpriteManager_checkLayers(this->spriteManager);
		}

		// update if it is the update cycle and the enough time has elpased
		if(currentTime - lastTime[kRender] > 1000 / __RENDER_FPS){

			// save current time
			lastTime[kRender] = currentTime;

			// increase the frame rate
			FrameRate_increaseFPS(this->frameRate);

			// draw the current game level
			Level_render((Level)StateMachine_getCurrentState(this->stateMachine));
			
			// render sprites as fast as possible
			SpriteManager_render(SpriteManager_getInstance());
		}

		if(currentTime - lastTime[kPhysics] > 1000 / __PHYSICS_FPS){
			
			lastTime[kPhysics] = currentTime;

			// simulate physics
			PhysicalWorld_update(this->physicalWorld);

			// process removed bodies
			PhysicalWorld_processRemovedBodies(this->physicalWorld);

			// simulate collisions
			CollisionManager_update(this->collisionManager);
			
			// process removed shapes
			CollisionManager_processRemovedShapes(this->collisionManager);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update engine's world's state
void Game_updateProfile(Game this){
/*
	u32 currentTime = 0;
	u32 lastTime = 0;
	
	int turn = 1;

	u32 time[]= {0,0,0};
	u32 profileTime = 0;

	
	
	while(true){

		currentTime = Clock_getTime(_clock);
		
		//wait for the frame rate to stabilizate
		if(turn && currentTime - lastTime > 1000 / __TARGET_FPS){
		
			// save current time
			lastTime = currentTime;
			
			// process user's input 
			Game_handleInput(this);

			// it is the update cycle
			ASSERT(this->stateMachine, Game: no state machine);

			profileTime = Clock_getTime(_clock);
			// update the game's logic
			StateMachine_update(this->stateMachine);
			profileTime = Clock_getTime(_clock) -  profileTime;
			if(profileTime >= time [0]){
				time[0] = profileTime;
				vbjPrintInt(profileTime, 30 , 0);
			}


			profileTime = Clock_getTime(_clock);
			// simulate collisions
			CollisionManager_update(this->collisionManager);
			profileTime = Clock_getTime(_clock) -  profileTime;
			if(profileTime >= time [1]){
				time[1] = profileTime;
				vbjPrintInt(profileTime, 30 , 1);
			}

			profileTime = Clock_getTime(_clock);
			// render the stage and its entities
			Stage_render(this->stage);
			profileTime = Clock_getTime(_clock) -  profileTime;
			if(profileTime >= time [2]){
				time[2] = profileTime;
				vbjPrintInt(profileTime, 30 , 2);
			}
			
			// increase the frame rate
			FrameRate_increaseFPS(this->frameRate);
			
			turn = 0;
		}
		else{
			
			if(!turn){
				
				// process removed entities				
				Stage_processRemovedEntities(this->stage);
	
				// process removed shapes
				CollisionManager_processRemovedShapes(this->collisionManager);
				
				turn = 1;
			}
		}
	}
	*/		
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
	/*
	vbjPrintText("CLASS				SIZE (B)", x, y);
	vbjPrintText("AnimatedSprite", x, ++y);
	vbjPrintInt(AnimatedSprite_getObjectSize(), x + 27, y);
	vbjPrintText("Background", x, ++y);
	vbjPrintInt(Background_getObjectSize(), x + 27, y);
	vbjPrintText("Character", x, ++y);
	vbjPrintInt(Actor_getObjectSize(), x + 27, y);
	vbjPrintText("CharGroup", x, ++y);
	vbjPrintInt(CharGroup_getObjectSize(), x + 27, y);
	vbjPrintText("Clock", x, ++y);
	vbjPrintInt(Clock_getObjectSize(), x + 27, y);
	vbjPrintText("Entity", x, ++y);
	vbjPrintInt(Entity_getObjectSize(), x + 27, y);
	vbjPrintText("Image", x, ++y);
	vbjPrintInt(Image_getObjectSize(), x + 27, y);
	vbjPrintText("InGameEntity", x, ++y);
	vbjPrintInt(InGameEntity_getObjectSize(), x + 27, y);
	vbjPrintText("Rect", x, ++y);
	vbjPrintInt(Rect_getObjectSize(), x + 27, y);
	vbjPrintText("Shape", x, ++y);
	vbjPrintInt(Shape_getObjectSize(), x + 27, y);
	vbjPrintText("Sprite", x, ++y);
	vbjPrintInt(Sprite_getObjectSize(), x + 27, y);
	vbjPrintText("State", x, ++y);
	vbjPrintInt(State_getObjectSize(), x + 27, y);
	vbjPrintText("StateMachine", x, ++y);
	vbjPrintInt(StateMachine_getObjectSize(), x + 27, y);
	vbjPrintText("Scroll", x, ++y);
	//vbjPrintInt(Scroll_getObjectSize(), x + 27, y);
	vbjPrintText("Telegram", x, ++y);
	vbjPrintInt(Telegram_getObjectSize(), x + 27, y);;
	vbjPrintText("Texture", x, ++y);
	vbjPrintInt(Texture_getObjectSize(), x + 27, y);
	vbjPrintText("VirtualList", x, ++y);
	vbjPrintInt(VirtualList_getObjectSize(), x + 27, y);
	vbjPrintText("VirtualNode", x, ++y);
	vbjPrintInt(VirtualNode_getObjectSize(), x + 27, y);
	*/
}