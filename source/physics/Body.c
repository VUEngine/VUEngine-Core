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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Body.h>
#include <PhysicalWorld.h>
#include <MessageDispatcher.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Body
__CLASS_DEFINITION(Body, Object);


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

// this should be improved and calculated dynamically based on framerate

#define STOPED_MOVING		0
#define STILL_MOVES			1
#define CHANGED_DIRECTION	2

#define THRESHOLD FTOFIX19_13(0.1f)

//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Body_constructor(Body this, SpatialObject owner, fix19_13 mass);
static void Body_awake(Body this, int axisStartedMovement);
static void Body_updateAcceleration(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* acceleration, fix19_13 appliedForce, fix19_13 frictionForce);
static int Body_updateMovement(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* position, fix19_13* velocity, fix19_13* acceleration, fix19_13 appliedForce, int movementType, fix19_13 frictionForce);
static void Body_setMovementType(Body this, int movementType, int axis);
static bool Body_bounceOnAxis(Body this, fix19_13* velocity, fix19_13* acceleration, int axis, fix19_13 otherBodyElasticity);
static const Force* const Body_calculateFrictionForce(Body this, int axisOfMovement, const Acceleration* const gravity);

enum CollidingObjectIndexes
{
	eXAxis = 0,
	eYAxis,
	eZAxis,
	eLastCollidingObject,
};


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Body, SpatialObject owner, fix19_13 mass)
__CLASS_NEW_END(Body, owner, mass);

