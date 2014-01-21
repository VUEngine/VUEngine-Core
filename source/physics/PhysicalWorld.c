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

#include <PhysicalWorld.h>
#include <MessageDispatcher.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define PhysicalWorld_ATTRIBUTES										\
																		\
	/* super's attributes */											\
	Object_ATTRIBUTES;													\
																		\
	/* a list of shapes which must detect collisions */					\
	VirtualList	bodies;													\
																		\
	/* a list of bodies which must be removed */						\
	VirtualList	removedBodies;											\
																		\
	/* flag to know if bodies must be prepared */						\
	int selectBodiesToCheck:1;											\
																		\
	/* gravity */														\
	VBVec3D gravity;													\
																		\
	/* friction */														\
	fix19_13 friction;													\
																		\
	/* time elapsed between updates*/									\
	fix19_13 elapsedTime;												\
																		\
	/* time for movement over each axis	*/								\
	unsigned long time;													\

// define the PhysicalWorld
__CLASS_DEFINITION(PhysicalWorld);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void PhysicalWorld_constructor(PhysicalWorld this);

// precalculate movable shape's position before doing collision detection on them
static void PhysicalWorld_selectBodiesToCheck(PhysicalWorld this);

// only process bodies which move and are active
Body bodies[__MAX_BODIES_PER_LEVEL] = {NULL};

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */




//////////////////////////////////////////////////////////////////////////////////////////////////////////////

