/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <DebugConfig.h>
#include <FrameRate.h>
#include <MessageDispatcher.h>
#include <PhysicalWorld.h>
#include <Printing.h>
#include <SpatialObject.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "Body.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualList;
friend class VirtualNode;


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#ifndef __MAXIMUM_BOUNCINESS_COEFFICIENT
	#define __MAXIMUM_BOUNCINESS_COEFFICIENT	1
#endif

// this should be improved and calculated dynamically based on framerate
#define STOPPED_MOVING		0
#define STILL_MOVES			1
#define CHANGED_DIRECTION	2

//#define __STOP_VELOCITY_THRESHOLD				__F_TO_FIXED(0.9f)
#ifndef __STOP_VELOCITY_THRESHOLD
#define __STOP_VELOCITY_THRESHOLD				__PIXELS_TO_METERS(1)
#endif
#ifndef __STOP_BOUNCING_VELOCITY_THRESHOLD
#define __STOP_BOUNCING_VELOCITY_THRESHOLD 		__PIXELS_TO_METERS(48)
#endif

#ifndef __FRICTION_FORCE_FACTOR_POWER
#define __FRICTION_FORCE_FACTOR_POWER					2
#endif


//---------------------------------------------------------------------------------------------------------
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

fix7_9_ext _currentPhysicsElapsedTime __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = 0;
static fixed_t _currentWorldFriction __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = 0;
static const Vector3D* _currentGravity __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = 0;

static void Body::setCurrentWorldFrictionCoefficient(fixed_t currentWorldFriction)
{
	_currentWorldFriction = currentWorldFriction;
}

static void Body::setCurrentElapsedTime(fix7_9_ext currentElapsedTime)
{
	_currentPhysicsElapsedTime = currentElapsedTime;
}

static fix7_9_ext Body::getCurrentElapsedTime()
{
	return _currentPhysicsElapsedTime;
}

static void Body::setCurrentGravity(const Vector3D* currentGravity)
{
	_currentGravity = currentGravity;
}

static const Vector3D* Body::getCurrentGravity()
{
	return _currentGravity;
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
	ListenerObject referent;
	Vector3D direction;
	fixed_t magnitude;

} NormalRegistry;



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Body::constructor(SpatialObject owner, const PhysicalProperties* physicalProperties, uint16 axisSubjectToGravity)
{
	Base::constructor();

	this->destroy = false;
	this->owner = owner;
	this->normals = NULL;
	this->mass = __BODY_MIN_MASS < physicalProperties->mass ? __BODY_MAX_MASS > physicalProperties->mass ? physicalProperties->mass : __BODY_MAX_MASS : __BODY_MIN_MASS;

	this->bounciness = physicalProperties->bounciness;
	this->frictionCoefficient = 0;
	this->surroundingFrictionCoefficient = 0;
	this->totalFrictionCoefficient = 0;
	this->frictionForceMagnitude = 0;

	this->active = true;
	this->awake = false;
	this->sendMessages = true;
	this->axisSubjectToGravity = axisSubjectToGravity;

	// clear movement type
	this->movementType.x = __NO_MOVEMENT;
	this->movementType.y = __NO_MOVEMENT;
	this->movementType.z = __NO_MOVEMENT;

	this->position 				= Vector3D::zero();
	this->velocity 				= Vector3D::zero();
	this->direction 			= Vector3D::zero();
	this->accelerating 			= (Vector3DFlag){false, false, false};
	this->externalForce	 		= Vector3D::zero();
	this->friction 				= Vector3D::zero();
	this->totalNormal			= Vector3D::zero();
	this->maximumVelocity 		= physicalProperties->maximumVelocity;
	this->maximumSpeed 			= physicalProperties->maximumSpeed;
	this->speed 				= 0;
	this->clearExternalForce 	= __NO_AXIS;
	this->skipCycles 			= 0;
	this->skipedCycles 			= 0;

	this->internalPosition.x 	= 0;
	this->internalPosition.y 	= 0;
	this->internalPosition.z 	= 0;
	this->internalVelocity.x 	= 0;
	this->internalVelocity.y 	= 0;
	this->internalVelocity.z 	= 0;
	
	this->movesIndependentlyOnEachAxis = 0 != this->maximumVelocity.x || 0 != this->maximumVelocity.y || 0 != this->maximumVelocity.z;

	this->gravity = Body::getGravity(this);

	Body::setFrictionCoefficient(this, physicalProperties->frictionCoefficient);
	Body::computeFrictionForceMagnitude(this);
}

