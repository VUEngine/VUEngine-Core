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
#include <Vector.h>
#include <PhysicalWorld.h>
#include <MessageDispatcher.h>
#include <Printing.h>
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


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// this should be improved and calculated dynamically based on framerate

#define STOPPED_MOVING		0
#define STILL_MOVES			1
#define CHANGED_DIRECTION	2

#define THRESHOLD 			__F_TO_FIX19_13(0.1f)


//---------------------------------------------------------------------------------------------------------
//												CLASS' METHODS
//---------------------------------------------------------------------------------------------------------

fix19_13 _currentWorldFriction = 0;
fix19_13 _currentElapsedTime = 0;
const Acceleration* _currentGravity = 0;

void Body_setCurrentWorldFriction(fix19_13 currentWorldFriction)
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

//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Body_awake(Body this, u16 axisOfAwakening, bool informAboutAwakening);
static void Body_updateAcceleration(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* acceleration, fix19_13 appliedForce, fix19_13 friction);
static void Body_setMovementType(Body this, int movementType, u16 axis);
int Body_updateMovement(Body this, fix19_13 gravity, fix19_13* position, fix19_13* velocity, fix19_13* acceleration, fix19_13 appliedForce, int movementType, fix19_13 friction);

enum CollidingObjectIndexes
{
	eXAxis = 0,
	eYAxis,
	eZAxis,
	eLastCollidingObject,
};

Clock _physhicsClock = NULL;
//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Body, SpatialObject owner, const PhysicalSpecification* physicalSpecification)
__CLASS_NEW_END(Body, owner, physicalSpecification);

