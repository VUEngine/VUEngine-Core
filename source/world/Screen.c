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

#include <Screen.h>
#include <Optics.h>
#include <Game.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define Screen_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* screen position */														\
	VBVec3D position;															\
																				\
	/* screen position displacement */											\
	VBVec3D focusEntityPositionDisplacement;									\
																				\
	/* actor to center the screen around */										\
	InGameEntity focusInGameEntity;												\
																				\
	/* world's screen's movement state */										\
	MovementState movementState;												\
																				\
	/* world's screen's last displacement */									\
	VBVec3D lastDisplacement;													\
																				\
	/* stage's size in pixels */												\
	Size stageSize;																\

// define the Screen
__CLASS_DEFINITION(Screen);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// global
VBVec3D* _screenDisplacement = NULL;

//class's constructor
static void Screen_constructor(Screen this);

static void Screen_capPosition(Screen this);
	
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												GLOBALS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

VBVec3D* _screenPosition = NULL;

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// it's a singleton
__SINGLETON(Screen);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Screen_constructor(Screen this){

	ASSERT(this, "Screen::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	// initialize world's screen's position	
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;
	
	this->focusEntityPositionDisplacement.x = 0;
	this->focusEntityPositionDisplacement.y = 0;
	this->focusEntityPositionDisplacement.z = 0;
			
	// clear focus actor pointer
	this->focusInGameEntity = NULL;
	
	//set world's screen's movement
	this->movementState.x = __ACTIVE;
	this->movementState.y = __ACTIVE;
	this->movementState.z = __ACTIVE;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
	
	_screenDisplacement = &this->lastDisplacement;
	_screenPosition = &this->position;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Screen_destructor(Screen this){

	ASSERT(this, "Screen::destructor: null this");

	// destroy base
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// center world's screen in function of focus actor's position
void Screen_positione(Screen this){

	ASSERT(this, "Screen::update: null this");

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __ANIMATION_EDITOR
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
	
	//if focusInGameEntity is defined
	if(this->focusInGameEntity){
		
		//get focusInGameEntity's movement state
		if(__VIRTUAL_CALL(int, InGameEntity, isMoving, this->focusInGameEntity)){

			// save last position
			this->lastDisplacement = this->position;

			//get focusInGameEntity's position
			this->position = Entity_getPosition((Entity)this->focusInGameEntity);
			this->position.x += this->focusEntityPositionDisplacement.x - ITOFIX19_13(__SCREEN_WIDTH >> 1);
			this->position.y += this->focusEntityPositionDisplacement.y - ITOFIX19_13(__SCREEN_HEIGHT >> 1);
			this->position.z += this->focusEntityPositionDisplacement.z;
						
			if(0 > this->position.x){
				
				this->position.x = 0;
			}
			else if(ITOFIX19_13(this->stageSize.x) < this->position.x + ITOFIX19_13(__SCREEN_WIDTH)){
				
				this->position.x = ITOFIX19_13(this->stageSize.x - __SCREEN_WIDTH);
			}
			
			if(0 > this->position.y){
				
				this->position.y = 0;
			}
			else if(ITOFIX19_13(this->stageSize.y) < this->position.y + ITOFIX19_13(__SCREEN_HEIGHT)){
				
				this->position.y = ITOFIX19_13(this->stageSize.y - __SCREEN_HEIGHT);
			}

			this->lastDisplacement.x = this->position.x - this->lastDisplacement.x;
			this->lastDisplacement.y = this->position.y - this->lastDisplacement.y;
			this->lastDisplacement.z = this->position.z - this->lastDisplacement.z;
		}
		else{
			
			//stop world's screen's movement
			this->lastDisplacement.x = 0;
			this->lastDisplacement.y = 0;
			this->lastDisplacement.z = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set the focus entity
void Screen_setFocusInGameEntity(Screen this, InGameEntity focusInGameEntity){
	
	ASSERT(this, "Screen::setFocusInGameEntity: null this");

	this->focusInGameEntity = focusInGameEntity;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve focus entity
InGameEntity Screen_getFocusInGameEntity(Screen this){
	
	ASSERT(this, "Screen::getFocusInGameEntity: null this");

	return this->focusInGameEntity;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// an actor has been deleted
void Screen_focusEntityDeleted(Screen this, InGameEntity actor) {
	
	ASSERT(this, "Screen::focusEntityDeleted: null this");

	if(this->focusInGameEntity == actor){
		
		this->focusInGameEntity = NULL;
		
		this->lastDisplacement.x = 0;
		this->lastDisplacement.y = 0;
		this->lastDisplacement.z = 0;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// translate screen
void Screen_move(Screen this, VBVec3D translation, int cap){

	ASSERT(this, "Screen::setPosition: null this");

	this->lastDisplacement = translation;

	this->position.x += translation.x;
	this->position.y += translation.y;
	this->position.z += translation.z;
	
	//set world's screen's movement
	this->movementState.x = __ACTIVE;
	this->movementState.y = __ACTIVE;
	this->movementState.z = __ACTIVE;
	
	if(cap){
	
		Screen_capPosition(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// translate screen
static void Screen_capPosition(Screen this){
	
	if(this->position.x < 0){

		this->position.x = 0;
	}
		
	if(this->position.x + ITOFIX19_13(__SCREEN_WIDTH) > ITOFIX19_13(this->stageSize.x)){
		
		this->position.x = ITOFIX19_13(this->stageSize.x - __SCREEN_WIDTH);					
	}

	if(this->position.y < 0){

		this->position.y = 0;
	}
		
	if(this->position.y + ITOFIX19_13(__SCREEN_HEIGHT) > ITOFIX19_13(this->stageSize.y)){
		
		this->position.y = ITOFIX19_13(this->stageSize.y - __SCREEN_HEIGHT);					
	}

	if(this->position.z < 0){

		this->position.z = 0;
	}
		
	if(this->position.z > ITOFIX19_13(this->stageSize.z)){
		
		this->position.z = ITOFIX19_13(this->stageSize.z);					
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get screen's position
VBVec3D Screen_getPosition(Screen this){
	
	return this->position;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set screen's position
void Screen_setPosition(Screen this, VBVec3D position){

	ASSERT(this, "Screen::setPosition: null this");

	this->position = position;
	
	Screen_capPosition(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set screen's position displacement
void Screen_setFocuesEntityPositionDisplacement(Screen this, VBVec3D focusEntityPositionDisplacement){

	ASSERT(this, "Screen::setPosition: null this");

	this->focusEntityPositionDisplacement = focusEntityPositionDisplacement;
	
	Screen_capPosition(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve last displacement
VBVec3D Screen_getLastDisplacement(Screen this){
	
	ASSERT(this, "Screen::getLastDisplacement: null this");

	return this->lastDisplacement;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get current stage's size
Size Screen_getStageSize(Screen this){
	
	ASSERT(this, "Screen::getStageSize: null this");

	return this->stageSize;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set current stage's size
void Screen_setStageSize(Screen this, Size size){
	
	ASSERT(this, "Screen::setStageSize: null this");

	this->stageSize = size;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create a fade delay
void Screen_FXFadeIn(Screen this, int wait) {

	ASSERT(this, "Screen::FXFadeIn: null this");

	int i = 0;
	//create the delay
	for (; i <= 32; i += 2) {
		
		if(wait){
			
			//create time delay
			Clock_delay(Game_getClock(Game_getInstance()), wait);
		}
		
		//increase bright
		SET_BRIGHT(i, i*2, i);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create a fade delay
void Screen_FXFadeOut(Screen this, int wait){
	
	ASSERT(this, "Screen::FXFadeOut: null this");

	int i = 32;
	
	//create the delay
	for (; i >= 0; i-=2) {
		
		if(wait){
			
			//create time delay
			Clock_delay(Game_getClock((Game)Game_getInstance()), wait);
		}
		//decrease bright
		SET_BRIGHT(i, i*2, i);
	}
}

