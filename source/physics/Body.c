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

//#define __STOP_VELOCITY_THRESHOLD				__F_TO_FIX10_6(0.9f)
#define __STOP_VELOCITY_THRESHOLD				__PIXELS_TO_METERS(8)
#define __STOP_BOUNCING_VELOCITY_THRESHOLD 		__PIXELS_TO_METERS(16)

#define __MIN_MASS								__F_TO_FIX10_6(0.1f)
#define __MAX_MASS								__I_TO_FIX10_6(1)
#define __MAXIMUM_FRICTION_COEFFICIENT			__I_TO_FIX10_6(1)

#define __FRICTION_FORCE_FACTOR_POWER					2

//---------------------------------------------------------------------------------------------------------
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

fix10_6 _currentWorldFriction = 0;
fix10_6 _currentElapsedTime = 0;
const Acceleration* _currentGravity = 0;

void Body_setCurrentWorldFrictionCoefficient(fix10_6 currentWorldFriction)
{
	_currentWorldFriction = currentWorldFriction;
}

void Body_setCurrentElapsedTime(fix10_6 currentElapsedTime)
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

typedef struct NormalRegistry
{
	Object referent;
	Vector3D direction;
	fix10_6 magnitude;

} NormalRegistry;


Clock _physhicsClock = NULL;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

MovementResult Body_updateMovement(Body this);
static void Body_awake(Body this, u16 axesOfAwakening);
static void Body_setMovementType(Body this, int movementType, u16 axes);
static void Body_clearNormalOnAxes(Body this, u16 axes);
Acceleration Body_getGravity(Body this);
static void Body_computeTotalNormal(Body this);
static void Body_computeTotalFrictionCoefficient(Body this);
void Body_sleep(Body body);


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
	this->mass = __MIN_MASS < physicalSpecification->mass ? __MAX_MASS > physicalSpecification->mass ? physicalSpecification->mass : __MAX_MASS : __MIN_MASS;
	this->elasticity = physicalSpecification->elasticity;
	this->frictionCoefficient = physicalSpecification->frictionCoefficient;
	this->surroundingFrictionCoefficient = 0;
	this->totalFrictionCoefficient = 0;
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
		this->externalForce.x += force->x;
		this->externalForce.y += force->y;
		this->externalForce.z += force->z;

		u16 axesOfExternalForce = __NO_AXIS;

		if(force->x)
		{
			axesOfExternalForce |= __X_AXIS;
		}

		if(force->y)
		{
			axesOfExternalForce |= __Y_AXIS;
		}

		if(force->z)
		{
			axesOfExternalForce |= __Z_AXIS;
		}

		if(axesOfExternalForce)
		{
			//Body_clearNormalOnAxes(this, axesOfExternalForce);
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
		Acceleration gravityForce = Vector3D_scalarProduct(Body_getGravity(this), this->mass);

		Force force =
		{
			__X_AXIS & axes ? gravityForce.x : 0,
			__Y_AXIS & axes ? gravityForce.y : 0,
			__Z_AXIS & axes ? gravityForce.z : 0,
		};

		Body_applyForce(this, &force);
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
				Body_stopMovement(this, movementResult.axesStoppedMovement);

				if(movementResult.axesStoppedMovement)
				{
					MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyStopped, &movementResult.axesStoppedMovement);
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

	fix10_6 elapsedTime = PhysicalWorld_getElapsedTime(Game_getPhysicalWorld(Game_getInstance()));

	displacement.x = __FIX10_6_MULT(this->velocity.x, elapsedTime);
	displacement.y = __FIX10_6_MULT(this->velocity.y, elapsedTime);
	displacement.z = __FIX10_6_MULT(this->velocity.z, elapsedTime);

	return displacement;
}

