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
		__VIRTUAL_DEC(die);


#define Actor_SET_VTABLE(ClassName)											\
		InGameEntity_SET_VTABLE(ClassName)									\
		__VIRTUAL_SET(ClassName, Actor, update);							\
		__VIRTUAL_SET(ClassName, Actor, render);							\
		__VIRTUAL_SET(ClassName, Actor, handleMessage);						\
		__VIRTUAL_SET(ClassName, Actor, moves);								\
		__VIRTUAL_SET(ClassName, Actor, isMoving);							\
		__VIRTUAL_SET(ClassName, Actor, getScale);							\
		__VIRTUAL_SET(ClassName, Actor, getInGameState);					\
		__VIRTUAL_SET(ClassName, Actor, updateSpritePosition);				\
		__VIRTUAL_SET(ClassName, Actor, updateSpriteScale);					\
		__VIRTUAL_SET(ClassName, Actor, setLocalPosition);					\


	
#define Actor_ATTRIBUTES								\
														\
	/* super's attributes */							\
	InGameEntity_ATTRIBUTES;							\
														\
	/* Pointer to the ROM definition */					\
	ActorDefinition* actorDefinition;					\
														\
	/* a state machine to handle entity's logic	*/		\
	StateMachine stateMachine;							\
														\
	/* a state machine to handle entity's logic	*/		\
	Body body;											\
														\
	Direction previousDirection;						\
														\
	/* gameworld's actor's state (ALIVE or DEAD)*/		\
	int inGameState;									


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
__CLASS_NEW_DECLARE(Actor, __PARAMETERS(ActorDefinition* actorDefinition, int inGameIndex));

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
void Actor_render(Actor this, Transformation environmentTransform);

// execute actor's logic
void Actor_update(Actor this);

// retrieve actor's scale
Scale Actor_getScale(Actor this);

// set actor's in game type
void Actor_setInGameType(Actor this, int inGameType);

// change direction
void Actor_moveOpositeDirecion(Actor this, int axis);

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

// play an animation
void Actor_playAnimation(Actor this, char* animationName);

// is play an animation
int Actor_isPlayingAnimation(Actor this, char* functionName);

// retrieve state when unloading the entity 
int Actor_getInGameState(Actor this);

// check if must update sprite's position
int Actor_updateSpritePosition(Actor this);

// check if must update sprite's scale
int Actor_updateSpriteScale(Actor this);

// retrieve body
void* Actor_getBody(Actor this);

void Actor_stopMovement(Actor this, int axis);

#endif