// class's destructor
void Body::destructor()
{
	if(this->normals)
	{
		VirtualList::deleteData(this->normals);
		delete this->normals;
		this->normals = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

// set game entity
void Body::setOwner(SpatialObject owner)
{
	this->owner = owner;
}

// get game entity
SpatialObject Body::getOwner()
{
	return this->owner;
}

// retrieve velocity
const Vector3D* Body::getVelocity()
{
	return &this->velocity;
}

void Body::setVelocity(Vector3D* velocity)
{
	if(velocity)
	{
		this->velocity = *velocity;
	}
}

const Vector3D* Body::getDirection()
{
	return &this->direction;
}

void Body::setDirection3D(const Vector3D* direction)
{
	if(NULL == direction)
	{
		return;
	}

	this->direction = *direction;
	this->velocity = Vector3D::scalarProduct(this->direction, this->speed);

	this->internalVelocity.x = __FIXED_TO_FIX7_9_EXT(this->velocity.x);
	this->internalVelocity.y = __FIXED_TO_FIX7_9_EXT(this->velocity.y);
	this->internalVelocity.z = __FIXED_TO_FIX7_9_EXT(this->velocity.z);
}

fixed_t Body::getSpeed()
{
	return this->speed;
}

void Body::modifyVelocity(const Vector3D* modifier)
{
	ASSERT(modifier, "Body::modifyVelocity: null multiplier");

	this->velocity.x += modifier->x;
	this->velocity.y += modifier->y;
	this->velocity.z += modifier->z;

	Body::clampVelocity(this, false);
}

// retrieve acceleration
Vector3DFlag Body::getAccelerationState()
{
	return this->accelerating;
}

// retrieve applied force
Vector3D Body::getAppliedForce()
{
	return this->externalForce;
}

// retrieve movement type
MovementType Body::getMovementType()
{
	return this->movementType;
}

// set movement type
void Body::setMovementType(int32 movementType, uint16 axis)
{
	if(__X_AXIS & axis)
	{
		this->movementType.x = movementType;
	}

	if(__Y_AXIS & axis)
	{
		this->movementType.y = movementType;
	}

	if(__Z_AXIS & axis)
	{
		this->movementType.z = movementType;
	}
}

void Body::clearAcceleration(uint16 axis)
{
	if(__X_AXIS & axis)
	{
		this->accelerating.x = false;
		this->externalForce.x = 0;
	}

	if(__Y_AXIS & axis)
	{
		this->accelerating.y = false;
		this->externalForce.y = 0;
	}

	if(__Z_AXIS & axis)
	{
		this->accelerating.z = false;
		this->externalForce.z = 0;
	}
}

// set movement type to accelerated
void Body::moveAccelerated(uint16 axis)
{
	Body::setMovementType(this, __ACCELERATED_MOVEMENT, axis);
}

// set movement type to uniform
void Body::moveUniformly(Vector3D velocity)
{
	uint16 axisOfUniformMovement = 0;

	if(0 != velocity.x)
	{
		axisOfUniformMovement |= __X_AXIS;
	}

	if(0 != velocity.y)
	{
		axisOfUniformMovement |= __Y_AXIS;
	}

	if(0 != velocity.z)
	{
		axisOfUniformMovement |= __Z_AXIS;
	}

	if(__NO_AXIS != axisOfUniformMovement)
	{
		Body::setMovementType(this, __UNIFORM_MOVEMENT, axisOfUniformMovement);
		Body::awake(this, axisOfUniformMovement);

		this->velocity = velocity;
		this->internalVelocity.x = __FIXED_TO_FIX7_9_EXT(velocity.x);
		this->internalVelocity.y = __FIXED_TO_FIX7_9_EXT(velocity.y);
		this->internalVelocity.z = __FIXED_TO_FIX7_9_EXT(velocity.z);

		Body::clampVelocity(this, false);
	}

	this->internalPosition.x = __FIXED_TO_FIX7_9_EXT(this->position.x);
	this->internalPosition.y = __FIXED_TO_FIX7_9_EXT(this->position.y);
	this->internalPosition.z = __FIXED_TO_FIX7_9_EXT(this->position.z);
}	

// clear force
void Body::clearExternalForce()
{
	this->externalForce.x = 0;
	this->externalForce.y = 0;
	this->externalForce.z = 0;
}

// apply force
uint8 Body::applyForce(const Vector3D* force)
{
	if(NULL == force)
	{
		return __NO_AXIS;
	}

	this->externalForce.x += force->x;
	this->externalForce.y += force->y;
	this->externalForce.z += force->z;

	uint16 axisOfExternalForce = __NO_AXIS;

	if(0 != force->x)
	{
		axisOfExternalForce |= __X_AXIS;
	}

	if(0 != force->y)
	{
		axisOfExternalForce |= __Y_AXIS;
	}

	if(0 != force->z)
	{
		axisOfExternalForce |= __Z_AXIS;
	}

	if(__NO_AXIS != axisOfExternalForce)
	{
		Body::clearNormalOnAxis(this, axisOfExternalForce);
		Body::setMovementType(this, __ACCELERATED_MOVEMENT, axisOfExternalForce);
		Body::awake(this, axisOfExternalForce);
	}

	this->clearExternalForce |= axisOfExternalForce;

	return axisOfExternalForce;
}

// apply gravity
uint8 Body::applyGravity(uint16 axis)
{
	if(axis)
	{
		axis &= this->axisSubjectToGravity;

		Vector3D force =
		{
			__X_AXIS & axis ? __FIXED_MULT(_currentGravity->x, this->mass) : 0,
			__Y_AXIS & axis ? __FIXED_MULT(_currentGravity->y, this->mass) : 0,
			__Z_AXIS & axis ? __FIXED_MULT(_currentGravity->z, this->mass) : 0,
		};

		return Body::applyForce(this, &force);
	}

	return __NO_AXIS;
}

// add force
void Body::applySustainedForce(const Vector3D* force)
{
	ASSERT(force, "Body::applySustainedForce: null force");

	this->clearExternalForce = ~Body::applyForce(this, force);
}

// update movement
void Body::update(uint16 cycle)
{
	if(!this->active || !this->awake)
	{
		return;
	}
	
	MovementResult movementResult;

	if(0 != this->skipCycles)
	{
		if(0 < this->skipCycles)
		{
			if(this->skipCycles > this->skipedCycles++)
			{
				return;
			}

			this->skipedCycles = 0;

			movementResult = Body::updateMovement(this, cycle);
		}
		else if(0 > this->skipCycles)
		{
			this->skipedCycles = 0;

			while(this->skipCycles <= this->skipedCycles--)
			{
				movementResult = Body::updateMovement(this, cycle);
			}
		}
	}
	else
	{
		movementResult = Body::updateMovement(this, cycle);
	}

	SpatialObject::setPosition(this->owner, &this->position);
	SpatialObject::setDirection(this->owner, &this->direction);

	// if stopped on any axis
	if(0 != movementResult.axisStoppedMovement)
	{
		Body::stopMovement(this, movementResult.axisStoppedMovement);

		if(0 != movementResult.axisStoppedMovement && this->sendMessages)
		{
			MessageDispatcher::dispatchMessage(0, ListenerObject::safeCast(this), ListenerObject::safeCast(this->owner), kMessageBodyStopped, &movementResult.axisStoppedMovement);
		}
	}

	// clear any force so the next update does not get influenced
	Body::clearExternalForce(this);
}

// retrieve last displacement
Vector3D Body::getLastDisplacement()
{
	Vector3D displacement = {0, 0, 0};

	fixed_t elapsedTime = PhysicalWorld::getElapsedTime(VUEngine::getPhysicalWorld(_vuEngine));

	displacement.x = __STOP_VELOCITY_THRESHOLD < __ABS(this->velocity.x) ? __FIXED_MULT(this->velocity.x, elapsedTime) : 0;
	displacement.y = __STOP_VELOCITY_THRESHOLD < __ABS(this->velocity.y) ? __FIXED_MULT(this->velocity.y, elapsedTime) : 0;
	displacement.z = __STOP_VELOCITY_THRESHOLD < __ABS(this->velocity.z) ? __FIXED_MULT(this->velocity.z, elapsedTime) : 0;

	return displacement;
}

MovementResult Body::getMovementResult(Vector3D previousVelocity)
{
	MovementResult movementResult = {__NO_AXIS, __NO_AXIS};

	// xor values, if result != 0, there is movement
	// xor values, if result >= 0, there is no change in direction
	Vector3D movementChange =
	{
		this->velocity.x ^ previousVelocity.x,
		this->velocity.y ^ previousVelocity.y,
		this->velocity.z ^ previousVelocity.z,
	};

	if(!this->movesIndependentlyOnEachAxis && 0 != (__ACCELERATED_MOVEMENT & (this->movementType.x | this->movementType.y | this->movementType.z)))
	{
		if(this->speed < __STOP_VELOCITY_THRESHOLD && 0 == (this->externalForce.x | this->externalForce.y | this->externalForce.z | this->gravity.x | this->gravity.y | this->gravity.z))
		{
			movementResult.axisStoppedMovement = __ALL_AXIS;
		}
	
		return movementResult;
	}

	// stop if no external force or opposing normal force is present
	// and if the velocity minimum threshold is not reached
	if(0 != previousVelocity.x && 0 == this->externalForce.x && 0 == this->gravity.x && __ACCELERATED_MOVEMENT == this->movementType.x)
	{
		if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.x) || (0 == this->externalForce.x && !this->accelerating.x) || (0 > movementChange.x))
		{
			movementResult.axisStoppedMovement |= __X_AXIS;
		}
	}

	if(0 != previousVelocity.y && 0 == this->externalForce.y && 0 == this->gravity.y && __ACCELERATED_MOVEMENT == this->movementType.y)
	{
		if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.y) || (0 == this->externalForce.y && !this->accelerating.y) || (0 > movementChange.y))
		{
			movementResult.axisStoppedMovement |= __Y_AXIS;
		}
	}

	if(0 != previousVelocity.z && 0 == this->externalForce.z && 0 == this->gravity.z && __ACCELERATED_MOVEMENT == this->movementType.z)
	{
		if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.z) || (0 == this->externalForce.z && !this->accelerating.z) || (0 > movementChange.z))
		{
			movementResult.axisStoppedMovement |= __Z_AXIS;
		}
	}

	return movementResult;
}