// class's constructor
void Body_constructor(Body this, SpatialObject owner, const PhysicalSpecification* physicalSpecification)
{
	ASSERT(this, "Body::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->owner = owner;
	this->mass = physicalSpecification->mass;
	this->elasticity = physicalSpecification->elasticity;
	this->friction = physicalSpecification->friction;

	this->awake = false;
	this->axisSubjectToGravity = __X_AXIS | __Y_AXIS | __Z_AXIS;

	// set position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;

	this->appliedForce.x = 0;
	this->appliedForce.y = 0;
	this->appliedForce.z = 0;

	// clear movement type
	this->movementType.x = __NO_MOVEMENT;
	this->movementType.y = __NO_MOVEMENT;
	this->movementType.z = __NO_MOVEMENT;

	this->velocity.x = 0;
	this->velocity.y = 0;
	this->velocity.z = 0;

	this->acceleration.x = 0;
	this->acceleration.y = 0;
	this->acceleration.z = 0;

	this->active = true;

	if(!_physhicsClock)
	{
		_physhicsClock = Game_getPhysicsClock(Game_getInstance());
	}
}

// class's destructor
void Body_destructor(Body this)
{
	ASSERT(this, "Body::destructor: null this");

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

	return this->appliedForce;
}

// retrieve movement type
MovementType Body_getMovementType(Body this)
{
	ASSERT(this, "Body::getMovementType: null this");

	return this->movementType;
}

// set movement type
static void Body_setMovementType(Body this, int movementType, u16 axis)
{
	ASSERT(this, "Body::setMovementType: null this");

	if(__X_AXIS & axis)
	{
		this->movementType.x = movementType;

		if(__NO_MOVEMENT == movementType || __UNIFORM_MOVEMENT == movementType)
		{
			this->appliedForce.x = 0;
			this->acceleration.x = 0;
		}
	}

	if(__Y_AXIS & axis)
	{
		this->movementType.y = movementType;

		if(__NO_MOVEMENT == movementType || __UNIFORM_MOVEMENT == movementType)
		{
			this->appliedForce.y = 0;
			this->acceleration.y = 0;
		}
	}

	if(__Z_AXIS & axis)
	{
		this->movementType.z = movementType;

		if(__NO_MOVEMENT == movementType || __UNIFORM_MOVEMENT == movementType)
		{
			this->appliedForce.z = 0;
			this->acceleration.z = 0;
		}
	}
}

void Body_clearAcceleration(Body this, u16 axis)
{
	ASSERT(this, "Body::moveAccelerated: null this");

	if(__X_AXIS & axis)
	{
		this->acceleration.x = 0;
		this->appliedForce.x = 0;
	}

	if(__Y_AXIS & axis)
	{
		this->acceleration.y = 0;
		this->appliedForce.y = 0;
	}

	if(__Z_AXIS & axis)
	{
		this->acceleration.z = 0;
		this->appliedForce.z = 0;
	}
}

// set movement type to accelerated
void Body_moveAccelerated(Body this, u16 axis)
{
	ASSERT(this, "Body::moveAccelerated: null this");

	Body_setMovementType(this, __ACCELERATED_MOVEMENT, axis);
}

// set movement type to uniform
void Body_moveUniformly(Body this, Velocity velocity)
{
	ASSERT(this, "Body::moveUniformly: null this");

	u16 axisOfUniformMovement = 0;

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
		axisOfUniformMovement |= __Y_AXIS;
		this->velocity.z = velocity.z;
	}

	if(axisOfUniformMovement)
	{
		Body_setMovementType(this, __UNIFORM_MOVEMENT, axisOfUniformMovement);
		Body_awake(this, axisOfUniformMovement, false);
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
void Body_applyForce(Body this, const Force* force, u16 clearAxis, bool informAboutAwakening)
{
	ASSERT(this, "Body::applyForce: null this");

	if(__X_AXIS & clearAxis)
	{
		this->velocity.x = 0;
		this->acceleration.x = 0;
	}

	if(__Y_AXIS & clearAxis)
	{
		this->velocity.y = 0;
		this->acceleration.y = 0;
	}

	if(__Z_AXIS & clearAxis)
	{
		this->velocity.z = 0;
		this->acceleration.z = 0;
	}

	this->appliedForce.x = force->x;
	this->appliedForce.y = force->y;
	this->appliedForce.z = force->z;

	if(this->mass)
	{
		this->acceleration.x += __FIX19_13_DIV(this->appliedForce.x, this->mass);
		this->acceleration.y += __FIX19_13_DIV(this->appliedForce.y, this->mass);
		this->acceleration.z += __FIX19_13_DIV(this->appliedForce.z, this->mass);
	}
	else
	{
		this->acceleration.x += this->appliedForce.x;
		this->acceleration.y += this->appliedForce.y;
		this->acceleration.z += this->appliedForce.z;
	}

	u16 axisAppliedForce = 0;

	if(this->appliedForce.x)
	{
		axisAppliedForce |= __X_AXIS;
	}

	if(this->appliedForce.y)
	{
		axisAppliedForce |= __Y_AXIS;
	}

	if(this->appliedForce.z)
	{
		axisAppliedForce |= __Z_AXIS;
	}

	if(axisAppliedForce)
	{
		Body_setMovementType(this, __ACCELERATED_MOVEMENT, axisAppliedForce);
		Body_awake(this, axisAppliedForce, informAboutAwakening);
	}
}

// apply gravity
void Body_applyGravity(Body this, const Acceleration* gravity)
{
	ASSERT(this, "Body::applyGravity: null this");

	if(gravity)
	{
		u16 axisStartedMovement = 0;

		if(gravity->x)
		{
			this->acceleration.x = gravity->x;
			axisStartedMovement |= __X_AXIS;
		}

		if(gravity->y)
		{
			this->acceleration.y = gravity->y;
			axisStartedMovement |= __Y_AXIS;
		}

		if(gravity->z)
		{
			this->acceleration.z = gravity->z;
			axisStartedMovement |= __Z_AXIS;
		}

		if(axisStartedMovement)
		{
			Body_setMovementType(this, __ACCELERATED_MOVEMENT, axisStartedMovement);
			Body_awake(this, axisStartedMovement, true);
		}
	}
}


// add force
void Body_addForce(Body this, const Force* force, bool informAboutAwakening)
{
	ASSERT(this, "Body::addForce: null this");
	ASSERT(force, "Body::addForce: null force");

	Body_applyForce(this, force, ~Body_getMovementOnAllAxes(this), informAboutAwakening);
}

// update movement
void Body_update(Body this)
{
	ASSERT(this, "Body::update: null this");

	if(this->awake && this->active)
	{
		u16 axisStoppedMovement = 0;
		u16 axisOfChangeOfMovement = 0;

		fix19_13 friction = Body_getTotalFriction(this);

		// update each axis
		if(this->velocity.x || this->acceleration.x || this->appliedForce.x || ((__ACCELERATED_MOVEMENT & this->movementType.x) && _currentGravity->x && this->acceleration.x))
		{
			int movementStatus = Body_updateMovement(this, __X_AXIS & this->axisSubjectToGravity? _currentGravity->x: 0, &this->position.x, &this->velocity.x, &this->acceleration.x, this->appliedForce.x, this->movementType.x, friction);

			if(movementStatus)
			{
				if(CHANGED_DIRECTION == movementStatus)
				{
					axisOfChangeOfMovement |= __X_AXIS;
				}
			}
			else
			{
				axisStoppedMovement |= __X_AXIS;
			}
		}

		if(this->velocity.y || this->acceleration.y || this->appliedForce.y || ((__ACCELERATED_MOVEMENT & this->movementType.y) && _currentGravity->y && this->acceleration.y))
		{
		friction = 0;
			int movementStatus = Body_updateMovement(this, __Y_AXIS & this->axisSubjectToGravity? _currentGravity->y: 0, &this->position.y, &this->velocity.y, &this->acceleration.y, this->appliedForce.y, this->movementType.y, friction);

			if(movementStatus)
			{
				if(CHANGED_DIRECTION == movementStatus)
				{
					axisOfChangeOfMovement |= __Y_AXIS;
				}
			}
			else
			{
				axisStoppedMovement |= __Y_AXIS;
			}
		}

		if(this->velocity.z || this->acceleration.z || this->appliedForce.z || ((__ACCELERATED_MOVEMENT & this->movementType.z) && _currentGravity->z && this->acceleration.z))
		{
			int movementStatus = Body_updateMovement(this, __Z_AXIS & this->axisSubjectToGravity? _currentGravity->z: 0, &this->position.z, &this->velocity.z, &this->acceleration.z, this->appliedForce.z, this->movementType.z, friction);

			if(movementStatus)
			{
				if(CHANGED_DIRECTION == movementStatus)
				{
					axisOfChangeOfMovement |= __Z_AXIS;
				}
			}
			else
			{
				axisStoppedMovement |= __Z_AXIS;
			}
		}

		// if stopped on any axis
		if(axisStoppedMovement)
		{
			MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyStopped, &axisStoppedMovement);
		}

		if(axisOfChangeOfMovement)
		{
			MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyChangedDirection, &axisOfChangeOfMovement);
		}

		// clear any force so the next update does not get influenced
		Body_clearForce(this);
	}
}