__SINGLETON(PhysicalWorld);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void PhysicalWorld_constructor(PhysicalWorld this){
	
	__CONSTRUCT_BASE(Object);

	// create the shape list
	this->bodies = __NEW(VirtualList);
	
	this->removedBodies = __NEW(VirtualList);

	this->selectBodiesToCheck = false;
	
	this->gravity.x = 0;
	this->gravity.y = 0;
	this->gravity.z = 0;
	
	// record this update's time
	this->time = Clock_getTime(_inGameClock);
	
	bodies[0] = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void PhysicalWorld_destructor(PhysicalWorld this){

	// delete the bodies
	VirtualNode node = VirtualList_begin(this->bodies);
	
	// delete all bodies registered
	for(;node; node = VirtualNode_getNext(node)){
		
		__DELETE((Body)VirtualNode_getData(node));
	}
	
	// delete list
	__DELETE(this->bodies);
	
	// delete the list of removed bodies
	__DELETE(this->removedBodies);
	
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// register a shape
Body PhysicalWorld_registerBody(PhysicalWorld this, Actor owner, fix19_13 weight){

	// if the entity is already registered
	Body body = PhysicalWorld_getBody(this, owner);

	if(body){
		
		return body;
	}	
	
	VirtualList_pushFront(this->bodies, (void*)__NEW(Body, __ARGUMENTS((Object)owner, weight)));
	
	// must prepare bodies in the next update
	this->selectBodiesToCheck = true;
	
	// return created shape
	return (Body)VirtualList_front(this->bodies);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remove a body
void PhysicalWorld_unregisterBody(PhysicalWorld this, Actor owner){

	// if the entity is already registered
	Body body = PhysicalWorld_getBody(this, owner);

	if(body){
		
		// deactivate teh shape,
		// will be removed in the next update
		Body_setActive(body, false);
		
		// place in  the removed bodies list
		VirtualList_pushFront(this->removedBodies, (BYTE*)body);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// find a body given an owner
Body PhysicalWorld_getBody(PhysicalWorld this, Actor owner){

	VirtualNode node = VirtualList_begin(this->bodies);
	
	for(; node; node = VirtualNode_getNext(node)){

		// current body
		Body body = (Body)VirtualNode_getData(node);
		
		// check if current shape's owner is the same as the entity calling this method
		if((Object)owner == Body_getOwner(body) && Body_isActive(body)){

			return body;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process removed bodies
void PhysicalWorld_processRemovedBodies(PhysicalWorld this){

	VirtualNode node = VirtualList_begin(this->removedBodies);

	if(node){
		
		for(; node; node = VirtualNode_getNext(node)){
	
			Body body = (Body)VirtualNode_getData(node);
			
			// remove from the list
			VirtualList_removeElement(this->bodies, (BYTE*) body);
				
			// delete it
			__DELETE(body);
		}
	
		// clear the list
		VirtualList_clear(this->removedBodies);
		
		// must prepare bodies in the next update
		this->selectBodiesToCheck = true;		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// precalculate movable shape's position before doing collision detection on them
static void PhysicalWorld_selectBodiesToCheck(PhysicalWorld this){
	
	VirtualNode node = NULL;
	
	int i = 0;
	
	this->selectBodiesToCheck = false;

	// prepare bodies which move 
	// this will place the shape in the owner's position
	for(node = VirtualList_begin(this->bodies); node; node = VirtualNode_getNext(node)){

		// load the current shape
		Body body = (Body)VirtualNode_getData(node);
		
		// first check it body needs setup
		/*
		if(!Body_isReady(body)){
			
			// setup body
			//__VIRTUAL_CALL(void, Body, setup, body);
		}

*/
		
		// only check entities which are active
		if(Body_isAwake(body)){
		
			// feed the array of movable bodies to check for collisions
			bodies[i++] = body;
		}
	}
	
	bodies[i] = NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate collisions
void PhysicalWorld_update(PhysicalWorld this){
	
	int i = 0;
	
	// get the elapsed time
	this->elapsedTime = FTOFIX19_13((Clock_getTime(_inGameClock) - this->time) / 100.0f);

	// check if must select bodies to process
	if(this->selectBodiesToCheck){
		
		PhysicalWorld_selectBodiesToCheck(this);	
	}

	// check the bodies
	for(i = 0; bodies[i] && i < __MAX_BODIES_PER_LEVEL; i++){

		Body_update(bodies[i], &this->gravity, this->elapsedTime);
	}

	// record this update's time
	this->time = 0;//Clock_getTime(_inGameClock);
	
	// process removed bodies
	PhysicalWorld_processRemovedBodies(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unregister all bodies
void PhysicalWorld_reset(PhysicalWorld this){

	VirtualNode node = VirtualList_begin(this->bodies);

	for(; node; node = VirtualNode_getNext(node)){

		// delete it
		__DELETE((Body)VirtualNode_getData(node));
	}
	
	// empty the lists
	VirtualList_clear(this->bodies);	
	VirtualList_clear(this->removedBodies);	

	// must prepare bodies in the next update
	this->selectBodiesToCheck = true;
	
	bodies[0] = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if an entity has been registered
int PhysicalWorld_isEntityRegistered(PhysicalWorld this, Actor owner){

	VirtualNode node = VirtualList_begin(this->bodies);
	
	for(; node; node = VirtualNode_getNext(node)){

		// current body
		Body body = (Body)VirtualNode_getData(node);
		
		// check if current body's owner is the same as the entity calling this method
		if((Object)owner == Body_getOwner(body)){
			
			// check if body is active.... maybe a body must be removed
			// and a new entity has been loaded in the same memory location
			// as the owner of the found body
			return Body_isActive(body);
		}
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve friction
fix19_13 PhysicalWorld_getFriction(PhysicalWorld this){

	return this->friction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set friction
void PhysicalWorld_setFriction(PhysicalWorld this, fix19_13 friction){
	
	this->friction = friction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// a body has awaked
void PhysicalWorld_bodyAwaked(PhysicalWorld this){
	
	// must prepare bodies in the next update
	this->selectBodiesToCheck = true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set gravity
void PhysicalWorld_setGravity(PhysicalWorld this, VBVec3D gravity) {

	this->gravity.x = gravity.x;
	this->gravity.y = gravity.y;
	this->gravity.z = gravity.z;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve gravity
const VBVec3D* PhysicalWorld_getGravity(PhysicalWorld this) {

	return (const VBVec3D*)&this->gravity.x;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get last elapsed time
fix19_13 PhysicalWorld_getElapsedTime(PhysicalWorld this){
	
	return this->elapsedTime;
}

