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
	u16 axesOfAcceleratedMovement;
	VBVec3DFlag axesOfChangeOfMovement;

} MovementResult;

Clock _physhicsClock = NULL;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

MovementResult Body_updateMovement(Body this);
static void Body_awake(Body this, u16 axesOfAwakening);
static void Body_setMovementType(Body this, int movementType, u16 axis);


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
	this->mass = 0 < physicalSpecification->mass ? physicalSpecification->mass : __I_TO_FIX19_13(1);
	this->elasticity = physicalSpecification->elasticity;
	this->frictionCoefficient = physicalSpecification->frictionCoefficient;
	this->frictionForceMagnitude = 0;

	this->active = true;
	this->awake = false;
	this->axisSubjectToGravity = __X_AXIS | __Y_AXIS | __Z_AXIS;

	// clear movement type
	this->movementType.x = __NO_MOVEMENT;
	this->movementType.y = __NO_MOVEMENT;
	this->movementType.z = __NO_MOVEMENT;

	this->position 			= (VBVec3D){0, 0, 0};
	this->velocity 			= (Velocity){0, 0, 0};
	this->acceleration 		= (Acceleration){0, 0, 0};
	this->gravity	 		= (Acceleration){0, 0, 0};
	this->externalForce 	= (Force){0, 0, 0};
	this->friction 			= (Force){0, 0, 0};
	this->normal			= (Force){0, 0, 0};
	this->weight 			= Vector_scalarProduct(this->gravity, this->mass);

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

	return this->externalForce;
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

void Body_clearAcceleration(Body this, u16 axis)
{
	ASSERT(this, "Body::moveAccelerated: null this");

	if(__X_AXIS & axis)
	{
		this->acceleration.x = 0;
		this->externalForce.x = 0;
	}

	if(__Y_AXIS & axis)
	{
		this->acceleration.y = 0;
		this->externalForce.y = 0;
	}

	if(__Z_AXIS & axis)
	{
		this->acceleration.z = 0;
		this->externalForce.z = 0;
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
		axisOfUniformMovement |= __Z_AXIS;
		this->velocity.z = velocity.z;
	}

	if(axisOfUniformMovement)
	{
		Body_setMovementType(this, __UNIFORM_MOVEMENT, axisOfUniformMovement);
		Body_awake(this, axisOfUniformMovement);
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
			Body_setMovementType(this, __ACCELERATED_MOVEMENT, axesOfExternalForce);
			Body_awake(this, axesOfExternalForce);
		}
	}
}

