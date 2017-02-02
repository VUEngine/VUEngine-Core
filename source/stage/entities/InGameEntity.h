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

#ifndef IN_GAME_ENTITY_H_
#define IN_GAME_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

struct Shape_str;

#define InGameEntity_METHODS(ClassName)																	\
		Entity_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, int, isMoving);														\
		__VIRTUAL_DEC(ClassName, int, getMovementState);												\
		__VIRTUAL_DEC(ClassName, u32, getInGameType);													\

#define InGameEntity_SET_VTABLE(ClassName)																\
		Entity_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, InGameEntity, moves);													\
		__VIRTUAL_SET(ClassName, InGameEntity, isMoving);												\
		__VIRTUAL_SET(ClassName, InGameEntity, getMovementState);										\
		__VIRTUAL_SET(ClassName, InGameEntity, getGap);													\
		__VIRTUAL_SET(ClassName, InGameEntity, getInGameType);											\

#define InGameEntity_ATTRIBUTES																			\
		/* it is derived from */																		\
		Entity_ATTRIBUTES																				\
		/* pointer to the ROM definition */																\
		InGameEntityDefinition* inGameEntityDefinition;													\
		/* direction */																					\
		Direction direction;																			\
		/* Gap to calculate collisions */																\
		Gap gap;																						\

__CLASS(InGameEntity);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a InGameEntity in ROM memory
typedef struct InGameEntityDefinition
{
	// it has an Entity at the beginning
	EntityDefinition entityDefinition;

	// gap for collision detection (to correct graphical collision with bgEntity's size) (in pixels)
	Gap gap;

	// gameworld's character's type
	u32 inGameType;

	// object's size over the x axis
	u16 width;

	// object's size over the y axis
	u16 height;

	// object's size over the z axis
	u16 depth;

} InGameEntityDefinition;

typedef const InGameEntityDefinition InGameEntityROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(InGameEntity, InGameEntityDefinition* inGameEntityDefinition, s16 id, s16 internalId, const char* const name);

void InGameEntity_constructor(InGameEntity this, InGameEntityDefinition* inGameEntityDefinition, s16 id, s16 internalId, const char* const name);
void InGameEntity_destructor(InGameEntity this);
void InGameEntity_setDefinition(InGameEntity this, InGameEntityDefinition* inGameEntityDefinition);
void InGameEntity_setCollisionGap(InGameEntity this, int upGap, int downGap, int leftGap, int rightGap);
Gap InGameEntity_getGap(InGameEntity this);
void InGameEntity_setGap(InGameEntity this);
u32 InGameEntity_getInGameType(InGameEntity this);
bool InGameEntity_moves(InGameEntity this);
int InGameEntity_isMoving(InGameEntity this);
int InGameEntity_getMovementState(InGameEntity this);
void InGameEntity_setDirection(InGameEntity this, Direction direction);
Direction InGameEntity_getDirection(InGameEntity this);
void InGameEntity_setShapeState(InGameEntity this, bool state);


#endif
