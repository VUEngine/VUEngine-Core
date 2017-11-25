/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Body.h>
#include <Game.h>
#include <Clock.h>
#include <Vector3D.h>
#include <PhysicalWorld.h>
#include <MessageDispatcher.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Body
 * @extends Object
 * @ingroup physics
 */
__CLASS_DEFINITION(Body, Object);
__CLASS_FRIEND_DEFINITION(VirtualList);
__CLASS_FRIEND_DEFINITION(VirtualNode);


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// this should be improved and calculated dynamically based on framerate

#define STOPPED_MOVING		0
#define STILL_MOVES			1
#define CHANGED_DIRECTION	2

#define __STOP_VELOCITY_THRESHOLD			__I_TO_FIX19_13(10)


//---------------------------------------------------------------------------------------------------------
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

fix19_13 _currentWorldFriction = 0;
fix19_13 _currentElapsedTime = 0;
const Acceleration* _currentGravity = 0;

void Body_setCurrentWorldFrictionCoefficient(fix19_13 currentWorldFriction)
{
	_currentWorldFriction = currentWorldFriction;
}

void Body_setCurrentElapsedTime(fix19_13 currentElapsedTime)
{
	_currentElapsedTime = currentElapsedTime;
}

void Body_setCurrentGravity(const Acceleration* currentGravity)
{
	_currentGravity = currentGravity;
}


enum CollidingObjectIndexes
{
	eXAxis = 0,
	eYAxis,
	eZAxis,
	eLastCollidingObject,
};

typedef struct MovementResult
{
	u16 axesStoppedMovement;
	u16 axesOfAcceleratedBouncing;
	u16 axesOfChangeOfMovement;

} MovementResult;

typedef struct NormalRegistry
{
	Object referent;
	Vector3D direction;
	fix19_13 magnitude;

} NormalRegistry;


Clock _physhicsClock = NULL;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

MovementResult Body_updateMovement(Body this);
static void Body_awake(Body this, u16 axesOfAwakening);
static void Body_setMovementType(Body this, int movementType, u16 axes);
static u16 Body_doStopMovement(Body this, u16 axes);
static void Body_clearNormalOnAxes(Body this, u16 axes);
Acceleration Body_getGravity(Body this);
static void Body_computeTotalNormal(Body this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Body, SpatialObject owner, const PhysicalSpecification* physicalSpecification,  u16 axesSubjectToGravity)
__CLASS_NEW_END(Body, owner, physicalSpecification,  axesSubjectToGravity);