static MovementResult Body_getMovementResult(Body this, Vector3D previousVelocity)
{
	ASSERT(this, "Body::checkIfStopped: null this");

	MovementResult movementResult = {__NO_AXIS, __NO_AXIS, __NO_AXIS, __NO_AXIS};

	Vector3D aux =
	{
		this->velocity.x ^ previousVelocity.x,
		this->velocity.y ^ previousVelocity.y,
		this->velocity.z ^ previousVelocity.z,
	};

	// xor values, if result != 0, there is movement
	movementResult.axesOfChangeOfMovement |= aux.x ? __X_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= aux.y ? __Y_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= aux.z ? __Z_AXIS : __NO_AXIS;

	// xor values, if result >= 0, there is no change in direction
	movementResult.axesOfChangeOfDirection |= 0 <= aux.x ? __NO_AXIS : __X_AXIS;
	movementResult.axesOfChangeOfDirection |= 0 <= aux.y ? __NO_AXIS : __Y_AXIS;
	movementResult.axesOfChangeOfDirection |= 0 <= aux.z ? __NO_AXIS : __Z_AXIS;

	// stop if no external force or opposing normal force is present
	// and if the velocity minimum threshold is not reached
	if(previousVelocity.x && !this->externalForce.x && __UNIFORM_MOVEMENT != this->movementType.x && (this->velocity.x | previousVelocity.x))
	{
		if(__X_AXIS & movementResult.axesOfChangeOfDirection)
		{
			movementResult.axesStoppedMovement |= __X_AXIS;
		}
		else if((__X_AXIS & movementResult.axesOfChangeOfMovement) && (0 <= this->totalNormal.x * this->velocity.x))
		{
			if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.x))
			{
				movementResult.axesStoppedMovement |= __X_AXIS;
			}
		}
	}

	if(previousVelocity.y && !this->externalForce.y && __UNIFORM_MOVEMENT != this->movementType.y && (this->velocity.y | previousVelocity.y))
	{
		if(__Y_AXIS & movementResult.axesOfChangeOfDirection)
		{
			movementResult.axesStoppedMovement |= __Y_AXIS;
		}
		else if((__Y_AXIS & movementResult.axesOfChangeOfMovement) && (0 <= this->totalNormal.y * this->velocity.y))
		{
			if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.y))
			{
				movementResult.axesStoppedMovement |= __Y_AXIS;
			}
		}
	}

	if(previousVelocity.z && !this->externalForce.z && __UNIFORM_MOVEMENT != this->movementType.z && (this->velocity.z | previousVelocity.z))
	{
		if(__Z_AXIS & movementResult.axesOfChangeOfDirection)
		{
			movementResult.axesStoppedMovement |= __Z_AXIS;
		}
		else if((__Z_AXIS & movementResult.axesOfChangeOfMovement) && (0 <= this->totalNormal.z * this->velocity.z))
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

// update movement over axes
MovementResult Body_updateMovement(Body this)
{
	ASSERT(this, "Body::updateMovement: null this");

	Acceleration gravity = Body_getGravity(this);
	this->weight = Vector3D_scalarProduct(gravity, this->mass);

	// yeah, * 4 (<< 2) is a magical number, but it works well enough with the range of mass and friction coefficient
	this->friction = Vector3D_scalarProduct(Vector3D_normalize(this->velocity), -(this->frictionForceMagnitude << __FRICTION_FORCE_FACTOR_POWER));

	// hack to avoid normalization
//	this->friction = Vector3D_scalarProduct(this->velocity, -(this->totalFrictionCoefficient << 2));

	fix10_6 elapsedTime = _currentElapsedTime;
	fix10_6 elapsedTimeHalfSquare = __FIX10_6_MULT(elapsedTime, elapsedTime) >> 1;

	Velocity previousVelocity = this->velocity;

	if(__UNIFORM_MOVEMENT == this->movementType.x)
	{
		this->position.x += __FIX10_6_MULT(this->velocity.x, elapsedTime);
	}
	else if((__ACCELERATED_MOVEMENT == this->movementType.x) | gravity.x | this->externalForce.x | this->totalNormal.x | this->friction.x)
	{
		// need to use extended types to prevent overflows
		fix10_6_ext acceleration = gravity.x + __FIX10_6_EXT_DIV(this->externalForce.x + this->totalNormal.x + this->friction.x, this->mass);
		fix10_6_ext velocityDelta = __FIX10_6_EXT_MULT(acceleration, elapsedTime);

		this->acceleration.x = __FIX10_6_EXT_TO_FIX10_6(acceleration);
		this->velocity.x += __FIX10_6_EXT_TO_FIX10_6(velocityDelta);
		this->position.x += __FIX10_6_MULT(this->velocity.x, elapsedTime) + __FIX10_6_MULT(this->acceleration.x, elapsedTimeHalfSquare);
	}

	if(__UNIFORM_MOVEMENT == this->movementType.y)
	{
		this->position.y += __FIX10_6_MULT(this->velocity.y, elapsedTime);
	}
	else if((__ACCELERATED_MOVEMENT == this->movementType.y) | gravity.y | this->externalForce.y | this->totalNormal.y | this->friction.y)
	{
		fix10_6_ext acceleration = gravity.y + __FIX10_6_EXT_DIV(this->externalForce.y + this->totalNormal.y + this->friction.y, this->mass);
		fix10_6_ext velocityDelta = __FIX10_6_EXT_MULT(acceleration, elapsedTime);

		this->acceleration.y = __FIX10_6_EXT_TO_FIX10_6(acceleration);
		this->velocity.y += __FIX10_6_EXT_TO_FIX10_6(velocityDelta);
		this->position.y += __FIX10_6_MULT(this->velocity.y, elapsedTime) + __FIX10_6_MULT(this->acceleration.y, elapsedTimeHalfSquare);
	}

	if(__UNIFORM_MOVEMENT == this->movementType.z)
	{
		this->position.z += __FIX10_6_MULT(this->velocity.z, elapsedTime);
	}
	else if((__ACCELERATED_MOVEMENT == this->movementType.z) | gravity.z | this->externalForce.z | this->totalNormal.z | this->friction.z)
	{
		fix10_6_ext acceleration = gravity.z + __FIX10_6_EXT_DIV(this->externalForce.z + this->totalNormal.z + this->friction.z, this->mass);
		fix10_6_ext velocityDelta = __FIX10_6_EXT_MULT(acceleration, elapsedTime);

		this->acceleration.z = __FIX10_6_EXT_TO_FIX10_6(acceleration);
		this->velocity.z += __FIX10_6_EXT_TO_FIX10_6(velocityDelta);
		this->position.z += __FIX10_6_MULT(this->velocity.z, elapsedTime) + __FIX10_6_MULT(this->acceleration.z, elapsedTimeHalfSquare);
	}

	return Body_getMovementResult(this, previousVelocity);
}

// stop movement over an axes
u16 Body_stopMovement(Body this, u16 axes)
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
fix10_6 Body_getElasticity(Body this)
{
	ASSERT(this, "Body::getElasticity: null this");

	return this->elasticity;
}

// set elasticity
void Body_setElasticity(Body this, fix10_6 elasticity)
{
	ASSERT(this, "Body::setElasticity: null this");

	if(__I_TO_FIX10_6(0) > elasticity)
	{
		elasticity = 0;
	}
	else if(__1I_FIX10_6 < elasticity)
	{
		elasticity = __1I_FIX10_6;
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

				this->totalNormal.x += normal.x;
				this->totalNormal.y += normal.y;
				this->totalNormal.z += normal.z;
			}
		}
	}

	Body_computeTotalFrictionCoefficient(this);

	if(this->totalNormal.x | this->totalNormal.y | this->totalNormal.z)
	{
		this->frictionForceMagnitude = __FIX10_6_MULT(Vector3D_length(this->totalNormal), this->totalFrictionCoefficient);
	}
	else
	{
		this->frictionForceMagnitude = __FIX10_6_MULT(Vector3D_length(this->weight), _currentWorldFriction) >> __FRICTION_FORCE_FACTOR_POWER;
	}
}