// class's constructor
static void Body_constructor(Body this, SpatialObject owner, fix19_13 mass)
{
	ASSERT(this, "Body::constructor: null this");

	__CONSTRUCT_BASE();

	this->owner = owner;

	this->mass = mass;

	this->awake = false;
	this->axisSubjectToGravity = __XAXIS | __YAXIS | __ZAXIS;

	// set position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;

	this->appliedForce.x = 0;
	this->appliedForce.y = 0;
	this->appliedForce.z = 0;

	this->friction.x = 0;
	this->friction.y = 0;
	this->friction.z = 0;

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

// class's destructor
void Body_destructor(Body this)
{
	ASSERT(this, "Body::destructor: null this");

	// destroy the super object
	__DESTROY_BASE;
}

// set game entity
void Body_setOwner(Body this, SpatialObject owner)
{
	ASSERT(this, "Body::setOwner: null this");

	this->owner = owner;
}

// get game entity
SpatialObject Body_getOwner(Body this)
{
	ASSERT(this, "Body::getOwner: null this");

	return this->owner;
}

// retrieve character's velocity
Velocity Body_getVelocity(Body this)
{
	ASSERT(this, "Body::getVelocity: null this");

	return this->velocity;
}

// retrieve acceleration
Acceleration Body_getAcceleration(Body this)
{
	ASSERT(this, "Body::getAcceleration: null this");

	return this->acceleration;
}

// retrieve movement type
MovementType Body_getMovementType(Body this)
{
	ASSERT(this, "Body::getMovementType: null this");

	return this->movementType;
}

// set movement type
static void Body_setMovementType(Body this, int movementType, int axis)
{
	ASSERT(this, "Body::setMovementType: null this");

	if (__XAXIS & axis)
	{
		this->movementType.x = movementType;

		if (__UNIFORM_MOVEMENT == movementType)
		{
			this->appliedForce.x = 0;
			this->acceleration.x = 0;
		}
	}

	if (__YAXIS & axis)
	{
		this->movementType.y = movementType;

		if (__UNIFORM_MOVEMENT == movementType)
		{
			this->appliedForce.y = 0;
			this->acceleration.y = 0;
		}
	}

	if (__ZAXIS & axis)
	{
		this->movementType.z = movementType;

		if (__UNIFORM_MOVEMENT == movementType)
		{
			this->appliedForce.z = 0;
			this->acceleration.z = 0;
		}
	}
}

void Body_clearAcceleration(Body this, u8 axis)
{
	ASSERT(this, "Body::moveAccelerated: null this");

	if (__XAXIS & axis)
	{
		this->acceleration.x = 0;
		this->appliedForce.x = 0;
	}

	if (__YAXIS & axis)
	{
		this->acceleration.y = 0;
		this->appliedForce.y = 0;
	}

	if (__ZAXIS & axis)
	{
		this->acceleration.z = 0;
		this->appliedForce.z = 0;
	}
}

// set movement type to accelerated
void Body_moveAccelerated(Body this, u8 axis)
{
	ASSERT(this, "Body::moveAccelerated: null this");

	if (__XAXIS & axis)
	{
		Body_setMovementType(this, __ACCELERATED_MOVEMENT, __XAXIS);
	}

	if (__YAXIS & axis)
	{
		Body_setMovementType(this, __ACCELERATED_MOVEMENT, __YAXIS);
	}

	if (__ZAXIS & axis)
	{
		Body_setMovementType(this, __ACCELERATED_MOVEMENT, __ZAXIS);
	}
}

// set movement type to uniform
void Body_moveUniformly(Body this, Velocity velocity)
{
	ASSERT(this, "Body::moveUniformly: null this");

	if (velocity.x)
	{
		Body_setMovementType(this, __UNIFORM_MOVEMENT, __XAXIS);
		Body_awake(this, __XAXIS);
		this->velocity.x = velocity.x;
	}

	if (velocity.y)
	{
		Body_setMovementType(this, __UNIFORM_MOVEMENT, __YAXIS);
		Body_awake(this, __YAXIS);
		this->velocity.y = velocity.y;
	}

	if (velocity.z)
	{
		Body_setMovementType(this, __UNIFORM_MOVEMENT, __ZAXIS);
		Body_awake(this, __ZAXIS);
		this->velocity.z = velocity.z;
	}
}

// clear force
void Body_clearForce(Body this)
{
	ASSERT(this, "Body::clearForce: null this");

	this->appliedForce.x = 0;
	this->appliedForce.y = 0;
	this->appliedForce.z = 0;
}

// apply force
void Body_applyForce(Body this, const Force* force, bool clearAxis)
{
	ASSERT(this, "Body::applyForce: null this");

	if (__XAXIS & clearAxis)
	{
		this->velocity.x = 0;
		this->acceleration.x = 0;
	}

	if (__YAXIS & clearAxis)
	{
		this->velocity.y = 0;
		this->acceleration.y = 0;
	}

	if (__ZAXIS & clearAxis)
	{
		this->velocity.z = 0;
		this->acceleration.z = 0;
	}

	this->appliedForce.x = force->x;
	this->appliedForce.y = force->y;
	this->appliedForce.z = force->z;
	
	this->acceleration.x += FIX19_13_DIV(this->appliedForce.x, this->mass);
	this->acceleration.y += FIX19_13_DIV(this->appliedForce.y, this->mass);
	this->acceleration.z += FIX19_13_DIV(this->appliedForce.z, this->mass);

	int axisStartedMovement = 0;

	if (this->appliedForce.x)
	{
		axisStartedMovement |= __XAXIS;
		Body_moveAccelerated(this, __XAXIS);
	}

	if (this->appliedForce.y)
	{
		axisStartedMovement |= __YAXIS;
		Body_moveAccelerated(this, __YAXIS);
	}

	if (this->appliedForce.z)
	{
		axisStartedMovement |= __ZAXIS;
		Body_moveAccelerated(this, __ZAXIS);
	}

	Body_awake(this, axisStartedMovement);
}

// apply gravity
void Body_applyGravity(Body this, const Acceleration* gravity)
{
	ASSERT(this, "Body::applyGravity: null this");

	if (gravity)
	{
		int axisStartedMovement = 0;

		if (gravity->x)
		{
			this->acceleration.x = gravity->x;
			axisStartedMovement |= __XAXIS;
			Body_moveAccelerated(this, __XAXIS);
		}

		if (gravity->y)
		{
			this->acceleration.y = gravity->y;
			axisStartedMovement |= __YAXIS;
			Body_moveAccelerated(this, __YAXIS);
		}

		if (gravity->z)
		{
			this->acceleration.z = gravity->z;
			axisStartedMovement |= __ZAXIS;
			Body_moveAccelerated(this, __YAXIS);
		}

		if (axisStartedMovement)
		{
			Body_awake(this, axisStartedMovement);
		}
	}
}


// add force
void Body_addForce(Body this, const Force* force)
{
	ASSERT(this, "Body::addForce: null this");
	ASSERT(force, "Body::addForce: null force");

	Body_applyForce(this, force, ~Body_isMoving(this));
}

// update movement
void Body_update(Body this, const Acceleration* gravity, fix19_13 elapsedTime)
{
	ASSERT(this, "Body::update: null this");

	if (this->awake)
	{
		if (elapsedTime)
		{
			int axisStopedMovement = 0;
			int axisOfMovement = 0;
 			int axisOfChangeOfMovement = 0;

			if (this->velocity.x || this->acceleration.x || this->appliedForce.x || (__ACCELERATED_MOVEMENT == this->movementType.x && gravity->x && this->acceleration.x))
			{
				axisOfMovement |= __XAXIS;
			}

			if (this->velocity.y || this->acceleration.y || this->appliedForce.y || (__ACCELERATED_MOVEMENT == this->movementType.y && gravity->y && this->acceleration.y))
			{
				axisOfMovement |= __YAXIS;
			}

		 	if (this->velocity.z || this->acceleration.z || this->appliedForce.z || (__ACCELERATED_MOVEMENT == this->movementType.z && gravity->z && this->acceleration.z))
			{
				axisOfMovement |= __ZAXIS;
		 	}

			const Force* const frictionForce = Body_calculateFrictionForce(this, axisOfMovement, gravity);

			// update each axis
	 	 	if (__XAXIS & axisOfMovement)
			{
	 	 		int movementStatus = Body_updateMovement(this, elapsedTime, __XAXIS & this->axisSubjectToGravity? gravity->x: 0, &this->position.x, &this->velocity.x, &this->acceleration.x, this->appliedForce.x, this->movementType.x, frictionForce->x);

	 	 		if (movementStatus)
				{
	 	 			if (CHANGED_DIRECTION == movementStatus)
					{
	 	 				axisOfChangeOfMovement |= __XAXIS;
	 	 			}
	 	 		}
	 	 		else
				{
	 	 			axisStopedMovement |= __XAXIS;
	 	 		}
	 	 	}

	 	 	if (__YAXIS & axisOfMovement)
			{
	 	 		int movementStatus = Body_updateMovement(this, elapsedTime, __YAXIS & this->axisSubjectToGravity? gravity->y: 0, &this->position.y, &this->velocity.y, &this->acceleration.y, this->appliedForce.y, this->movementType.y, frictionForce->y);

	 	 		if (movementStatus)
				{
	 	 			if (CHANGED_DIRECTION == movementStatus)
					{
	 	 				axisOfChangeOfMovement |= __YAXIS;
	 	 			}
	 	 		}
	 	 		else
				{
	 	 			axisStopedMovement |= __YAXIS;
	 	 		}
	 	 	}

	 	 	if (__ZAXIS & axisOfMovement)
			{
	 	 		int movementStatus = Body_updateMovement(this, elapsedTime, __ZAXIS & this->axisSubjectToGravity? gravity->z: 0, &this->position.z, &this->velocity.z, &this->acceleration.z, this->appliedForce.z, this->movementType.z, frictionForce->z);

	 	 		if (movementStatus)
				{
	 	 			if (CHANGED_DIRECTION == movementStatus)
					{
	 	 				axisOfChangeOfMovement |= __ZAXIS;
	 	 			}
	 	 		}
	 	 		else
				{
	 	 			axisStopedMovement |= __ZAXIS;
	 	 		}
	 	 	}

		 	// if stopped on any axis
		 	if (axisStopedMovement)
			{
	 			MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this->owner), kBodyStoped, &axisStopedMovement);
		 	}

		 	if (axisOfChangeOfMovement)
			{
		 		MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this->owner), kBodyChangedDirection, &axisOfChangeOfMovement);
		 	}

		 	// clear any force so the next update does not get influenced
			Body_clearForce(this);
		}
	}
}