// class's constructor
void Body_constructor(Body this, SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axesSubjectToGravity)
{
	ASSERT(this, "Body::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->owner = owner;
	this->normals = NULL;
	this->mass = 0 < physicalSpecification->mass ? physicalSpecification->mass : __I_TO_FIX19_13(1);
	this->elasticity = physicalSpecification->elasticity;
	this->frictionCoefficient = physicalSpecification->frictionCoefficient;
	this->frictionForceMagnitude = 0;

	this->active = true;
	this->awake = false;
	this->axesSubjectToGravity = axesSubjectToGravity;

	// clear movement type
	this->movementType.x = __NO_MOVEMENT;
	this->movementType.y = __NO_MOVEMENT;
	this->movementType.z = __NO_MOVEMENT;

	this->position 				= (Vector3D){0, 0, 0};
	this->velocity 				= (Velocity){0, 0, 0};
	this->acceleration 			= (Acceleration){0, 0, 0};
	this->externalForce	 		= (Force){0, 0, 0};
	this->friction 				= (Force){0, 0, 0};
	this->totalNormal			= (Force){0, 0, 0};
	this->weight 				= Vector3D_scalarProduct(*_currentGravity, this->mass);

	if(!_physhicsClock)
	{
		_physhicsClock = Game_getPhysicsClock(Game_getInstance());
	}
}

// class's destructor
void Body_destructor(Body this)
{
	ASSERT(this, "Body::destructor: null this");

	if(this->normals)
	{
		VirtualNode node = this->normals->head;

		for(; node; node = node->next)
		{
			__DELETE_BASIC(node->data);
		}

		__DELETE(this->normals);
		this->normals = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
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

// retrieve applied force
Force Body_getAppliedForce(Body this)
{
	ASSERT(this, "Body::getAppliedForce: null this");

	return this->externalForce;
}

// retrieve movement type
MovementType Body_getMovementType(Body this)
{
	ASSERT(this, "Body::getMovementType: null this");

	return this->movementType;
}

// set movement type
static void Body_setMovementType(Body this, int movementType, u16 axes)
{
	ASSERT(this, "Body::setMovementType: null this");

	if(__X_AXIS & axes)
	{
		this->movementType.x = movementType;
	}

	if(__Y_AXIS & axes)
	{
		this->movementType.y = movementType;
	}

	if(__Z_AXIS & axes)
	{
		this->movementType.z = movementType;
	}
}

void Body_clearAcceleration(Body this, u16 axes)
{
	ASSERT(this, "Body::moveAccelerated: null this");

	if(__X_AXIS & axes)
	{
		this->acceleration.x = 0;
		this->externalForce.x = 0;
	}

	if(__Y_AXIS & axes)
	{
		this->acceleration.y = 0;
		this->externalForce.y = 0;
	}

	if(__Z_AXIS & axes)
	{
		this->acceleration.z = 0;
		this->externalForce.z = 0;
	}
}

// set movement type to accelerated
void Body_moveAccelerated(Body this, u16 axes)
{
	ASSERT(this, "Body::moveAccelerated: null this");

	Body_setMovementType(this, __ACCELERATED_MOVEMENT, axes);
}

// set movement type to uniform
void Body_moveUniformly(Body this, Velocity velocity)
{
	ASSERT(this, "Body::moveUniformly: null this");

	u16 axesOfUniformMovement = 0;

	if(velocity.x)
	{
		axesOfUniformMovement |= __X_AXIS;
		this->velocity.x = velocity.x;
	}

	if(velocity.y)
	{
		axesOfUniformMovement |= __Y_AXIS;
		this->velocity.y = velocity.y;
	}

	if(velocity.z)
	{
		axesOfUniformMovement |= __Z_AXIS;
		this->velocity.z = velocity.z;
	}

	if(axesOfUniformMovement)
	{
		Body_setMovementType(this, __UNIFORM_MOVEMENT, axesOfUniformMovement);
		Body_awake(this, axesOfUniformMovement);
	}
}

// clear force
void Body_clearExternalForce(Body this)
{
	ASSERT(this, "Body::clearExternalForce: null this");

	this->externalForce.x = 0;
	this->externalForce.y = 0;
	this->externalForce.z = 0;
}

// apply force
void Body_applyForce(Body this, const Force* force)
{
	ASSERT(this, "Body::applyForce: null this");

	if(force)
	{
		this->externalForce = *force;
		u16 axesOfExternalForce = __NO_AXIS;

		if(this->externalForce.x)
		{
			axesOfExternalForce |= __X_AXIS;
		}

		if(this->externalForce.y)
		{
			axesOfExternalForce |= __Y_AXIS;
		}

		if(this->externalForce.z)
		{
			axesOfExternalForce |= __Z_AXIS;
		}

		if(axesOfExternalForce)
		{
			Body_clearNormalOnAxes(this, axesOfExternalForce);
			Body_setMovementType(this, __ACCELERATED_MOVEMENT, axesOfExternalForce);
			Body_awake(this, axesOfExternalForce);
		}
	}
}

// apply gravity
void Body_applyGravity(Body this, u16 axes)
{
	ASSERT(this, "Body::applyGravity: null this");

	if(axes)
	{
		Body_awake(this, axes);
	}
}

// add force
void Body_addForce(Body this, const Force* force)
{
	ASSERT(this, "Body::addForce: null this");
	ASSERT(force, "Body::addForce: null force");

	Body_applyForce(this, force);
}

// update movement
void Body_update(Body this)
{
	ASSERT(this, "Body::update: null this");

	if(this->active)
	{
		if(this->awake)
		{
			MovementResult movementResult = Body_updateMovement(this);

			// if stopped on any axes
			if(movementResult.axesStoppedMovement)
			{
				u16 axesOfStopping = Body_doStopMovement(this, movementResult.axesStoppedMovement);

				if(axesOfStopping)
				{
					MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyStopped, &axesOfStopping);
				}
			}

			// no one uses this
/*			if(movementResult.axesOfChangeOfMovement)
			{
				MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyChangedDirection, &movementResult.axesOfChangeOfMovement);
			}
*/		}

		// clear any force so the next update does not get influenced
		Body_clearExternalForce(this);
	}
}

// retrieve last displacement
Vector3D Body_getLastDisplacement(Body this)
{
	ASSERT(this, "Body::getLastDisplacement: null this");

	Vector3D displacement = {0, 0, 0};

	fix19_13 elapsedTime = PhysicalWorld_getElapsedTime(Game_getPhysicalWorld(Game_getInstance()));

	displacement.x = __FIX19_13_MULT(this->velocity.x, elapsedTime);
	displacement.y = __FIX19_13_MULT(this->velocity.y, elapsedTime);
	displacement.z = __FIX19_13_MULT(this->velocity.z, elapsedTime);

	return displacement;
}

static MovementResult Body_getMovementResult(Body this, Vector3D previousVelocity)
{
	ASSERT(this, "Body::checkIfStopped: null this");

	MovementResult movementResult = {__NO_AXIS, __NO_AXIS, __NO_AXIS};

	// xor values, if equal, result is 0
	movementResult.axesOfChangeOfMovement |= __FIX19_13_INT_PART(this->velocity.x ^ previousVelocity.x) ? __X_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= __FIX19_13_INT_PART(this->velocity.y ^ previousVelocity.y) ? __Y_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= __FIX19_13_INT_PART(this->velocity.z ^ previousVelocity.z) ? __Z_AXIS : __NO_AXIS;

	// stop if no external force or opposing normal force is present
	// and if the velocity minimum threshold is not reached
	if(__UNIFORM_MOVEMENT != this->movementType.x)
	{
		if((__X_AXIS & movementResult.axesOfChangeOfMovement) && (!this->externalForce.x || (0 < this->totalNormal.x * this->velocity.x)))
		{
			if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.x))
			{
				movementResult.axesStoppedMovement |= __X_AXIS;
			}
		}
	}

	if(__UNIFORM_MOVEMENT != this->movementType.y)
	{
		if((__Y_AXIS & movementResult.axesOfChangeOfMovement) && (!this->externalForce.y || (0 < this->totalNormal.y * this->velocity.y)))
		{
			if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.y))
			{
				movementResult.axesStoppedMovement |= __Y_AXIS;
			}
		}
	}

	if(__UNIFORM_MOVEMENT != this->movementType.z)
	{
		if((__Z_AXIS & movementResult.axesOfChangeOfMovement) && (!this->externalForce.z || (0 < this->totalNormal.z * this->velocity.z)))
		{
			if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.z))
			{
				movementResult.axesStoppedMovement |= __Z_AXIS;
			}
		}
	}

	// cannot change direction if movement stopped on that axes
	movementResult.axesOfChangeOfMovement &= ~movementResult.axesStoppedMovement;

	return movementResult;
}

