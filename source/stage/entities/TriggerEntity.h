/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef TRIGGER_ENTITY_H_
#define TRIGGER_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InGameEntity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define TriggerEntity_METHODS(ClassName)																\
		InGameEntity_METHODS(ClassName)																	\

#define TriggerEntity_SET_VTABLE(ClassName)																\
		InGameEntity_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, TriggerEntity, moves);													\

#define TriggerEntity_ATTRIBUTES																		\
		/* super's attributes */																		\
		InGameEntity_ATTRIBUTES																			\
		/* ROM definition */																			\
		TriggerEntityDefinition* triggerEntityDefinition;

__CLASS(TriggerEntity);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
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
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(TriggerEntity, TriggerEntityDefinition* triggerEntityDefinition, s16 id, s16 internalId, const char* const name);

void TriggerEntity_constructor(TriggerEntity this, TriggerEntityDefinition* triggerEntityDefinition, s16 id, s16 internalId, const char* const name);
void TriggerEntity_destructor(TriggerEntity this);
void TriggerEntity_setDefinition(TriggerEntity this, TriggerEntityDefinition* triggerEntityDefinition);
bool TriggerEntity_moves(TriggerEntity this);
void TriggerEntity_transform(TriggerEntity this, const Transformation* environmentTransform);


#endif