// update force
fix19_13 Body_getTotalFriction(Body this)
{
	ASSERT(this, "Body::calculateFriction: null this");

	return __FIX19_13_MULT((this->friction + _currentWorldFriction), this->mass);
}

// update force
static void Body_updateAcceleration(Body this, fix19_13 elapsedTime, fix19_13 gravity, fix19_13* acceleration, fix19_13 appliedForce, fix19_13 friction)
{
	ASSERT(this, "Body::updateAcceleration: null this");

	fix19_13 sign = __I_TO_FIX19_13(0 <= gravity ? -1 : 1);

	if(gravity)
	{
		if(0 > __FIX19_13_MULT((*acceleration - gravity), sign))
		{
			gravity = 0;
		}
		else if(__FIX19_13_MULT((*acceleration + __FIX19_13_MULT(gravity, elapsedTime) - gravity), sign))
		{
			gravity = gravity - *acceleration;
		}

		*acceleration += __FIX19_13_MULT(gravity, elapsedTime);
	}

	fix19_13 frictionAcceleration = this->mass ? __FIX19_13_DIV(friction, this->mass) : friction;

	if(appliedForce)
	{
		if(__ABS(frictionAcceleration) < __ABS(*acceleration))
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

	fix19_13 elapsedTime = PhysicalWorld_getElapsedTime(Game_getPhysicalWorld(Game_getInstance()));

	displacement.x = __FIX19_13_MULT(this->velocity.x, elapsedTime);
	displacement.y = __FIX19_13_MULT(this->velocity.y, elapsedTime);
	displacement.z = __FIX19_13_MULT(this->velocity.z, elapsedTime);

	return displacement;
}

// udpdate movement over axis
int Body_updateMovement(Body this, fix19_13 gravity, fix19_13* position, fix19_13* velocity, fix19_13* acceleration, fix19_13 appliedForce, int movementType, fix19_13 friction)
{
	ASSERT(this, "Body::updateMovement: null this");

	// the movement displacement
	fix19_13 displacement = 0;

	int moving = STILL_MOVES;

	// determine the movement type
	// calculate displacement based in velocity, time and acceleration
	if(__ACCELERATED_MOVEMENT == movementType)
	{
		if(0 <= *velocity)
		{
			friction = -friction;
		}

		Body_updateAcceleration(this, _currentElapsedTime, gravity, acceleration, appliedForce, friction);

		fix19_13 previousVelocity = *velocity;

		// update the velocity
		*velocity += __FIX19_13_MULT(*acceleration, _currentElapsedTime);

		displacement =
			__FIX19_13_MULT(*velocity, _currentElapsedTime)
			+ __FIX19_13_DIV(__FIX19_13_MULT(*acceleration, __FIX19_13_MULT(_currentElapsedTime, _currentElapsedTime)), __I_TO_FIX19_13(2));

		if(!gravity)
		{
			if((0 < previousVelocity && 0 > *velocity) || (0 > previousVelocity && 0 < *velocity))
			{
				*velocity = 0;
				*acceleration = 0;
				displacement = 0;
			}
		}

		if(!appliedForce && !*velocity)
		{
			moving = STOPPED_MOVING;
		}
		else
		{
			if((0 < previousVelocity && 0 > *velocity) || (0 > previousVelocity && 0 < *velocity))
			{
				moving = CHANGED_DIRECTION;
			}
		}
	}
	else if(__UNIFORM_MOVEMENT == movementType)
	{
		// update the velocity
		displacement = __FIX19_13_MULT(*velocity, _currentElapsedTime);
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

	Printing_text(Printing_getInstance(), "Active:", x, y, NULL);
	Printing_text(Printing_getInstance(), this->active? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 8, y++, NULL);
	Printing_text(Printing_getInstance(), "Awake:", x, y, NULL);
	Printing_text(Printing_getInstance(), this->awake? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 8, y++, NULL);

	Printing_text(Printing_getInstance(), "X             Y             Z", x, y++, NULL);

	Printing_text(Printing_getInstance(), "Position", x, y++, NULL);
	Printing_text(Printing_getInstance(), "                             ", x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(this->position.x), x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(this->position.y), x+14, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(this->position.z), x+14*2, y++, NULL);

	Printing_text(Printing_getInstance(), "Velocity", x, y++, NULL);
	Printing_text(Printing_getInstance(), "                             ", x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->velocity.x), x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->velocity.y), x+14, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->velocity.z), x+14*2, y++, NULL);
	Printing_text(Printing_getInstance(), "Acceleration", x, y++, NULL);
	Printing_text(Printing_getInstance(), "                             ", x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->acceleration.x), x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->acceleration.y), x+14, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->acceleration.z), x+14*2, y++, NULL);
	Printing_text(Printing_getInstance(), "Force", x, y++, NULL);
	Printing_text(Printing_getInstance(), "                             ", x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->appliedForce.x), x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->appliedForce.y), x+14, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->appliedForce.z), x+14*2, y++, NULL);
}