Acceleration Body_getGravity(Body this)
{
	return (Acceleration)
	{
		__X_AXIS & this->axesSubjectToGravity ? _currentGravity->x : 0,
		__Y_AXIS & this->axesSubjectToGravity ? _currentGravity->y : 0,
		__Z_AXIS & this->axesSubjectToGravity ? _currentGravity->z : 0,
	};
}

// udpdate movement over axes
MovementResult Body_updateMovement(Body this)
{
	ASSERT(this, "Body::updateMovement: null this");

	Acceleration gravity = Body_getGravity(this);
	this->weight = Vector3D_scalarProduct(gravity, this->mass);

	Velocity velocity =
	{
		__FIX19_13_INT_PART(this->velocity.x),
		__FIX19_13_INT_PART(this->velocity.y),
		__FIX19_13_INT_PART(this->velocity.z),
	};

	//this->friction = Vector3D_scalarProduct(Vector3D_normalize(velocity), -this->frictionForceMagnitude);
	this->friction = Vector3D_scalarProduct(this->velocity, -this->frictionCoefficient << 5);

	fix19_13 elapsedTime = _currentElapsedTime;
	fix19_13 elapsedTimeHalfSquare = __FIX19_13_MULT(elapsedTime, elapsedTime) >> 1;

	Velocity previousVelocity = this->velocity;

	if(__UNIFORM_MOVEMENT == this->movementType.x)
	{
		this->acceleration.x = 0;
		this->velocity.x += __FIX19_13_MULT(this->acceleration.x, elapsedTime);
		this->position.x += __FIX19_13_MULT(this->velocity.x, elapsedTime);
	}
	else if((__ACCELERATED_MOVEMENT == this->movementType.x) | gravity.x | this->externalForce.x | this->totalNormal.x | this->friction.x)
	{
		this->acceleration.x = gravity.x + __FIX19_13_DIV(this->externalForce.x + this->totalNormal.x + this->friction.x, this->mass);
		this->velocity.x += __FIX19_13_MULT(this->acceleration.x, elapsedTime);
		this->position.x += __FIX19_13_MULT(this->velocity.x, elapsedTime) + __FIX19_13_MULT(this->acceleration.x, elapsedTimeHalfSquare);
	}

	if(__UNIFORM_MOVEMENT == this->movementType.y)
	{
		this->acceleration.y = 0;
		this->velocity.y += __FIX19_13_MULT(this->acceleration.y, elapsedTime);
		this->position.y += __FIX19_13_MULT(this->velocity.y, elapsedTime);
	}
	else if((__ACCELERATED_MOVEMENT == this->movementType.y) | gravity.y | this->externalForce.y | this->totalNormal.y | this->friction.y)
	{
		this->acceleration.y = gravity.y + __FIX19_13_DIV(this->externalForce.y + this->totalNormal.y + this->friction.y, this->mass);
		this->velocity.y += __FIX19_13_MULT(this->acceleration.y, elapsedTime);
		this->position.y += __FIX19_13_MULT(this->velocity.y, elapsedTime) + __FIX19_13_MULT(this->acceleration.y, elapsedTimeHalfSquare);
	}

	if(__UNIFORM_MOVEMENT == this->movementType.z)
	{
		this->acceleration.z = 0;
		this->velocity.z += __FIX19_13_MULT(this->acceleration.z, elapsedTime);
		this->position.z += __FIX19_13_MULT(this->velocity.z, elapsedTime);
	}
	else if((__ACCELERATED_MOVEMENT == this->movementType.z) | gravity.z | this->externalForce.z | this->totalNormal.z | this->friction.z)
	{
		this->acceleration.z = gravity.z + __FIX19_13_DIV(this->externalForce.z + this->totalNormal.z + this->friction.z, this->mass);
		this->velocity.z += __FIX19_13_MULT(this->acceleration.z, elapsedTime);
		this->position.z += __FIX19_13_MULT(this->velocity.z, elapsedTime) + __FIX19_13_MULT(this->acceleration.z, elapsedTimeHalfSquare);
	}
/*
	this->acceleration = (Acceleration)
	{
		__UNIFORM_MOVEMENT == this->movementType.x ? 0 : gravity.x + __FIX19_13_DIV(this->externalForce.x + this->totalNormal.x + this->friction.x, this->mass),
		__UNIFORM_MOVEMENT == this->movementType.y ? 0 : gravity.y + __FIX19_13_DIV(this->externalForce.y + this->totalNormal.y + this->friction.y, this->mass),
		__UNIFORM_MOVEMENT == this->movementType.z ? 0 : gravity.z + __FIX19_13_DIV(this->externalForce.z + this->totalNormal.z + this->friction.z, this->mass),
	};

	// update the velocity
	this->velocity.x += __FIX19_13_MULT(this->acceleration.x, elapsedTime);
	this->velocity.y += __FIX19_13_MULT(this->acceleration.y, elapsedTime);
	this->velocity.z += __FIX19_13_MULT(this->acceleration.z, elapsedTime);

	Vector3D displacement =
	{
		__FIX19_13_MULT(this->velocity.x, elapsedTime) + __FIX19_13_MULT(this->acceleration.x, elapsedTimeHalfSquare),
		__FIX19_13_MULT(this->velocity.y, elapsedTime) + __FIX19_13_MULT(this->acceleration.y, elapsedTimeHalfSquare),
		__FIX19_13_MULT(this->velocity.z, elapsedTime) + __FIX19_13_MULT(this->acceleration.z, elapsedTimeHalfSquare),
	};

	// update position

	this->position.x += displacement.x;
	this->position.y += displacement.y;
	this->position.z += displacement.z;
*/
	return Body_getMovementResult(this, previousVelocity);
}

