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

#include <Body.h>
#include <Game.h>
#include <Clock.h>
#include <PhysicalWorld.h>
#include <MessageDispatcher.h>
#include <FrameRate.h>
#include <debugConfig.h>


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

#define __MIN_MASS								__F_TO_FIXED(0.1f)
#define __MAX_MASS								__I_TO_FIXED(1)

#ifndef __FRICTION_FORCE_FACTOR_POWER
#define __FRICTION_FORCE_FACTOR_POWER					2
#endif


//---------------------------------------------------------------------------------------------------------
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

fixed_t _currentWorldFriction = 0;
fix7_9_ext _currentPhysicsElapsedTime = 0;
const Acceleration* _currentGravity = 0;

static void Body::setCurrentWorldFrictionCoefficient(fixed_t currentWorldFriction)
{
	_currentWorldFriction = currentWorldFriction;
}

static void Body::setCurrentElapsedTime(fix7_9_ext currentElapsedTime)
{
	_currentPhysicsElapsedTime = currentElapsedTime;
}

static void Body::setCurrentGravity(const Acceleration* currentGravity)
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
	ListenerObject referent;
	Vector3D direction;
	fixed_t magnitude;

} NormalRegistry;

Clock _physhicsClock = NULL;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Body::constructor(SpatialObject owner, const PhysicalSpecification* physicalSpecification, uint16 axisSubjectToGravity)
{
	Base::constructor();

	this->destroy = false;
	this->owner = owner;
	this->normals = NULL;
	this->mass = __MIN_MASS < physicalSpecification->mass ? __MAX_MASS > physicalSpecification->mass ? physicalSpecification->mass : __MAX_MASS : __MIN_MASS;
	this->bounciness = physicalSpecification->bounciness;
	this->frictionCoefficient = 0;
	this->surroundingFrictionCoefficient = 0;
	this->totalFrictionCoefficient = 0;
	this->frictionForceMagnitude = 0;

	this->active = true;
	this->awake = false;
	this->changedDirection = false;
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
	this->weight 				= Vector3D::scalarProduct(*_currentGravity, this->mass);
	this->maximumVelocity 		= physicalSpecification->maximumVelocity;
	this->maximumSpeed 			= physicalSpecification->maximumSpeed;
	this->speed 				= 0;
	this->clearExternalForce 	= __NO_AXIS;

	this->internalPosition.x 	= 0;
	this->internalPosition.y 	= 0;
	this->internalPosition.z 	= 0;
	this->internalVelocity.x 	= 0;
	this->internalVelocity.y 	= 0;
	this->internalVelocity.z 	= 0;
	
	this->movesIndependentlyOnEachAxis = 0 != this->maximumVelocity.x || 0 != this->maximumVelocity.y || 0 != this->maximumVelocity.z;

	Body::setFrictionCoefficient(this, physicalSpecification->frictionCoefficient);
	Body::computeFrictionForceMagnitude(this);

	if(!_physhicsClock)
	{
		_physhicsClock = Game::getPhysicsClock(Game::getInstance());
	}
}

