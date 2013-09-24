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

#include <Body.h>
#include <PhysicalWorld.h>
#include <MessageDispatcher.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the Body
__CLASS_DEFINITION(Body);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// class's constructor
static void Body_constructor(Body this, Actor owner, Mass mass);

// update acceleration
static void Body_updateAcceleration(Body this, fix19_13 timeElapsed, const VBVec3D* gravity);

// udpdate movement over axis
static void Body_updateMovement(Body this, fix19_13 timeElapsed, const VBVec3D* gravity);

// set movement type
static void Body_setMovementType(Body this, int movementType);

// apply force
static void Body_applyForce(Body this, int clear);

// clear force
static void Body_clearAcceleration(Body this);

enum CollidingObjectIndexes{
	eXAxis = 0,
	eYAxis,
	eZAxis,
	eLastCollidingObject,
};

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// always call these to macros next to each other
__CLASS_NEW_DEFINITION(Body, __PARAMETERS(Actor owner, Mass mass))
__CLASS_NEW_END(Body, __ARGUMENTS(owner, mass));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Body_constructor(Body this, Actor owner, Mass mass){

	__CONSTRUCT_BASE(Object);


	this->owner = owner;
	
	this->mass = mass;
	
	this->awake = false;

	// set position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;

	// reset action timer
	this->time = 0;
	
	this->appliedForce.x = 0;
	this->appliedForce.y = 0;
	this->appliedForce.z = 0;
	

	// clear movement type
	this->movementType = 0;
	
	this->velocity.x = 0;
	this->velocity.y = 0;
	this->velocity.z = 0;
	
	this->acceleration.x = 0;
	this->acceleration.y = 0;
	this->acceleration.z = 0;
	
	this->velocity.x = 0;
	this->velocity.y = 0;
	this->velocity.z = 0;
	
	this->active = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Body_destructor(Body this){

	// destroy the super object
	__DESTROY_BASE(Object);
}

// set game entity
void Body_setOwner(Body this, Actor owner){
	
	this->owner = owner;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get game entity
Actor Body_getOwner(Body this){
	
	return this->owner;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve character's velocity
Velocity Body_getVelocity(Body this){
	
	return this->velocity;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set movement type
static void Body_setMovementType(Body this, int movementType){

	this->movementType = movementType;
	
	if (__UNIFORM_MOVEMENT == movementType){
		
		Body_clearForce(this);
		Body_clearAcceleration(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set movement type to accelerated
void Body_moveAccelerated(Body this){
	
	Body_setMovementType(this, __ACCELERATED_MOVEMENT);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set movement type to uniform
void Body_moveUniformly(Body this, Velocity velocity){

	this->velocity.x = velocity.x;
	this->velocity.y = velocity.y;
	this->velocity.z = velocity.z;

	Body_setMovementType(this, __UNIFORM_MOVEMENT);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clear force
static void Body_clearAcceleration(Body this){
	
	this->acceleration.x = 0;
	this->acceleration.y = 0;
	this->acceleration.z = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clear force
void Body_clearForce(Body this){
	
	this->appliedForce.x = 0;
	this->appliedForce.y = 0;
	this->appliedForce.z = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply force
static void Body_applyForce(Body this, int clear){
	
	fix19_13 weight = Mass_getWeight(this->mass);

	if (clear) {
		
		this->acceleration.x = 0;
		this->acceleration.y = 0;
		this->acceleration.z = 0;
	}

	this->acceleration.x += FIX19_13_DIV(this->appliedForce.x, weight);
	this->acceleration.y += FIX19_13_DIV(this->appliedForce.y, weight);
	this->acceleration.z += FIX19_13_DIV(this->appliedForce.z, weight);

	if(!this->awake) {
		
		Body_awake(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add force
void Body_addForce(Body this, const Force* force){
	
	if (force) {
		
		this->appliedForce.x = force->x;
		this->appliedForce.y = force->y;
		this->appliedForce.z = force->z;

		if(Body_isMoving(this)) {
			
			Body_applyForce(this, false);
		}
		else {
			
			Body_applyForce(this, true);
		}
		
		Body_moveAccelerated(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update movement
void Body_update(Body this, const VBVec3D* gravity){

	if (this->awake) {
		
		// get the elapsed time
		fix19_13 timeElapsed = FTOFIX19_13((Clock_getTime(_inGameClock) - this->time) / 100.0f);

	 	if(this->movementType){
			
	 		Body_updateAcceleration(this, timeElapsed, gravity);
		}
	
		Body_updateMovement(this, timeElapsed, gravity);
		
		Body_clearForce(this);
		
		// record this update's time
		this->time = Clock_getTime(_inGameClock);
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update force
static void Body_updateAcceleration(Body this, fix19_13 timeElapsed, const VBVec3D* gravity){
	
	// get friction fBody from the game world
	fix19_13 friction = PhysicalWorld_getFriction(PhysicalWorld_getInstance());
	fix19_13 weight = Mass_getWeight(this->mass);
	
	VBVec3D fictionVector = {0, 0, 0};

	Direction direction = {
			this->acceleration.x? 0 <= this->velocity.x? 1: -1: 0, 
			this->acceleration.y? 0 <= this->velocity.y? 1: -1: 0, 
			this->acceleration.z? 0 <= this->velocity.z? 1: -1: 0, 
			{0}, {0}, {0}
	};
	/*
	// if I'm over something
	if(this->objectBelow){
		
		// grab it's friction fBody
		friction += Actor_getFrictionBody(this->objectBelow));
	}
	*/
	
	fictionVector.x = this->acceleration.x? friction: 0;
	fictionVector.y = this->acceleration.y? friction: 0;
	fictionVector.z = this->acceleration.z? friction: 0;

	this->acceleration.x = this->acceleration.x - FIX19_13_MULT(ITOFIX19_13(direction.x), FIX19_13_MULT(FIX19_13_DIV(fictionVector.x, weight), timeElapsed)) + FIX19_13_MULT(gravity->x, timeElapsed);
	this->acceleration.y = this->acceleration.y - FIX19_13_MULT(ITOFIX19_13(direction.y), FIX19_13_MULT(FIX19_13_DIV(fictionVector.y, weight), timeElapsed)) + FIX19_13_MULT(gravity->y, timeElapsed);
	this->acceleration.z = this->acceleration.z - FIX19_13_MULT(ITOFIX19_13(direction.z), FIX19_13_MULT(FIX19_13_DIV(fictionVector.z, weight), timeElapsed)) + FIX19_13_MULT(gravity->z, timeElapsed);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// udpdate movement over axis
static void Body_updateMovement(Body this, fix19_13 timeElapsed, const VBVec3D* gravity){
	
	// the movement displacement
	VBVec3D displacement = {0, 0, 0};
	
	// if no time has elapsed
	if(!timeElapsed){

		this->time = Clock_getTime(_inGameClock);

		//stop processing 
		return;
	}

	// determine the movement type	
	// calculate displacement based in velocity, time and acceleration
 	if(this->movementType){

		displacement.x =
			FIX19_13_MULT(this->velocity.x, timeElapsed) 
			+ FIX19_13_MULT(this->acceleration.x, FIX19_13_MULT(timeElapsed, timeElapsed) >> 1);
	
		displacement.y =
			FIX19_13_MULT(this->velocity.y, timeElapsed) 
			+ FIX19_13_MULT(this->acceleration.y, FIX19_13_MULT(timeElapsed, timeElapsed) >> 1);
	
		displacement.z =
			FIX19_13_MULT(this->velocity.z, timeElapsed) 
			+ FIX19_13_MULT(this->acceleration.z, FIX19_13_MULT(timeElapsed, timeElapsed) >> 1);
		
		// update the velocity
		this->velocity.x += FIX19_13_MULT(this->acceleration.x, timeElapsed);
		this->velocity.y += FIX19_13_MULT(this->acceleration.y, timeElapsed);
		this->velocity.z += FIX19_13_MULT(this->acceleration.z, timeElapsed);

 		if(!gravity->x && !this->appliedForce.x && 1 > abs(FIX19_13TOI(displacement.x))){
 			
 			this->velocity.x = 0;
 			this->acceleration.x = 0; 
 		}
 		
 		if(!gravity->y && !this->appliedForce.y && 1 > abs(FIX19_13TOI(displacement.y))){
 			
 			this->velocity.y = 0;
 			this->acceleration.y = 0; 
 		}

 		if(!gravity->z && !this->appliedForce.z && 1 > abs(FIX19_13TOI(displacement.z))){
 			
 			this->velocity.z = 0;
 			this->acceleration.z = 0; 
 		}
 		
 		if(!this->appliedForce.x && !this->appliedForce.y && !this->appliedForce.z){

 	 		if(!this->velocity.x && !this->velocity.y && !this->velocity.z){
 			
 	 			MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->owner, kBodyStoped, NULL);
 	 		}
 			//Body_sleep(this);
 		}
 	}
 	else {
 		
		// update the velocity
		displacement.x = FIX19_13_MULT(this->velocity.x, timeElapsed);
		displacement.y = FIX19_13_MULT(this->velocity.y, timeElapsed);
		displacement.z = FIX19_13_MULT(this->velocity.z, timeElapsed);
 	}

 	this->position.x += displacement.x;
	this->position.y += displacement.y;
	this->position.z += displacement.z;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Body_printPhysics(Body this, int x, int y){
	
	vbjPrintText("X             Y             Z",x,y++);
	vbjPrintText("Position",x,y++);
//	vbjPrintInt(FIX19_13TOI(this->transform.globalPosition.x ),x,y);
//	vbjPrintInt(FIX19_13TOI(this->transform.globalPosition.y),x+14,y);
//	vbjPrintInt(FIX19_13TOI(this->transform.globalPosition.z),x+14*2,y++);

	vbjPrintText("Velocity",x,y++);
	vbjPrintFloat(FIX19_13TOF(this->velocity.x),x,y);
	vbjPrintFloat(FIX19_13TOF(this->velocity.y),x+14,y);
	vbjPrintFloat(FIX19_13TOF(this->velocity.z),x+14*2,y++);
	vbjPrintText("Acceleration",x,y++);
	vbjPrintFloat(FIX19_13TOF(this->acceleration.x),x,y);
	vbjPrintFloat(FIX19_13TOF(this->acceleration.y),x+14,y);
	vbjPrintFloat(FIX19_13TOF(this->acceleration.z),x+14*2,y++);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stop movement over an axis
void Body_stopMovement(Body this, int axis){
	
	if(__XAXIS & axis){
	
		// not moving anymore
		this->velocity.x = 0;
	}
	
	if(__YAXIS & axis){
	
		// not moving anymore
		this->velocity.y = 0;
	}	
	
	if(__ZAXIS & axis){
	
		// not moving anymore
		this->velocity.z = 0;
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset timer
void Body_resetTimer(Body this){

	//record initial time	
	this->time = Clock_getTime(_inGameClock);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// the the object below me
void Body_setObjectBelow(Body this, Actor objectBelow){
	
	//this->objectBelow = objectBelow;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set active
void Body_setActive(Body this, int active){
	
	// it is active
	this->active = active;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is active?
int Body_isActive(Body this){
	
	return this->active;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve position
VBVec3D Body_getPosition(Body this){
	
	return this->position;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve position
void Body_setPosition(Body this, const VBVec3D* position, Actor caller) {
	
	if (this->owner == caller){

		// set position
		this->position.x = position->x;
		this->position.y = position->y;
		this->position.z = position->z;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve state
int Body_isAwake(Body this) {
	
	return this->awake && this->active;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// awake body
void Body_awake(Body this) {
	
	if (!this->awake) {
		
		this->awake = true;
		
		PhysicalWorld_bodyAwaked(PhysicalWorld_getInstance());
		
		this->time = Clock_getTime(_inGameClock);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// go to sleep
void Body_sleep(Body this) {
	
	this->awake = false;
	
	PhysicalWorld_bodyAwaked(PhysicalWorld_getInstance());
		
	MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->owner, kBodySleep, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is it moving?
int Body_isMoving(Body this){

	int result = 0;
	result |= (this->velocity.x)? __XAXIS: 0;
	result |= (this->velocity.y)? __YAXIS: 0;
	result |= (this->velocity.z)? __ZAXIS: 0;

	return this->awake && this->active && result;
}