Vector3D Body::getFriction()
{
	return this->friction;
}

Vector3D Body::getGravity()
{
	return (Vector3D)
	{
		__X_AXIS & this->axisSubjectToGravity ? _currentGravity->x : 0,
		__Y_AXIS & this->axisSubjectToGravity ? _currentGravity->y : 0,
		__Z_AXIS & this->axisSubjectToGravity ? _currentGravity->z : 0,
	};
}

static inline fix7_9_ext Body::doComputeInstantaneousSpeed(fixed_t forceMagnitude, fixed_t gravity, fixed_t mass, fixed_t friction, fix7_9_ext elapsedTime)
{
	fix7_9_ext acceleration = __FIXED_TO_FIX7_9_EXT(gravity) + __FIX7_9_EXT_DIV(__FIXED_TO_FIX7_9_EXT(forceMagnitude + friction), __FIXED_TO_FIX7_9_EXT(mass));

	return __FIX7_9_EXT_MULT(acceleration, elapsedTime);
}

static inline fixed_t Body::computeInstantaneousSpeed(fixed_t forceMagnitude, fixed_t gravity, fixed_t mass, fixed_t friction, fixed_t maximumSpeed)
{
	fixed_t instantaneousSpeed = __FIX7_9_EXT_TO_FIXED(Body::doComputeInstantaneousSpeed(forceMagnitude, gravity, mass, friction, _currentPhysicsElapsedTime));

	return 0 != maximumSpeed && maximumSpeed < __ABS(instantaneousSpeed) ? maximumSpeed : instantaneousSpeed;
}

void Body::computeDirectionAndSpeed(bool useExternalForceForDirection)
{
	if(useExternalForceForDirection)
	{
		this->speed = Vector3D::length(this->velocity);
		this->direction = Vector3D::normalize(this->externalForce);
		this->velocity = Vector3D::scalarProduct(this->direction, this->speed);
	}
	else
	{
#ifndef __PHYSICS_HIGH_PRECISION
		this->speed = Vector3D::length(this->velocity);

		this->direction = Vector3D::scalarDivision(this->velocity, this->speed);
#else
		fix7_9_ext speed = __F_TO_FIX7_9_EXT(Math::squareRoot(__FIXED_EXT_TO_F(Vector3D::squareLength(this->velocity))));

		this->speed = __FIX7_9_EXT_TO_FIXED(speed);

		if(0 < speed)
		{
			this->direction.x = __FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_DIV(this->internalVelocity.x, speed));
			this->direction.y = __FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_DIV(this->internalVelocity.y, speed));
			this->direction.z = __FIX7_9_EXT_TO_FIXED(__FIX7_9_EXT_DIV(this->internalVelocity.z, speed));
		}
#endif	
	}
}