// update force
static const Force* const Body_calculateFrictionForce(Body this, int axisOfMovement, const Acceleration* const gravity)
{
	ASSERT(this, "Body::calculateFriction: null this");

	// get friction fBody from the game world
	fix19_13 worldFriction = PhysicalWorld_getFriction(PhysicalWorld_getInstance()) * 2;

	static Force frictionForce = 
	{
		0, 0, 0
	};
	
	frictionForce.x = 0;
	frictionForce.y = 0;
	frictionForce.z = 0;

	fix19_13 weight = this->mass;

	if((__XAXIS & axisOfMovement))
	{
//		fix19_13 weight = (__XAXIS & this->axisSubjectToGravity) && gravity->x? FIX19_13_MULT(gravity->x, this->mass): this->mass;

		if(this->appliedForce.x)
		{
			frictionForce.x = (0 < this->appliedForce.x)? -FIX19_13_MULT((this->friction.x + worldFriction), weight): FIX19_13_MULT((this->friction.x + worldFriction), weight);
		}
		else if(this->velocity.x)
		{
			frictionForce.x = (0 < this->velocity.x)? -FIX19_13_MULT((this->friction.x + worldFriction), weight): FIX19_13_MULT((this->friction.x + worldFriction), weight);
		}
	}

	if((__YAXIS & axisOfMovement))
	{
//		fix19_13 weight = (__YAXIS & this->axisSubjectToGravity) && gravity->y? FIX19_13_MULT(gravity->y, this->mass): this->mass;

		if(this->appliedForce.y)
		{
			frictionForce.y = (0 < this->appliedForce.y)? -FIX19_13_MULT((this->friction.y + worldFriction), weight): FIX19_13_MULT((this->friction.y + worldFriction), weight);
		}
		else if(this->velocity.y)
		{
			frictionForce.y = (0 < this->velocity.y)? -FIX19_13_MULT((this->friction.y + worldFriction), weight): FIX19_13_MULT((this->friction.y + worldFriction), weight);
		}
	}

	if((__ZAXIS & axisOfMovement))
	{
//		fix19_13 weight = (__ZAXIS & this->axisSubjectToGravity) && gravity->z? FIX19_13_MULT(gravity->z, this->mass): this->mass;

		if(this->appliedForce.z)
		{
			frictionForce.z = (0 < this->appliedForce.z)? -FIX19_13_MULT((this->friction.z + worldFriction), weight): FIX19_13_MULT((this->friction.z + worldFriction), weight);
		}
		else if(this->velocity.z)
		{
			frictionForce.z = (0 < this->velocity.z)? -FIX19_13_MULT((this->friction.z + worldFriction), weight): FIX19_13_MULT((this->friction.z + worldFriction), weight);
		}
	}

	return &frictionForce;
}

