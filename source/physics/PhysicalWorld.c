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

#include <PhysicsWorld.h>
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

#define __MAXSHAPESTOCHECK		32

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
	int selectBodiesToCheck:1;
	

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
Shape bodies[__MAXSHAPESTOCHECK] = {NULL};
Shape bodiesToCheck[__MAXSHAPESTOCHECK] = {NULL};

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
Shape PhysicalWorld_registerEntity(PhysicalWorld this, InGameEntity owner, Mass mass){

	// if the entity is already registered
	Body body = PhysicalWorld_getBody(this, owner);
	
	if(body){
		
		return body;
	}	
	
	VirtualList_pushFront(this->bodies, (void*)__NEW(Body, __ARGUMENTS(owner, mass)));
	
	// must prepare bodies in the next update
	this->selectBodiesToCheck = true;
	
	// return created shape
	return (Body)VirtualList_front(this->bodies);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remove a body
void PhysicalWorld_unregisterBody(PhysicalWorld this, InGameEntity owner){

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
Body PhysicalWorld_getBody(PhysicalWorld this, InGameEntity owner){

	VirtualNode node = VirtualList_begin(this->bodies);
	
	for(; node; node = VirtualNode_getNext(node)){

		// current body
		Body body = (Shape)VirtualNode_getData(node);
		
		// check if current shape's owner is the same as the entity calling this method
		if(owner == Body_getOwner(shape) && Body_isActive(body)){

			return body;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process removed bodies
void PhysicalWorld_processRemovedBodies(PhysicalWorld this)

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
	int j = 0;
	
	this->selectBodiesToCheck = false;

	// prepare bodies which move 
	// this will place the shape in the owner's position
	for(node = VirtualList_begin(this->bodies); node; node = VirtualNode_getNext(node)){

		// load the current shape
		Body body = (Body)VirtualNode_getData(node);
		
		// first check it body needs setup
		if(!Body_isReady(body)){
			
			// setup body
			__VIRTUAL_CALL(void, Body, setup, body);
		}

		// only check entities which are active
		if(Body_isActive(body)){
		
			// and moves 
			if(Body_moves(body)){
				
				// feed the array of movable bodies to check for collisions
				bodies[i++] = body;
			}
			
			// feed the array of all bodies
			bodiesToCheck[j++] = body;
		}
	}
	
	bodies[i] = NULL;
	bodiesToCheck[j] = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate collisions
void PhysicalWorld_update(PhysicalWorld this){
	
	int i = 0;
	int j = 0;
	
	// check if must select bodies to process
	if(this->selectBodiesToCheck){
		
		PhysicalWorld_selectBodiesToCheck(this);	
	}

	// check the bodies
	for(i = 0; bodies[i] && i < __MAXSHAPESTOCHECK; i++){

		// current to check body's rectangle 
		__VIRTUAL_CALL(void, Body, positione, bodies[i]);
	}
	
	// check the bodies
	for(i = 0; bodies[i] && i < __MAXSHAPESTOCHECK; i++){

		// if body is active to be processed
		if(Body_isActive(bodies[i])){
			
			// load the current body
			Body body = bodies[i];
			
			// get owner
			InGameEntity owner = Body_getOwner(body);

			// the result thrown by the collision algorithm
			int collisionResult = kNoCollision;

			// initialy assume that there is nothing below
			int noBodyBelow = true;

			// determine over which axis is moving
			int axisMovement = __VIRTUAL_CALL(int, InGameEntity, isMoving, owner);
	
			// if owner is moving
			if(axisMovement){
			
				// dont' check again the current body when processing other movable bodies
				Body_checked(body, true);
			}
			
			// comparing against the other bodies
			for(j = 0; bodiesToCheck[j] && j < __MAXSHAPESTOCHECK; j++){
				
				// load the current body to check against
				Body shapeToCheck = bodiesToCheck[j];
				
				// don't compare with current movable body, 
				// when the body already has been checked 
				// and when it is not active
				if(body != shapeToCheck && !Body_isChecked(shapeToCheck) && Body_isActive(shapeToCheck)){
					
					// check if bodies overlap
					collisionResult = __VIRTUAL_CALL(int, Body, overlaps, body, __ARGUMENTS(shapeToCheck, axisMovement));
					
					if(kNoCollision != collisionResult){
						
						if(kBodyBelow != collisionResult){
	
							// inform the owner about the collision
							if(MessageDispatcher_dispatchMessage(0, NULL, (Object)owner, collisionResult, (void*)Body_getOwner(shapeToCheck))){
							
								// stop processing
								break;
							}
						}
						else{
	
							noBodyBelow = false;
						}
					}
				}
			}
			              
			// if algorithm determined that there is no other body below
			if(!bodiesToCheck[j] && noBodyBelow && !(__YAXIS & axisMovement)){
						
				// tell the owner that there is nothing below
				MessageDispatcher_dispatchMessage(0, NULL, (Object)owner, kNoObjectBelow, (void*)NULL);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update a body
void PhysicalWorld_updateBody(PhysicalWorld this, Body body, const VBVec3D* const position){

	__VIRTUAL_CALL(void, Body, setup, body, __ARGUMENTS(position));
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
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if an entity has been registered
int PhysicalWorld_isEntityRegistered(PhysicalWorld this, InGameEntity owner){

	VirtualNode node = VirtualList_begin(this->bodies);
	
	for(; node; node = VirtualNode_getNext(node)){

		// current body
		Body body = (Body)VirtualNode_getData(node);
		
		// check if current body's owner is the same as the entity calling this method
		if(owner == Body_getOwner(body)){
			
			// check if body is active.... maybe a body must be removed
			// and a new entity has been loaded in the same memory location
			// as the owner of the found body
			return Body_isActive(body);
		}
	}
	
	return false;
}