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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

struct Shape_str;

#define InGameEntity_METHODS													\
		Entity_METHODS															\
		__VIRTUAL_DEC(isMoving);												\
		__VIRTUAL_DEC(getInGameType);											\
		
#define InGameEntity_SET_VTABLE(ClassName)										\
		Entity_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, InGameEntity, moves);							\
		__VIRTUAL_SET(ClassName, InGameEntity, isMoving);						\
		__VIRTUAL_SET(ClassName, InGameEntity, getGap);							\
		__VIRTUAL_SET(ClassName, InGameEntity, getInGameType);					\
		
#define InGameEntity_ATTRIBUTES													\
																				\
	/* it is derivated from */													\
	Entity_ATTRIBUTES															\
																				\
	/* pointer to the ROM definition */											\
	InGameEntityDefinition* inGameEntityDefinition;								\
																				\
	/* direction */																\
	Direction direction;														\
																				\
	/* Gap to calculate collisions */											\
	Gap gap;																	\

__CLASS(InGameEntity);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a InGameEntity in ROM memory
typedef struct InGameEntityDefinition
{
	// It has an Entity at the beggining
	EntityDefinition entityDefinition;

	// gap for collision detection (to correct graphical collision with bgEntity's size)
	// in pixels
	Gap gap;

	/* gameworld's character's type	*/
	u16 inGameType;

	// object's size over the z axis
	u16 width;

	// object's size over the z axis
	u16 height;

	// object's size over the z axis
	u16 depth;

} InGameEntityDefinition;

typedef const InGameEntityDefinition InGameEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(InGameEntity, InGameEntityDefinition* inGameEntityDefinition, s16 id);

void InGameEntity_constructor(InGameEntity this, InGameEntityDefinition* inGameEntityDefinition, s16 id);
void InGameEntity_destructor(InGameEntity this);
void InGameEntity_setCollisionGap(InGameEntity this, int upGap, int downGap, int leftGap, int rightGap);
Gap InGameEntity_getGap(InGameEntity this);
void InGameEntity_setGap(InGameEntity this);
u16 InGameEntity_getInGameType(InGameEntity this);
bool InGameEntity_moves(InGameEntity this);
u8 InGameEntity_isMoving(InGameEntity this);
void InGameEntity_setDirection(InGameEntity this, Direction direction);
Direction InGameEntity_getDirection(InGameEntity this);
void InGameEntity_setShapeState(InGameEntity this, bool state);


#endif