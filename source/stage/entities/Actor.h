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

#ifndef ACTOR_H_
#define ACTOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimatedEntity.h>
#include <Body.h>
#include <StateMachine.h>
#include <CollisionSolver.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Actor_METHODS(ClassName)																		\
		AnimatedEntity_METHODS(ClassName)																\
		__VIRTUAL_DEC(ClassName, void, takeHitFrom, Actor other);										\
		__VIRTUAL_DEC(ClassName, void, collisionsProcessingDone, const CollisionInformation* collisionInformation);			\
		__VIRTUAL_DEC(ClassName, void, syncPositionWithBody);											\
		__VIRTUAL_DEC(ClassName, void, syncRotationWithBody);											\
		__VIRTUAL_DEC(ClassName, fix19_13, getElasticityOnCollision, SpatialObject collidingObject, const Vector3D* collidingObjectNormal);					\
		__VIRTUAL_DEC(ClassName, fix19_13, getFrictionOnCollision, SpatialObject collidingObject, const Vector3D* collidingObjectNormal);					\

#define Actor_SET_VTABLE(ClassName)																		\
		AnimatedEntity_SET_VTABLE(ClassName)															\
		__VIRTUAL_SET(ClassName, Actor, update);														\
		__VIRTUAL_SET(ClassName, Actor, transform);														\
		__VIRTUAL_SET(ClassName, Actor, initialTransform);												\
		__VIRTUAL_SET(ClassName, Actor, resume);														\
		__VIRTUAL_SET(ClassName, Actor, handleMessage);													\
		__VIRTUAL_SET(ClassName, Actor, isMoving);														\
		__VIRTUAL_SET(ClassName, Actor, getMovementState);												\
		__VIRTUAL_SET(ClassName, Actor, setLocalPosition);												\
		__VIRTUAL_SET(ClassName, Actor, takeHitFrom);													\
		__VIRTUAL_SET(ClassName, Actor, getElasticity);													\
		__VIRTUAL_SET(ClassName, Actor, getPosition);													\
		__VIRTUAL_SET(ClassName, Actor, setPosition);													\
		__VIRTUAL_SET(ClassName, Actor, isSubjectToGravity);											\
		__VIRTUAL_SET(ClassName, Actor, getVelocity);													\
		__VIRTUAL_SET(ClassName, Actor, exitCollision);												\
		__VIRTUAL_SET(ClassName, Actor, collisionsProcessingDone);										\
		__VIRTUAL_SET(ClassName, Actor, changeEnvironment);												\
		__VIRTUAL_SET(ClassName, Actor, setDefinition);													\
		__VIRTUAL_SET(ClassName, Actor, getElasticityOnCollision);										\
		__VIRTUAL_SET(ClassName, Actor, getFrictionOnCollision);										\
		__VIRTUAL_SET(ClassName, Actor, enterCollision);												\
		__VIRTUAL_SET(ClassName, Actor, syncPositionWithBody);											\
		__VIRTUAL_SET(ClassName, Actor, syncRotationWithBody);											\


#define Actor_ATTRIBUTES																				\
		/* super's attributes */																		\
		AnimatedEntity_ATTRIBUTES																		\
		/* definition */																				\
		const ActorDefinition* actorDefinition;															\
		/* a state machine to handle entity's logic	*/													\
		StateMachine stateMachine;																		\
		/* a physical body	*/																			\
		Body body;																						\
		/* collision solver */																			\
		CollisionSolver collisionSolver;																\
		/* previous velocity */																			\
		Rotation previousRotation;																		\

__CLASS(Actor);

typedef struct ActorDefinition
{
	/// it has an Entity at the beginning
	AnimatedEntityDefinition animatedEntityDefinition;

	/// create collision solver
	bool createCollisionSolver;

	/// true to create a body
	bool createBody;

	// axes subject to gravity
	u16 axesSubjectToGravity;

} ActorDefinition;

typedef const ActorDefinition ActorROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Actor, const ActorDefinition* actorDefinition, s16 id, s16 internalId, const char* const name);

void Actor_constructor(Actor this, const ActorDefinition* actorDefinition, s16 id, s16 internalId, const char* const name);
void Actor_destructor(Actor this);
void Actor_setDefinition(Actor this, void* actorDefinition);
void Actor_setLocalPosition(Actor this, const Vector3D* position);
void Actor_initialTransform(Actor this, Transformation* environmentTransform, u32 recursive);
void Actor_transform(Actor this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
void Actor_resume(Actor this);
void Actor_update(Actor this, u32 elapsedTime);
bool Actor_hasChangedDirection(Actor this, u16 axis);
void Actor_changeDirectionOnAxis(Actor this, u16 axis);
bool Actor_isInsideGame(Actor this);
bool Actor_isSubjectToGravity(Actor this, Acceleration gravity);
bool Actor_canMoveTowards(Actor this, Vector3D direction);
fix19_13 Actor_getElasticityOnCollision(Actor this, SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
fix19_13 Actor_getFrictionOnCollision(Actor this, SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
bool Actor_enterCollision(Actor this, const CollisionInformation* collisionInformation);
bool Actor_handleMessage(Actor this, Telegram telegram);
StateMachine Actor_getStateMachine(Actor this);
bool Actor_isMoving(Actor this);
u16 Actor_getMovementState(Actor this);
void Actor_changeEnvironment(Actor this, Transformation* environmentTransform);
const Vector3D* Actor_getPosition(Actor this);
void Actor_setPosition(Actor this, const Vector3D* position);
void Actor_takeHitFrom(Actor this, Actor other);
fix19_13 Actor_getElasticity(Actor this);
void Actor_addForce(Actor this, const Force* force);
void Actor_moveUniformly(Actor this, Velocity* velocity);
void Actor_stopAllMovement(Actor this);
void Actor_stopMovement(Actor this, u16 axis);
void Actor_resetCollisionStatus(Actor this);
Velocity Actor_getVelocity(Actor this);
void Actor_collisionsProcessingDone(Actor this, const CollisionInformation* collisionInformation);
void Actor_exitCollision(Actor this, Shape shape, Shape shapeNotColliding, bool isNonPenetrableShape);
void Actor_syncPositionWithBody(Actor this);
void Actor_syncRotationWithBody(Actor this);

#endif