// class's destructor
void Body::destructor()
{
	if(this->normals)
	{
		VirtualNode node = this->normals->head;

		for(; NULL != node; node = node->next)
		{
			delete node->data;
		}

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
Velocity Body::getVelocity()
{
	return this->velocity;
}

void Body::setVelocity(Velocity* velocity)
{
	if(velocity)
	{
		this->velocity = *velocity;
	}
}

const Direction3D* Body::getDirection3D()
{
	return &this->direction;
}

fixed_t Body::getSpeed()
{
	return this->speed;
}

void Body::modifyVelocity(const Velocity* modifier)
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
Force Body::getAppliedForce()
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
void Body::moveUniformly(Velocity velocity)
{
	uint16 axisOfUniformMovement = 0;

	if(velocity.x)
	{
		axisOfUniformMovement |= __X_AXIS;
		this->velocity.x = velocity.x;
	}

	if(velocity.y)
	{
		axisOfUniformMovement |= __Y_AXIS;
		this->velocity.y = velocity.y;
	}

	if(velocity.z)
	{
		axisOfUniformMovement |= __Z_AXIS;
		this->velocity.z = velocity.z;
	}

	if(axisOfUniformMovement)
	{
		Body::setMovementType(this, __UNIFORM_MOVEMENT, axisOfUniformMovement);
		Body::awake(this, axisOfUniformMovement);
	}
}

// clear force
void Body::clearExternalForce()
{
	if(__X_AXIS & this->clearExternalForce)
	{
		this->externalForce.x = 0;
	}

	if(__Y_AXIS & this->clearExternalForce)
	{
		this->externalForce.y = 0;
	}

	if(__Z_AXIS & this->clearExternalForce)
	{
		this->externalForce.z = 0;
	}
}

// apply force
uint8 Body::applyForce(const Force* force)
{
	if(force)
	{
		this->externalForce.x += force->x;
		this->externalForce.y += force->y;
		this->externalForce.z += force->z;

		uint16 axisOfExternalForce = __NO_AXIS;

		if(force->x)
		{
			axisOfExternalForce |= __X_AXIS;
		}

		if(force->y)
		{
			axisOfExternalForce |= __Y_AXIS;
		}

		if(force->z)
		{
			axisOfExternalForce |= __Z_AXIS;
		}

		if(axisOfExternalForce)
		{
			Body::clearNormalOnAxis(this, axisOfExternalForce);
			Body::setMovementType(this, __ACCELERATED_MOVEMENT, axisOfExternalForce);
			Body::awake(this, axisOfExternalForce);
		}

		this->clearExternalForce |= axisOfExternalForce;

		return axisOfExternalForce;
	}

	return __NO_AXIS;
}

// apply gravity
uint8 Body::applyGravity(uint16 axis)
{
	if(axis)
	{
		Acceleration gravityForce = Vector3D::scalarProduct(Body::getGravity(this), this->mass);

		Force force =
		{
			__X_AXIS & axis ? gravityForce.x : 0,
			__Y_AXIS & axis ? gravityForce.y : 0,
			__Z_AXIS & axis ? gravityForce.z : 0,
		};

		return Body::applyForce(this, &force);
	}

	return __NO_AXIS;
}

// add force
void Body::applySustainedForce(const Force* force)
{
	ASSERT(force, "Body::applySustainedForce: null force");

	this->clearExternalForce = ~Body::applyForce(this, force);
}

// update movement
void Body::update()
{
	if(this->active)
	{
		if(this->awake)
		{
			MovementResult movementResult = Body::updateMovement(this);

			// if stopped on any axis
			if(movementResult.axisStoppedMovement)
			{
				Body::stopMovement(this, movementResult.axisStoppedMovement);

				if(movementResult.axisStoppedMovement && this->sendMessages)
				{
					MessageDispatcher::dispatchMessage(0, ListenerObject::safeCast(this), ListenerObject::safeCast(this->owner), kMessageBodyStopped, &movementResult.axisStoppedMovement);
				}
			}

			// no one uses this
/*			if(movementResult.axisOfChangeOfMovement)
			{
				MessageDispatcher::dispatchMessage(0, ListenerObject::safeCast(this), ListenerObject::safeCast(this->owner), kMessageBodyChangedDirection, &movementResult.axisOfChangeOfMovement);
			}
*/		}

		// clear any force so the next update does not get influenced
		Body::clearExternalForce(this);
	}
}

// retrieve last displacement
Vector3D Body::getLastDisplacement()
{
	Vector3D displacement = {0, 0, 0};

	fixed_t elapsedTime = PhysicalWorld::getElapsedTime(Game::getPhysicalWorld(Game::getInstance()));

	displacement.x = __STOP_VELOCITY_THRESHOLD < __ABS(this->velocity.x) ? __FIXED_MULT(this->velocity.x, elapsedTime) : 0;
	displacement.y = __STOP_VELOCITY_THRESHOLD < __ABS(this->velocity.y) ? __FIXED_MULT(this->velocity.y, elapsedTime) : 0;
	displacement.z = __STOP_VELOCITY_THRESHOLD < __ABS(this->velocity.z) ? __FIXED_MULT(this->velocity.z, elapsedTime) : 0;

	return displacement;
}

MovementResult Body::getMovementResult(Vector3D previousVelocity, Acceleration gravity)
{
	MovementResult movementResult = {__NO_AXIS, __NO_AXIS, __NO_AXIS, __NO_AXIS};

	Vector3D aux =
	{
		this->velocity.x ^ previousVelocity.x,
		this->velocity.y ^ previousVelocity.y,
		this->velocity.z ^ previousVelocity.z,
	};

	// xor values, if result != 0, there is movement
	movementResult.axisOfChangeOfMovement |= aux.x ? __X_AXIS : __NO_AXIS;
	movementResult.axisOfChangeOfMovement |= aux.y ? __Y_AXIS : __NO_AXIS;
	movementResult.axisOfChangeOfMovement |= aux.z ? __Z_AXIS : __NO_AXIS;

	// xor values, if result >= 0, there is no change in direction
	movementResult.axisOfChangeOfDirection |= 0 <= aux.x ? __NO_AXIS : __X_AXIS;
	movementResult.axisOfChangeOfDirection |= 0 <= aux.y ? __NO_AXIS : __Y_AXIS;
	movementResult.axisOfChangeOfDirection |= 0 <= aux.z ? __NO_AXIS : __Z_AXIS;

	if(!this->movesIndependentlyOnEachAxis && (__ACCELERATED_MOVEMENT == (this->movementType.x | this->movementType.y | this->movementType.z)))
	{
		if(this->speed < __STOP_VELOCITY_THRESHOLD && !this->externalForce.x && !this->externalForce.y && !this->externalForce.z && !gravity.x && !gravity.y && !gravity.z)
		{
			movementResult.axisStoppedMovement = __ALL_AXIS;
		}
	
		return movementResult;
	}

	// stop if no external force or opposing normal force is present
	// and if the velocity minimum threshold is not reached
	if(previousVelocity.x && !this->externalForce.x && !gravity.x && __ACCELERATED_MOVEMENT == this->movementType.x)
	{
		if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.x) || (0 == this->externalForce.x && !this->accelerating.x) || (__X_AXIS & movementResult.axisOfChangeOfDirection))
		{
			movementResult.axisStoppedMovement |= __X_AXIS;
		}
	}

	if(previousVelocity.y && !this->externalForce.y && !gravity.y && __ACCELERATED_MOVEMENT == this->movementType.y)
	{
		if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.y) || (0 == this->externalForce.y && !this->accelerating.y) || (__Y_AXIS & movementResult.axisOfChangeOfDirection))
		{
			movementResult.axisStoppedMovement |= __Y_AXIS;
		}
	}

	if(previousVelocity.z && !this->externalForce.z && !gravity.z && __ACCELERATED_MOVEMENT == this->movementType.z)
	{
		if(__STOP_VELOCITY_THRESHOLD > __ABS(this->velocity.z) || (0 == this->externalForce.z && !this->accelerating.z) || (__Z_AXIS & movementResult.axisOfChangeOfDirection))
		{
			movementResult.axisStoppedMovement |= __Z_AXIS;
		}
	}

	// cannot change direction if movement stopped on that axis
	movementResult.axisOfChangeOfMovement &= ~movementResult.axisStoppedMovement;

	return movementResult;
}