// apply gravity
void Body_applyGravity(Body this, const Acceleration* gravity)
{
	ASSERT(this, "Body::applyGravity: null this");

	if(gravity)
	{
		this->gravity = *gravity;
		this->weight = Vector_scalarProduct(this->gravity, this->mass);

		u16 axesOfAppliedGravity = __NO_AXIS;

		if(this->gravity.x)
		{
			axesOfAppliedGravity |= __X_AXIS;
		}

		if(this->gravity.y)
		{
			axesOfAppliedGravity |= __Y_AXIS;
		}

		if(this->gravity.z)
		{
			axesOfAppliedGravity |= __Z_AXIS;
		}

		if(axesOfAppliedGravity)
		{
			Body_awake(this, axesOfAppliedGravity);
		}
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
		this->gravity = *_currentGravity;
		this->weight = Vector_scalarProduct(this->gravity, this->mass);
		u16 previousMovementState = Body_getMovementOnAllAxes(this);

		this->friction = Vector_scalarProduct(Vector_normalize(this->velocity), -this->frictionForceMagnitude);

		if(!this->awake)
		{
			this->acceleration = (Acceleration)
			{
				this->gravity.x + __FIX19_13_DIV(this->externalForce.x + this->normal.x + this->friction.x, this->mass),
				this->gravity.y + __FIX19_13_DIV(this->externalForce.y + this->normal.y + this->friction.y, this->mass),
				this->gravity.z + __FIX19_13_DIV(this->externalForce.z + this->normal.z + this->friction.z, this->mass),
			};

			u16 axisStartedMovement = __NO_AXIS;

			if(!(__X_AXIS & previousMovementState) && this->acceleration.x)
			{
				axisStartedMovement |= __X_AXIS;
			}

			if(!(__Y_AXIS & previousMovementState) && this->acceleration.y)
			{
				axisStartedMovement |= __Y_AXIS;
			}

			if(!(__Z_AXIS & previousMovementState) && this->acceleration.z)
			{
				axisStartedMovement |= __Z_AXIS;
			}

			if(axisStartedMovement)
			{
				Body_setMovementType(this, __ACCELERATED_MOVEMENT, axisStartedMovement);
				Body_awake(this, axisStartedMovement);
			}
		}

		if(this->awake)
		{
			MovementResult movementResult = Body_updateMovement(this);

			// if stopped on any axis
			if(movementResult.axesStoppedMovement)
			{
				Body_stopMovement(this, movementResult.axesStoppedMovement);
			}

			//if(movementResult.axesOfChangeOfMovement)
			{
				//MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyChangedDirection, &movementResult.axesOfChangeOfMovement);
			}
		}

		// clear any force so the next update does not get influenced
		Body_clearExternalForce(this);
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

static MovementResult Body_getMovementResult(Body this, VBVec3D previousVelocity)
{
	ASSERT(this, "Body::checkIfStopped: null this");

	MovementResult movementResult = {__NO_AXIS, __NO_AXIS, {__NO_AXIS, __NO_AXIS, __NO_AXIS}};

	movementResult.axesOfChangeOfMovement.x = this->velocity.x != previousVelocity.x;
	movementResult.axesOfChangeOfMovement.y = this->velocity.y != previousVelocity.y;
	movementResult.axesOfChangeOfMovement.z = this->velocity.z != previousVelocity.z;

	if(!this->externalForce.x && (!this->gravity.x || this->normal.x))
	{
		if((0 < previousVelocity.x && 0 > this->velocity.x) || (0 > previousVelocity.x && 0 < this->velocity.x) || abs(this->velocity.x) < abs(previousVelocity.x))
		{
			movementResult.axesStoppedMovement |= __X_AXIS;
		}
	}

	if(!this->externalForce.y && (!this->gravity.y || this->normal.y))
	{
		if((0 < previousVelocity.y && 0 > this->velocity.y) || (0 > previousVelocity.y && 0 < this->velocity.y) || abs(this->velocity.y) < abs(previousVelocity.y))
		{
			movementResult.axesStoppedMovement |= __Y_AXIS;
		}
	}

	if(!this->externalForce.z && (!this->gravity.z || this->normal.z))
	{
		if((0 < previousVelocity.z && 0 > this->velocity.z) || (0 > previousVelocity.z && 0 < this->velocity.z) || abs(this->velocity.z) < abs(previousVelocity.z))
		{
			movementResult.axesStoppedMovement |= __Z_AXIS;
		}
	}

	if(movementResult.axesOfChangeOfMovement.x && (__X_AXIS & this->axisSubjectToGravity))
	{
		movementResult.axesOfAcceleratedMovement |= __X_AXIS;
	}

	if(movementResult.axesOfChangeOfMovement.y && (__Y_AXIS & this->axisSubjectToGravity))
	{
		movementResult.axesOfAcceleratedMovement |= __Y_AXIS;
	}

	if(movementResult.axesOfChangeOfMovement.z && (__Z_AXIS & this->axisSubjectToGravity))
	{
		movementResult.axesOfAcceleratedMovement |= __Z_AXIS;
	}

	return movementResult;
}

// udpdate movement over axis
MovementResult Body_updateMovement(Body this)
{
	ASSERT(this, "Body::updateMovement: null this");

	fix19_13 elapsedTime = _currentElapsedTime;
	fix19_13 elapsedTimeHalfSquare = __FIX19_13_DIV(__FIX19_13_MULT(elapsedTime, elapsedTime), __I_TO_FIX19_13(2));

	Velocity previousVelocity = this->velocity;

	this->acceleration = (Acceleration)
	{
		__UNIFORM_MOVEMENT == this->movementType.x ? 0 : this->gravity.x + __FIX19_13_DIV(this->externalForce.x + this->normal.x + this->friction.x, this->mass),
		__UNIFORM_MOVEMENT == this->movementType.y ? 0 : this->gravity.y + __FIX19_13_DIV(this->externalForce.y + this->normal.y + this->friction.y, this->mass),
		__UNIFORM_MOVEMENT == this->movementType.z ? 0 : this->gravity.z + __FIX19_13_DIV(this->externalForce.z + this->normal.z + this->friction.z, this->mass),
	};

	// update the velocity
	this->velocity.x += __FIX19_13_MULT(this->acceleration.x, elapsedTime);
	this->velocity.y += __FIX19_13_MULT(this->acceleration.y, elapsedTime);
	this->velocity.z += __FIX19_13_MULT(this->acceleration.z, elapsedTime);

	VBVec3D displacement =
	{
		__FIX19_13_MULT(this->velocity.x, elapsedTime) + __FIX19_13_MULT(this->acceleration.x, elapsedTimeHalfSquare),
		__FIX19_13_MULT(this->velocity.y, elapsedTime) + __FIX19_13_MULT(this->acceleration.y, elapsedTimeHalfSquare),
		__FIX19_13_MULT(this->velocity.z, elapsedTime) + __FIX19_13_MULT(this->acceleration.z, elapsedTimeHalfSquare),
	};

	// update position
	this->position.x += displacement.x;
	this->position.y += displacement.y;
	this->position.z += displacement.z;

	return Body_getMovementResult(this, previousVelocity);
}

void Body_printPhysics(Body this, int x, int y)
{
	ASSERT(this, "Body::printPhysics: null this");

	Printing_text(Printing_getInstance(), "Active:", x, y, NULL);
	Printing_text(Printing_getInstance(), this->active? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 10, y++, NULL);
	Printing_text(Printing_getInstance(), "Awake:", x, y, NULL);
	Printing_text(Printing_getInstance(), this->awake? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + 10, y++, NULL);

	Printing_text(Printing_getInstance(), "               X         Y         Z", x, y++, NULL);

	int xDisplacement = 15;
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

	Printing_text(Printing_getInstance(), "Mov. type", x, y, NULL);
	Printing_text(Printing_getInstance(), "                                ", xDisplacement + x, y, NULL);
	Printing_text(Printing_getInstance(), __UNIFORM_MOVEMENT == this->movementType.x ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.x ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.x ? "Accel" : "None", xDisplacement + x, y, NULL);
	Printing_text(Printing_getInstance(), __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.y ? "Accel" : "None", xDisplacement + x + 10, y, NULL);
	Printing_text(Printing_getInstance(), __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __UNIFORM_MOVEMENT == this->movementType.y ? "Uniform" : __ACCELERATED_MOVEMENT == this->movementType.z ? "Accel" : "None", xDisplacement + x + 10 * 2, y++, NULL);

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

	Printing_text(Printing_getInstance(), "Gravity", x, y, NULL);
	Printing_text(Printing_getInstance(), "                               ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->gravity.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->gravity.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->gravity.z), xDisplacement + x + 10 * 2, y++, NULL);

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
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->normal.x), xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->normal.y), xDisplacement + x + 10, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->normal.z), xDisplacement + x + 10 * 2, y++, NULL);

	Printing_text(Printing_getInstance(), "Normal Force", x, y, NULL);
	Printing_text(Printing_getInstance(), "                              ", xDisplacement + x, y, NULL);
	Printing_float(Printing_getInstance(), __FIX19_13_TO_F(this->frictionForceMagnitude), xDisplacement + x, y, NULL);


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
		this->externalForce.x = 0;
		axisOfStopping |= __X_AXIS;
	}

	if((axisOfMovement & __Y_AXIS) && (axis & __Y_AXIS))
	{
		// not moving anymore
		this->velocity.y = 0;
		this->acceleration.y = 0;
		this->externalForce.y = 0;
		axisOfStopping |= __Y_AXIS;
	}

	if((axisOfMovement & __Z_AXIS) && (axis & __Z_AXIS))
	{
		// not moving anymore
		this->velocity.z = 0;
		this->acceleration.z = 0;
		this->externalForce.z = 0;
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

	this->normal = (VBVec3D){0, 0, 0};
	this->frictionForceMagnitude = abs(__FIX19_13_DIV(this->frictionForceMagnitude, this->frictionCoefficient));
	this->frictionCoefficient = frictionCoefficient;
	this->frictionForceMagnitude = abs(__FIX19_13_MULT(this->frictionForceMagnitude, this->frictionCoefficient));
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
	this->weight = Vector_scalarProduct(this->gravity, this->mass);
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
void Body_bounce(Body this, VBVec3D bouncingPlaneNormal, u16 axesForBouncing, fix19_13 frictionCoefficient, fix19_13 elasticity)
{
	ASSERT(this, "Body::bounce: null this");

	fix19_13 totalElasticity = this->elasticity + elasticity;

	fix19_13 cosAngle = abs(__FIX19_13_DIV(Vector_dotProduct(this->gravity, bouncingPlaneNormal), __FIX19_13_MULT(Vector_length(this->gravity), Vector_length(bouncingPlaneNormal))));
	fix19_13 normalForce = __FIX19_13_MULT(Vector_length(this->weight), cosAngle);
	this->normal = Vector_scalarProduct(bouncingPlaneNormal, normalForce);

	this->frictionCoefficient = frictionCoefficient;
	this->frictionForceMagnitude = __FIX19_13_MULT(normalForce, this->frictionCoefficient);
	this->friction = Vector_scalarProduct(Vector_normalize(this->velocity), -this->frictionForceMagnitude);

	if(__1I_FIX19_13 < totalElasticity)
	{
		totalElasticity = __1I_FIX19_13;
	}
	else if(0 > totalElasticity)
	{
		totalElasticity = 0;
	}

	VBVec3D velocity = this->velocity;

	// compute bouncing velocity vector
	VBVec3D u = Vector_scalarProduct(bouncingPlaneNormal, Vector_dotProduct(velocity, bouncingPlaneNormal));
	VBVec3D w =
	{
		velocity.x - u.x,
		velocity.y - u.y,
		velocity.z - u.z,
	};

	u = Vector_scalarProduct(u, totalElasticity);
	w = Vector_scalarProduct(w, (__I_TO_FIX19_13(1) - this->frictionCoefficient));

	this->velocity.x = w.x - u.x;
	this->velocity.y = w.y - u.y;
	this->velocity.z = w.z - u.z;

	this->friction = Vector_scalarProduct(Vector_normalize(this->velocity), -this->frictionCoefficient);

	// check it must stop
	fix19_13 elapsedTime = PhysicalWorld_getElapsedTime(Game_getPhysicalWorld(Game_getInstance()));
	Velocity velocityDelta =
	{
		__FIX19_13_MULT(this->acceleration.x, elapsedTime),
		__FIX19_13_MULT(this->acceleration.y, elapsedTime),
		__FIX19_13_MULT(this->acceleration.z, elapsedTime),
	};

	MovementResult movementResult = Body_getMovementResult(this, velocity);

	// determine the type of movement on each axis
	if(movementResult.axesOfAcceleratedMovement)
	{
		Body_moveAccelerated(this, movementResult.axesOfAcceleratedMovement);
	}

	if(!(movementResult.axesStoppedMovement & __X_AXIS))
	{
		Body_stopMovement(this, movementResult.axesStoppedMovement);
	}

/*	if(movementResult.axisOnWhichBounced)
	{
	//	MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this->owner), kBodyBounced, &axisOnWhichBounced);
	}
*/
}

// take a hit
void Body_takeHitFrom(Body this __attribute__ ((unused)), Body other __attribute__ ((unused)))
{
	ASSERT(this, "Body::takeHitFrom: null this");

	//TODO:
}
