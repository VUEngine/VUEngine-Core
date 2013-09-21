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
 
#ifndef INGAMEENTITY_H_
#define INGAMEENTITY_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Entity.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define InGameEntity_METHODS							\
		Entity_METHODS									\
		__VIRTUAL_DEC(moves);							\
		__VIRTUAL_DEC(isMoving);
	
	

#define InGameEntity_SET_VTABLE(ClassName)								\
		Entity_SET_VTABLE(ClassName)									\
		__VIRTUAL_SET(ClassName, InGameEntity, setLocalPosition);		\
		__VIRTUAL_SET(ClassName, InGameEntity, moves);					\
		__VIRTUAL_SET(ClassName, InGameEntity, render);					\
		__VIRTUAL_SET(ClassName, InGameEntity, isMoving);
		

// A InGameEntity which represent a generic object inside a Stage
#define InGameEntity_ATTRIBUTES						\
													\
	/* it is derivated from*/						\
	Entity_ATTRIBUTES								\
													\
	/* pointer to the ROM definition */				\
	InGameEntityDefinition* inGameEntityDefinition;	\
													\
	/* direction */									\
	Direction direction;							\
													\
	/* gameworld's character's type	*/				\
	int inGameType;									\
													\
	/* Gap to calculate collisions */				\
	Gap gap;										\
													\
	/* shape for collision detection */				\
	Shape shape;

__CLASS(InGameEntity);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// defines a InGameEntity in ROM memory
typedef struct InGameEntityDefinition{

	// It has an Entity at the beggining
	EntityDefinition entityDefinition;
	
	// object's size over the z axis
	int deep;
	
	// friction factor of the object
	fix7_9 frictionFactor;

	// gap for collision detection (to correct graphical collision with bgEntity's size)
	// in pixels
	Gap gap;

	/* gameworld's character's type	*/
	int inGameType;

}InGameEntityDefinition;

typedef const InGameEntityDefinition InGameEntityROMDef;

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(InGameEntity, __PARAMETERS(InGameEntityDefinition* inGameEntityDefinition, int inGameIndex));

// class's constructor
void InGameEntity_constructor(InGameEntity this, InGameEntityDefinition* inGameEntityDefinition, int inGameIndex);

// class's destructor
void InGameEntity_destructor(InGameEntity this);

// retrieve InGameEntity's friction
fix7_9 InGameEntity_getFrictionFactor(InGameEntity this);

// set graphical gap
void InGameEntity_setCollisionGap(InGameEntity this, int upGap, int downGap, int leftGap, int rightGap);

// retrieve z grossor
int InGameEntity_getDeep(InGameEntity this);

// set class's position
void InGameEntity_setLocalPosition(InGameEntity this, VBVec3D position);

// retrieve gap
Gap InGameEntity_getGap(InGameEntity this);

// calculate gap
void InGameEntity_setGap(InGameEntity this);

// retrieve in game type
int InGameEntity_getInGameType(InGameEntity this);

// retrieve deep
int InGameEntity_getDeep(InGameEntity this);

// does it moves?
int InGameEntity_moves(InGameEntity this);

// is it moving?
int InGameEntity_isMoving(InGameEntity this);

// set direction
void InGameEntity_setDirection(InGameEntity this, Direction direction);
	
// get direction
Direction InGameEntity_getDirection(InGameEntity this);

// set shape state
void InGameEntity_setShapeState(InGameEntity this, int state);

//render class
void InGameEntity_render(InGameEntity this, Transformation environmentTransform);



#endif /*INGAMEENTITY_H_*/
