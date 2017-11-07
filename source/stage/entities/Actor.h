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
		__VIRTUAL_DEC(ClassName, u16, getAxisFreeForMovement);											\
		__VIRTUAL_DEC(ClassName, void, updateSurroundingFriction);										\
		__VIRTUAL_DEC(ClassName, int, getAxisAllowedForBouncing);										\
		__VIRTUAL_DEC(ClassName, void, collisionsProcessingDone, const CollisionInformation* collisionInformation);			\
		__VIRTUAL_DEC(ClassName, void, syncPositionWithBody);											\
		__VIRTUAL_DEC(ClassName, void, syncRotationWithBody);											\

#define Actor_SET_VTABLE(ClassName)																		\
		AnimatedEntity_SET_VTABLE(ClassName)															\
		__VIRTUAL_SET(ClassName, Actor, update);														\
		__VIRTUAL_SET(ClassName, Actor, transform);														\
		__VIRTUAL_SET(ClassName, Actor, resume);														\
		__VIRTUAL_SET(ClassName, Actor, handleMessage);													\
		__VIRTUAL_SET(ClassName, Actor, moves);															\
		__VIRTUAL_SET(ClassName, Actor, isMoving);														\
		__VIRTUAL_SET(ClassName, Actor, getMovementState);												\
		__VIRTUAL_SET(ClassName, Actor, setLocalPosition);												\
		__VIRTUAL_SET(ClassName, Actor, takeHitFrom);													\
		__VIRTUAL_SET(ClassName, Actor, getAxisFreeForMovement);										\
		__VIRTUAL_SET(ClassName, Actor, getElasticity);													\
		__VIRTUAL_SET(ClassName, Actor, getPosition);													\
		__VIRTUAL_SET(ClassName, Actor, setPosition);													\
		__VIRTUAL_SET(ClassName, Actor, getAxisAllowedForMovement);										\
		__VIRTUAL_SET(ClassName, Actor, updateSurroundingFriction);										\
		__VIRTUAL_SET(ClassName, Actor, getAxisAllowedForBouncing);										\
		__VIRTUAL_SET(ClassName, Actor, getVelocity);													\
		__VIRTUAL_SET(ClassName, Actor, collisionsProcessingDone);										\
		__VIRTUAL_SET(ClassName, Actor, changeEnvironment);												\
		__VIRTUAL_SET(ClassName, Actor, setDefinition);													\
		__VIRTUAL_SET(ClassName, Actor, processCollision);												\
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
	// it has an Entity at the beginning
	AnimatedEntityDefinition animatedEntityDefinition;

} ActorDefinition;

typedef const ActorDefinition ActorROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Actor, const ActorDefinition* actorDefinition, s16 id, s16 internalId, const char* const name);

void Actor_constructor(Actor this, const ActorDefinition* actorDefinition, s16 id, s16 internalId, const char* const name);
void Actor_destructor(Actor this);
void Actor_setDefinition(Actor this, void* actorDefinition);
void Actor_setLocalPosition(Actor this, const VBVec3D* position);
void Actor_transform(Actor this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
void Actor_resume(Actor this);
void Actor_update(Actor this, u32 elapsedTime);
int Actor_changedDirection(Actor this, u16 axis);
void Actor_changeDirectionOnAxis(Actor this, u16 axis);
bool Actor_isInsideGame(Actor this);
u16 Actor_getAxisAllowedForMovement(Actor this, const Acceleration* acceleration);
u16 Actor_getAxisFreeForMovement(Actor this);
bool Actor_processCollision(Actor this, const CollisionInformation* collisionInformation);
bool Actor_handleMessage(Actor this, Telegram telegram);
StateMachine Actor_getStateMachine(Actor this);
bool Actor_moves(Actor this);
bool Actor_isMoving(Actor this);
u16 Actor_getMovementState(Actor this);
void Actor_changeEnvironment(Actor this, Transformation* environmentTransform);
const VBVec3D* Actor_getPosition(Actor this);
void Actor_setPosition(Actor this, const VBVec3D* position);
int Actor_getAxisAllowedForBouncing(Actor this);
void Actor_takeHitFrom(Actor this, Actor other);
fix19_13 Actor_getElasticity(Actor this);
void Actor_addForce(Actor this, const Force* force, bool informAboutBodyAwakening);
void Actor_moveUniformly(Actor this, Velocity* velocity);
void Actor_stopAllMovement(Actor this);
void Actor_stopMovement(Actor this, u16 axis);
void Actor_updateSurroundingFriction(Actor this);
void Actor_resetCollisionStatus(Actor this, u16 movementAxis);
Velocity Actor_getVelocity(Actor this);
void Actor_collisionsProcessingDone(Actor this, const CollisionInformation* collisionInformation);
void Actor_syncPositionWithBody(Actor this);
void Actor_syncRotationWithBody(Actor this);

#endif
