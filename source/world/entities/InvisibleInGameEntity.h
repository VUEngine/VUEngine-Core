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

#ifndef INVISIBLE_IN_GAME_ENTITY_H_
#define INVISIBLE_IN_GAME_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InanimatedInGameEntity.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define InvisibleInGameEntity_METHODS											\
		InanimatedInGameEntity_METHODS													\

#define InvisibleInGameEntity_SET_VTABLE(ClassName)								\
		InanimatedInGameEntity_SET_VTABLE(ClassName)							\

// A InvisibleInGameEntity which represent a generic object inside a Stage
#define InvisibleInGameEntity_ATTRIBUTES										\
																				\
	/* super's attributes */													\
	InanimatedInGameEntity_ATTRIBUTES											\

__CLASS(InvisibleInGameEntity);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a InvisibleInGameEntity in ROM memory
typedef struct InvisibleInGameEntityDefinition
{
	// It has an InGameEntity at the beggining
	InanimatedInGameEntityDefinition inanimatedInGameEntityDefinition;

	// shape's width
	u16 width;

	// shape's height
	u16 height;

	// shape's height
	u16 deep;

} InvisibleInGameEntityDefinition;

typedef const InvisibleInGameEntityDefinition InvisibleInGameEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(InvisibleInGameEntity, __PARAMETERS(InvisibleInGameEntityDefinition* invisibleInGameEntityDefinition, s16 id));

void InvisibleInGameEntity_constructor(InvisibleInGameEntity this, InvisibleInGameEntityDefinition* invisibleInGameEntityDefinition, s16 id);
void InvisibleInGameEntity_destructor(InvisibleInGameEntity this);


#endif