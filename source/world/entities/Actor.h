/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <InGameEntity.h>
#include <AnimatedSprite.h>
#include <Body.h>
#include <Clock.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// defines an Actor
typedef struct ActorDefinition{

	// It has an InGameEntity at the beggining
	InGameEntityDefinition inGameEntityDefinition;
	
	// the animation
	AnimationDescription* animationDescription;
	
}ActorDefinition;


typedef const ActorDefinition ActorROMDef;


// TODO: MOVE TO MISCSTRUCTS
//spacial state vector
typedef struct GeneralAxisFlag{
	
	int x: 2;
	int y: 2;
	int z: 2;
	
}GeneralAxisFlag;

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */




// declare the virtual methods
#define Actor_METHODS								\
		InGameEntity_METHODS						\
		__VIRTUAL_DEC(die);							\
		__VIRTUAL_DEC(takeHitFrom);					\
		__VIRTUAL_DEC(getAxisFreeForMovement);		\


#define Actor_SET_VTABLE(ClassName)											\
		InGameEntity_SET_VTABLE(ClassName)									\
		__VIRTUAL_SET(ClassName, Actor, update);							\
		__VIRTUAL_SET(ClassName, Actor, transform);							\
		__VIRTUAL_SET(ClassName, Actor, handleMessage);						\
		__VIRTUAL_SET(ClassName, Actor, moves);								\
		__VIRTUAL_SET(ClassName, Actor, isMoving);							\
		__VIRTUAL_SET(ClassName, Actor, getScale);							\
		__VIRTUAL_SET(ClassName, Actor, getInGameState);					\
		__VIRTUAL_SET(ClassName, Actor, updateSpritePosition);				\
		__VIRTUAL_SET(ClassName, Actor, updateSpriteScale);					\
		__VIRTUAL_SET(ClassName, Actor, setLocalPosition);					\
		__VIRTUAL_SET(ClassName, Actor, takeHitFrom);						\
		__VIRTUAL_SET(ClassName, Actor, getAxisFreeForMovement);			\
		__VIRTUAL_SET(ClassName, Actor, getElasticity);						\
		__VIRTUAL_SET(ClassName, Actor, getPosition);						\
		__VIRTUAL_SET(ClassName, Actor, getPreviousPosition);				\
		

	
#define Actor_ATTRIBUTES								\
														\
	/* super's attributes */							\
	InGameEntity_ATTRIBUTES;							\
														\
	/* Pointer to the ROM definition */					\
	ActorDefinition* actorDefinition;					\
														\
	/* Pointer to the animation description */			\
	AnimationDescription* animationDescription;			\
														\
	/* a state machine to handle entity's logic	*/		\
	StateMachine stateMachine;							\
														\
	/* a state machine to handle entity's logic	*/		\
	Body body;											\
														\
	Direction previousDirection;						\
														\
	/* previous position for collision handling */		\
	VBVec3D previousGlobalPosition;						\
														\
	/* last collinding entity */						\
	InGameEntity lastCollidingEntity[3];				\
														\
	/* gameworld's actor's state (ALIVE or DEAD)*/		\
	int inGameState;									\
														\
	/* flags to apply friction on each axis */			\
	GeneralAxisFlag sensibleToFriction;					\
														\
	/* flag to influence with gravity */				\
	int isAffectedBygravity: 1;							\
														\
	/* clock to pass to the animated sprites */			\
	Clock clock;										\


__CLASS(Actor);													


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(Actor, __PARAMETERS(ActorDefinition* actorDefinition, int ID));

// class's constructor
void Actor_constructor(Actor this, ActorDefinition* actorDefinition, int ID);

// class's destructor
void Actor_destructor(Actor this);

//set class's local position
void Actor_setLocalPosition(Actor this, VBVec3D position);

// set actor's collision gap
void Actor_setCollisionGap(Actor this, int upGap, int downGap, int leftGap, int rightGap);

// write actor to graphic memory
void Actor_write(Actor this);

// graphically refresh of actors that are visible
void Actor_transform(Actor this, Transformation* environmentTransform);

// execute actor's logic
void Actor_update(Actor this);

// retrieve previous position
VBVec3D Actor_getPreviousPosition(Actor this);

// retrieve actor's scale
Scale Actor_getScale(Actor this);

// retrieve global position
VBVec3D Actor_getPosition(Actor this);

// set actor's in game type
void Actor_setInGameType(Actor this, int inGameType);

// change direction
void Actor_moveOpositeDirecion(Actor this, int axis);

// whether changed direction in the last cycle or not
int Actor_changedDirection(Actor this, int axis);

// change direction over axis
void Actor_changeDirectionOnAxis(Actor this, int axis);

// check if gravity must apply to this actor
int Actor_canMoveOverAxis(Actor this, const Acceleration* acceleration);

// retrieve axis free for movement
int Actor_getAxisFreeForMovement(Actor this);

// true if inside the screen range 
int Actor_isInsideGame(Actor this);

// allocate a write in graphic memory again
void Actor_resetMemoryState(Actor this, int worldLayer);

// process a telegram
int Actor_handleMessage(Actor this, Telegram telegram);

// retrieve state machine
StateMachine Actor_getStateMachine(Actor this);

// align actor to other entity on collision
void Actor_alignTo(Actor this, InGameEntity entity, int axis, int pad);

// does it moves?
int Actor_moves(Actor this);

// is moving?
int Actor_isMoving(Actor this);

// retrieve scale
Scale Actor_getScale(Actor this);

// pause animation
void Actor_pauseAnimation(Actor this, int pause);

// play an animation
void Actor_playAnimation(Actor this, char* animationName);

// is animation loaded
int Actor_isAnimationLoaded(Actor this, char* functionName);

// is play an animation
int Actor_isPlayingAnimation(Actor this);

// retrieve state when unloading the entity 
int Actor_getInGameState(Actor this);

// check if must update sprite's position
int Actor_updateSpritePosition(Actor this);

// check if must update sprite's scale
int Actor_updateSpriteScale(Actor this);

// retrieve body
const Body Actor_getBody(Actor this);

// stop movement completelty
void Actor_stopMovement(Actor this);

// stop movement over axis
void Actor_stopMovementOnAxis(Actor this, int axis);

// take hit
void Actor_takeHitFrom(Actor this, Actor other);

// get elasticiy
fix19_13 Actor_getElasticity(Actor this);

// get animation description
AnimationDescription* Actor_getAnimationDescription(Actor this);

// set animation description
void Actor_setAnimationDescription(Actor this, AnimationDescription* animationDescription);

// set animation clock
void Actor_setClock(Actor this, Clock clock);

#endif
