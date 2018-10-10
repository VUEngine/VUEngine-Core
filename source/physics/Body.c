/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <PhysicalWorld.h>
#include <MessageDispatcher.h>
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

//#define __STOP_VELOCITY_THRESHOLD				__F_TO_FIX10_6(0.9f)
#define __STOP_VELOCITY_THRESHOLD				__PIXELS_TO_METERS(8)
#define __STOP_BOUNCING_VELOCITY_THRESHOLD 		__PIXELS_TO_METERS(48)

#define __MIN_MASS								__F_TO_FIX10_6(0.1f)
#define __MAX_MASS								__I_TO_FIX10_6(1)
#define __MAXIMUM_FRICTION_COEFFICIENT			__I_TO_FIX10_6(1)

#ifndef __FRICTION_FORCE_FACTOR_POWER
#define __FRICTION_FORCE_FACTOR_POWER					2
#endif
//---------------------------------------------------------------------------------------------------------
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

fix10_6 _currentWorldFriction = 0;
fix10_6 _currentElapsedTime = 0;
const Acceleration* _currentGravity = 0;

static void Body::setCurrentWorldFrictionCoefficient(fix10_6 currentWorldFriction)
{
	_currentWorldFriction = currentWorldFriction;
}

static void Body::setCurrentElapsedTime(fix10_6 currentElapsedTime)
{
	_currentElapsedTime = currentElapsedTime;
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
	Object referent;
	Vector3D direction;
	fix10_6 magnitude;

} NormalRegistry;

