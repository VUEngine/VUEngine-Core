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
		__VIRTUAL_DEC(beThrown);					\
		__VIRTUAL_DEC(die);


#define Actor_SET_VTABLE(ClassName)											\
		InGameEntity_SET_VTABLE(ClassName)									\
		__VIRTUAL_SET(ClassName, Actor, update);							\
		__VIRTUAL_SET(ClassName, Actor, render);							\
		__VIRTUAL_SET(ClassName, Actor, handleMessage);						\
		__VIRTUAL_SET(ClassName, Actor, moves);								\
		__VIRTUAL_SET(ClassName, Actor, isMoving);							\
		__VIRTUAL_SET(ClassName, Actor, getScale);							\
		__VIRTUAL_SET(ClassName, Actor, beThrown);							\
		__VIRTUAL_SET(ClassName, Actor, getInGameState);					\
		__VIRTUAL_SET(ClassName, Actor, updateSpritePosition);				\
		__VIRTUAL_SET(ClassName, Actor, updateSpriteScale);					\


	
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
	/* actor velocity on each instante */				\
	Velocity velocity;									\
														\
	/* state of actor's movement over each axis */		\
	/*MovementState movementState;*/					\
														\
	/* acelearion structure */							\
	Acceleration acceleration;							\
														\
	Direction previousDirection;						\
														\
	/* time for movement over each axis	*/				\
	unsigned long timeX;								\
	unsigned long timeY;								\
	unsigned long timeZ;								\
														\
	/* max fly time in seconds	*/						\
	unsigned long maxFlyTime;							\
														\
	/* type of movement (accelerated or not) */			\
	int moveType;										\
														\
	/* gameworld's actor's state (ALIVE or DEAD)*/		\
	int inGameState;									\
														\
	/* a pointer to the object I'm walking on */		\
	/* to being able to turn aroung and don't */		\
	/* fall to dead */									\
	InGameEntity objectBelow;


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

// set actor's angle
void Actor_setAngle(Actor this, Angle angle);

// calculate actor's direction angle
void Actor_calculateAngles(Actor this, int axis);

// set velocity
void Actor_setVelocity(Actor this, int xVel, int yVel, int zVel);

// set aceleration
void Actor_setAceleration(Actor this, int xAcel, int yAcel, int zAcel);

// set actor's collision gap
void Actor_setCollisionGap(Actor this, int upGap, int downGap, int leftGap, int rightGap);

// write actor to graphic memory
void Actor_write(Actor this);

// graphically refresh of actors that are visible
void Actor_render(Actor this, Transformation environmentTransform);

// execute actor's logic
void Actor_update(Actor this);

// retrieve actor's velocity
Velocity Actor_getVelocity(Actor this);

// retrieve actor's scale
Scale Actor_getScale(Actor this);

// set actor's in game type
void Actor_setInGameType(Actor this, int inGameType);

// change direction
void Actor_moveOpositeDirecion(Actor this, int axis);

// update movement
void Actor_move(Actor this);

// make a actor to jump
void Actor_jump(Actor this, fix19_13 velocity, fix19_13 acceleration);

// make a actor fall
void Actor_fall(Actor this);

// start the movement over the x axis
void Actor_startMovement(Actor this, int axis, int moveType, fix19_13 velocity, fix19_13 acceleration);

// calculate acceleration over an axis
void Actor_calculateAcceleration(Actor this, int axis);

// udpdate movement over axis
void Actor_keepYMovement(Actor this);

// udpdate movement over axis
void Actor_keepXMovement(Actor this);

// udpdate movement over axis
void Actor_keepZMovement(Actor this);

// for debug
void Actor_printPhysics(Actor this, int x, int y);

// stop movement over an axis
void Actor_stopMovement(Actor this, int axis);

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

// be thrown over an axis and a direction
void Actor_beThrown(Actor this, int axis, int direction);

// reset timers
void Actor_resetTimers(Actor this);

// retrieve state when unloading the entity 
int Actor_getInGameState(Actor this);

// the the object below me
void Actor_setObjectBelow(Actor this, InGameEntity objectBelow);

// check if must update sprite's position
int Actor_updateSpritePosition(Actor this);

// check if must update sprite's scale
int Actor_updateSpriteScale(Actor this);


#endif