static void Body_addNormal(Body this, Object referent, Vector3D direction, fix10_6 magnitude)
{
	ASSERT(this, "Body::addNormal: null this");
	ASSERT(referent, "Body::addNormal: null referent");

	if(!this->normals)
	{
		this->normals = __NEW(VirtualList);
	}

	VirtualNode node = this->normals->head;

	for(; node; node = node->next)
	{
		ASSERT(__IS_BASIC_OBJECT_ALIVE(node->data), "DEAD");
		ASSERT(((NormalRegistry*)node->data)->referent != referent, "ERRR");
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

	if(!this->normals || !this->normals->head)
	{
		return (Force){0, 0, 0};
	}

	while(this->normals->head && !__IS_OBJECT_ALIVE(((NormalRegistry*)VirtualList_back(this->normals))->referent))
	{
		ASSERT(__IS_BASIC_OBJECT_ALIVE(VirtualList_back(this->normals)), "Body::getLastNormalDirection: dead normal registry");

		__DELETE_BASIC(VirtualList_popBack(this->normals));
	}

	Body_computeTotalNormal(this);

	if(!this->normals->head)
	{
		return (Force){0, 0, 0};
	}

	return ((NormalRegistry*)VirtualList_back(this->normals))->direction;
}

void Body_reset(Body this)
{
	ASSERT(this, "Body::reset: null this");

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

	Body_computeTotalNormal(this);
	Body_setSurroundingFrictionCoefficient(this, 0);

	this->velocity 				= (Velocity){0, 0, 0};
	this->acceleration 			= (Acceleration){0, 0, 0};
	this->externalForce	 		= (Force){0, 0, 0};
	this->friction 				= (Force){0, 0, 0};
	this->totalNormal			= (Force){0, 0, 0};
	this->weight 				= Vector3D_scalarProduct(*_currentGravity, this->mass);
}

static void Body_clearNormalOnAxes(Body this, u16 axes)
{
	ASSERT(this, "Body::clearNormal: null this");
/*
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

			if(__IS_BASIC_OBJECT_ALIVE(node->data))
			{
			__DELETE_BASIC(node->data);
			}
			else
			{
				Printing_text(Printing_getInstance(), __GET_CLASS_NAME(this->owner), 20, 1, NULL);
				while(1);
			}

		}

		__DELETE(normalsToRemove);
	}
*/
	Body_computeTotalNormal(this);
}

void Body_clearNormal(Body this, Object referent)
{
	ASSERT(this, "Body::clearNormal: null this");
	ASSERT(__IS_OBJECT_ALIVE(referent), "Body::clearNormal: dead referent");

	if(this->normals)
	{
		VirtualNode node = this->normals->head;

		for(; node; node = node->next)
		{
			NormalRegistry* normalRegistry = (NormalRegistry*)node->data;

			// search if registry exists for the referent
			// it is Ok if it doesn't exist
			if(normalRegistry->referent == referent)
			{
				ASSERT(__IS_BASIC_OBJECT_ALIVE(normalRegistry), "Body::clearNormal: dead normal registry");
				VirtualList_removeElement(this->normals, normalRegistry);
				__DELETE_BASIC(normalRegistry);
				break;
			}
		}
	}

	Body_computeTotalNormal(this);
}

fix10_6 Body_getFrictionCoefficient(Body this)
{
	ASSERT(this, "Body::setFriction: null this");

	return this->frictionCoefficient;
}

static void Body_computeTotalFrictionCoefficient(Body this)
{
	ASSERT(this, "Body::computeFrictionForceMagnitude: null this");

	this->totalFrictionCoefficient = this->frictionCoefficient;

	this->totalFrictionCoefficient += _currentWorldFriction + this->surroundingFrictionCoefficient;

	if(0 > this->totalFrictionCoefficient)
	{
		this->totalFrictionCoefficient = 0;
	}
	else if(__MAXIMUM_FRICTION_COEFFICIENT < this->totalFrictionCoefficient)
	{
		this->totalFrictionCoefficient = __MAXIMUM_FRICTION_COEFFICIENT;
	}
}

// set friction
void Body_setFrictionCoefficient(Body this, fix10_6 frictionCoefficient)
{
	ASSERT(this, "Body::setFriction: null this");

	if(0 > frictionCoefficient)
	{
		frictionCoefficient = 0;
	}
	else if(__MAXIMUM_FRICTION_COEFFICIENT < frictionCoefficient)
	{
		frictionCoefficient = __MAXIMUM_FRICTION_COEFFICIENT;
	}

	this->frictionCoefficient = frictionCoefficient;
	Body_computeTotalFrictionCoefficient(this);
}

void Body_setSurroundingFrictionCoefficient(Body this, fix10_6 surroundingFrictionCoefficient)
{
	ASSERT(this, "Body::setSurroundingFrictionCoefficient: null this");

	if(0 > surroundingFrictionCoefficient)
	{
		surroundingFrictionCoefficient = 0;
	}
	else if(__MAXIMUM_FRICTION_COEFFICIENT < surroundingFrictionCoefficient)
	{
		surroundingFrictionCoefficient = __MAXIMUM_FRICTION_COEFFICIENT;
	}

	this->surroundingFrictionCoefficient = surroundingFrictionCoefficient;
	Body_computeTotalFrictionCoefficient(this);
}

fix10_6 Body_getMass(Body this)
{
	ASSERT(this, "Body::getMass: null this");

	return this->mass;
}

void Body_setMass(Body this, fix10_6 mass)
{
	ASSERT(this, "Body::setMass: null this");

	this->mass = __MIN_MASS < mass ? __MAX_MASS > mass ? mass : __MAX_MASS : __MIN_MASS;
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
	}
}

