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

#ifndef TRIGGER_ENTITY_H_
#define TRIGGER_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InGameEntity.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define TriggerEntity_METHODS													\
		InGameEntity_METHODS													\


#define TriggerEntity_SET_VTABLE(ClassName)										\
		InGameEntity_SET_VTABLE(ClassName)										\
		__VIRTUAL_SET(ClassName, TriggerEntity, moves);							\


// A TriggerEntity which represent a generic object inside a Stage
#define TriggerEntity_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	InGameEntity_ATTRIBUTES														\
																				\
	/* ROM definition */														\
	TriggerEntityDefinition* triggerEntityDefinition;

__CLASS(TriggerEntity);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a TriggerEntity in ROM memory
typedef struct TriggerEntityDefinition
{
	// it has an InGameEntity at the beginning
	InGameEntityDefinition inGameEntityDefinition;

	// shape type
	u8 shapeType;
	
	// moves?
	u8 moves;

} TriggerEntityDefinition;

typedef const TriggerEntityDefinition TriggerEntityROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(TriggerEntity, TriggerEntityDefinition* triggerEntityDefinition, s16 id, const char* const name);

void TriggerEntity_constructor(TriggerEntity this, TriggerEntityDefinition* triggerEntityDefinition, s16 id, const char* const name);
void TriggerEntity_destructor(TriggerEntity this);
bool TriggerEntity_moves(TriggerEntity this);
void TriggerEntity_transform(TriggerEntity this, const Transformation* environmentTransform);


#endif