Force Body::getFriction()
{
	return this->friction;
}

Acceleration Body::getGravity()
{
	return (Acceleration)
	{
		__X_AXIS & this->axisSubjectToGravity ? _currentGravity->x : 0,
		__Y_AXIS & this->axisSubjectToGravity ? _currentGravity->y : 0,
		__Z_AXIS & this->axisSubjectToGravity ? _currentGravity->z : 0,
	};
}


static inline fix7_9_ext Body::doComputeInstantaneousSpeed(fixed_t forceMagnitude, fixed_t gravity, fixed_t mass, fixed_t friction, fixed_t maximumSpeed)
{
	fix7_9_ext acceleration = __FIXED_TO_FIX7_9_EXT(gravity) + __FIX7_9_EXT_DIV(__FIXED_TO_FIX7_9_EXT(forceMagnitude + friction), __FIXED_TO_FIX7_9_EXT(mass));

	fix7_9_ext initialSpeed = __FIX7_9_EXT_MULT(acceleration, _currentPhysicsElapsedTime);

	return 0 != maximumSpeed && __FIXED_TO_FIX7_9_EXT(maximumSpeed) < __ABS(initialSpeed) ? __FIXED_TO_FIX7_9_EXT(maximumSpeed) : initialSpeed;
}

static inline fixed_t Body::computeInstantaneousSpeed(fixed_t forceMagnitude, fixed_t gravity, fixed_t mass, fixed_t friction, fixed_t maximumSpeed)
{
	return __FIX7_9_EXT_TO_FIXED(Body::doComputeInstantaneousSpeed(forceMagnitude, gravity, mass, friction, maximumSpeed));
}

