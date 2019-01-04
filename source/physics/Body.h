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

#ifndef BODY_H_
#define BODY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <SpatialObject.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// movement type
#define	__NO_MOVEMENT				0x00
#define	__UNIFORM_MOVEMENT			0x01
#define	__ACCELERATED_MOVEMENT		0x20


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * Movement result
 *
 * @memberof Body
 */
typedef struct MovementResult
{
	u16 axisStoppedMovement;
	u16 axisOfAcceleratedBouncing;
	u16 axisOfChangeOfMovement;
	u16 axisOfChangeOfDirection;

} MovementResult;

// defines a body
typedef struct PhysicalSpecification
{
	/// mass
	fix10_6 mass;
	/// friction coefficient
	fix10_6 frictionCoefficient;
	/// bounciness
	fix10_6 bounciness;
	/// maximum velocity
	Velocity maximumVelocity;

} PhysicalSpecification;

typedef const PhysicalSpecification PhysicalSpecificationROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
class Body : Object
{
	// owner
	SpatialObject owner;
	// direction
	Force weight;
	// direction
	Force externalForce;
	// friction surrounding object
	Force friction;
	// total normal forces applied to the body
	Force totalNormal;
	// List of normal forces affecting the body
	VirtualList normals;
	// spatial position
	Vector3D position;
	// velocity on each instance
	Velocity velocity;
	// maximum velocity on each instance
	Velocity maximumVelocity;
	// acceleration structure
	Acceleration acceleration;
	// bounciness
	fix10_6 bounciness;
	// friction coefficient
	fix10_6 frictionCoefficient;
	// friction coefficient of the surroundings
	fix10_6 surroundingFrictionCoefficient;
	// friction force magnitude
	fix10_6 totalFrictionCoefficient;
	// total friction force magnitude
	fix10_6 frictionForceMagnitude;
	// mass
	fix10_6 mass;
	// movement type on each axis
	MovementType movementType;
	// axis that are subject to gravity
	u16 axisSubjectToGravity;
	// raise flag to make the body active
	bool active;
	// raise flag to update body's physics
	bool awake;

	/// @publicsection
	static void setCurrentElapsedTime(fix10_6 currentElapsedTime);
	static void setCurrentWorldFrictionCoefficient(fix10_6 _currentWorldFriction);
	static void setCurrentGravity(const Acceleration* currentGravity);
	void constructor(SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axisSubjectToGravity);
	void addForce(const Force* force);
	void applyForce(const Force* force);
	void applyGravity(u16 axis);
	void bounce(Object bounceReferent, Vector3D bouncingPlaneNormal, fix10_6 frictionCoefficient, fix10_6 bounciness);
	void clearAcceleration(u16 axis);
	void clearExternalForce();
	Acceleration getAcceleration();
	Force getAppliedForce();
	u16 getaxisSubjectToGravity();
	fix10_6 getBounciness();
	Vector3D getLastDisplacement();
	fix10_6 getMass();
	MovementType getMovementType();
	SpatialObject getOwner();
	const Vector3D* getPosition();
	Velocity getVelocity();
	fix10_6 getSpeed();
	fix10_6_ext getSpeedSquare();
	void modifyVelocity(const Velocity* multiplier);
	bool isActive();
	bool isAwake();
	u16 getMovementOnAllAxis();
	void moveAccelerated(u16 axis);
	void moveUniformly(Velocity velocity);
	void setActive(bool active);
	void setAxisSubjectToGravity(u16 axisSubjectToGravity);
	void setBounciness(fix10_6 bounciness);
	Force getNormal();
	Force getLastNormalDirection();
	void reset();
	void clearNormal(Object referent);
	fix10_6 getFrictionCoefficient();
	void setFrictionCoefficient(fix10_6 frictionCoefficient);
	void setSurroundingFrictionCoefficient(fix10_6 surroundingFrictionCoefficient);
	void setMass(fix10_6 mass);
	void setOwner(SpatialObject owner);
	void setPosition(const Vector3D* position, SpatialObject caller);
	u16 stopMovement(u16 axis);
	void takeHitFrom(Body other);
	void setMaximumVelocity(Velocity maximumVelocity);
	Velocity getMaximumVelocity();
	void print(int x, int y);
	MovementResult updateMovement();
	virtual void update();
}


#endif
