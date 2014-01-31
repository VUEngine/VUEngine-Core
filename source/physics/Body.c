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
 * 												PROTOTYPESo
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// this should be improved and calculated dynamically based on framerate

#define STOPED_MOVING		0
#define STILL_MOVES			1
#define CHANGED_DIRECTION	2

#define THRESHOLD FTOFIX19_13(0.5f * (60.0f / __PHYSICS_FPS))

// class's constructor
static void Body_constructor(Body this, Object owner, fix19_13 weight);

// update acceleration
static void Body_updateAcceleration(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* acceleration, fix19_13 velocity, fix19_13 friction);

// udpdate movement over axis
static int Body_updateMovement(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* position, fix19_13* velocity, fix19_13* acceleration, fix19_13 appliedForce, int movementType, fix19_13 friction);

// set movement type
static void Body_setMovementType(Body this, int movementType, int axis);

// bounce back
static int Body_bounceOnAxis(Body this, fix19_13* velocity, fix19_13* acceleration, int axis, fix19_13 otherBodyElasticity);

// is it moving?
static int Body_isMovingInternal(Body this);

// update force
static void Body_calculateFriction(Body this, int axisOfMovement, Force* friction);


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
__CLASS_NEW_DEFINITION(Body, __PARAMETERS(Object owner, fix19_13 weight))
__CLASS_NEW_END(Body, __ARGUMENTS(owner, weight));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Body_constructor(Body this, Object owner, fix19_13 weight){

	__CONSTRUCT_BASE(Object);

	this->owner = owner;
	
	this->mass = __NEW(Mass, __ARGUMENTS(ITOFIX19_13(10)));
	
	this->awake = false;

	// set position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;

	this->appliedForce.x = 0;
	this->appliedForce.y = 0;
	this->appliedForce.z = 0;
	
	// clear movement type
	this->movementType.x = __UNIFORM_MOVEMENT;
	this->movementType.y = __UNIFORM_MOVEMENT;
	this->movementType.z = __UNIFORM_MOVEMENT;
		
	this->velocity.x = 0;
	this->velocity.y = 0;
	this->velocity.z = 0;
	
	this->acceleration.x = 0;
	this->acceleration.y = 0;
	this->acceleration.z = 0;
	
	this->active = true;
	
	this->elasticity = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Body_destructor(Body this){

	// destroy the mass
	__DELETE(this->mass);
	
	// destroy the super object
	__DESTROY_BASE(Object);
}

// set game entity
void Body_setOwner(Body this, Object owner){
	
	this->owner = owner;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get game entity
Object Body_getOwner(Body this){
	
	return this->owner;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve character's velocity
Velocity Body_getVelocity(Body this){
	
	return this->velocity;
}

// retrieve acceleration
Acceleration Body_getAcceleration(Body this){
	
	return this->acceleration;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set movement type
static void Body_setMovementType(Body this, int movementType, int axis){

	if (__XAXIS & axis) {
		
		this->movementType.x = movementType;

		if (__UNIFORM_MOVEMENT == movementType){

			this->appliedForce.x = 0;
			this->acceleration.x = 0;
		}
	}

	if (__YAXIS & axis) {
		
		this->movementType.y = movementType;

		if (__UNIFORM_MOVEMENT == movementType){

			this->appliedForce.y = 0;
			this->acceleration.y = 0;
		}
	}
	
	if (__ZAXIS & axis) {
		
		this->movementType.z = movementType;

		if (__UNIFORM_MOVEMENT == movementType){

			this->appliedForce.z = 0;
			this->acceleration.z = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set movement type to accelerated
void Body_moveAccelerated(Body this, int axis){
	
	if (__XAXIS & axis) {
	
		Body_setMovementType(this, __ACCELERATED_MOVEMENT, __XAXIS);
	}

	if (__YAXIS & axis) {
	
		Body_setMovementType(this, __ACCELERATED_MOVEMENT, __YAXIS);
	}

	if (__ZAXIS & axis) {
	
		Body_setMovementType(this, __ACCELERATED_MOVEMENT, __ZAXIS);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set movement type to uniform
void Body_moveUniformly(Body this, Velocity velocity){

	if (velocity.x) {
	
		Body_setMovementType(this, __UNIFORM_MOVEMENT, __XAXIS);
		this->velocity.x = velocity.x;
	}

	if (velocity.y) {
	
		Body_setMovementType(this, __UNIFORM_MOVEMENT, __YAXIS);
		this->velocity.y = velocity.y;
	}

	if (velocity.z) {
	
		Body_setMovementType(this, __UNIFORM_MOVEMENT, __ZAXIS);
		this->velocity.z = velocity.z;
	}
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
void Body_applyForce(Body this, const Force* force, int clearAxis){
	
	fix19_13 weight = Mass_getWeight(this->mass);

	if (__XAXIS & clearAxis) {
		
		this->velocity.x = 0;
		this->acceleration.x = 0;
	}
	
	if (__YAXIS & clearAxis) {
		
		this->velocity.y = 0;
		this->acceleration.y = 0;
	}
	
	if (__ZAXIS & clearAxis) {
		
		this->velocity.z = 0;
		this->acceleration.z = 0;
	}

	this->appliedForce.x = force->x;
	this->appliedForce.y = force->y;
	this->appliedForce.z = force->z;

	this->acceleration.x += FIX19_13_DIV(this->appliedForce.x, weight);
	this->acceleration.y += FIX19_13_DIV(this->appliedForce.y, weight);
	this->acceleration.z += FIX19_13_DIV(this->appliedForce.z, weight);

	Body_awake(this);

	if (this->appliedForce.x) {
	
		Body_moveAccelerated(this, __XAXIS);
	}

	if (this->appliedForce.y) {

		Body_moveAccelerated(this, __YAXIS);
	}

	if (this->appliedForce.z) {
	
		Body_moveAccelerated(this, __ZAXIS);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// apply gravity
void Body_applyGravity(Body this, const Acceleration* gravity){
	
	if (gravity) {

		int axisStartedMovement = 0;

		if(gravity->x) {

			this->acceleration.x = gravity->x;
			axisStartedMovement |= __XAXIS;
			Body_moveAccelerated(this, __XAXIS);
		}

		if(gravity->y){
			
			this->acceleration.y = gravity->y;
			axisStartedMovement |= __YAXIS;
			Body_moveAccelerated(this, __YAXIS);
		}

		if(gravity->z) {

			this->acceleration.z = gravity->z;
			axisStartedMovement |= __ZAXIS;
			Body_moveAccelerated(this, __YAXIS);
		}

		if(axisStartedMovement) {

			Body_awake(this);

			MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->owner, kBodyStartedMoving, &axisStartedMovement);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add force
void Body_addForce(Body this, const Force* force){
	
	ASSERT(force, "Body::addForce: NULL force");

	Body_applyForce(this, force, !Body_isMovingInternal(this));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update movement
void Body_update(Body this, const Acceleration* gravity, fix19_13 elapsedTime, Force* friction){

	if (this->awake) {
		
		if (elapsedTime) {
			
			int axisStopedMovement = 0;
			int axisOfMovement = 0;
 			int axisOfChangeOfMovement = 0;


			if(this->velocity.x || this->acceleration.x || this->appliedForce.x || (__ACCELERATED_MOVEMENT == this->movementType.x && gravity->x && this->acceleration.x)) {
			
				axisOfMovement |= __XAXIS;
			}

			if(this->velocity.y || this->acceleration.y || this->appliedForce.y || (__ACCELERATED_MOVEMENT == this->movementType.y && gravity->y && this->acceleration.y)) {

				axisOfMovement |= __YAXIS;
			}
			
		 	if(this->velocity.z || this->acceleration.z || this->appliedForce.z || (__ACCELERATED_MOVEMENT == this->movementType.z && gravity->z && this->acceleration.z)) {

				axisOfMovement |= __ZAXIS;
		 	}

			Body_calculateFriction(this, axisOfMovement, friction);

			// update each axis
	 	 	if(__XAXIS & axisOfMovement){ 
	 	 		
	 	 		int movementStatus = Body_updateMovement(this, elapsedTime, gravity->x, &this->position.x, &this->velocity.x, &this->acceleration.x, this->appliedForce.x, this->movementType.x, friction->x);

	 	 		if(movementStatus){

	 	 			if(CHANGED_DIRECTION == movementStatus) {
	 	 			
	 	 				axisOfChangeOfMovement |= __XAXIS;
	 	 			}
	 	 		}
	 	 		else {
	 	 			
	 	 			axisStopedMovement |= __XAXIS;
	 	 		}
	 	 	}

	 	 	if(__YAXIS & axisOfMovement){ 
	 	 		
	 	 		int movementStatus = Body_updateMovement(this, elapsedTime, gravity->y, &this->position.y, &this->velocity.y, &this->acceleration.y, this->appliedForce.y, this->movementType.y, friction->y);

	 	 		if(movementStatus){

	 	 			if(CHANGED_DIRECTION == movementStatus) {
	 	 			
	 	 				axisOfChangeOfMovement |= __YAXIS;
	 	 			}
	 	 		}
	 	 		else {
	 	 			
	 	 			axisStopedMovement |= __YAXIS;
	 	 		}
	 	 	}

	 	 		
	 	 	if(__ZAXIS & axisOfMovement){ 
	 	 		
	 	 		int movementStatus = Body_updateMovement(this, elapsedTime, gravity->z, &this->position.z, &this->velocity.z, &this->acceleration.z, this->appliedForce.z, this->movementType.z, friction->z);

	 	 		if(movementStatus){

	 	 			if(CHANGED_DIRECTION == movementStatus) {
	 	 			
	 	 				axisOfChangeOfMovement |= __ZAXIS;
	 	 			}
	 	 		}
	 	 		else {
	 	 			
	 	 			axisStopedMovement |= __ZAXIS;
	 	 		}
	 	 	}
 	 	
		 	// if stopped on any axis
		 	if(axisStopedMovement) {
		 		
	 			MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->owner, kBodyStoped, &axisStopedMovement);
		 	}
		 	
		 	if(axisOfChangeOfMovement){
		 		
		 		MessageDispatcher_dispatchMessage(0, (Object)this, (Object)this->owner, kBodyChangedDirection, &axisOfChangeOfMovement);
		 	}
		 	
		 	// clear any force so the next update does not get influenced
			Body_clearForce(this);
		}
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update force
static void Body_calculateFriction(Body this, int axisOfMovement, Force* friction){
	
	// get friction fBody from the game world
	fix19_13 worldFriction = PhysicalWorld_getFriction(PhysicalWorld_getInstance());

	friction->x = __XAXIS & axisOfMovement? 0 < this->velocity.x? -(friction->x + worldFriction): friction->x + worldFriction : 0;
	friction->y = __YAXIS & axisOfMovement? 0 < this->velocity.y? -(friction->y + worldFriction): friction->y +  worldFriction : 0;
	friction->z = __ZAXIS & axisOfMovement? 0 < this->velocity.z? -(friction->z + worldFriction): friction->z +  worldFriction : 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update force
static void Body_updateAcceleration(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* acceleration, fix19_13 velocity, fix19_13 friction){
	
	fix19_13 sign = ITOFIX19_13(0 <= gravity? -1: 1);

	if(gravity) {
	
		if (0 > FIX19_13_MULT((*acceleration - gravity), sign)) {
			
			gravity = 0;
		}
		else if (FIX19_13_MULT((*acceleration + FIX19_13_MULT(gravity, elapsedTime) - gravity), sign)) {
			
			gravity = gravity - *acceleration;
		}
		
		*acceleration += FIX19_13_MULT(gravity, elapsedTime);
	}

	*acceleration += friction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve last displacement
VBVec3D Body_getLastDisplacement(Body this) {
	
	VBVec3D displacement = {0, 0, 0}; 
	
	fix19_13 elapsedTime = PhysicalWorld_getElapsedTime(PhysicalWorld_getInstance());
	
	displacement.x = FIX19_13_MULT(this->velocity.x, elapsedTime);
	displacement.y = FIX19_13_MULT(this->velocity.y, elapsedTime);
	displacement.z = FIX19_13_MULT(this->velocity.z, elapsedTime);

 	return displacement;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// udpdate movement over axis
static int Body_updateMovement(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* position, fix19_13* velocity, fix19_13* acceleration, fix19_13 appliedForce, int movementType, fix19_13 friction){
	
	// the movement displacement
	fix19_13 displacement = 0;

	int moving = STILL_MOVES;
	
	// determine the movement type	
	// calculate displacement based in velocity, time and acceleration
 	if(__ACCELERATED_MOVEMENT == movementType){

 		Body_updateAcceleration(this, elapsedTime, gravity, acceleration, *velocity, friction);

 		fix19_13 previousVelocity = *velocity;
 		
		// update the velocity
		*velocity += FIX19_13_MULT(*acceleration, elapsedTime);

		displacement =
			FIX19_13_MULT(*velocity, elapsedTime) 
			+ FIX19_13_DIV(FIX19_13_MULT(*acceleration, FIX19_13_MULT(elapsedTime, elapsedTime)), ITOFIX19_13(2));


 		if(!gravity && (!appliedForce && (THRESHOLD > abs(displacement) || THRESHOLD > abs(*velocity)))){
 			
 			*velocity = 0;
 			*acceleration = 0; 
 		}
 		
 		if(!appliedForce && !*velocity){

 			moving = STOPED_MOVING;
 		}
 		else {
 			
 			if((0 < previousVelocity && 0 > *velocity) || (0 > previousVelocity && 0 < *velocity)) {

 				moving = CHANGED_DIRECTION;
 			}
 		}
 	}
 	else if(__UNIFORM_MOVEMENT == movementType){

		// update the velocity
		displacement = FIX19_13_MULT(*velocity, elapsedTime);
 	}
 	else {
 		
 		ASSERT(false, "Body: wrong movement type");
 	}

 	// update position
 	*position += displacement;
 	
 	// return movement state
 	return moving;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Body_printPhysics(Body this, int x, int y){
	
	__ACCELERATED_MOVEMENT == this->movementType.x? Printing_text("Accelerated", x, y++): Printing_text("Uniform", x, y++);

	Printing_text("X             Y             Z",x,y++);

	Printing_text("X             Y             Z",x,y++);
	Printing_text("Position",x,y++);
	Printing_int(FIX19_13TOI(this->position.x ),x,y);
	Printing_int(FIX19_13TOI(this->position.y),x+14,y);
	Printing_int(FIX19_13TOI(this->position.z),x+14*2,y++);

	Printing_text("Velocity",x,y++);
	Printing_float(FIX19_13TOF(this->velocity.x),x,y);
	Printing_float(FIX19_13TOF(this->velocity.y),x+14,y);
	Printing_float(FIX19_13TOF(this->velocity.z),x+14*2,y++);
	Printing_text("Acceleration",x,y++);
	Printing_float(FIX19_13TOF(this->acceleration.x),x,y);
	Printing_float(FIX19_13TOF(this->acceleration.y),x+14,y);
	Printing_float(FIX19_13TOF(this->acceleration.z),x+14*2,y++);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stop movement over an axis
void Body_stopMovement(Body this, int axis){
	
	if(__XAXIS & axis){
	
		// not moving anymore
		this->velocity.x = 0;
		this->acceleration.x = 0;
		this->appliedForce.x = 0;
	}
	
	if(__YAXIS & axis){
	
		// not moving anymore
		this->velocity.y = 0;
		this->acceleration.y = 0;
		this->appliedForce.y = 0;
	}	
	
	if(__ZAXIS & axis){
	
		// not moving anymore
		this->velocity.z = 0;
		this->acceleration.z = 0;
		this->appliedForce.z = 0;
	}
	
	if(!Body_isMoving(this)) {
		
		Body_sleep(this);
	}
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
void Body_setPosition(Body this, const VBVec3D* position, Object caller) {
	
	if (this->owner == caller){

		// set position
		this->position.x = position->x;
		this->position.y = position->y;
		this->position.z = position->z;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get elasticiy
fix19_13 Body_getElasticity(Body this){

	return this->elasticity;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set elasticiy
void Body_setElasticity(Body this, fix19_13 elasticity) {

	if(ITOFIX19_13(0) > elasticity) {
		
		elasticity = 0;
	}
	else if(ITOFIX19_13(1) < elasticity){
		
		elasticity = ITOFIX19_13(1);
	}
	
	this->elasticity = elasticity;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get friction
fix19_13 Body_getFriction(Body this){

	return this->friction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set elasticity
void Body_setFriction(Body this, fix19_13 friction){
	
	this->friction = friction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve state
int Body_isAwake(Body this) {
	
	return this->awake && this->active;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// awake body
void Body_awake(Body this) {

	if(!this->awake) {
	
		this->awake = true;
		
		PhysicalWorld_bodyAwaked(PhysicalWorld_getInstance());
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

	result |= ((int)FIX19_13TOI(this->velocity.x))? __XAXIS: 0;
	result |= ((int)FIX19_13TOI(this->velocity.y))? __YAXIS: 0;
	result |= ((int)FIX19_13TOI(this->velocity.z))? __ZAXIS: 0;
	
	return this->awake && this->active? result: 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is it moving?
static int Body_isMovingInternal(Body this){

	int result = 0;

	result |= (this->velocity.x)? __XAXIS: 0;
	result |= (this->velocity.y)? __YAXIS: 0;
	result |= (this->velocity.z)? __ZAXIS: 0;
	
	return this->awake && this->active? result: 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bounce back
void Body_bounce(Body this, int axis, fix19_13 otherBodyElasticity){

	int axisOnWhichStoped = 0;
	
	if ((__XAXIS & axis) && Body_bounceOnAxis(this, &this->velocity.x, &this->acceleration.x , axis, otherBodyElasticity)){
		
		axisOnWhichStoped |= __XAXIS;
	}
	
	if ((__YAXIS & axis) && Body_bounceOnAxis(this, &this->velocity.y, &this->acceleration.y, axis, otherBodyElasticity)){
		
		axisOnWhichStoped |= __YAXIS;
	}

	if ((__ZAXIS & axis) && Body_bounceOnAxis(this, &this->velocity.z, &this->acceleration.z, axis, otherBodyElasticity)){
		
		axisOnWhichStoped |= __ZAXIS;
	}

	if (axisOnWhichStoped) {
	
		Body_stopMovement(this, axisOnWhichStoped);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bounce back
static int Body_bounceOnAxis(Body this, fix19_13* velocity, fix19_13* acceleration, int axis, fix19_13 otherBodyElasticity){
	
	// TODO: still not sure it must be divided by 2 (<< deltaFactor)
	int deltaFactor = 1;

	// get the elapsed time
	fix19_13 elapsedTime = PhysicalWorld_getElapsedTime(PhysicalWorld_getInstance());

	fix19_13 totalElasticity = this->elasticity + otherBodyElasticity;
	
	if (ITOFIX19_13(1) < totalElasticity) {
		
		totalElasticity = ITOFIX19_13(1);
	}
	
	fix19_13 bounceCoeficient = ITOFIX19_13(1) - totalElasticity;
	
	fix19_13 velocityDelta = FIX19_13_MULT(*acceleration, elapsedTime);
	
	*velocity -= velocityDelta << deltaFactor; 

	*velocity = -*velocity;

	*velocity = FIX19_13_MULT(*velocity, bounceCoeficient);

	*acceleration = 0;
	
	return (THRESHOLD + (velocityDelta << deltaFactor) >= abs(*velocity));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// take a hit
void Body_takeHitFrom(Body this, Body other){
	//TODO:
}