void Body::computeDirectionAndSpeed(bool useExternalForceForDirection)
{
	this->speed = Vector3D::length(this->velocity);

	if(useExternalForceForDirection)
	{
		this->direction = Vector3D::normalize(this->externalForce);
		this->velocity = Vector3D::scalarProduct(this->direction, this->speed);
		this->changedDirection = true;
	}
	else
	{
		Vector3D newDirection = Vector3D::scalarDivision(this->velocity, this->speed);
	
		this->changedDirection = this->direction.x != newDirection.x || this->direction.y != newDirection.y || this->direction.z != newDirection.z;

		if(this->changedDirection)
		{
			this->direction = newDirection;
		}
	}
}

void Body::clampVelocity(bool useExternalForceForDirection)
{
	Body::computeDirectionAndSpeed(this, useExternalForceForDirection);

	// First check if must clamp speed
	if(this->maximumSpeed && this->maximumSpeed < this->speed)
	{
		this->speed = this->maximumSpeed;

		this->velocity = Vector3D::scalarProduct(this->direction, this->maximumSpeed);

		this->internalVelocity.x = __FIXED_TO_FIX7_9_EXT(this->velocity.x);
		this->internalVelocity.y = __FIXED_TO_FIX7_9_EXT(this->velocity.y);
		this->internalVelocity.z = __FIXED_TO_FIX7_9_EXT(this->velocity.z);
	}

	// Then clamp speed based on each axis configuration
	if(this->maximumVelocity.x)
	{
		if(__ABS(this->maximumVelocity.x) < __ABS(this->velocity.x))
		{
			int32 sign = 0 <= this->velocity.x ? 1 : -1;

			this->velocity.x = (this->maximumVelocity.x * sign);
			this->internalVelocity.x = __FIXED_TO_FIX7_9_EXT(this->velocity.x);
		}
	}

	if(this->maximumVelocity.y)
	{
		if(__ABS(this->maximumVelocity.y) < __ABS(this->velocity.y))
		{
			int32 sign = 0 <= this->velocity.y ? 1 : -1;

			this->velocity.y = (this->maximumVelocity.y * sign);
			this->internalVelocity.y = __FIXED_TO_FIX7_9_EXT(this->velocity.y);
		}
	}

	if(this->maximumVelocity.z)
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
MovementResult Body::updateMovement()
{
	Acceleration gravity = Body::getGravity(this);
	this->weight = Vector3D::scalarProduct(gravity, this->mass);

	// yeah, * 4 (<< 2) is a magical number, but it works well enough with the range of mass and friction coefficient
	this->friction = Vector3D::scalarProduct(this->direction, -__FIXED_MULT(this->frictionForceMagnitude, __I_TO_FIXED(1 << __FRICTION_FORCE_FACTOR_POWER)));

	fix7_9_ext elapsedTime = _currentPhysicsElapsedTime;

	Velocity previousVelocity = this->velocity;

	if(__ACCELERATED_MOVEMENT == this->movementType.x)
	{
		fix7_9_ext instantaneousSpeed = Body::doComputeInstantaneousSpeed(this->externalForce.x + this->totalNormal.x, gravity.x, this->mass, this->friction.x, 0);

		this->accelerating.x = 0 != instantaneousSpeed;
		this->internalVelocity.x += instantaneousSpeed;
		this->velocity.x = __FIX7_9_EXT_TO_FIXED(this->internalVelocity.x);
	}
	else if(__UNIFORM_MOVEMENT == this->movementType.x)
	{
		if(__ABS(this->velocity.x) < __PIXELS_TO_METERS((1 << __PIXELS_PER_METER_2_POWER)))
		{
			if(0 < __ABS(__METERS_TO_PIXELS(this->velocity.x)) && 0 == FrameRate::getFps(FrameRate::getInstance()) % (__TARGET_FPS / __METERS_TO_PIXELS(this->velocity.x)))
			{
				this->position.x += 0 <= this->velocity.x ? __PIXELS_TO_METERS(1) : -__PIXELS_TO_METERS(1);
			}
		}
		else
		{
			this->position.x += __FIXED_MULT(this->velocity.x, elapsedTime);
		}
	}

	if(__ACCELERATED_MOVEMENT == this->movementType.y)
	{
		fix7_9_ext instantaneousSpeed = Body::doComputeInstantaneousSpeed(this->externalForce.y + this->totalNormal.y, gravity.y, this->mass, this->friction.y, 0);

		this->accelerating.y = 0 != instantaneousSpeed;
		this->internalVelocity.y += instantaneousSpeed;
		this->velocity.y = __FIX7_9_EXT_TO_FIXED(this->internalVelocity.y);
	}
	else if(__UNIFORM_MOVEMENT == this->movementType.y)
	{
		if(__ABS(this->velocity.y) < __PIXELS_TO_METERS((1 << __PIXELS_PER_METER_2_POWER)))
		{
			if(0 < __ABS(__METERS_TO_PIXELS(this->velocity.y)) && 0 == FrameRate::getFps(FrameRate::getInstance()) % (__TARGET_FPS / __METERS_TO_PIXELS(this->velocity.y)))
			{
				this->position.y += 0 <= this->velocity.y ? __PIXELS_TO_METERS(1) : -__PIXELS_TO_METERS(1);
			}
		}
		else
		{
			this->position.y += __FIXED_MULT(this->velocity.y, elapsedTime);
		}
	}

	if(__ACCELERATED_MOVEMENT == this->movementType.z)
	{
		fix7_9_ext instantaneousSpeed = Body::doComputeInstantaneousSpeed(this->externalForce.z + this->totalNormal.z, gravity.z, this->mass, this->friction.z, 0);

		this->accelerating.z = 0 != instantaneousSpeed;
		this->internalVelocity.z += instantaneousSpeed;
		this->velocity.z = __FIX7_9_EXT_TO_FIXED(this->internalVelocity.z);
	}
	else if(__UNIFORM_MOVEMENT == this->movementType.z)
	{
		if(__ABS(this->velocity.z) < __PIXELS_TO_METERS((1 << __PIXELS_PER_METER_2_POWER)))
		{
			if(0 < __ABS(__METERS_TO_PIXELS(this->velocity.z)) && 0 == FrameRate::getFps(FrameRate::getInstance()) % (__TARGET_FPS / __METERS_TO_PIXELS(this->velocity.z)))
			{
				this->position.z += 0 <= this->velocity.z ? __PIXELS_TO_METERS(1) : -__PIXELS_TO_METERS(1);
			}
		}
		else
		{
			this->position.z += __FIXED_MULT(this->velocity.z, elapsedTime);
		}
	}

	Body::clampVelocity(this, 0 == previousVelocity.x && 0 == previousVelocity.y && 0 == previousVelocity.z);

	if(__ACCELERATED_MOVEMENT == this->movementType.x)
	{
		this->internalPosition.x += __FIX7_9_EXT_MULT(this->internalVelocity.x, elapsedTime);
		this->position.x = __FIX7_9_EXT_TO_FIXED(this->internalPosition.x);
	}

	if(__ACCELERATED_MOVEMENT == this->movementType.y)
	{
		this->internalPosition.y += __FIX7_9_EXT_MULT(this->internalVelocity.y, elapsedTime);
		this->position.y = __FIX7_9_EXT_TO_FIXED(this->internalPosition.y);
	}

	if(__ACCELERATED_MOVEMENT == this->movementType.z)
	{
		this->internalPosition.z += __FIX7_9_EXT_MULT(this->internalVelocity.z, elapsedTime);
		this->position.z = __FIX7_9_EXT_TO_FIXED(this->internalPosition.z);
	}

	return Body::getMovementResult(this, previousVelocity, gravity);
}

bool Body::changedDirection()
{
	return this->changedDirection;
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
		this->changedDirection = false;
		Body::sleep(this);
	}

	return axisOfStopping;
}

