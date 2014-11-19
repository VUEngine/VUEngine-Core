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

#include <GameWorld.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define GameWorld_ATTRIBUTES					\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* GameWorld size in pixels */				\
	Size size;									\
												\
	/* GameWorld's friction */					\
	fix7_9 friction;							\
												\
	/* friction factor for GameWorld's floor */	\
	float floorFriction;						\
												\
	/* pointer to background music */			\
	u16 (*bgm)[6];								\


// define the GameWorld
__CLASS_DEFINITION(GameWorld);




/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void GameWorld_constructor(GameWorld this);

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

__SINGLETON(GameWorld);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void GameWorld_constructor(GameWorld this){

	ASSERT(this, "GameWorld::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	this->friction = __AIR_FRICTION;
	this->floorFriction = __FLOOR_FRICTION;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void GameWorld_destructor(GameWorld this){

	ASSERT(this, "GameWorld::destructor: null this");

	// destroy the super object
	__DESTROY_BASE(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set GameWorld's size
void GameWorld_setSize(GameWorld this, Size size){
	
	ASSERT(this, "GameWorld::constructor: null this");

	this->size = size;
	
	// world z size must be at least 1
	if(0 == this->size.z){
		
		this->size.z = 1;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve GameWorld size
Size GameWorld_getSize(GameWorld this){
	
	ASSERT(this, "GameWorld::getSize: null this");

	return this->size;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get GameWorld's friction attribure
float GameWorld_getFriction(GameWorld this){
	
	ASSERT(this, "GameWorld::getFriction: null this");

	return this->friction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
float GameWorld_getFloorFriction(GameWorld this){
	
	ASSERT(this, "GameWorld::getFloorFriction: null this");

	return this->floorFriction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get GameWorld's friction attribure
void GameWorld_setFriction(GameWorld this, fix7_9 friction){

	ASSERT(this, "GameWorld::setFriction: null this");

	this->friction = friction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GameWorld_setFloorFriction(GameWorld this, fix7_9 floorFriction){
	
	ASSERT(this, "GameWorld::setFloorFriction: null this");

	this->floorFriction = floorFriction;
}