// stop movement over an axis
void Body_stopMovement(Body this, u16 axis)
{
	ASSERT(this, "Body::stopMovement: null this");

	u16 axisOfMovement = Body_getMovementOnAllAxes(this);
	u16 axisOfStopping = 0;

	if((axisOfMovement & __X_AXIS) && (axis & __X_AXIS))
	{
		// not moving anymore
		this->velocity.x = 0;
		this->acceleration.x = 0;
		this->appliedForce.x = 0;
		axisOfStopping |= __X_AXIS;
	}

	if((axisOfMovement & __Y_AXIS) && (axis & __Y_AXIS))
	{
		// not moving anymore
		this->velocity.y = 0;
		this->acceleration.y = 0;
		this->appliedForce.y = 0;
		axisOfStopping |= __Y_AXIS;
	}

	if((axisOfMovement & __Z_AXIS) && (axis & __Z_AXIS))
	{
		// not moving anymore
		this->velocity.z = 0;
		this->acceleration.z = 0;
		this->appliedForce.z = 0;
		axisOfStopping |= __Z_AXIS;
	}

	if(axisOfStopping)
	{
		Body_setMovementType(this, __NO_MOVEMENT, axisOfStopping);

		if(!Body_getMovementOnAllAxes(this))
		{
			Body_sleep(this);
		}

		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyStopped, &axisOfStopping);
	}
}