// get axis subject to gravity
uint16 Body::getaxisSubjectToGravity()
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

void Body::computeTotalNormal()
{
	this->totalNormal = Vector3D::zero();

	if(this->normals)
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

	Body::computeTotalFrictionCoefficient(this);

	Body::computeFrictionForceMagnitude(this);
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

Force Body::getNormal()
{
	return this->totalNormal;
}

Force Body::getLastNormalDirection()
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
		VirtualNode node = this->normals->head;

		for(; NULL != node; node = node->next)
		{
			delete node->data;
		}

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
	this->weight 				= Vector3D::scalarProduct(*_currentGravity, this->mass);
}

void Body::clearNormalOnAxis(uint16 axis __attribute__ ((unused)))
{
	if(this->normals && !isDeleted(this->normals->head))
	{
		for(VirtualNode node = this->normals->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			NormalRegistry* normalRegistry = (NormalRegistry*)node->data;

			if(isDeleted(normalRegistry->referent) ||
				((__X_AXIS & axis) && normalRegistry->direction.x) ||
				((__Y_AXIS & axis) && normalRegistry->direction.y) ||
				((__Z_AXIS & axis) && normalRegistry->direction.z)
			)
			{
				VirtualList::removeNode(this->normals, node);
				delete node->data;
			}
		}
	}

	Body::computeTotalNormal(this);
}

