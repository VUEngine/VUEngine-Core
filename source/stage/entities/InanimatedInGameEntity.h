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

#ifndef INANIMATED_IN_GAME_ENTITY_H_
#define INANIMATED_IN_GAME_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InGameEntity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define InanimatedInGameEntity_METHODS(ClassName)														\
		InGameEntity_METHODS(ClassName)																	\

#define InanimatedInGameEntity_SET_VTABLE(ClassName)													\
		InGameEntity_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, InanimatedInGameEntity, getElasticity);								\
		__VIRTUAL_SET(ClassName, InanimatedInGameEntity, getFriction);									\

#define InanimatedInGameEntity_ATTRIBUTES																\
		/* super's attributes */																		\
		InGameEntity_ATTRIBUTES																			\
		/* ROM definition */																			\
		InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition;								\

__CLASS(InanimatedInGameEntity);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a InanimatedInGameEntity in ROM memory
typedef struct InanimatedInGameEntityDefinition
{
	// it has an InGameEntity at the beginning
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
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(InanimatedInGameEntity, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition, s16 id, s16 internalId, const char* const name);

void InanimatedInGameEntity_constructor(InanimatedInGameEntity this, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition, s16 id, s16 internalId, const char* const name);
void InanimatedInGameEntity_destructor(InanimatedInGameEntity this);
void InanimatedInGameEntity_setDefinition(InanimatedInGameEntity this, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition);
fix19_13 InanimatedInGameEntity_getElasticity(InanimatedInGameEntity this);
fix19_13 InanimatedInGameEntity_getFriction(InanimatedInGameEntity this);


#endif
