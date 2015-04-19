/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef ACTOR_H_
#define ACTOR_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimatedInGameEntity.h>
#include <Body.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

enum Axes
{
	kXAxis = 0,
	kYAxis,
	kZAxis,
	kLastAxis
};

// TODO: MOVE TO MISCSTRUCTS
//spacial state vector
typedef struct GeneralAxisFlag
{
	int x: 2;
	int y: 2;
	int z: 2;

} GeneralAxisFlag;


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Actor_METHODS															\
		AnimatedInGameEntity_METHODS											\
		__VIRTUAL_DEC(die);														\
		__VIRTUAL_DEC(takeHitFrom);												\
		__VIRTUAL_DEC(getAxisFreeForMovement);									\

#define Actor_SET_VTABLE(ClassName)												\
		AnimatedInGameEntity_SET_VTABLE(ClassName)								\
		__VIRTUAL_SET(ClassName, Actor, update);								\
		__VIRTUAL_SET(ClassName, Actor, transform);								\
		__VIRTUAL_SET(ClassName, Actor, handleMessage);							\
		__VIRTUAL_SET(ClassName, Actor, moves);									\
		__VIRTUAL_SET(ClassName, Actor, isMoving);								\
		__VIRTUAL_SET(ClassName, Actor, updateSpritePosition);					\
		__VIRTUAL_SET(ClassName, Actor, updateSpriteScale);						\
		__VIRTUAL_SET(ClassName, Actor, setLocalPosition);						\
		__VIRTUAL_SET(ClassName, Actor, takeHitFrom);							\
		__VIRTUAL_SET(ClassName, Actor, getAxisFreeForMovement);				\
		__VIRTUAL_SET(ClassName, Actor, getElasticity);							\
		__VIRTUAL_SET(ClassName, Actor, getPosition);							\
		__VIRTUAL_SET(ClassName, Actor, getPreviousPosition);					\
		__VIRTUAL_SET(ClassName, Actor, canMoveOverAxis);						\


#define Actor_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	AnimatedInGameEntity_ATTRIBUTES;											\
																				\
	/* a state machine to handle entity's logic	*/								\
	StateMachine stateMachine;													\
																				\
	/* a state machine to handle entity's logic	*/								\
	Body body;																	\
																				\
	/* previous position for collision handling */								\
	VBVec3D previousGlobalPosition;												\
																				\
	/* last collinding entity */												\
	InGameEntity lastCollidingEntity[kLastAxis];								\
																				\
	/* flags to apply friction on each axis */									\
	GeneralAxisFlag sensibleToFriction;											\

__CLASS(Actor);

typedef AnimatedInGameEntityDefinition ActorDefinition;
typedef const ActorDefinition ActorROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Actor, ActorDefinition* actorDefinition, s16 id);

void Actor_constructor(Actor this, ActorDefinition* actorDefinition, s16 id);
void Actor_destructor(Actor this);
void Actor_setLocalPosition(Actor this, VBVec3D position);
void Actor_transform(Actor this, Transformation* environmentTransform);
void Actor_update(Actor this);
const VBVec3D* Actor_getPreviousPosition(Actor this);
void Actor_moveOpositeDirecion(Actor this, int axis);
int Actor_changedDirection(Actor this, int axis);
void Actor_changeDirectionOnAxis(Actor this, int axis);
bool Actor_isInsideGame(Actor this);
u8 Actor_canMoveOverAxis(Actor this, const Acceleration* acceleration);
u8 Actor_getAxisFreeForMovement(Actor this);
bool Actor_handleMessage(Actor this, Telegram telegram);
StateMachine Actor_getStateMachine(Actor this);
bool Actor_moves(Actor this);
u8 Actor_isMoving(Actor this);
VBVec3D Actor_getPosition(Actor this);
bool Actor_updateSpritePosition(Actor this);
bool Actor_updateSpriteScale(Actor this);
void Actor_stopMovement(Actor this);
void Actor_alignTo(Actor this, InGameEntity entity, int axis, int pad);
const Body Actor_getBody(Actor this);
void Actor_takeHitFrom(Actor this, Actor other);
fix19_13 Actor_getElasticity(Actor this);
void Actor_addForce(Actor this, const Force* force);


#endif