// stop movement over an axes
static u16 Body_doStopMovement(Body this, u16 axes)
{
	ASSERT(this, "Body::stopMovement: null this");

	u16 axesOfMovement = Body_getMovementOnAllAxes(this);
	u16 axesOfStopping = __NO_AXIS;

	if(axes & __X_AXIS)
	{
		// not moving anymore
		this->velocity.x = 0;
		this->acceleration.x = 0;
		this->externalForce.x = 0;
		axesOfStopping |= axesOfMovement & __X_AXIS;
	}

	if(axes & __Y_AXIS)
	{
		// not moving anymore
		this->velocity.y = 0;
		this->acceleration.y = 0;
		this->externalForce.y = 0;
		axesOfStopping |= axesOfMovement & __Y_AXIS;
	}

	if(axes & __Z_AXIS)
	{
		// not moving anymore
		this->velocity.z = 0;
		this->acceleration.z = 0;
		this->externalForce.z = 0;
		axesOfStopping |= axesOfMovement & __Z_AXIS;
	}

	Body_setMovementType(this, __NO_MOVEMENT, axesOfStopping);

	if(!Body_getMovementOnAllAxes(this))
	{
		Body_sleep(this);
	}

	return axesOfStopping;
}

// stop movement over an axes
void Body_stopMovement(Body this, u16 axes)
{
	ASSERT(this, "Body::stopMovement: null this");

	u16 axesOfMovement = Body_getMovementOnAllAxes(this);
	u16 axesOfStopping = __NO_AXIS;

	if(axes & __X_AXIS)
	{
		// not moving anymore
		this->velocity.x = 0;
		this->acceleration.x = 0;
		this->externalForce.x = 0;
		axesOfStopping |= axesOfMovement & __X_AXIS;
	}

	if(axes & __Y_AXIS)
	{
		// not moving anymore
		this->velocity.y = 0;
		this->acceleration.y = 0;
		this->externalForce.y = 0;
		axesOfStopping |= axesOfMovement & __Y_AXIS;
	}

	if(axes & __Z_AXIS)
	{
		// not moving anymore
		this->velocity.z = 0;
		this->acceleration.z = 0;
		this->externalForce.z = 0;
		axesOfStopping |= axesOfMovement & __Z_AXIS;
	}

	Body_setMovementType(this, __NO_MOVEMENT, axesOfStopping);

//	Body_doStopMovement(this, axes);
}