// get axis subject to gravity
u8 Body_getAxisSubjectToGravity(Body this)
{
	ASSERT(this, "Body::getAxisSubjectToGravity: null this");

	return this->axisSubjectToGravity;
}

// set axis subject to gravity
void Body_setAxisSubjectToGravity(Body this, u16 axisSubjectToGravity)
{
	ASSERT(this, "Body::setAxisSubjectToGravity: null this");

	this->axisSubjectToGravity = axisSubjectToGravity;
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
const VBVec3D* Body_getPosition(Body this)
{
	ASSERT(this, "Body::getPosition: null this");

	return &this->position;
}

// retrieve position
void Body_setPosition(Body this, const VBVec3D* position, SpatialObject caller)
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

// get friction
fix19_13 Body_getFriction(Body this)
{
	ASSERT(this, "Body::getFriction: null this");

	return this->friction;
}

// set elasticity
void Body_setFriction(Body this, fix19_13 friction)
{
	ASSERT(this, "Body::setFriction: null this");

	this->friction = friction;
}

fix19_13 Body_getMass(Body this)
{
	ASSERT(this, "Body::getMass: null this");

	return this->mass;
}

void Body_setMass(Body this, fix19_13 mass)
{
	ASSERT(this, "Body::setMass: null this");

	this->mass = mass;
}

// retrieve state
bool Body_isAwake(Body this)
{
	ASSERT(this, "Body::isAwake: null this");

	return this->awake && this->active;
}

// awake body
static void Body_awake(Body this, u16 axisOfAwakening, bool informAboutAwakening)
{
	ASSERT(this, "Body::awake: null this");

	bool dispatchMessage = false;

	if(!this->awake)
	{
		this->awake = true;

		PhysicalWorld_bodyAwake(Game_getPhysicalWorld(Game_getInstance()), this);
	}

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

	if(dispatchMessage && informAboutAwakening)
	{
		MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyStartedMoving, &axisOfAwakening);
	}
}

// go to sleep
void Body_sleep(Body this)
{
	ASSERT(this, "Body::sleep: null this");

	this->awake = false;

	PhysicalWorld_bodySleep(Game_getPhysicalWorld(Game_getInstance()), this);

	MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodySleep, NULL);
}

// is it moving?
u16 Body_getMovementOnAllAxes(Body this)
{
	ASSERT(this, "Body::isMoving: null this");

	u16 result = 0;

	result |= ((int)__FIX19_13_TO_I(this->velocity.x) | this->acceleration.x) ? __X_AXIS : 0;
	result |= ((int)__FIX19_13_TO_I(this->velocity.y) | this->acceleration.y) ? __Y_AXIS : 0;
	result |= ((int)__FIX19_13_TO_I(this->velocity.z) | this->acceleration.z) ? __Z_AXIS : 0;

	return this->awake && this->active ? result : 0;
}