// update force
static void Body_updateAcceleration(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* acceleration, fix19_13 appliedForce, fix19_13 frictionForce)
{
	ASSERT(this, "Body::updateAcceleration: null this");

	fix19_13 sign = ITOFIX19_13(0 <= gravity ? -1 : 1);

	if (gravity)
	{
		if (0 > FIX19_13_MULT((*acceleration - gravity), sign))
		{
			gravity = 0;
		}
		else if (FIX19_13_MULT((*acceleration + FIX19_13_MULT(gravity, elapsedTime) - gravity), sign))
		{
			gravity = gravity - *acceleration;
		}

		*acceleration += FIX19_13_MULT(gravity, elapsedTime);
	}

	fix19_13 frictionAcceleration = FIX19_13_DIV(frictionForce, this->mass);

	if(appliedForce)
	{
		if(abs(frictionAcceleration) < abs(*acceleration))
		{
			*acceleration += frictionAcceleration;
		}
	}
	else
	{
		*acceleration += frictionAcceleration;
	}
}

// retrieve last displacement
VBVec3D Body_getLastDisplacement(Body this)
{
	ASSERT(this, "Body::getLastDisplacement: null this");

	VBVec3D displacement = {0, 0, 0};

	fix19_13 elapsedTime = PhysicalWorld_getElapsedTime(PhysicalWorld_getInstance());

	displacement.x = FIX19_13_MULT(this->velocity.x, elapsedTime);
	displacement.y = FIX19_13_MULT(this->velocity.y, elapsedTime);
	displacement.z = FIX19_13_MULT(this->velocity.z, elapsedTime);

 	return displacement;
}