// get axes subject to gravity
u16 Body_getAxesSubjectToGravity(Body this)
{
	ASSERT(this, "Body::getAxesSubjectToGravity: null this");

	return this->axesSubjectToGravity;
}

// set axes subject to gravity
void Body_setAxesSubjectToGravity(Body this, u16 axesSubjectToGravity)
{
	ASSERT(this, "Body::setAxesSubjectToGravity: null this");

	this->axesSubjectToGravity = axesSubjectToGravity;
}

// set active
void Body_setActive(Body this, bool active)
{
	ASSERT(this, "Body::setActive: null this");

	this->active = active;
}

// is active?
bool Body_isActive(Body this)
{
	ASSERT(this, "Body::isActive: null this");

	return this->active;
}

// retrieve position
const Vector3D* Body_getPosition(Body this)
{
	ASSERT(this, "Body::getPosition: null this");

	return &this->position;
}

// retrieve position
void Body_setPosition(Body this, const Vector3D* position, SpatialObject caller)
{
	ASSERT(this, "Body::setPosition: null this");

	if(this->owner == caller)
	{
		this->position = *position;
	}
}

// get elasticity
fix19_13 Body_getElasticity(Body this)
{
	ASSERT(this, "Body::getElasticity: null this");

	return this->elasticity;
}

// set elasticity
void Body_setElasticity(Body this, fix19_13 elasticity)
{
	ASSERT(this, "Body::setElasticity: null this");

	if(__I_TO_FIX19_13(0) > elasticity)
	{
		elasticity = 0;
	}
	else if(__1I_FIX19_13 < elasticity)
	{
		elasticity = __1I_FIX19_13;
	}

	this->elasticity = elasticity;
}

static void Body_computeTotalNormal(Body this)
{
	ASSERT(this, "Body::computeTotalNormal: null this");

	this->totalNormal = (Force){0, 0, 0};

	if(this->normals)
	{
		VirtualNode node = this->normals->head;

		for(; node; node = node->next)
		{
			NormalRegistry* normalRegistry = (NormalRegistry*)node->data;

			if(__IS_OBJECT_ALIVE(normalRegistry->referent))
			{
				Vector3D normal = Vector3D_scalarProduct(normalRegistry->direction, normalRegistry->magnitude);

				normal.x = __FIX19_13_INT_PART(normal.x + __0_5F_FIX19_13);
            	normal.y = __FIX19_13_INT_PART(normal.y + __0_5F_FIX19_13);
            	normal.z = __FIX19_13_INT_PART(normal.z + __0_5F_FIX19_13);

				this->totalNormal.x += normal.x;
				this->totalNormal.y += normal.y;
				this->totalNormal.z += normal.z;
			}
		}
	}
}

