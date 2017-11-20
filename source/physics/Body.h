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

#define Body_METHODS(ClassName)																			\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, update);															\

#define Body_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Body, update);															\

#define Body_ATTRIBUTES																					\
		Object_ATTRIBUTES																				\
		/**
		 * @var SpatialObject 	owner
		 * @brief				owner
		 * @memberof 			Body
		 */																								\
		SpatialObject owner;																			\
		/**
		 * @var Force 			weight
		 * @brief				direction
		 * @memberof 			Body
		 */																								\
		Force weight;																					\
		/**
		 * @var Force 			externalForce
		 * @brief				direction
		 * @memberof 			Body
		 */																								\
		Force externalForce;																			\
		/**
		 * @var Force 			friction
		 * @brief				friction surrounding object
		 * @memberof 			Body
		 */																								\
		Force friction;																					\
		/**
		 * @var Force 			normal
		 * @brief				friction surrounding object
		 * @memberof 			Body
		 */																								\
		Force normal;																					\
		/**
		 * @var Vector3D 		bouncingPlaneNormal
		 * @brief				normal of the bouncing plane
		 * @memberof 			Body
		 */																								\
		Vector3D bouncingPlaneNormal;																	\
		/**
		 * @var Vector3D 		position
		 * @brief				spatial position
		 * @memberof 			Body
		 */																								\
		Vector3D position;																				\
		/**
		 * @var Velocity 		velocity
		 * @brief				velocity on each instance
		 * @memberof 			Body
		 */																								\
		Velocity velocity;																				\
		/**
		 * @var Acceleration 	acceleration
		 * @brief				acceleration structure
		 * @memberof 			Body
		 */																								\
		Acceleration acceleration;																		\
		/**
		 * @var fix19_13 		elasticity
		 * @brief				elasticity
		 * @memberof 			Body
		 */																								\
		fix19_13 elasticity;																			\
		/**
		 * @var fix19_13 		frictionCoefficient
		 * @brief				friction coefficient
		 * @memberof 			Body
		 */																								\
		fix19_13 frictionCoefficient;																	\
		/**
		 * @var fix19_13 		frictionForceMagnitude
		 * @brief				friction force magnitude
		 * @memberof 			Body
		 */																								\
		fix19_13 frictionForceMagnitude;																\
		/**
		 * @var fix19_13 		mass
		 * @brief				mass
		 * @memberof 			Body
		 */																								\
		fix19_13 mass;																					\
		/**
		 * @var MovementType 	movementType
		 * @brief				movement type on each axis
		 * @memberof 			Body
		 */																								\
		MovementType movementType;																		\
		/**
		 * @var u8 				axesSubjectToGravity
		 * @brief				axis that is subject to gravity
		 * @memberof 			Body
		 */																								\
		u16 axesSubjectToGravity;																		\
		/**
		 * @var u16 			axes of applied gravity
		 * @brief				gravity structure
		 * @memberof 			Body
		 */																								\
		u16 axesOfAppliedGravity;																		\
		/**
		 * @var bool 			active
		 * @brief				raise flag to make the body active
		 * @memberof 			Body
		 */																								\
		bool active;																					\
		/**
		 * @var bool			awake
		 * @brief				raise flag to update body's physics
		 * @memberof 			Body
		 */																								\
		bool awake;																						\

__CLASS(Body);


// defines a body
typedef struct PhysicalSpecification
{
	/// mass
	fix19_13 mass;
	/// friction coefficient
	fix19_13 frictionCoefficient;
	/// elasticity
	fix19_13 elasticity;

} PhysicalSpecification;

typedef const PhysicalSpecification PhysicalSpecificationROMDef;


//---------------------------------------------------------------------------------------------------------
//										CLASS' STATIC METHODS
//---------------------------------------------------------------------------------------------------------

void Body_setCurrentWorldFrictionCoefficient(fix19_13 _currentWorldFriction);
void Body_setCurrentElapsedTime(fix19_13 currentElapsedTime);
void Body_setCurrentGravity(const Acceleration* currentGravity);


//---------------------------------------------------------------------------------------------------------
//										CLASS' INSTANCE METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Body, SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axesSubjectToGravity);

void Body_constructor(Body this, SpatialObject owner, const PhysicalSpecification* physicalSpecification, u16 axesSubjectToGravity);
void Body_destructor(Body this);

void Body_addForce(Body this, const Force* force);
void Body_applyForce(Body this, const Force* force);
void Body_applyGravity(Body this, u16 axis);
void Body_bounce(Body this, Vector3D bouncingPlaneNormal, fix19_13 frictionCoefficient, fix19_13 elasticity);
void Body_clearAcceleration(Body this, u16 axis);
void Body_clearExternalForce(Body this);
Acceleration Body_getAcceleration(Body this);
Force Body_getAppliedForce(Body this);
u16 Body_getAxesSubjectToGravity(Body this);
fix19_13 Body_getElasticity(Body this);
Vector3D Body_getLastDisplacement(Body this);
fix19_13 Body_getMass(Body this);
MovementType Body_getMovementType(Body this);
SpatialObject Body_getOwner(Body this);
const Vector3D* Body_getPosition(Body this);
Velocity Body_getVelocity(Body this);
bool Body_isActive(Body this);
bool Body_isAwake(Body body);
u16 Body_getMovementOnAllAxes(Body this);
void Body_moveAccelerated(Body this, u16 axis);
void Body_moveUniformly(Body this, Velocity velocity);
void Body_setActive(Body this, bool active);
void Body_setAxesSubjectToGravity(Body this, u16 axesSubjectToGravity);
void Body_setElasticity(Body this, fix19_13 elasticity);
Force Body_getNormal(Body this);
void Body_clearNormal(Body this);
Vector3D Body_getBouncingPlaneNormal(Body this);
void Body_setFrictionCoefficient(Body this, fix19_13 frictionCoefficient);
void Body_setMass(Body this, fix19_13 mass);
void Body_setOwner(Body this, SpatialObject owner);
void Body_setPosition(Body this, const Vector3D* position, SpatialObject caller);
void Body_sleep(Body body);
void Body_stopMovement(Body this, u16 axis);
void Body_takeHitFrom(Body this, Body other);
void Body_update(Body this);
void Body_print(Body this, int x, int y);


#endif