void Body::clearNormal(ListenerObject referent)
{
	ASSERT(!isDeleted(referent), "Body::clearNormal: dead referent");

	if(this->normals)
	{
		for(VirtualNode node = this->normals->head; NULL != node; node = node->next)
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
	}

	Body::computeTotalNormal(this);
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

void Body::computeFrictionForceMagnitude()
{
	if(0 == this->frictionCoefficient)
	{
		this->frictionForceMagnitude = 0;
		return;
	}

	if(this->totalNormal.x || this->totalNormal.y || this->totalNormal.z)
	{
		this->frictionForceMagnitude = __ABS(__FIXED_MULT(Vector3D::length(this->totalNormal), this->totalFrictionCoefficient));
	}
	else
	{
		Acceleration gravity = Body::getGravity(this);
		this->weight = Vector3D::scalarProduct(gravity, this->mass);
		fixed_t weight = Vector3D::length(this->weight);

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
	this->mass = __MIN_MASS < mass ? __MAX_MASS > mass ? mass : __MAX_MASS : __MIN_MASS;
}

// retrieve state
bool Body::isAwake()
{
	return this->awake && this->active;
}

// awake body
void Body::awake(uint16 axisOfAwakening)
{
	bool dispatchMessage = false;

	this->awake = true;
	this->active = true;

	if(this->sendMessages)
	{
		if(!this->velocity.x && (__X_AXIS & axisOfAwakening))
		{
			dispatchMessage |= (__X_AXIS & axisOfAwakening);
		}

		if(!this->velocity.y && (__Y_AXIS & axisOfAwakening))
		{
			dispatchMessage |= (__Y_AXIS & axisOfAwakening);
		}

		if(!this->velocity.z && (__Z_AXIS & axisOfAwakening))
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
	uint16 result = 0;

	result |= (this->velocity.x || this->accelerating.x) ? __X_AXIS : 0;
	result |= (this->velocity.y || this->accelerating.y) ? __Y_AXIS : 0;
	result |= (this->velocity.z || this->accelerating.z) ? __Z_AXIS : 0;

	return this->awake && this->active ? result : 0;
}

MovementResult Body::getBouncingResult(Vector3D previousVelocity, Vector3D bouncingPlaneNormal)
{
	MovementResult movementResult = {__NO_AXIS, __NO_AXIS, __NO_AXIS, __NO_AXIS};

	Vector3D aux =
	{
		this->velocity.x ^ previousVelocity.x,
		this->velocity.y ^ previousVelocity.y,
		this->velocity.z ^ previousVelocity.z,
	};

	// xor values, if result != 0, there is movement
	movementResult.axisOfChangeOfMovement |= aux.x ? __X_AXIS : __NO_AXIS;
	movementResult.axisOfChangeOfMovement |= aux.y ? __Y_AXIS : __NO_AXIS;
	movementResult.axisOfChangeOfMovement |= aux.z ? __Z_AXIS : __NO_AXIS;

	// xor values, if result >= 0, there is no change in direction
	movementResult.axisOfChangeOfDirection |= 0 <= aux.x ? __NO_AXIS : __X_AXIS;
	movementResult.axisOfChangeOfDirection |= 0 <= aux.y ? __NO_AXIS : __Y_AXIS;
	movementResult.axisOfChangeOfDirection |= 0 <= aux.z ? __NO_AXIS : __Z_AXIS;

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
	if(__UNIFORM_MOVEMENT != this->movementType.x)
	{
		movementResult.axisOfAcceleratedBouncing |= __X_AXIS & movementResult.axisOfChangeOfMovement;
	}

	if(__UNIFORM_MOVEMENT != this->movementType.y)
	{
		movementResult.axisOfAcceleratedBouncing |= __Y_AXIS & movementResult.axisOfChangeOfMovement;
	}

	if(__UNIFORM_MOVEMENT != this->movementType.z)
	{
		movementResult.axisOfAcceleratedBouncing |= __Z_AXIS & movementResult.axisOfChangeOfMovement;
	}

	// don't bounce if movement stopped on that axis
	movementResult.axisOfAcceleratedBouncing &= ~movementResult.axisStoppedMovement;

	return movementResult;
}

// bounce back
void Body::bounce(ListenerObject bounceReferent, Vector3D bouncingPlaneNormal, fixed_t frictionCoefficient, fixed_t bounciness)
{
	Acceleration gravity = Body::getGravity(this);

	// set friction
	Body::setSurroundingFrictionCoefficient(this, frictionCoefficient);

	// compute bouncing vector
	fixed_t cosAngle = __I_TO_FIXED(bouncingPlaneNormal.x | bouncingPlaneNormal.y | bouncingPlaneNormal.z) && (gravity.x | gravity.y | gravity.z) ? __ABS(__FIXED_EXT_DIV(Vector3D::dotProduct(gravity, bouncingPlaneNormal), Vector3D::lengthProduct(gravity, bouncingPlaneNormal))) : __1I_FIXED;
	fixed_t normalMagnitude = __FIXED_EXT_MULT(Vector3D::length(this->weight), cosAngle);

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

	Body::clampVelocity(this, false);

	if(__NO_MOVEMENT == this->movementType.x && this->velocity.x)
	{
		this->movementType.x = __ACCELERATED_MOVEMENT;
	}

	if(__NO_MOVEMENT == this->movementType.y && this->velocity.y)
	{
		this->movementType.y = __ACCELERATED_MOVEMENT;
	}

	if(__NO_MOVEMENT == this->movementType.z && this->velocity.z)
	{
		this->movementType.z = __ACCELERATED_MOVEMENT;
	}

	// determine bouncing result
	MovementResult movementResult = Body::getBouncingResult(this, velocity, bouncingPlaneNormal);

	// stop over the axis where there is no bouncing
	if(movementResult.axisStoppedMovement)
	{
		uint16 axisOfStopping = Body::stopMovement(this, movementResult.axisStoppedMovement);

		if(axisOfStopping && this->sendMessages)
		{
			MessageDispatcher::dispatchMessage(0, ListenerObject::safeCast(this), ListenerObject::safeCast(this->owner), kMessageBodyStopped, &axisOfStopping);
		}
	}

	if(movementResult.axisOfAcceleratedBouncing)
	{
	//	Body::setSurroundingFrictionCoefficient(this, 0);
		Body::clearNormalOnAxis(this, movementResult.axisOfAcceleratedBouncing);
	}

	if(!Body::getMovementOnAllAxis(this))
	{
		Body::sleep(this);
	}
}

void Body::setMaximumVelocity(Velocity maximumVelocity)
{
	this->maximumVelocity = maximumVelocity;
}

Velocity Body::getMaximumVelocity()
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
	Printing::text(Printing::getInstance(), "Active:", x, y, NULL);
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

	Printing::text(Printing::getInstance(), "Weight", x, y, NULL);
	Printing::text(Printing::getInstance(), "                             ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->weight.x), xDisplacement + x, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->weight.y), xDisplacement + x + 8, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(this->weight.z), xDisplacement + x + 8 * 2, y++, 2, NULL);

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

	Printing::text(Printing::getInstance(), "Direction", x, y, NULL);
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

	Acceleration gravity = Body::getGravity(this);

	Printing::text(Printing::getInstance(), "Gravity", x, y, NULL);
	Printing::text(Printing::getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(gravity.x), xDisplacement + x, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(gravity.y), xDisplacement + x + 8, y, 2, NULL);
	Printing::float(Printing::getInstance(), __FIXED_TO_F(gravity.z), xDisplacement + x + 8 * 2, y++, 2, NULL);

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