// udpdate movement over axis
static int Body_updateMovement(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* position, fix19_13* velocity, fix19_13* acceleration, fix19_13 appliedForce, int movementType, fix19_13 frictionForce)
{
	ASSERT(this, "Body::updateMovement: null this");

	// the movement displacement
	fix19_13 displacement = 0;

	int moving = STILL_MOVES;

	// determine the movement type
	// calculate displacement based in velocity, time and acceleration
 	if (__ACCELERATED_MOVEMENT == movementType)
	{
 		Body_updateAcceleration(this, elapsedTime, gravity, acceleration, appliedForce, frictionForce);

 		fix19_13 previousVelocity = *velocity;

		// update the velocity
		*velocity += FIX19_13_MULT(*acceleration, elapsedTime);

		displacement =
			FIX19_13_MULT(*velocity, elapsedTime)
			+ FIX19_13_DIV(FIX19_13_MULT(*acceleration, FIX19_13_MULT(elapsedTime, elapsedTime)), ITOFIX19_13(2));

		if(!gravity)
		{
			if((0 < previousVelocity && 0 > *velocity) || (0 > previousVelocity && 0 < *velocity))
			{
	 			*velocity = 0;
	 			*acceleration = 0;
	 			displacement = 0;
			}
		}

 		if (!appliedForce && !*velocity)
		{
 			moving = STOPED_MOVING;
 		}
 		else
		{
 			if ((0 < previousVelocity && 0 > *velocity) || (0 > previousVelocity && 0 < *velocity))
			{
 				moving = CHANGED_DIRECTION;
 			}
 		}
 	}
 	else if (__UNIFORM_MOVEMENT == movementType)
	{
		// update the velocity
		displacement = FIX19_13_MULT(*velocity, elapsedTime);
 	}
 	else
	{
 		ASSERT(false, "Body::updateMovement: wrong movement type");
 	}

 	// update position
 	*position += displacement;

 	// return movement state
 	return moving;
}

void Body_printPhysics(Body this, int x, int y)
{
	ASSERT(this, "Body::printPhysics: null this");

	__ACCELERATED_MOVEMENT == this->movementType.x
	    ? Printing_text(Printing_getInstance(), "Accelerated", x, y++, NULL)
	    : Printing_text(Printing_getInstance(), "Uniform", x, y++, NULL);

	Printing_text(Printing_getInstance(), "X             Y             Z", x, y++, NULL);

	Printing_text(Printing_getInstance(), "Position", x, y++, NULL);
	Printing_text(Printing_getInstance(), "                             ", x, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(this->position.x ), x, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(this->position.y), x+14, y, NULL);
	Printing_int(Printing_getInstance(), FIX19_13TOI(this->position.z), x+14*2, y++, NULL);

	Printing_text(Printing_getInstance(), "Velocity", x, y++, NULL);
	Printing_text(Printing_getInstance(), "                             ", x, y, NULL);
	Printing_float(Printing_getInstance(), FIX19_13TOF(this->velocity.x), x, y, NULL);
	Printing_float(Printing_getInstance(), FIX19_13TOF(this->velocity.y), x+14, y, NULL);
	Printing_float(Printing_getInstance(), FIX19_13TOF(this->velocity.z), x+14*2, y++, NULL);
	Printing_text(Printing_getInstance(), "Acceleration", x, y++, NULL);
	Printing_text(Printing_getInstance(), "                             ", x, y, NULL);
	Printing_float(Printing_getInstance(), FIX19_13TOF(this->acceleration.x), x, y, NULL);
	Printing_float(Printing_getInstance(), FIX19_13TOF(this->acceleration.y), x+14, y, NULL);
	Printing_float(Printing_getInstance(), FIX19_13TOF(this->acceleration.z), x+14*2, y++, NULL);
}