void Body::clampVelocity(bool useExternalForceForDirection)
{
	Body::computeDirectionAndSpeed(this, useExternalForceForDirection);

	// First check if must clamp speed
	if(0 != this->maximumSpeed && this->maximumSpeed < this->speed)
	{
		this->speed = this->maximumSpeed;

		this->velocity = Vector3D::scalarProduct(this->direction, this->maximumSpeed);

		this->internalVelocity.x = __FIXED_TO_FIX7_9_EXT(this->velocity.x);
		this->internalVelocity.y = __FIXED_TO_FIX7_9_EXT(this->velocity.y);
		this->internalVelocity.z = __FIXED_TO_FIX7_9_EXT(this->velocity.z);
	}

	// Then clamp speed based on each axis configuration
	if(0 != this->maximumVelocity.x)
	{
		if(__ABS(this->maximumVelocity.x) < __ABS(this->velocity.x))
		{
			int32 sign = 0 <= this->velocity.x ? 1 : -1;

			this->velocity.x = (this->maximumVelocity.x * sign);
			this->internalVelocity.x = __FIXED_TO_FIX7_9_EXT(this->velocity.x);
		}
	}

	if(0 != this->maximumVelocity.y)
	{
		if(__ABS(this->maximumVelocity.y) < __ABS(this->velocity.y))
		{
			int32 sign = 0 <= this->velocity.y ? 1 : -1;

			this->velocity.y = (this->maximumVelocity.y * sign);
			this->internalVelocity.y = __FIXED_TO_FIX7_9_EXT(this->velocity.y);
		}
	}

	if(0 != this->maximumVelocity.z)
	{
		if(__ABS(this->maximumVelocity.z) < __ABS(this->velocity.z))
		{
			int32 sign = 0 <= this->velocity.z ? 1 : -1;

			this->velocity.z = (this->maximumVelocity.z * sign);
			this->internalVelocity.z = __FIXED_TO_FIX7_9_EXT(this->velocity.z);
		}
	}
}

// update movement over axis
MovementResult Body::updateMovement(uint16 cycle)
{
	// yeah, * 4 (<< 2) is a magical number, but it works well enough with the range of mass and friction coefficient
	this->friction = Vector3D::scalarProduct(this->direction, -__FIXED_MULT(this->frictionForceMagnitude, __I_TO_FIXED(1 << __FRICTION_FORCE_FACTOR_POWER)));

	Vector3D previousVelocity = this->velocity;

	if(__ACCELERATED_MOVEMENT == this->movementType.x)
	{
		fix7_9_ext instantaneousSpeed = Body::doComputeInstantaneousSpeed(this->externalForce.x + this->totalNormal.x, this->gravity.x, this->mass, this->friction.x, _currentPhysicsElapsedTime);

		this->accelerating.x = 0 != instantaneousSpeed;
		this->internalVelocity.x += instantaneousSpeed;
		this->velocity.x = __FIX7_9_EXT_TO_FIXED(this->internalVelocity.x);
	}

	if(__ACCELERATED_MOVEMENT == this->movementType.y)
	{
		fix7_9_ext instantaneousSpeed = Body::doComputeInstantaneousSpeed(this->externalForce.y + this->totalNormal.y, this->gravity.y, this->mass, this->friction.y, _currentPhysicsElapsedTime);

		this->accelerating.y = 0 != instantaneousSpeed;
		this->internalVelocity.y += instantaneousSpeed;
		this->velocity.y = __FIX7_9_EXT_TO_FIXED(this->internalVelocity.y);
	}

	if(__ACCELERATED_MOVEMENT == this->movementType.z)
	{
		fix7_9_ext instantaneousSpeed = Body::doComputeInstantaneousSpeed(this->externalForce.z + this->totalNormal.z, this->gravity.z, this->mass, this->friction.z, _currentPhysicsElapsedTime);

		this->accelerating.z = 0 != instantaneousSpeed;
		this->internalVelocity.z += instantaneousSpeed;
		this->velocity.z = __FIX7_9_EXT_TO_FIXED(this->internalVelocity.z);
	}

	if(previousVelocity.x != this->velocity.x || previousVelocity.y != this->velocity.y || previousVelocity.z != this->velocity.z)
	{
		Body::clampVelocity(this, 0 == previousVelocity.x && 0 == previousVelocity.y && 0 == previousVelocity.z);
	}	

	if(__NO_MOVEMENT != this->movementType.x)
	{
		if((__UNIFORM_MOVEMENT == this->movementType.x) && (__ABS(this->velocity.x) < __PIXELS_TO_METERS((1 << __PIXELS_PER_METER_2_POWER))))
		{
			if(0 < __ABS(__METERS_TO_PIXELS(this->velocity.x)) && 0 == cycle % (__TARGET_FPS / __METERS_TO_PIXELS(this->velocity.x)))
			{
				this->position.x += 0 <= this->velocity.x ? __PIXELS_TO_METERS(1) : -__PIXELS_TO_METERS(1);
			}
		}
		else
		{
			this->internalPosition.x += __FIX7_9_EXT_MULT(this->internalVelocity.x, _currentPhysicsElapsedTime);
			this->position.x = __FIX7_9_EXT_TO_FIXED(this->internalPosition.x);
		}
	}

	if(__NO_MOVEMENT != this->movementType.y)
	{
		if((__UNIFORM_MOVEMENT == this->movementType.y) && (__ABS(this->velocity.y) < __PIXELS_TO_METERS((1 << __PIXELS_PER_METER_2_POWER))))
		{
			if(0 < __ABS(__METERS_TO_PIXELS(this->velocity.y)) && 0 == cycle % (__TARGET_FPS / __METERS_TO_PIXELS(this->velocity.y)))
			{
				this->position.y += 0 <= this->velocity.y ? __PIXELS_TO_METERS(1) : -__PIXELS_TO_METERS(1);
			}
		}
		else
		{
			this->internalPosition.y += __FIX7_9_EXT_MULT(this->internalVelocity.y, _currentPhysicsElapsedTime);
			this->position.y = __FIX7_9_EXT_TO_FIXED(this->internalPosition.y);
		}
	}

	if(__NO_MOVEMENT != this->movementType.z)
	{
		if((__UNIFORM_MOVEMENT == this->movementType.z) && (__ABS(this->velocity.z) < __PIXELS_TO_METERS((1 << __PIXELS_PER_METER_2_POWER))))
		{
			if(0 < __ABS(__METERS_TO_PIXELS(this->velocity.z)) && 0 == cycle % (__TARGET_FPS / __METERS_TO_PIXELS(this->velocity.z)))
			{
				this->position.z += 0 <= this->velocity.z ? __PIXELS_TO_METERS(1) : -__PIXELS_TO_METERS(1);
			}
		}
		else
		{
			this->internalPosition.z += __FIX7_9_EXT_MULT(this->internalVelocity.z, _currentPhysicsElapsedTime);
			this->position.z = __FIX7_9_EXT_TO_FIXED(this->internalPosition.z);
		}
	}

	return Body::getMovementResult(this, previousVelocity);
}