static void Body_addNormal(Body this, Object referent, Vector3D direction, fix19_13 magnitude)
{
	ASSERT(this, "Body::addNormal: null this");
	ASSERT(referent, "Body::addNormal: null referent");

	if(!this->normals)
	{
		this->normals = __NEW(VirtualList);
	}

	NormalRegistry* normalRegistry = __NEW_BASIC(NormalRegistry);
	normalRegistry->referent = referent;
	normalRegistry->direction = direction;

	normalRegistry->magnitude = magnitude;

	VirtualList_pushBack(this->normals, normalRegistry);

	Body_computeTotalNormal(this);
}

Force Body_getNormal(Body this)
{
	ASSERT(this, "Body::getNormal: null this");

	return this->totalNormal;
}

Force Body_getLastNormalDirection(Body this)
{
	ASSERT(this, "Body::getLastNormalDirection: null this");

	if(!this->normals)
	{
		return (Force){0, 0, 0};
	}

	return ((NormalRegistry*)VirtualList_back(this->normals))->direction;
}

static void Body_clearNormalOnAxes(Body this, u16 axes)
{
	ASSERT(this, "Body::clearNormal: null this");

	if(this->normals)
	{
		VirtualList normalsToRemove = __NEW(VirtualList);

		VirtualNode node = this->normals->head;

		for(; node; node = node->next)
		{
			NormalRegistry* normalRegistry = (NormalRegistry*)node->data;

			if(!__IS_OBJECT_ALIVE(normalRegistry->referent) ||
				((__X_AXIS & axes) && normalRegistry->direction.x) ||
				((__Y_AXIS & axes) && normalRegistry->direction.y) ||
				((__Z_AXIS & axes) && normalRegistry->direction.z)
			)
			{
				VirtualList_pushBack(normalsToRemove, normalRegistry);
			}
		}

		node = normalsToRemove->head;

		for(; node; node = node->next)
		{
			VirtualList_removeElement(this->normals, node->data);

			__DELETE_BASIC(node->data);
		}

		__DELETE(normalsToRemove);
	}

	Body_computeTotalNormal(this);
}

void Body_clearNormal(Body this, Object referent)
{
	ASSERT(this, "Body::clearNormal: null this");

	if(this->normals)
	{
		VirtualNode node = this->normals->head;

		for(; node; node = node->next)
		{
			NormalRegistry* normalRegistry = (NormalRegistry*)node->data;

			if(!__IS_OBJECT_ALIVE(normalRegistry->referent) || normalRegistry->referent == referent)
			{
				VirtualList_removeElement(this->normals, normalRegistry);

				__DELETE_BASIC(normalRegistry);

				break;
			}
		}
	}

	Body_computeTotalNormal(this);
}

fix19_13 Body_getFrictionCoefficient(Body this)
{
	ASSERT(this, "Body::setFriction: null this");

	return this->frictionCoefficient;
}

// set elasticity
void Body_setFrictionCoefficient(Body this, fix19_13 frictionCoefficient)
{
	ASSERT(this, "Body::setFriction: null this");

	if(0 > frictionCoefficient)
	{
		frictionCoefficient = 0;
	}
	else if(__I_TO_FIX19_13(1) < frictionCoefficient)
	{
		frictionCoefficient = __I_TO_FIX19_13(1);
	}

/*	if(this->frictionCoefficient)
	{
		this->frictionForceMagnitude = __ABS(__FIX19_13_DIV(this->frictionForceMagnitude, this->frictionCoefficient));
	}
*/
	this->frictionCoefficient = frictionCoefficient;
	this->frictionForceMagnitude = __ABS(__FIX19_13_MULT(this->frictionForceMagnitude, this->frictionCoefficient));
}

fix19_13 Body_getMass(Body this)
{
	ASSERT(this, "Body::getMass: null this");

	return this->mass;
}

void Body_setMass(Body this, fix19_13 mass)
{
	ASSERT(this, "Body::setMass: null this");

	this->mass = 0 < mass ? mass : __I_TO_FIX19_13(1);
}

// retrieve state
bool Body_isAwake(Body this)
{
	ASSERT(this, "Body::isAwake: null this");

	return this->awake && this->active;
}

