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

#ifndef INANIMATED_IN_GAME_ENTITY_H_
#define INANIMATED_IN_GAME_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InGameEntity.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define InanimatedInGameEntity_METHODS											\
		InGameEntity_METHODS													\

#define InanimatedInGameEntity_SET_VTABLE(ClassName)							\
		InGameEntity_SET_VTABLE(ClassName)										\
		__VIRTUAL_SET(ClassName, InanimatedInGameEntity, getElasticity);		\
		__VIRTUAL_SET(ClassName, InanimatedInGameEntity, getFriction);			\

// A InanimatedInGameEntity which represent a generic object inside a Stage
#define InanimatedInGameEntity_ATTRIBUTES										\
																				\
	/* super's attributes */													\
	InGameEntity_ATTRIBUTES														\
																				\
	/* ROM definition */														\
	InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition;			\

__CLASS(InanimatedInGameEntity);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a InanimatedInGameEntity in ROM memory
typedef struct InanimatedInGameEntityDefinition
{
	// It has an InGameEntity at the beggining
	InGameEntityDefinition inGameEntityDefinition;

	// friction for physics
	fix19_13 friction;

	// elasticity for physics
	fix19_13 elasticity;

	// whether it must be registered with the collision detection system
	u8 registerShape;

} InanimatedInGameEntityDefinition;

typedef const InanimatedInGameEntityDefinition InanimatedInGameEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(InanimatedInGameEntity, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition, s16 id);

void InanimatedInGameEntity_constructor(InanimatedInGameEntity this, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition, s16 id);
void InanimatedInGameEntity_destructor(InanimatedInGameEntity this);
fix19_13 InanimatedInGameEntity_getElasticity(InanimatedInGameEntity this);
fix19_13 InanimatedInGameEntity_getFriction(InanimatedInGameEntity this);


#endif