// is it moving?
u16 Body_getMovementOnAllAxes(Body this)
{
	ASSERT(this, "Body::isMoving: null this");

	u16 result = 0;

	result |= (this->velocity.x | __FIX10_6_INT_PART(this->acceleration.x)) ? __X_AXIS : 0;
	result |= (this->velocity.y | __FIX10_6_INT_PART(this->acceleration.y)) ? __Y_AXIS : 0;
	result |= (this->velocity.z | __FIX10_6_INT_PART(this->acceleration.z)) ? __Z_AXIS : 0;

	return this->awake && this->active ? result : 0;
}

static MovementResult Body_getBouncingResult(Body this, Vector3D previousVelocity, Vector3D bouncingPlaneNormal)
{
	ASSERT(this, "Body::checkIfStopped: null this");

	MovementResult movementResult = {__NO_AXIS, __NO_AXIS, __NO_AXIS, __NO_AXIS};

	movementResult.axesOfChangeOfMovement |= this->velocity.x ^ previousVelocity.x ? __X_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= this->velocity.y ^ previousVelocity.y ? __Y_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= this->velocity.z ^ previousVelocity.z ? __Z_AXIS : __NO_AXIS;

	// stop if minimum velocity threshold is not reached
	// and if there is possible movement in the other components
	if(__STOP_BOUNCING_VELOCITY_THRESHOLD > __ABS(this->velocity.x) && !__FIX10_6_INT_PART(bouncingPlaneNormal.y | bouncingPlaneNormal.z))
	{
		movementResult.axesStoppedMovement |= __X_AXIS;
	}

	if(__STOP_BOUNCING_VELOCITY_THRESHOLD > __ABS(this->velocity.y) && !__FIX10_6_INT_PART(bouncingPlaneNormal.x | bouncingPlaneNormal.z))
	{
		movementResult.axesStoppedMovement |= __Y_AXIS;
	}

	if(__STOP_BOUNCING_VELOCITY_THRESHOLD > __ABS(this->velocity.z) && !__FIX10_6_INT_PART(bouncingPlaneNormal.x | bouncingPlaneNormal.y))
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
void Body_bounce(Body this, Object bounceReferent, Vector3D bouncingPlaneNormal, fix10_6 frictionCoefficient, fix10_6 elasticity)
{
	ASSERT(this, "Body::bounce: null this");
	Acceleration gravity = Body_getGravity(this);

	// set friction
	Body_setSurroundingFrictionCoefficient(this, frictionCoefficient);

	// compute bouncing vector
	fix10_6 cosAngle = __I_TO_FIX10_6(bouncingPlaneNormal.x | bouncingPlaneNormal.y | bouncingPlaneNormal.z) && (gravity.x | gravity.y | gravity.z) ? __ABS(__FIX10_6_DIV(Vector3D_dotProduct(gravity, bouncingPlaneNormal), Vector3D_lengthProduct(gravity, bouncingPlaneNormal))) : __1I_FIX10_6;
	fix10_6 normalMagnitude = __FIX10_6_MULT(Vector3D_length(this->weight), cosAngle);

	// register normal affecting the body
	Body_addNormal(this, bounceReferent, bouncingPlaneNormal, normalMagnitude);

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

	elasticity += this->elasticity;

	if(__I_TO_FIX10_6(0) > elasticity)
	{
		elasticity = 0;
	}
	else if(__1I_FIX10_6 < elasticity)
	{
		elasticity = __1I_FIX10_6;
	}

	// add elasticity and friction
	u = Vector3D_scalarProduct(u, elasticity);
	w = Vector3D_scalarProduct(w, (__MAXIMUM_FRICTION_COEFFICIENT - this->totalFrictionCoefficient));

	this->velocity.x = __UNIFORM_MOVEMENT != this->movementType.x ? w.x - u.x : this->velocity.x;
	this->velocity.y = __UNIFORM_MOVEMENT != this->movementType.y ? w.y - u.y : this->velocity.y;
	this->velocity.z = __UNIFORM_MOVEMENT != this->movementType.z ? w.z - u.z : this->velocity.z;

	// determine bouncing result
	MovementResult movementResult = Body_getBouncingResult(this, velocity, bouncingPlaneNormal);

	// stop over the axes where there is no bouncing
	if(movementResult.axesStoppedMovement)
	{
		u16 axesOfStopping = Body_stopMovement(this, movementResult.axesStoppedMovement);

		if(axesOfStopping)
		{
			MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyStopped, &axesOfStopping);
		}
	}

	if(movementResult.axesOfAcceleratedBouncing)
	{
	//	Body_setSurroundingFrictionCoefficient(this, 0);
	//	Body_clearNormalOnAxes(this, movementResult.axesOfAcceleratedBouncing);
	}

	if(!Body_getMovementOnAllAxes(__SAFE_CAST(Body, this)))
	{
		Body_sleep(__SAFE_CAST(Body, this));
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
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->weight.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->weight.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->weight.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Position", x, y, NULL);
	Printing_text(Printing_getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->position.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->position.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->position.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Velocity", x, y, NULL);
	Printing_text(Printing_getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->velocity.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->velocity.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->velocity.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Acceleration", x, y, NULL);
	Printing_text(Printing_getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->acceleration.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->acceleration.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->acceleration.z), xDisplacement + x + 10 * 2, y++, NULL);

	Acceleration gravity = Body_getGravity(this);

	Printing_text(Printing_getInstance(), "Gravity", x, y, NULL);
	Printing_text(Printing_getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(gravity.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(gravity.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(gravity.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "External Force", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->externalForce.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->externalForce.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->externalForce.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Normal", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->totalNormal.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->totalNormal.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->totalNormal.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Normal Force", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->frictionForceMagnitude), xDisplacement + x, y++, NULL);

	Printing_text(Printing_getInstance(), "Normals", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_int(Printing_getInstance(), this->normals ? VirtualList_getSize(this->normals) : 0, xDisplacement + x, y++, NULL);

	Printing_text(Printing_getInstance(), "Friction", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->friction.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->friction.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->friction.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Total friction coef.", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(this->totalFrictionCoefficient), xDisplacement + x, y++, NULL);


	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(__STOP_VELOCITY_THRESHOLD), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX10_6_TO_F(__STOP_BOUNCING_VELOCITY_THRESHOLD), 10 +xDisplacement + x, y, NULL);

}