// stop movement over an axis
void Body_stopMovement(Body this, u8 axis)
{
	ASSERT(this, "Body::stopMovement: null this");

	u8 axisOfStopping = 0;
	
	if (__XAXIS & axis)
	{
		// not moving anymore
		this->velocity.x = 0;
		this->acceleration.x = 0;
		this->appliedForce.x = 0;
		axisOfStopping |= __XAXIS;
	}

	if (__YAXIS & axis)
	{
		// not moving anymore
		this->velocity.y = 0;
		this->acceleration.y = 0;
		this->appliedForce.y = 0;
		axisOfStopping |= __YAXIS;
	}

	if (__ZAXIS & axis)
	{
		// not moving anymore
		this->velocity.z = 0;
		this->acceleration.z = 0;
		this->appliedForce.z = 0;
		axisOfStopping |= __ZAXIS;
	}

	if (!Body_isMoving(this))
	{
		Body_sleep(this);
		MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this->owner), kBodyStoped, &axisOfStopping);
	}
}

// set axis subjet to gravity
u8 Body_getAxisSubjectToGravity(Body this)
{
	ASSERT(this, "Body::getAxisSubjectToGravity: null this");

	return this->axisSubjectToGravity;
}

// set axis subjet to gravity
void Body_setAxisSubjectToGravity(Body this, u8 axisSubjectToGravity)
{
	ASSERT(this, "Body::setAxisSubjectToGravity: null this");

	this->axisSubjectToGravity = axisSubjectToGravity;
}

// set active
void Body_setActive(Body this, bool active)
{
	ASSERT(this, "Body::setActive: null this");

	// it is active
	this->active = active;
}

// is active?
bool Body_isActive(Body this)
{
	ASSERT(this, "Body::isActive: null this");

	return this->active;
}

// retrieve position
VBVec3D Body_getPosition(Body this)
{
	ASSERT(this, "Body::getPosition: null this");

	return this->position;
}

// retrieve position
void Body_setPosition(Body this, VBVec3D position, SpatialObject caller)
{
	ASSERT(this, "Body::setPosition: null this");

	if (this->owner == caller)
	{
		// set position
		this->position = position;
	}
}

// get elasticiy
fix19_13 Body_getElasticity(Body this)
{
	ASSERT(this, "Body::getElasticity: null this");

	return this->elasticity;
}

// set elasticiy
void Body_setElasticity(Body this, fix19_13 elasticity)
{
	ASSERT(this, "Body::setElasticity: null this");

	if (ITOFIX19_13(0) > elasticity)
	{
		elasticity = 0;
	}
	else if (ITOFIX19_13(1) < elasticity)
	{
		elasticity = ITOFIX19_13(1);
	}

	this->elasticity = elasticity;
}

// get friction
Force Body_getFriction(Body this)
{
	ASSERT(this, "Body::getFriction: null this");

	return this->friction;
}

// set elasticity
void Body_setFriction(Body this, Force friction)
{
	ASSERT(this, "Body::setFriction: null this");

	this->friction = friction;
}

// retrieve state
bool Body_isAwake(Body this)
{
	ASSERT(this, "Body::isAwake: null this");

	return this->awake && this->active;
}

// awake body
static void Body_awake(Body this, int axisStartedMovement)
{
	ASSERT(this, "Body::awake: null this");

	bool dispatchMessage = false;

	if (!this->awake)
	{
		this->awake = true;

		PhysicalWorld_bodyAwaked(PhysicalWorld_getInstance(), this);
	}

	if (!this->velocity.x && (__XAXIS & axisStartedMovement))
	{
		dispatchMessage |= (__XAXIS & axisStartedMovement);
	}

	if (!this->velocity.y && (__YAXIS & axisStartedMovement))
	{
		dispatchMessage |= (__YAXIS & axisStartedMovement);
	}

	if (!this->velocity.z && (__ZAXIS & axisStartedMovement))
	{
		dispatchMessage |= (__ZAXIS & axisStartedMovement);
	}

	if (dispatchMessage)
	{
		MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this->owner), kBodyStartedMoving, &axisStartedMovement);
	}
}

// go to sleep
void Body_sleep(Body this)
{
	ASSERT(this, "Body::sleep: null this");

	this->awake = false;

	PhysicalWorld_bodySleep(PhysicalWorld_getInstance(), this);

	MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this->owner), kBodySleep, NULL);
}

