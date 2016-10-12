/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef ACTOR_H_
#define ACTOR_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimatedInGameEntity.h>
#include <Body.h>
#include <StateMachine.h>
#include <CollisionSolver.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Actor_METHODS(ClassName)																		\
		AnimatedInGameEntity_METHODS(ClassName)															\
		__VIRTUAL_DEC(ClassName, void, takeHitFrom, Actor other);										\
		__VIRTUAL_DEC(ClassName, int, getAxisFreeForMovement);											\
		__VIRTUAL_DEC(ClassName, void, updateSurroundingFriction);										\
		__VIRTUAL_DEC(ClassName, int, getAxisAllowedForBouncing);										\
		__VIRTUAL_DEC(ClassName, void, collisionsProcessingDone, VirtualList collidingSpatialObjects);	\

#define Actor_SET_VTABLE(ClassName)																		\
		AnimatedInGameEntity_SET_VTABLE(ClassName)														\
		__VIRTUAL_SET(ClassName, Actor, update);														\
		__VIRTUAL_SET(ClassName, Actor, transform);														\
		__VIRTUAL_SET(ClassName, Actor, resume);														\
		__VIRTUAL_SET(ClassName, Actor, handleMessage);													\
		__VIRTUAL_SET(ClassName, Actor, moves);															\
		__VIRTUAL_SET(ClassName, Actor, isMoving);														\
		__VIRTUAL_SET(ClassName, Actor, getMovementState);												\
		__VIRTUAL_SET(ClassName, Actor, updateSpritePosition);											\
		__VIRTUAL_SET(ClassName, Actor, updateSpriteScale);									\
		__VIRTUAL_SET(ClassName, Actor, setLocalPosition);												\
		__VIRTUAL_SET(ClassName, Actor, takeHitFrom);													\
		__VIRTUAL_SET(ClassName, Actor, getAxisFreeForMovement);										\
		__VIRTUAL_SET(ClassName, Actor, getElasticity);													\
		__VIRTUAL_SET(ClassName, Actor, getFriction);													\
		__VIRTUAL_SET(ClassName, Actor, getPosition);													\
		__VIRTUAL_SET(ClassName, Actor, setPosition);													\
		__VIRTUAL_SET(ClassName, Actor, canMoveOverAxis);												\
		__VIRTUAL_SET(ClassName, Actor, updateSurroundingFriction);										\
		__VIRTUAL_SET(ClassName, Actor, getAxisAllowedForBouncing);										\
		__VIRTUAL_SET(ClassName, Actor, getVelocity);													\
		__VIRTUAL_SET(ClassName, Actor, collisionsProcessingDone);										\


#define Actor_ATTRIBUTES																				\
        /* super's attributes */																		\
        AnimatedInGameEntity_ATTRIBUTES																	\
        /* definition */																				\
        const ActorDefinition* actorDefinition;															\
        /* a state machine to handle entity's logic	*/													\
        StateMachine stateMachine;																		\
        /* a physical body	*/																			\
        Body body;																						\
        /* collision solver */																			\
        CollisionSolver collisionSolver;																\

__CLASS(Actor);

typedef struct ActorDefinition
{
	// it has an InGameEntity at the beginning
	AnimatedInGameEntityDefinition animatedInGameEntityDefinition;

	// friction for physics
	fix19_13 friction;

	// elasticity for physics
	fix19_13 elasticity;

	// animation to play automatically
	fix19_13 mass;

} ActorDefinition;

typedef const ActorDefinition ActorROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Actor, const ActorDefinition* actorDefinition, s16 id, const char* const name);

void Actor_constructor(Actor this, const ActorDefinition* actorDefinition, s16 id, const char* const name);
void Actor_destructor(Actor this);
void Actor_setLocalPosition(Actor this, const VBVec3D* position);
void Actor_transform(Actor this, const Transformation* environmentTransform);
void Actor_resume(Actor this);
void Actor_update(Actor this, u32 elapsedTime);
void Actor_moveOppositeDirection(Actor this, int axis);
int Actor_changedDirection(Actor this, int axis);
void Actor_changeDirectionOnAxis(Actor this, int axis);
bool Actor_isInsideGame(Actor this);
int Actor_canMoveOverAxis(Actor this, const Acceleration* acceleration);
int Actor_getAxisFreeForMovement(Actor this);
bool Actor_handleMessage(Actor this, Telegram telegram);
StateMachine Actor_getStateMachine(Actor this);
bool Actor_moves(Actor this);
int Actor_isMoving(Actor this);
int Actor_getMovementState(Actor this);
void Actor_changeEnvironment(Actor this, Transformation* environmentTransform);
const VBVec3D* Actor_getPosition(Actor this);
void Actor_setPosition(Actor this, const VBVec3D* position);
bool Actor_updateSpritePosition(Actor this);
bool Actor_updateSpriteScale(Actor this);
int Actor_getAxisAllowedForBouncing(Actor this);
void Actor_alignTo(Actor this, SpatialObject spatialObject, bool registerObject);
void Actor_takeHitFrom(Actor this, Actor other);
fix19_13 Actor_getElasticity(Actor this);
fix19_13 Actor_getFriction(Actor this);
void Actor_addForce(Actor this, const Force* force);
void Actor_moveUniformly(Actor this, Velocity* velocity);
void Actor_stopMovement(Actor this, u32 stopShape);
void Actor_updateSurroundingFriction(Actor this);
void Actor_resetCollisionStatus(Actor this, int movementAxis);
Velocity Actor_getVelocity(Actor this);
void Actor_collisionsProcessingDone(Actor this, VirtualList collidingSpatialObjects);

#endif