Clock _physhicsClock = NULL;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Body::constructor(SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axesSubjectToGravity)
{
	Base::constructor();

	this->owner = owner;
	this->normals = NULL;
	this->mass = __MIN_MASS < physicalSpecification->mass ? __MAX_MASS > physicalSpecification->mass ? physicalSpecification->mass : __MAX_MASS : __MIN_MASS;
	this->bounciness = physicalSpecification->bounciness;
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
	this->weight 				= Vector3D::scalarProduct(*_currentGravity, this->mass);
	this->maximumVelocity 		= physicalSpecification->maximumVelocity;

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

		for(; node; node = node->next)
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

// retrieve character's velocity
Velocity Body::getVelocity()
{
	return this->velocity;
}

void Body::modifyVelocity(const Velocity* modifier)
{
	ASSERT(modifier, "Body::modifyVelocity: null multiplier");

	this->velocity.x += modifier->x;
	this->velocity.y += modifier->y;
	this->velocity.z += modifier->z;

	Body::capVelocity(this);
}

// retrieve acceleration
Acceleration Body::getAcceleration()
{
	return this->acceleration;
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
void Body::setMovementType(int movementType, u16 axes)
{
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

void Body::clearAcceleration(u16 axes)
{
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
void Body::moveAccelerated(u16 axes)
{
	Body::setMovementType(this, __ACCELERATED_MOVEMENT, axes);
}

// set movement type to uniform
void Body::moveUniformly(Velocity velocity)
{
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
		Body::setMovementType(this, __UNIFORM_MOVEMENT, axesOfUniformMovement);
		Body::awake(this, axesOfUniformMovement);
	}
}

// clear force
void Body::clearExternalForce()
{
	this->externalForce.x = 0;
	this->externalForce.y = 0;
	this->externalForce.z = 0;
}

// apply force
void Body::applyForce(const Force* force)
{
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
			//Body::clearNormalOnAxes(this, axesOfExternalForce);
			Body::setMovementType(this, __ACCELERATED_MOVEMENT, axesOfExternalForce);
			Body::awake(this, axesOfExternalForce);
		}
	}
}

// apply gravity
void Body::applyGravity(u16 axes)
{
	if(axes)
	{
		Acceleration gravityForce = Vector3D::scalarProduct(Body::getGravity(this), this->mass);

		Force force =
		{
			__X_AXIS & axes ? gravityForce.x : 0,
			__Y_AXIS & axes ? gravityForce.y : 0,
			__Z_AXIS & axes ? gravityForce.z : 0,
		};

		Body::applyForce(this, &force);
	}
}

// add force
void Body::addForce(const Force* force)
{
	ASSERT(force, "Body::addForce: null force");

	Body::applyForce(this, force);
}

// update movement
void Body::update()
{
	if(this->active)
	{
		if(this->awake)
		{
			MovementResult movementResult = Body::updateMovement(this);

			// if stopped on any axes
			if(movementResult.axesStoppedMovement)
			{
				Body::stopMovement(this, movementResult.axesStoppedMovement);

				if(movementResult.axesStoppedMovement)
				{
					MessageDispatcher::dispatchMessage(0, Object::safeCast(this), Object::safeCast(this->owner), kBodyStopped, &movementResult.axesStoppedMovement);
				}
			}

			// no one uses this
/*			if(movementResult.axesOfChangeOfMovement)
			{
				MessageDispatcher::dispatchMessage(0, Object::safeCast(this), Object::safeCast(this->owner), kBodyChangedDirection, &movementResult.axesOfChangeOfMovement);
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

	fix10_6 elapsedTime = PhysicalWorld::getElapsedTime(Game::getPhysicalWorld(Game::getInstance()));

	displacement.x = __STOP_VELOCITY_THRESHOLD < __ABS(this->velocity.x) ? __FIX10_6_MULT(this->velocity.x, elapsedTime) : 0;
	displacement.y = __STOP_VELOCITY_THRESHOLD < __ABS(this->velocity.y) ? __FIX10_6_MULT(this->velocity.y, elapsedTime) : 0;
	displacement.z = __STOP_VELOCITY_THRESHOLD < __ABS(this->velocity.z) ? __FIX10_6_MULT(this->velocity.z, elapsedTime) : 0;

	return displacement;
}

MovementResult Body::getMovementResult(Vector3D previousVelocity)
{
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
	if(previousVelocity.x && __UNIFORM_MOVEMENT != this->movementType.x && (this->velocity.x | previousVelocity.x))
//	if(previousVelocity.x && !this->externalForce.x && __UNIFORM_MOVEMENT != this->movementType.x && (this->velocity.x | previousVelocity.x))
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

	if(previousVelocity.y && __UNIFORM_MOVEMENT != this->movementType.y && (this->velocity.y | previousVelocity.y))
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

	if(previousVelocity.z && __UNIFORM_MOVEMENT != this->movementType.z && (this->velocity.z | previousVelocity.z))
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

Acceleration Body::getGravity()
{
	return (Acceleration)
	{
		__X_AXIS & this->axesSubjectToGravity ? _currentGravity->x : 0,
		__Y_AXIS & this->axesSubjectToGravity ? _currentGravity->y : 0,
		__Z_AXIS & this->axesSubjectToGravity ? _currentGravity->z : 0,
	};
}

void Body::capVelocity()
{
	if(this->maximumVelocity.x)
	{
		if(__ABS(this->maximumVelocity.x) < __ABS(this->velocity.x))
		{
			int sign = 0 <= this->velocity.x ? 1 : -1;

			this->velocity.x = (this->maximumVelocity.x * sign);
		}
	}

	if(this->maximumVelocity.y)
	{
		if(__ABS(this->maximumVelocity.y) < __ABS(this->velocity.y))
		{
			int sign = 0 <= this->velocity.y ? 1 : -1;

			this->velocity.y = (this->maximumVelocity.y * sign);
		}
	}

	if(this->maximumVelocity.z)
	{
		if(__ABS(this->maximumVelocity.z) < __ABS(this->velocity.z))
		{
			int sign = 0 <= this->velocity.z ? 1 : -1;

			this->velocity.z = (this->maximumVelocity.z * sign);
		}
	}
}

// update movement over axes
MovementResult Body::updateMovement()
{
	Acceleration gravity = Body::getGravity(this);
	this->weight = Vector3D::scalarProduct(gravity, this->mass);

#ifndef __USE_HACK_FOR_FRICTION
	// yeah, * 4 (<< 2) is a magical number, but it works well enough with the range of mass and friction coefficient
	this->friction = Vector3D::scalarProduct(Vector3D::normalize(this->velocity), -(this->frictionForceMagnitude << __FRICTION_FORCE_FACTOR_POWER));
#else
	// hack to avoid normalization
	this->friction = Vector3D::scalarProduct(this->velocity, -(this->totalFrictionCoefficient << __FRICTION_FORCE_FACTOR_POWER));
#endif

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

	Body::capVelocity(this);

	return Body::getMovementResult(this, previousVelocity);
}

// stop movement over an axes
u16 Body::stopMovement(u16 axes)
{
	u16 axesOfMovement = Body::getMovementOnAllAxes(this);
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

	Body::setMovementType(this, __NO_MOVEMENT, axesOfStopping);

	if(!Body::getMovementOnAllAxes(this))
	{
		Body::sleep(this);
	}

	return axesOfStopping;
}

// get axes subject to gravity
u16 Body::getAxesSubjectToGravity()
{
	return this->axesSubjectToGravity;
}

// set axes subject to gravity
void Body::setAxesSubjectToGravity(u16 axesSubjectToGravity)
{
	this->axesSubjectToGravity = axesSubjectToGravity;
}

// set active
void Body::setActive(bool active)
{
	this->active = active;

	PhysicalWorld::bodySetInactive(Game::getPhysicalWorld(Game::getInstance()), this);
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
	}
}

// get bounciness
fix10_6 Body::getBounciness()
{
	return this->bounciness;
}

// set bounciness
void Body::setBounciness(fix10_6 bounciness)
{
	if(__I_TO_FIX10_6(0) > bounciness)
	{
		bounciness = 0;
	}
	else if(__F_TO_FIX10_6(__MAXIMUM_BOUNCINESS_COEFFICIENT) < bounciness)
	{
		bounciness = __F_TO_FIX10_6(__MAXIMUM_BOUNCINESS_COEFFICIENT);
	}

	this->bounciness = bounciness;
}

void Body::computeTotalNormal()
{
	this->totalNormal = (Force){0, 0, 0};

	if(this->normals)
	{
		VirtualNode node = this->normals->head;

		for(; node; node = node->next)
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

	if(this->totalNormal.x || this->totalNormal.y || this->totalNormal.z)
	{
		this->frictionForceMagnitude = __ABS(__FIX10_6_MULT(Vector3D::length(this->totalNormal), this->totalFrictionCoefficient));
	}
	else
	{
		this->frictionForceMagnitude = __ABS(__FIX10_6_MULT(Vector3D::length(this->weight), _currentWorldFriction) >> __FRICTION_FORCE_FACTOR_POWER);
	}
}

void Body::addNormal(Object referent, Vector3D direction, fix10_6 magnitude)
{
	ASSERT(referent, "Body::addNormal: null referent");

	if(!this->normals)
	{
		this->normals = new VirtualList();
	}

	VirtualNode node = this->normals->head;

	for(; node; node = node->next)
	{
		ASSERT(!isDeleted(node->data), "DEAD");
	//	ASSERT(((NormalRegistry*)node->data)->referent != referent, "ERRR");
	}

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
		return (Force){0, 0, 0};
	}

	while(this->normals->head && isDeleted(((NormalRegistry*)VirtualList::back(this->normals))->referent))
	{
		ASSERT(!isDeleted(VirtualList::back(this->normals)), "Body::getLastNormalDirection: dead normal registry");

		delete VirtualList::popBack(this->normals);
	}

	Body::computeTotalNormal(this);

	if(!this->normals->head)
	{
		return (Force){0, 0, 0};
	}

	return ((NormalRegistry*)VirtualList::back(this->normals))->direction;
}

void Body::reset()
{
	if(this->normals)
	{
		VirtualNode node = this->normals->head;

		for(; node; node = node->next)
		{
			delete node->data;
		}

		delete this->normals;
		this->normals = NULL;
	}

	Body::computeTotalNormal(this);
	Body::setSurroundingFrictionCoefficient(this, 0);

	this->velocity 				= (Velocity){0, 0, 0};
	this->acceleration 			= (Acceleration){0, 0, 0};
	this->externalForce	 		= (Force){0, 0, 0};
	this->friction 				= (Force){0, 0, 0};
	this->totalNormal			= (Force){0, 0, 0};
	this->weight 				= Vector3D::scalarProduct(*_currentGravity, this->mass);
}

/*
void Body::clearNormalOnAxes(u16 axes __attribute__ ((unused))) __attribute__ ((unused))
{
	if(this->normals)
	{
		VirtualList normalsToRemove = new VirtualList();

		VirtualNode node = this->normals->head;

		for(; node; node = node->next)
		{
			NormalRegistry* normalRegistry = (NormalRegistry*)node->data;

			if(isDeleted(normalRegistry->referent) ||
				((__X_AXIS & axes) && normalRegistry->direction.x) ||
				((__Y_AXIS & axes) && normalRegistry->direction.y) ||
				((__Z_AXIS & axes) && normalRegistry->direction.z)
			)
			{
				VirtualList::pushBack(normalsToRemove, normalRegistry);
			}
		}

		node = normalsToRemove->head;

		for(; node; node = node->next)
		{
			VirtualList::removeElement(this->normals, node->data);

			if(!isDeleted(node->data))
			{
			delete node->data;
			}
			else
			{
				Printing::text(Printing::getInstance(), __GET_CLASS_NAME(this->owner), 20, 1, NULL);
				while(1);
			}

		}

		delete normalsToRemove;
	}

	Body::computeTotalNormal(this);
}
*/

void Body::clearNormal(Object referent)
{
	ASSERT(!isDeleted(referent), "Body::clearNormal: dead referent");

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
				ASSERT(!isDeleted(normalRegistry), "Body::clearNormal: dead normal registry");
				VirtualList::removeElement(this->normals, normalRegistry);
				delete normalRegistry;
				break;
			}
		}
	}

	Body::computeTotalNormal(this);
}

fix10_6 Body::getFrictionCoefficient()
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

// set friction
void Body::setFrictionCoefficient(fix10_6 frictionCoefficient)
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

void Body::setSurroundingFrictionCoefficient(fix10_6 surroundingFrictionCoefficient)
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

fix10_6 Body::getMass()
{
	return this->mass;
}

void Body::setMass(fix10_6 mass)
{
	this->mass = __MIN_MASS < mass ? __MAX_MASS > mass ? mass : __MAX_MASS : __MIN_MASS;
}

// retrieve state
bool Body::isAwake()
{
	return this->awake && this->active;
}

// awake body
void Body::awake(u16 axesOfAwakening)
{
	bool dispatchMessage = false;

	if(!this->awake)
	{
		this->awake = true;

		PhysicalWorld::bodyAwake(Game::getPhysicalWorld(Game::getInstance()), this);
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
		MessageDispatcher::dispatchMessage(0, Object::safeCast(this), Object::safeCast(this->owner), kBodyStartedMoving, &axesOfAwakening);
	}
}

// go to sleep
void Body::sleep()
{
	if(this->awake)
	{
		this->awake = false;

		PhysicalWorld::bodySleep(Game::getPhysicalWorld(Game::getInstance()), this);
	}
}

// is it moving?
u16 Body::getMovementOnAllAxes()
{
	u16 result = 0;

	result |= (this->velocity.x | __FIX10_6_INT_PART(this->acceleration.x)) ? __X_AXIS : 0;
	result |= (this->velocity.y | __FIX10_6_INT_PART(this->acceleration.y)) ? __Y_AXIS : 0;
	result |= (this->velocity.z | __FIX10_6_INT_PART(this->acceleration.z)) ? __Z_AXIS : 0;

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
	movementResult.axesOfChangeOfMovement |= aux.x ? __X_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= aux.y ? __Y_AXIS : __NO_AXIS;
	movementResult.axesOfChangeOfMovement |= aux.z ? __Z_AXIS : __NO_AXIS;

	// xor values, if result >= 0, there is no change in direction
	movementResult.axesOfChangeOfDirection |= 0 <= aux.x ? __NO_AXIS : __X_AXIS;
	movementResult.axesOfChangeOfDirection |= 0 <= aux.y ? __NO_AXIS : __Y_AXIS;
	movementResult.axesOfChangeOfDirection |= 0 <= aux.z ? __NO_AXIS : __Z_AXIS;

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
void Body::bounce(Object bounceReferent, Vector3D bouncingPlaneNormal, fix10_6 frictionCoefficient, fix10_6 bounciness)
{
	Acceleration gravity = Body::getGravity(this);

	// set friction
	Body::setSurroundingFrictionCoefficient(this, frictionCoefficient);

	// compute bouncing vector
	fix10_6 cosAngle = __I_TO_FIX10_6(bouncingPlaneNormal.x | bouncingPlaneNormal.y | bouncingPlaneNormal.z) && (gravity.x | gravity.y | gravity.z) ? __ABS(__FIX10_6_EXT_DIV(Vector3D::dotProduct(gravity, bouncingPlaneNormal), Vector3D::lengthProduct(gravity, bouncingPlaneNormal))) : __1I_FIX10_6;
	fix10_6 normalMagnitude = __FIX10_6_EXT_MULT(Vector3D::length(this->weight), cosAngle);

	// register normal affecting the body
	Body::addNormal(this, bounceReferent, bouncingPlaneNormal, normalMagnitude);

	// compute bouncing vector
	Vector3D velocity = this->velocity;

	// compute bouncing velocity vector
	Vector3D u = Vector3D::scalarProduct(bouncingPlaneNormal, Vector3D::dotProduct(velocity, bouncingPlaneNormal));
	Vector3D w =
	{
		velocity.x - u.x,
		velocity.y - u.y,
		velocity.z - u.z,
	};

	bounciness += this->bounciness;

	if(__I_TO_FIX10_6(0) > bounciness)
	{
		bounciness = 0;
	}
	else if(__F_TO_FIX10_6(__MAXIMUM_BOUNCINESS_COEFFICIENT) < bounciness)
	{
		bounciness = __F_TO_FIX10_6(__MAXIMUM_BOUNCINESS_COEFFICIENT);
	}

	// add bounciness and friction
	u = Vector3D::scalarProduct(u, bounciness);
	w = Vector3D::scalarProduct(w, (__MAXIMUM_FRICTION_COEFFICIENT - this->totalFrictionCoefficient));

	this->velocity.x = w.x - u.x;
	this->velocity.y = w.y - u.y;
	this->velocity.z = w.z - u.z;

	Body::capVelocity(this);

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

	// stop over the axes where there is no bouncing
	if(movementResult.axesStoppedMovement)
	{
		u16 axesOfStopping = Body::stopMovement(this, movementResult.axesStoppedMovement);

		if(axesOfStopping)
		{
			MessageDispatcher::dispatchMessage(0, Object::safeCast(this), Object::safeCast(this->owner), kBodyStopped, &axesOfStopping);
		}
	}

	if(movementResult.axesOfAcceleratedBouncing)
	{
	//	Body::setSurroundingFrictionCoefficient(this, 0);
	//	Body::clearNormalOnAxes(this, movementResult.axesOfAcceleratedBouncing);
	}

	if(!Body::getMovementOnAllAxes(this))
	{
		Body::sleep(this);
	}
}

// take a hit
void Body::takeHitFrom(Body other __attribute__ ((unused)))
{
	//TODO:
}

void Body::setMaximumVelocity(Velocity maximumVelocity)
{
	this->maximumVelocity = maximumVelocity;
}

Velocity Body::getMaximumVelocity()
{
	return this->maximumVelocity;
}

void Body::print(int x, int y)
{
	Printing::text(Printing::getInstance(), "Active:", x, y, NULL);
	Printing::text(Printing::getInstance(), this->active ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 10, y++, NULL);
	Printing::text(Printing::getInstance(), "Awake:", x, y, NULL);
	Printing::text(Printing::getInstance(), this->awake ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 10, y++, NULL);

	Printing::text(Printing::getInstance(), "               X         Y         Z", x, y++, NULL);

	int xDisplacement = 15;

	Printing::text(Printing::getInstance(), "Mov. type", x, y, NULL);
	Printing::text(Printing::getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing::text(Printing::getInstance(), __UNIFORM_MOVEMENT == this->movementType.x ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.x ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.x ? "Accel" : "None", xDisplacement + x, y, NULL);
	Printing::text(Printing::getInstance(), __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.y ? "Accel" : "None", xDisplacement + x + 10, y, NULL);
	Printing::text(Printing::getInstance(), __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.z ? "Accel" : "None", xDisplacement + x + 10 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "Weight", x, y, NULL);
	Printing::text(Printing::getInstance(), "                             ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->weight.x), xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->weight.y), xDisplacement + x + 10, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->weight.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "Position", x, y, NULL);
	Printing::text(Printing::getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->position.x), xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->position.y), xDisplacement + x + 10, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->position.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "Velocity", x, y, NULL);
	Printing::text(Printing::getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->velocity.x), xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->velocity.y), xDisplacement + x + 10, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->velocity.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "Speed", x, y, NULL);
	Printing::text(Printing::getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(Vector3D::length(this->velocity)), xDisplacement + x, y++, NULL);

	Printing::text(Printing::getInstance(), "Acceleration", x, y, NULL);
	Printing::text(Printing::getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->acceleration.x), xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->acceleration.y), xDisplacement + x + 10, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->acceleration.z), xDisplacement + x + 10 * 2, y++, NULL);

	Acceleration gravity = Body::getGravity(this);

	Printing::text(Printing::getInstance(), "Gravity", x, y, NULL);
	Printing::text(Printing::getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(gravity.x), xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(gravity.y), xDisplacement + x + 10, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(gravity.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "External Force", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->externalForce.x), xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->externalForce.y), xDisplacement + x + 10, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->externalForce.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "Normal", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->totalNormal.x), xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->totalNormal.y), xDisplacement + x + 10, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->totalNormal.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "Normal Force", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->frictionForceMagnitude), xDisplacement + x, y++, NULL);

	Printing::text(Printing::getInstance(), "Normals", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::int(Printing::getInstance(), this->normals ? VirtualList::getSize(this->normals) : 0, xDisplacement + x, y++, NULL);

	Printing::text(Printing::getInstance(), "Friction", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->friction.x), xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->friction.y), xDisplacement + x + 10, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->friction.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing::text(Printing::getInstance(), "Total friction coef.", x, y, NULL);
	Printing::text(Printing::getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(this->totalFrictionCoefficient), xDisplacement + x, y++, NULL);


	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(__STOP_VELOCITY_THRESHOLD), xDisplacement + x, y, NULL);
	Printing::float(Printing::getInstance(), __FIX10_6_TO_F(__STOP_BOUNCING_VELOCITY_THRESHOLD), 10 +xDisplacement + x, y, NULL);

}
