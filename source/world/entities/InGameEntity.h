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
 * 												MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __LEFT		 ((int)-1)
#define __RIGHT		 ((int)1)
#define __UP		 ((int)-1)
#define __DOWN		 ((int)1)
#define __NEAR		 ((int)-1)
#define __FAR		 ((int)1)

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

struct Shape_str;

#define InGameEntity_METHODS													\
		Entity_METHODS															\
		__VIRTUAL_DEC(isMoving);												\
		__VIRTUAL_DEC(getElasticity);											\
		__VIRTUAL_DEC(getFriction);												\
	
#define InGameEntity_SET_VTABLE(ClassName)										\
		Entity_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, InGameEntity, moves);							\
		__VIRTUAL_SET(ClassName, InGameEntity, isMoving);						\
		__VIRTUAL_SET(ClassName, InGameEntity, getDeep);						\
		__VIRTUAL_SET(ClassName, InGameEntity, getElasticity);					\
		__VIRTUAL_SET(ClassName, InGameEntity, getFriction);					\
		__VIRTUAL_SET(ClassName, InGameEntity, getPreviousPosition);			\
		__VIRTUAL_SET(ClassName, InGameEntity, getGap);							\

#define InGameEntity_ATTRIBUTES													\
																				\
	/* it is derivated from*/													\
	Entity_ATTRIBUTES															\
																				\
	/* pointer to the ROM definition */											\
	InGameEntityDefinition* inGameEntityDefinition;								\
																				\
	/* direction */																\
	Direction direction;														\
																				\
	/* gameworld's character's type	*/											\
	u16 inGameType;																\
																				\
	/* Gap to calculate collisions */											\
	Gap gap;																	\

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
	u8 deep;
	
	// gap for collision detection (to correct graphical collision with bgEntity's size)
	// in pixels
	Gap gap;

	/* gameworld's character's type	*/
	u16 inGameType;
	
	// flag to precalculte displacement on screen movement
	// to reduce flickering
	u8 moves;

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
__CLASS_NEW_DECLARE(InGameEntity, __PARAMETERS(InGameEntityDefinition* inGameEntityDefinition, s16 ID));

// class's constructor
void InGameEntity_constructor(InGameEntity this, InGameEntityDefinition* inGameEntityDefinition, s16 ID);

// class's destructor
void InGameEntity_destructor(InGameEntity this);

// set graphical gap
void InGameEntity_setCollisionGap(InGameEntity this, int upGap, int downGap, int leftGap, int rightGap);

// retrieve gap
Gap InGameEntity_getGap(InGameEntity this);

// calculate gap
void InGameEntity_setGap(InGameEntity this);

// retrieve in game type
u16 InGameEntity_getInGameType(InGameEntity this);

// retrieve deep
u8 InGameEntity_getDeep(InGameEntity this);

// does it moves?
u8 InGameEntity_moves(InGameEntity this);

// is it moving?
u8 InGameEntity_isMoving(InGameEntity this);

// set direction
void InGameEntity_setDirection(InGameEntity this, Direction direction);
	
// get direction
Direction InGameEntity_getDirection(InGameEntity this);

// set shape state
void InGameEntity_setShapeState(InGameEntity this, u8 state);

// get elasticiy
fix19_13 InGameEntity_getElasticity(InGameEntity this);

// get friction
fix19_13 InGameEntity_getFriction(InGameEntity this);

// retrieve previous position
VBVec3D InGameEntity_getPreviousPosition(InGameEntity this);

#endif /*INGAMEENTITY_H_*/