// is it moving?
u8 Body_isMoving(Body this)
{
	ASSERT(this, "Body::isMoving: null this");

	u8 result = 0;

	result |= ((int)FIX19_13TOI(this->velocity.x) || this->acceleration.x) ? __XAXIS : 0;
	result |= ((int)FIX19_13TOI(this->velocity.y) || this->acceleration.y) ? __YAXIS : 0;
	result |= ((int)FIX19_13TOI(this->velocity.z) || this->acceleration.z) ? __ZAXIS : 0;

	return this->awake && this->active ? result : 0;
}

// bounce back
void Body_bounce(Body this, u8 axis, fix19_13 otherBodyElasticity)
{
	ASSERT(this, "Body::bounce: null this");

	int axisOnWhichStoped = 0;
	int axisOnWhichBounced = 0;

	if ((__XAXIS & axis))
	{
		if (Body_bounceOnAxis(this, &this->velocity.x, &this->acceleration.x, axis, otherBodyElasticity))
		{
			axisOnWhichBounced |= __XAXIS;
		}
		else
		{
			axisOnWhichStoped |= __XAXIS;
		}
	}

	if ((__YAXIS & axis))
	{
		if (Body_bounceOnAxis(this, &this->velocity.y, &this->acceleration.y, axis, otherBodyElasticity))
		{
			axisOnWhichBounced |= __YAXIS;
		}
		else
		{
			axisOnWhichStoped |= __YAXIS;
		}
	}

	if ((__ZAXIS & axis))
	{
		if (Body_bounceOnAxis(this, &this->velocity.z, &this->acceleration.z, axis, otherBodyElasticity))
		{
			axisOnWhichBounced |= __ZAXIS;
		}
		else
		{
			axisOnWhichStoped |= __ZAXIS;
		}
	}

	if (axisOnWhichStoped)
	{
		Body_stopMovement(this, axisOnWhichStoped);
	}

	if (axisOnWhichBounced)
	{
 		MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this->owner), kBodyBounced, &axisOnWhichBounced);
	}
}

// bounce back
static bool Body_bounceOnAxis(Body this, fix19_13* velocity, fix19_13* acceleration, int axis, fix19_13 otherBodyElasticity)
{
	ASSERT(this, "Body::bounceOnAxis: null this");

	// get the elapsed time
	PhysicalWorld physicalWorld = PhysicalWorld_getInstance();
	fix19_13 elapsedTime = PhysicalWorld_getElapsedTime(physicalWorld);
	const VBVec3D* gravity = PhysicalWorld_getGravity(physicalWorld);
	fix19_13 totalElasticity = this->elasticity + otherBodyElasticity;
	
	fix19_13 deltaFactor = 0;
	
	switch(axis)
	{
		case __XAXIS:
			
			deltaFactor = gravity->x? FIX19_13_DIV(ITOFIX19_13(__GRAVITY), gravity->x): ITOFIX19_13(1);
			break;
		
		case __YAXIS:
			
			deltaFactor = gravity->x? FIX19_13_DIV(ITOFIX19_13(__GRAVITY), gravity->y): ITOFIX19_13(1);
			break;
			
		case __ZAXIS:
			
			deltaFactor = gravity->x? FIX19_13_DIV(ITOFIX19_13(__GRAVITY), gravity->z): ITOFIX19_13(1);
			break;
	}
	
	if (ITOFIX19_13(1) < totalElasticity)
	{
		totalElasticity = ITOFIX19_13(1);
	}

	fix19_13 bounceCoeficient = ITOFIX19_13(1) - totalElasticity;

	fix19_13 velocityDelta = FIX19_13_MULT(*acceleration, elapsedTime);
	velocityDelta = FIX19_13_MULT(velocityDelta, deltaFactor);
	
	*velocity += velocityDelta;
	
	ASSERT(deltaFactor, "Body::bounceOnAxis: null 0 deltaFactor");

	*velocity = FIX19_13_MULT(-*velocity, bounceCoeficient);
	*velocity = FIX19_13_DIV(*velocity, deltaFactor);

	*acceleration = 0;

	return ((velocityDelta) < abs(*velocity));
}

// take a hit
void Body_takeHitFrom(Body this, Body other)
{
	ASSERT(this, "Body::takeHitFrom: null this");

	//TODO:
}