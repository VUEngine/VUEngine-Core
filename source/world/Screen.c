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

#define Screen_ATTRIBUTES						\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* screen position */						\
	VBVec3D position;							\
												\
	/* actor to center the screen around */		\
	InGameEntity focusInGameEntity;				\
												\
	/* world's screen's movement state */		\
	MovementState movementState;				\
												\
	/* world's screen's last displacement */	\
	VBVec3D lastDisplacement;					\



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

//class's constructor
static void Screen_constructor(Screen this);

static void Screen_capPosition(Screen this);
	
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S ATTRIBUTES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it's a singleton
__SINGLETON(Screen);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Screen_constructor(Screen this){

	ASSERT(this, "Screen::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	// initialize world's screen's position	
	this->position.x = 1000;
	this->position.y = 0;
	this->position.z = __ZZERO;
			
	// clear focus actor pointer
	this->focusInGameEntity = NULL;
	
	//set world's screen's movement
	this->movementState.x = __ACTIVE;
	this->movementState.y = __ACTIVE;
	this->movementState.z = __ACTIVE;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
	
	_screenMovementState = &this->movementState;
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
void Screen_update(Screen this){

	ASSERT(this, "Screen::update: null this");

	//if focusInGameEntity is defined
	if(this->focusInGameEntity){
		
		//get focusInGameEntity's movement state
		if(__VIRTUAL_CALL(int, InGameEntity, isMoving, this->focusInGameEntity)){
			
			//get focusInGameEntity's position
			VBVec3D focusPosition = Entity_getPosition((Entity)this->focusInGameEntity);
			
			//set world's screen's movement
			this->movementState.x = __ACTIVE;
			this->movementState.y = __ACTIVE;
			this->movementState.z = __ACTIVE;

			this->lastDisplacement = this->position;
			
			// if screen isn't at the left limit
			// set it's position in function of
			// focusInGameEntity's position
			this->position.x = 0;
			
	
			if(focusPosition.x - ITOFIX19_13(__SCREEN_WIDTH / 2) >= 0){
				
				this->position.x = focusPosition.x - ITOFIX19_13(__SCREEN_WIDTH / 2);
				
				if(focusPosition.x + ITOFIX19_13(__SCREEN_WIDTH / 2) > ITOFIX19_13(GameWorld_getSize(GameWorld_getInstance()).x)){
					
					this->position.x = ITOFIX19_13(GameWorld_getSize(GameWorld_getInstance()).x - __SCREEN_WIDTH);					
				}
			}
			
			this->lastDisplacement.x = this->position.x - this->lastDisplacement.x;
			this->lastDisplacement.y = this->position.y - this->lastDisplacement.y;
			this->lastDisplacement.z = this->position.z - this->lastDisplacement.z;
		}
		else{
			
			//stop world's screen's movement
			this->movementState.x = __PASSIVE;
			this->movementState.y = __PASSIVE;
			this->movementState.z = __PASSIVE;
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
void Screen_move(Screen this, VBVec3D translation){

	ASSERT(this, "Screen::setPosition: null this");

	this->position.x += translation.x;
	this->position.y += translation.y;
	this->position.z += translation.z;
	
	//set world's screen's movement
	this->movementState.x = __ACTIVE;
	this->movementState.y = __ACTIVE;
	this->movementState.z = __ACTIVE;
	
	Screen_capPosition(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// translate screen
static void Screen_capPosition(Screen this){
	
	if(this->position.x < 0){

		this->position.x = 0;
	}
		
	if(this->position.x + ITOFIX19_13(__SCREEN_WIDTH) > ITOFIX19_13(GameWorld_getSize(GameWorld_getInstance()).x)){
		
		this->position.x = ITOFIX19_13(GameWorld_getSize(GameWorld_getInstance()).x - __SCREEN_WIDTH);					
	}

	if(this->position.y < 0){

		this->position.y = 0;
	}
		
	if(this->position.y + ITOFIX19_13(__SCREEN_HEIGHT) > ITOFIX19_13(GameWorld_getSize(GameWorld_getInstance()).y)){
		
		this->position.y = ITOFIX19_13(GameWorld_getSize(GameWorld_getInstance()).y - __SCREEN_HEIGHT);					
	}

	if(this->position.z < 0){

		this->position.z = 0;
	}
		
	if(this->position.z > ITOFIX19_13(GameWorld_getSize(GameWorld_getInstance()).z)){
		
		this->position.z = ITOFIX19_13(GameWorld_getSize(GameWorld_getInstance()).z);					
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
// retrieve last displacement
VBVec3D Screen_getLastDisplacement(Screen this){
	
	ASSERT(this, "Screen::getLastDisplacement: null this");

	return this->lastDisplacement;
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
