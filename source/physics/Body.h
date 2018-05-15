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
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * Movement result
 *
 * @memberof Body
 */
typedef struct MovementResult
{
	u16 axesStoppedMovement;
	u16 axesOfAcceleratedBouncing;
	u16 axesOfChangeOfMovement;
	u16 axesOfChangeOfDirection;

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

typedef const PhysicalSpecification PhysicalSpecificationROMDef;


class Body : Object
{
	/**
	* @var SpatialObject 	owner
	* @brief				owner
	* @memberof 			Body
	*/
	SpatialObject owner;
	/**
	* @var Force 			weight
	* @brief				direction
	* @memberof 			Body
	*/
	Force weight;
	/**
	* @var Force 			externalForce
	* @brief				direction
	* @memberof 			Body
	*/
	Force externalForce;
	/**
	* @var Force 			friction
	* @brief				friction surrounding object
	* @memberof 			Body
	*/
	Force friction;
	/**
	* @var Force 			totalNormal
	* @brief				total normal forces applied to the body
	* @memberof 			Body
	*/
	Force totalNormal;
	/**
	* @var VirtualList 	normals
	* @brief				List of normal forces affecting the body
	* @memberof 			Body
	*/
	VirtualList normals;
	/**
	* @var Vector3D 		position
	* @brief				spatial position
	* @memberof 			Body
	*/
	Vector3D position;
	/**
	* @var Velocity 		velocity
	* @brief				velocity on each instance
	* @memberof 			Body
	*/
	Velocity velocity;
	/**
	* @var Velocity 		maximum velocity
	* @brief				maximum velocity on each instance
	* @memberof 			Body
	*/
	Velocity maximumVelocity;
	/**
	* @var Acceleration 	acceleration
	* @brief				acceleration structure
	* @memberof 			Body
	*/
	Acceleration acceleration;
	/**
	* @var fix10_6 		bounciness
	* @brief				bounciness
	* @memberof 			Body
	*/
	fix10_6 bounciness;
	/**
	* @var fix10_6 		frictionCoefficient
	* @brief				friction coefficient
	* @memberof 			Body
	*/
	fix10_6 frictionCoefficient;
	/**
	* @var fix10_6 		surroundingFrictionCoefficient
	* @brief				friction coefficient of the surroundings
	* @memberof 			Body
	*/
	fix10_6 surroundingFrictionCoefficient;
	/**
	* @var fix10_6 		frictionForceMagnitude
	* @brief				friction force magnitude
	* @memberof 			Body
	*/
	fix10_6 totalFrictionCoefficient;
	/**
	* @var fix10_6 		totalFrictionCoefficient
	* @brief				total friction force magnitude
	* @memberof 			Body
	*/
	fix10_6 frictionForceMagnitude;
	/**
	* @var fix10_6 		mass
	* @brief				mass
	* @memberof 			Body
	*/
	fix10_6 mass;
	/**
	* @var MovementType 	movementType
	* @brief				movement type on each axis
	* @memberof 			Body
	*/
	MovementType movementType;
	/**
	* @var u8 				axesSubjectToGravity
	* @brief				axes that are subject to gravity
	* @memberof 			Body
	*/
	u16 axesSubjectToGravity;
	/**
	* @var bool 			active
	* @brief				raise flag to make the body active
	* @memberof 			Body
	*/
	bool active;
	/**
	* @var bool			awake
	* @brief				raise flag to update body's physics
	* @memberof 			Body
	*/
	bool awake;

	static void setCurrentElapsedTime(fix10_6 currentElapsedTime);
	static void setCurrentWorldFrictionCoefficient(fix10_6 _currentWorldFriction);
	static void setCurrentGravity(const Acceleration* currentGravity);

	void constructor(Body this, SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axesSubjectToGravity);
	void addForce(Body this, const Force* force);
	void applyForce(Body this, const Force* force);
	void applyGravity(Body this, u16 axes);
	void bounce(Body this, Object bounceReferent, Vector3D bouncingPlaneNormal, fix10_6 frictionCoefficient, fix10_6 bounciness);
	void clearAcceleration(Body this, u16 axes);
	void clearExternalForce(Body this);
	Acceleration getAcceleration(Body this);
	Force getAppliedForce(Body this);
	u16 getAxesSubjectToGravity(Body this);
	fix10_6 getBounciness(Body this);
	Vector3D getLastDisplacement(Body this);
	fix10_6 getMass(Body this);
	MovementType getMovementType(Body this);
	SpatialObject getOwner(Body this);
	const Vector3D* getPosition(Body this);
	Velocity getVelocity(Body this);
	void modifyVelocity(Body this, const Velocity* multiplier);
	bool isActive(Body this);
	bool isAwake(Body this);
	u16 getMovementOnAllAxes(Body this);
	void moveAccelerated(Body this, u16 axes);
	void moveUniformly(Body this, Velocity velocity);
	void setActive(Body this, bool active);
	void setAxesSubjectToGravity(Body this, u16 axesSubjectToGravity);
	void setBounciness(Body this, fix10_6 bounciness);
	Force getNormal(Body this);
	Force getLastNormalDirection(Body this);
	void reset(Body this);
	void clearNormal(Body this, Object referent);
	fix10_6 getFrictionCoefficient(Body this);
	void setFrictionCoefficient(Body this, fix10_6 frictionCoefficient);
	void setSurroundingFrictionCoefficient(Body this, fix10_6 surroundingFrictionCoefficient);
	void setMass(Body this, fix10_6 mass);
	void setOwner(Body this, SpatialObject owner);
	void setPosition(Body this, const Vector3D* position, SpatialObject caller);
	u16 stopMovement(Body this, u16 axes);
	void takeHitFrom(Body this, Body other);
	void setMaximumVelocity(Body this, Velocity maximumVelocity);
	Velocity getMaximumVelocity(Body this);
	void print(Body this, int x, int y);
	virtual void update(Body this);
}


#endif