// awake body
static void Body_awake(Body this, u16 axesOfAwakening)
{
	ASSERT(this, "Body::awake: null this");

	bool dispatchMessage = false;

	if(!this->awake)
	{
		this->awake = true;

		PhysicalWorld_bodyAwake(Game_getPhysicalWorld(Game_getInstance()), this);
	}

	if(!this->velocity.x && (__X_AXIS & axesOfAwakening))
	{
		dispatchMessage |= (__X_AXIS & axesOfAwakening);
	}

	if(!this->velocity.y && (__Y_AXIS & axesOfAwakening))
	{
		dispatchMessage |= (__Y_AXIS & axesOfAwakening);
	}

	if(!this->velocity.z && (__Z_AXIS & axesOfAwakening))
	{
		dispatchMessage |= (__Z_AXIS & axesOfAwakening);
	}

	if(dispatchMessage)
	{
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyStartedMoving, &axesOfAwakening);
	}
}

// go to sleep
void Body_sleep(Body this)
{
	ASSERT(this, "Body::sleep: null this");

	if(this->awake)
	{
		this->awake = false;

		PhysicalWorld_bodySleep(Game_getPhysicalWorld(Game_getInstance()), this);

		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodySleep, NULL);
	}
}

// is it moving?
u16 Body_getMovementOnAllAxes(Body this)
{
	ASSERT(this, "Body::isMoving: null this");

	u16 result = 0;

	result |= (this->velocity.x | this->acceleration.x) ? __X_AXIS : 0;
	result |= (this->velocity.y | this->acceleration.y) ? __Y_AXIS : 0;
	result |= (this->velocity.z | this->acceleration.z) ? __Z_AXIS : 0;

	return this->awake && this->active ? result : 0;
}

static MovementResult Body_getBouncingResult(Body this, Vector3D previousVelocity, Vector3D bouncingPlaneNormal)
{
	ASSERT(this, "Body::checkIfStopped: null this");

	MovementResult movementResult = {__NO_AXIS, __NO_AXIS, __NO_AXIS};

	movementResult.axesOfChangeOfMovement |= this->velocity.x ^ previousVelocity.x ? __X_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= this->velocity.y ^ previousVelocity.y ? __Y_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= this->velocity.z ^ previousVelocity.z ? __Z_AXIS : __NO_AXIS;

	// stop if minimum velocity threshold is not reached
	// and if there is possible movement in the other components
	if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.x) && !__FIX19_13_INT_PART(bouncingPlaneNormal.y | bouncingPlaneNormal.z))
	{
		movementResult.axesStoppedMovement |= __X_AXIS;
	}

	if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.y) && !__FIX19_13_INT_PART(bouncingPlaneNormal.x | bouncingPlaneNormal.z))
	{
		movementResult.axesStoppedMovement |= __Y_AXIS;
	}

	if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.z) && !__FIX19_13_INT_PART(bouncingPlaneNormal.x | bouncingPlaneNormal.y))
	{
		movementResult.axesStoppedMovement |= __Z_AXIS;
	}

	// bounce accelerated if movement changed direction and the previous movement was not uniform
	if(__UNIFORM_MOVEMENT != this->movementType.x)
	{
		movementResult.axesOfAcceleratedBouncing |= __X_AXIS & movementResult.axesOfChangeOfMovement;
	}

	if(__UNIFORM_MOVEMENT != this->movementType.y)
	{
		movementResult.axesOfAcceleratedBouncing |= __Y_AXIS & movementResult.axesOfChangeOfMovement;
	}

	if(__UNIFORM_MOVEMENT != this->movementType.z)
	{
		movementResult.axesOfAcceleratedBouncing |= __Z_AXIS & movementResult.axesOfChangeOfMovement;
	}

	// don't bounce if movement stopped on that axes
	movementResult.axesOfAcceleratedBouncing &= ~movementResult.axesStoppedMovement;

	return movementResult;
}