// stop movement over an axis
uint16 Body::stopMovement(uint16 axis)
{
	uint16 axisOfMovement = Body::getMovementOnAllAxis(this);
	uint16 axisOfStopping = __NO_AXIS;

	if(axis & __X_AXIS)
	{
		// not moving anymore
		this->velocity.x = 0;
		this->internalVelocity.x = 0;
		this->accelerating.x = false;
		this->externalForce.x = 0;
		axisOfStopping |= axisOfMovement & __X_AXIS;
	}

	if(axis & __Y_AXIS)
	{
		// not moving anymore
		this->velocity.y = 0;
		this->internalVelocity.y = 0;
		this->accelerating.y = false;
		this->externalForce.y = 0;
		axisOfStopping |= axisOfMovement & __Y_AXIS;
	}

	if(axis & __Z_AXIS)
	{
		// not moving anymore
		this->velocity.z = 0;
		this->internalVelocity.z = 0;
		this->accelerating.z = false;
		this->externalForce.z = 0;
		axisOfStopping |= axisOfMovement & __Z_AXIS;
	}

	Body::setMovementType(this, __NO_MOVEMENT, axisOfStopping);

	if(!Body::getMovementOnAllAxis(this))
	{
		Body::sleep(this);
	}

	return axisOfStopping;
}

// get axis subject to gravity
uint16 Body::getAxisSubjectToGravity()
{
	return this->axisSubjectToGravity;
}

// set axis subject to gravity
void Body::setAxisSubjectToGravity(uint16 axisSubjectToGravity)
{
	this->axisSubjectToGravity = axisSubjectToGravity;
}

// set active
void Body::setActive(bool active)
{
	this->active = active;
}

// is active?
bool Body::isActive()
{
	return this->active;
}

// retrieve position
const Vector3D* Body::getPosition()
{
	return &this->position;
}

// retrieve position
void Body::setPosition(const Vector3D* position, SpatialObject caller)
{
	if(this->owner == caller)
	{
		this->position = *position;

		this->internalPosition.x = __FIXED_TO_FIX7_9_EXT(this->position.x);
		this->internalPosition.y = __FIXED_TO_FIX7_9_EXT(this->position.y);
		this->internalPosition.z = __FIXED_TO_FIX7_9_EXT(this->position.z);
	}
}

// get bounciness
fixed_t Body::getBounciness()
{
	return this->bounciness;
}

// set bounciness
void Body::setBounciness(fixed_t bounciness)
{
	if(__I_TO_FIXED(0) > bounciness)
	{
		bounciness = 0;
	}
	else if(__F_TO_FIXED(__MAXIMUM_BOUNCINESS_COEFFICIENT) < bounciness)
	{
		bounciness = __F_TO_FIXED(__MAXIMUM_BOUNCINESS_COEFFICIENT);
	}

	this->bounciness = bounciness;
}

void Body::setSkipCycles(int8 skipCycles)
{
	this->skipCycles = skipCycles;
	this->skipedCycles = 0;
}

void Body::computeTotalNormal()
{
	this->totalNormal = Vector3D::zero();

	if(NULL != this->normals)
	{
		VirtualNode node = this->normals->head;

		for(; NULL != node; node = node->next)
		{
			NormalRegistry* normalRegistry = (NormalRegistry*)node->data;

			if(!isDeleted(normalRegistry->referent))
			{
				Vector3D normal = Vector3D::scalarProduct(normalRegistry->direction, normalRegistry->magnitude);

				this->totalNormal.x += normal.x;
				this->totalNormal.y += normal.y;
				this->totalNormal.z += normal.z;
			}
		}
	}

	if(0 != this->totalNormal.x || 0 != this->totalNormal.y || 0 != this->totalNormal.z)
	{
		Body::computeTotalFrictionCoefficient(this);
		Body::computeFrictionForceMagnitude(this);
	}
}

void Body::addNormal(ListenerObject referent, Vector3D direction, fixed_t magnitude)
{
	ASSERT(referent, "Body::addNormal: null referent");

	if(!this->normals)
	{
		this->normals = new VirtualList();
	}

#ifndef __RELEASE
	for(VirtualNode node = this->normals->head; NULL != node; node = node->next)
	{
		ASSERT(!isDeleted(node->data), "Body::addNormal: null normal");
	}
#endif

	NormalRegistry* normalRegistry = new NormalRegistry;
	normalRegistry->referent = referent;
	normalRegistry->direction = direction;
	normalRegistry->magnitude = magnitude;

	VirtualList::pushBack(this->normals, normalRegistry);

	Body::computeTotalNormal(this);
}

Vector3D Body::getNormal()
{
	return this->totalNormal;
}