// bounce back
void Body_bounce(Body this, VBVec3D bouncingPlaneNormal, u16 axesForBouncing, fix19_13 friction, fix19_13 elasticity)
{
	ASSERT(this, "Body::bounce: null this");

	fix19_13 weight = this->mass;
	friction = this->friction + friction + _currentWorldFriction;
    fix19_13 totalFriction = this->mass ? __FIX19_13_DIV(friction, this->mass) : friction;
	fix19_13 totalElasticity = this->elasticity + elasticity;

	if(__1I_FIX19_13 < totalElasticity)
	{
		totalElasticity = __1I_FIX19_13;
	}
	else if(0 > totalElasticity)
	{
		totalElasticity = 0;
	}

	VBVec3D velocity =
	{
		this->velocity.x,
		this->velocity.y,
		this->velocity.z,
	};

	// compute bouncing velocity vector
	VBVec3D u = Vector_scalarProduct(bouncingPlaneNormal, Vector_dotProduct(velocity, bouncingPlaneNormal));
	VBVec3D v =
	{
		velocity.x - u.x,
		velocity.y - u.y,
		velocity.z - u.z,
	};

	totalFriction = __F_TO_FIX19_13(1.0f);

	v = Vector_scalarProduct(v, totalFriction);
	u = Vector_scalarProduct(u, totalElasticity);

	this->velocity.x = v.x - u.x;
	this->velocity.y = v.y - u.y;
	this->velocity.z = v.z - u.z;

	// check it must stop
	fix19_13 elapsedTime = PhysicalWorld_getElapsedTime(Game_getPhysicalWorld(Game_getInstance()));
	Velocity velocityDelta =
	{
		__FIX19_13_MULT(this->acceleration.x, elapsedTime),
		__FIX19_13_MULT(this->acceleration.y, elapsedTime),
		__FIX19_13_MULT(this->acceleration.z, elapsedTime),
	};

	u16 axisOnWhichStopped = __NO_AXIS;
	u16 axisOnWhichBounced = __NO_AXIS;

	if(__ABS(velocityDelta.x) > __ABS(this->velocity.x))
	{
		axisOnWhichStopped |= __X_AXIS;
	}

	if(__ABS(velocityDelta.y) > __ABS(this->velocity.y))
	{
		axisOnWhichStopped |= __Y_AXIS;
	}

	if(__ABS(velocityDelta.z) > __ABS(this->velocity.z))
	{
		axisOnWhichStopped |= __Z_AXIS;
	}

	// determine the type of movement on each axis
	VBVec3DFlag axisOfChangeOfMovement = {__NO_AXIS, __NO_AXIS, __NO_AXIS};

	axisOfChangeOfMovement.x = this->velocity.x != velocity.x;
	axisOfChangeOfMovement.y = this->velocity.y != velocity.y;
	axisOfChangeOfMovement.z = this->velocity.z != velocity.z;

	u16 axisOfAcceleratedMovement = __NO_AXIS;

	if(axisOfChangeOfMovement.x && (__X_AXIS & this->axisSubjectToGravity) && this->movementType.x != __UNIFORM_MOVEMENT)
	{
		axisOfAcceleratedMovement |= __X_AXIS;
	}

	if(axisOfChangeOfMovement.y && (__Y_AXIS & this->axisSubjectToGravity))// && this->movementType.y != __UNIFORM_MOVEMENT)
	{
		axisOfAcceleratedMovement |= __Y_AXIS;
	}

	if(axisOfChangeOfMovement.z && (__Z_AXIS & this->axisSubjectToGravity) && this->movementType.z != __UNIFORM_MOVEMENT)
	{
		axisOfAcceleratedMovement |= __Z_AXIS;
	}

	Body_moveAccelerated(this, axisOfAcceleratedMovement);

/*	Printing_int(Printing_getInstance(), __FIX19_13_TO_F(velocity.x), 1, 1, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_F(velocity.y), 1, 2, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_F(velocity.z), 1, 3, NULL);

	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(Vector_dotProduct(velocity, bouncingPlaneNormal)), 10, 0, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(bouncingPlaneNormal.x), 10, 1, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(bouncingPlaneNormal.y), 10, 2, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(bouncingPlaneNormal.z), 10, 3, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(u.x), 20, 1, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(u.y), 20, 2, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(u.z), 20, 3, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(v.x), 40, 1, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(v.y), 40, 2, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(v.z), 40, 3, NULL);
*/
	if(axisOnWhichStopped)
	{
		Body_stopMovement(this, axisOnWhichStopped);
	}

	if(axisOnWhichBounced)
	{
	//	MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyBounced, &axisOnWhichBounced);
	}
}

// take a hit
void Body_takeHitFrom(Body this __attribute__ ((unused)), Body other __attribute__ ((unused)))
{
	ASSERT(this, "Body::takeHitFrom: null this");

	//TODO:
}