// bounce back
void Body_bounce(Body this, Object bounceReferent, Vector3D bouncingPlaneNormal, fix19_13 frictionCoefficient, fix19_13 elasticity)
{
	ASSERT(this, "Body::bounce: null this");
	Acceleration gravity = Body_getGravity(this);

	// set friction and elasticity
	Body_setElasticity(this, elasticity);
	Body_setFrictionCoefficient(this, frictionCoefficient);

	// compute bouncing vector
	fix19_13 cosAngle = (bouncingPlaneNormal.x | bouncingPlaneNormal.y | bouncingPlaneNormal.z) && (gravity.x | gravity.y | gravity.z) ? __ABS(__FIX19_13_DIV(Vector3D_dotProduct(gravity, bouncingPlaneNormal), Vector3D_lengthProduct(gravity, bouncingPlaneNormal))) : __1I_FIX19_13;
	fix19_13 normalMagnitude = __FIX19_13_MULT(Vector3D_length(this->weight), cosAngle);

	// register normal affecting the body
	Body_addNormal(this, bounceReferent, bouncingPlaneNormal, normalMagnitude);

	this->frictionForceMagnitude = __FIX19_13_MULT(normalMagnitude, this->frictionCoefficient);

	// avoid rounding errors

	// compute bouncing vector
	Vector3D velocity = this->velocity;

	// compute bouncing velocity vector
	Vector3D u = Vector3D_scalarProduct(bouncingPlaneNormal, Vector3D_dotProduct(velocity, bouncingPlaneNormal));
	Vector3D w =
	{
		velocity.x - u.x,
		velocity.y - u.y,
		velocity.z - u.z,
	};

	// add elasticity and friction
	u = Vector3D_scalarProduct(u, this->elasticity);
	w = Vector3D_scalarProduct(w, (__I_TO_FIX19_13(1) - this->frictionCoefficient));

	this->velocity.x = w.x - u.x;
	this->velocity.y = w.y - u.y;
	this->velocity.z = w.z - u.z;

	// determine bouncing result
	MovementResult movementResult = Body_getBouncingResult(this, velocity, bouncingPlaneNormal);

	// bounce over the computed axes
	if(movementResult.axesOfAcceleratedBouncing)
	{
		Body_moveAccelerated(this, movementResult.axesOfAcceleratedBouncing);

		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyBounced, &movementResult.axesOfAcceleratedBouncing);
	}

	// stop over the axes where there is no bouncing
	if(movementResult.axesStoppedMovement)
	{
		u16 axesOfStopping = Body_doStopMovement(this, movementResult.axesStoppedMovement);

		if(axesOfStopping)
		{
			MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyStopped, &axesOfStopping);
		}
	}
}

// take a hit
void Body_takeHitFrom(Body this __attribute__ ((unused)), Body other __attribute__ ((unused)))
{
	ASSERT(this, "Body::takeHitFrom: null this");

	//TODO:
}

void Body_print(Body this, int x, int y)
{
	ASSERT(this, "Body::print: null this");

	Printing_text(Printing_getInstance(), "Active:", x, y, NULL);
	Printing_text(Printing_getInstance(), this->active? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 10, y++, NULL);
	Printing_text(Printing_getInstance(), "Awake:", x, y, NULL);
	Printing_text(Printing_getInstance(), this->awake? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 10, y++, NULL);

	Printing_text(Printing_getInstance(), "               X         Y         Z", x, y++, NULL);

	int xDisplacement = 15;

	Printing_text(Printing_getInstance(), "Mov. type", x, y, NULL);
	Printing_text(Printing_getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing_text(Printing_getInstance(), __UNIFORM_MOVEMENT == this->movementType.x ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.x ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.x ? "Accel" : "None", xDisplacement + x, y, NULL);
	Printing_text(Printing_getInstance(), __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.y ? "Accel" : "None", xDisplacement + x + 10, y, NULL);
	Printing_text(Printing_getInstance(), __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.z ? "Accel" : "None", xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Weight", x, y, NULL);
	Printing_text(Printing_getInstance(), "                             ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->weight.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->weight.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->weight.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Position", x, y, NULL);
	Printing_text(Printing_getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->position.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->position.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->position.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Velocity", x, y, NULL);
	Printing_text(Printing_getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->velocity.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->velocity.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->velocity.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Acceleration", x, y, NULL);
	Printing_text(Printing_getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->acceleration.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->acceleration.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->acceleration.z), xDisplacement + x + 10 * 2, y++, NULL);

	Acceleration gravity = Body_getGravity(this);

	Printing_text(Printing_getInstance(), "Gravity", x, y, NULL);
	Printing_text(Printing_getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(gravity.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(gravity.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(gravity.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "External Force", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->externalForce.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->externalForce.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->externalForce.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Friction", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->friction.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->friction.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->friction.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Normal", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->totalNormal.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->totalNormal.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->totalNormal.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Normal Force", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->frictionForceMagnitude), xDisplacement + x, y++, NULL);

	Printing_text(Printing_getInstance(), "Normals", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_int(Printing_getInstance(), this->normals ? VirtualList_getSize(this->normals) : 0, xDisplacement + x, y++, NULL);
}