Vector3D Body::getLastNormalDirection()
{
	if(!this->normals || !this->normals->head)
	{
		return Vector3D::zero();
	}

	while(this->normals->head && isDeleted(((NormalRegistry*)VirtualList::back(this->normals))->referent))
	{
		ASSERT(!isDeleted(VirtualList::back(this->normals)), "Body::getLastNormalDirection: dead normal registry");

		delete VirtualList::popBack(this->normals);
	}

	Body::computeTotalNormal(this);

	if(!this->normals->head)
	{
		return Vector3D::zero();
	}

	return ((NormalRegistry*)VirtualList::back(this->normals))->direction;
}

void Body::reset()
{
	if(this->normals)
	{
		VirtualList::deleteData(this->normals);
		delete this->normals;
		this->normals = NULL;
	}

	Body::computeTotalNormal(this);
	Body::setSurroundingFrictionCoefficient(this, 0);

	this->velocity 				= Vector3D::zero();
	this->accelerating 			= (Vector3DFlag){false, false, false};
	this->externalForce	 		= Vector3D::zero();
	this->friction 				= Vector3D::zero();
	this->totalNormal			= Vector3D::zero();
	this->gravity 				= Body::getGravity(this);
}

void Body::clearNormalOnAxis(uint16 axis __attribute__ ((unused)))
{
	if(!isDeleted(this->normals))
	{
		bool computeTotalNormal = false;

		for(VirtualNode node = this->normals->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			NormalRegistry* normalRegistry = (NormalRegistry*)node->data;

			if
			(
				isDeleted(normalRegistry->referent) ||
				((__X_AXIS & axis) && normalRegistry->direction.x) ||
				((__Y_AXIS & axis) && normalRegistry->direction.y) ||
				((__Z_AXIS & axis) && normalRegistry->direction.z)
			)
			{
				VirtualList::removeNode(this->normals, node);
				delete node->data;
				computeTotalNormal = true;
			}
		}

		if(computeTotalNormal)
		{
			Body::computeTotalNormal(this);
		}
	}
}

void Body::clearNormal(ListenerObject referent)
{
	ASSERT(!isDeleted(referent), "Body::clearNormal: dead referent");

	if(!isDeleted(this->normals))
	{
		VirtualNode node = this->normals->head;

		for(; NULL != node; node = node->next)
		{
			NormalRegistry* normalRegistry = (NormalRegistry*)node->data;

			// search if registry exists for the referent
			// it is Ok if it doesn't exist
			if(normalRegistry->referent == referent)
			{
				ASSERT(!isDeleted(normalRegistry), "Body::clearNormal: dead normal registry");
				VirtualList::removeNode(this->normals, node);
				delete normalRegistry;
				break;
			}
		}

		if(NULL != node)
		{
			Body::computeTotalNormal(this);
		}
	}
}

fixed_t Body::getFrictionForceMagnitude()
{
	return this->frictionForceMagnitude;
}

fixed_t Body::getFrictionCoefficient()
{
	return this->frictionCoefficient;
}

void Body::computeTotalFrictionCoefficient()
{
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

Vector3D Body::getWeight()
{
	return Vector3D::scalarProduct(this->gravity, this->mass);
}

void Body::computeFrictionForceMagnitude()
{
	if(0 == this->frictionCoefficient)
	{
		this->frictionForceMagnitude = 0;
		return;
	}

	if(0 != this->totalNormal.x || 0 != this->totalNormal.y || 0 != this->totalNormal.z)
	{
		this->frictionForceMagnitude = __ABS(__FIXED_MULT(Vector3D::length(this->totalNormal), this->totalFrictionCoefficient));
	}
	else
	{
		fixed_t weight = Vector3D::length(Body::getWeight(this));

		if(weight)
		{
			this->frictionForceMagnitude = __ABS(__FIXED_MULT(weight, _currentWorldFriction));
		}
		else
		{
			this->frictionForceMagnitude = __ABS(_currentWorldFriction + this->frictionCoefficient);

			if(0 > this->frictionForceMagnitude)
			{
				this->frictionForceMagnitude = 0;
			}
			else if(__MAXIMUM_FRICTION_COEFFICIENT < this->frictionForceMagnitude)
			{
				this->frictionForceMagnitude = __MAXIMUM_FRICTION_COEFFICIENT;
			}
		}
	}
}

// set friction
void Body::setFrictionCoefficient(fixed_t frictionCoefficient)
{
	if(0 > frictionCoefficient)
	{
		frictionCoefficient = 0;
	}
	else if(__MAXIMUM_FRICTION_COEFFICIENT < frictionCoefficient)
	{
		frictionCoefficient = __MAXIMUM_FRICTION_COEFFICIENT;
	}

	this->frictionCoefficient = frictionCoefficient;
	Body::computeTotalFrictionCoefficient(this);
}

void Body::setSurroundingFrictionCoefficient(fixed_t surroundingFrictionCoefficient)
{
	if(0 > surroundingFrictionCoefficient)
	{
		surroundingFrictionCoefficient = 0;
	}
	else if(__MAXIMUM_FRICTION_COEFFICIENT < surroundingFrictionCoefficient)
	{
		surroundingFrictionCoefficient = __MAXIMUM_FRICTION_COEFFICIENT;
	}

	this->surroundingFrictionCoefficient = surroundingFrictionCoefficient;
	Body::computeTotalFrictionCoefficient(this);
}

fixed_t Body::getMass()
{
	return this->mass;
}

void Body::setMass(fixed_t mass)
{
	this->mass = __BODY_MIN_MASS < mass ? __BODY_MAX_MASS > mass ? mass : __BODY_MAX_MASS : __BODY_MIN_MASS;
}

bool Body::reachedMaximumSpeedpeed()
{
	return this->speed == this->maximumSpeed;
}

// retrieve state
bool Body::isAwake()
{
	return this->awake && this->active;
}

// awake body
void Body::awake(uint16 axisOfAwakening)
{
	this->awake = true;
	this->active = true;

	if(this->sendMessages)
	{
		bool dispatchMessage = false;

		if(0 == this->velocity.x && 0 != (__X_AXIS & axisOfAwakening))
		{
			dispatchMessage |= (__X_AXIS & axisOfAwakening);
		}

		if(0 == this->velocity.y && 0 != (__Y_AXIS & axisOfAwakening))
		{
			dispatchMessage |= (__Y_AXIS & axisOfAwakening);
		}

		if(0 == this->velocity.z && 0 != (__Z_AXIS & axisOfAwakening))
		{
			dispatchMessage |= (__Z_AXIS & axisOfAwakening);
		}

		if(dispatchMessage)
		{
			MessageDispatcher::dispatchMessage(0, ListenerObject::safeCast(this), ListenerObject::safeCast(this->owner), kMessageBodyStartedMoving, &axisOfAwakening);
		}
	}
}

// go to sleep
void Body::sleep()
{
	this->awake = false;
}

// is it moving?
uint16 Body::getMovementOnAllAxis()
{
	if(!this->awake || !this->active)
	{
		return __NO_AXIS;
	}

	uint16 result = 0;

	if(this->velocity.x || this->accelerating.x)
	{
		result |= __X_AXIS;
	}

	if(this->velocity.y || this->accelerating.y)
	{
		result |= __Y_AXIS;
	}

	if(this->velocity.z || this->accelerating.z)
	{
		result |= __Z_AXIS;
	}

	return result;
}

MovementResult Body::getBouncingResult(Vector3D previousVelocity, Vector3D bouncingPlaneNormal)
{
	MovementResult movementResult = {__NO_AXIS, __NO_AXIS};

	// xor values, if result != 0, there is movement
	// xor values, if result >= 0, there is no change in direction
	Vector3D movementChange =
	{
		this->velocity.x ^ previousVelocity.x,
		this->velocity.y ^ previousVelocity.y,
		this->velocity.z ^ previousVelocity.z,
	};

	// stop if minimum velocity threshold is not reached
	// and if there is possible movement in the other components
	if(__STOP_BOUNCING_VELOCITY_THRESHOLD > __ABS(this->velocity.x) && !__FIXED_INT_PART(bouncingPlaneNormal.y | bouncingPlaneNormal.z))
	{
		movementResult.axisStoppedMovement |= __X_AXIS;
	}

	if(__STOP_BOUNCING_VELOCITY_THRESHOLD > __ABS(this->velocity.y) && !__FIXED_INT_PART(bouncingPlaneNormal.x | bouncingPlaneNormal.z))
	{
		movementResult.axisStoppedMovement |= __Y_AXIS;
	}

	if(__STOP_BOUNCING_VELOCITY_THRESHOLD > __ABS(this->velocity.z) && !__FIXED_INT_PART(bouncingPlaneNormal.x | bouncingPlaneNormal.y))
	{
		movementResult.axisStoppedMovement |= __Z_AXIS;
	}

	// bounce accelerated if movement changed direction and the previous movement was not uniform
	if(__UNIFORM_MOVEMENT != this->movementType.x && 0 != movementChange.x)
	{
		movementResult.axisOfAcceleratedBouncing |= __X_AXIS;
	}

	if(__UNIFORM_MOVEMENT != this->movementType.y && 0 != movementChange.y)
	{
		movementResult.axisOfAcceleratedBouncing |= __Y_AXIS;
	}

	if(__UNIFORM_MOVEMENT != this->movementType.z && 0 != movementChange.z)
	{
		movementResult.axisOfAcceleratedBouncing |= __Z_AXIS;
	}

	// don't bounce if movement stopped on that axis
	movementResult.axisOfAcceleratedBouncing &= ~movementResult.axisStoppedMovement;

	return movementResult;
}

// bounce back
void Body::bounce(ListenerObject bounceReferent, Vector3D bouncingPlaneNormal, fixed_t frictionCoefficient, fixed_t bounciness)
{
	// set friction
	Body::setSurroundingFrictionCoefficient(this, frictionCoefficient);

	// compute bouncing vector
	fixed_t cosAngle = __I_TO_FIXED(bouncingPlaneNormal.x | bouncingPlaneNormal.y | bouncingPlaneNormal.z) && (this->gravity.x | this->gravity.y | this->gravity.z) ? __ABS(__FIXED_EXT_DIV(Vector3D::dotProduct(this->gravity, bouncingPlaneNormal), Vector3D::lengthProduct(this->gravity, bouncingPlaneNormal))) : __1I_FIXED;
	fixed_t normalMagnitude = __FIXED_EXT_MULT(Vector3D::length(Body::getWeight(this)), cosAngle);

	// register normal affecting the body
	Body::addNormal(this, bounceReferent, bouncingPlaneNormal, normalMagnitude);

	// compute bouncing vector
	Vector3D velocity = this->velocity;

	// compute bouncing velocity vector
	Vector3D u = Vector3D::scalarProduct(bouncingPlaneNormal, Vector3D::dotProduct(velocity, bouncingPlaneNormal));

	Vector3D w = Vector3D::get(u, velocity);

	bounciness += this->bounciness;

	if(__I_TO_FIXED(0) > bounciness)
	{
		bounciness = 0;
	}
	else if(__F_TO_FIXED(__MAXIMUM_BOUNCINESS_COEFFICIENT) < bounciness)
	{
		bounciness = __F_TO_FIXED(__MAXIMUM_BOUNCINESS_COEFFICIENT);
	}

	if(0 > frictionCoefficient)
	{
		frictionCoefficient = 0;
	}
	else if(__MAXIMUM_FRICTION_COEFFICIENT < frictionCoefficient)
	{
		frictionCoefficient = __MAXIMUM_FRICTION_COEFFICIENT;
	}

	// add bounciness and friction
	// This is the physically correct computation, but causes
	// wrong angles of bouncing
	u = Vector3D::scalarProduct(u, bounciness);
	w = Vector3D::scalarProduct(w, (__MAXIMUM_FRICTION_COEFFICIENT - frictionCoefficient));

	this->velocity = Vector3D::get(u, w);

	this->internalVelocity.x = __FIXED_TO_FIX7_9_EXT(this->velocity.x);
	this->internalVelocity.y = __FIXED_TO_FIX7_9_EXT(this->velocity.y);
	this->internalVelocity.z = __FIXED_TO_FIX7_9_EXT(this->velocity.z);

	Body::clampVelocity(this, false);

	if(__NO_MOVEMENT == this->movementType.x && 0 != this->velocity.x)
	{
		this->movementType.x = __ACCELERATED_MOVEMENT;
	}

	if(__NO_MOVEMENT == this->movementType.y && 0 != this->velocity.y)
	{
		this->movementType.y = __ACCELERATED_MOVEMENT;
	}

	if(__NO_MOVEMENT == this->movementType.z && 0 != this->velocity.z)
	{
		this->movementType.z = __ACCELERATED_MOVEMENT;
	}

	// determine bouncing result
	MovementResult movementResult = Body::getBouncingResult(this, velocity, bouncingPlaneNormal);

	// stop over the axis where there is no bouncing
	if(movementResult.axisStoppedMovement)
	{
		uint16 axisOfStopping = Body::stopMovement(this, movementResult.axisStoppedMovement);

		if(__NO_AXIS != axisOfStopping && this->sendMessages)
		{
			MessageDispatcher::dispatchMessage(0, ListenerObject::safeCast(this), ListenerObject::safeCast(this->owner), kMessageBodyStopped, &axisOfStopping);
		}
	}

	if(0 != movementResult.axisOfAcceleratedBouncing)
	{
	//	Body::setSurroundingFrictionCoefficient(this, 0);
		Body::clearNormalOnAxis(this, movementResult.axisOfAcceleratedBouncing);
	}

	if(!Body::getMovementOnAllAxis(this))
	{
		Body::sleep(this);
	}
}

void Body::setMaximumVelocity(Vector3D maximumVelocity)
{
	this->maximumVelocity = maximumVelocity;
}

Vector3D Body::getMaximumVelocity()
{
	return this->maximumVelocity;
}

void Body::setMaximumSpeed(fixed_t maximumSpeed)
{
	this->maximumSpeed = maximumSpeed;
}

fixed_t Body::getMaximumSpeed()
{
	return this->maximumSpeed;
}

void Body::sendMessages(bool value)
{
	this->sendMessages = value;
}

void Body::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "BODY", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Active:", x, ++y, NULL);
	Printing::text(Printing::getInstance(), this->active ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 8, y++, NULL);
	Printing::text(Printing::getInstance(), "Awake:", x, y, NULL);
	Printing::text(Printing::getInstance(), this->awake ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "                    X       Y       Z", x, y++, NULL);

	int32 xDisplacement = 20;

	Printing::text(Printing::getInstance(), "Mov. type", x, y, NULL);
	Printing::text(Printing::getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing::text(Printing::getInstance(), __UNIFORM_MOVEMENT == this->movementType.x ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.x ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.x ? "Accel" : "None", xDisplacement + x, y, NULL);
	Printing::text(Printing::getInstance(), __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.y ? "Accel" : "None", xDisplacement + x + 8, y, NULL);
	Printing::text(Printing::getInstance(), __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.z ? "Accel" : "None", xDisplacement + x + 8 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "Position", x, y, NULL);
	Printing::text(Printing::getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->position.x), xDisplacement + x, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->position.y), xDisplacement + x + 8, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->position.z), xDisplacement + x + 8 * 2, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "Velocity", x, y, NULL);
	Printing::text(Printing::getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->velocity.x), xDisplacement + x, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->velocity.y), xDisplacement + x + 8, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->velocity.z), xDisplacement + x + 8 * 2, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "NormalizedDirection", x, y, NULL);
	Printing::text(Printing::getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->direction.x), xDisplacement + x, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->direction.y), xDisplacement + x + 8, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->direction.z), xDisplacement + x + 8 * 2, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "Maximum Speed", x, y, NULL);
	Printing::text(Printing::getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->maximumSpeed), xDisplacement + x, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "Speed", x, y, NULL);
	Printing::text(Printing::getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(Vector3D::length(this->velocity)), xDisplacement + x, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "Acceleration", x, y, NULL);
	Printing::text(Printing::getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing::int32(Printing::getInstance(), this->accelerating.x, xDisplacement + x, y, NULL);
	Printing::int32(Printing::getInstance(), this->accelerating.y, xDisplacement + x + 8, y, NULL);
	Printing::int32(Printing::getInstance(), this->accelerating.z, xDisplacement + x + 8 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "Gravity", x, y, NULL);
	Printing::text(Printing::getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->gravity.x), xDisplacement + x, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->gravity.y), xDisplacement + x + 8, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->gravity.z), xDisplacement + x + 8 * 2, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "External Force", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->externalForce.x), xDisplacement + x, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->externalForce.y), xDisplacement + x + 8, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->externalForce.z), xDisplacement + x + 8 * 2, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "Normal", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->totalNormal.x), xDisplacement + x, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->totalNormal.y), xDisplacement + x + 8, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->totalNormal.z), xDisplacement + x + 8 * 2, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "Normal Force", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->frictionForceMagnitude), xDisplacement + x, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "Normals", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::int32(Printing::getInstance(), this->normals ? VirtualList::getSize(this->normals) : 0, xDisplacement + x, y++, NULL);

	Printing::text(Printing::getInstance(), "Friction", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->friction.x), xDisplacement + x, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->friction.y), xDisplacement + x + 8, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->friction.z), xDisplacement + x + 8 * 2, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "Total frict. coef.", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->totalFrictionCoefficient), xDisplacement + x, y++, 2, NULL);

	Printing::text(Printing::getInstance(), "Friction magnitude", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->frictionForceMagnitude), xDisplacement + x, y++, 2, NULL);
}
