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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InGameEntity.h>
#include <Prototypes.h>
#ifdef __DEBUG
#include <DirectDraw.h>
#endif


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the InGameEntity
__CLASS_DEFINITION(InGameEntity, Entity);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(InGameEntity, InGameEntityDefinition* inGameEntityDefinition, s16 id, const char* const name)
__CLASS_NEW_END(InGameEntity, inGameEntityDefinition, id, name);

// class's constructor
void InGameEntity_constructor(InGameEntity this, InGameEntityDefinition* inGameEntityDefinition, s16 id, const char* const name)
{
	ASSERT(this, "InGameEntity::constructor: null this");
	ASSERT(inGameEntityDefinition, "InGameEntity::constructor: null definition");

	__CONSTRUCT_BASE(&inGameEntityDefinition->entityDefinition, id, name);

	this->inGameEntityDefinition = inGameEntityDefinition;

	this->size.x = inGameEntityDefinition->width;
	this->size.y = inGameEntityDefinition->height;
	this->size.z = inGameEntityDefinition->depth;

	this->gap = this->inGameEntityDefinition->gap;

	this->direction.x = __RIGHT;
	this->direction.y = __DOWN;
	this->direction.z = __FAR;

	this->shape = NULL;
}

// class's destructor
void InGameEntity_destructor(InGameEntity this)
{
	ASSERT(this, "InGameEntity::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// retrieve gap
Gap InGameEntity_getGap(InGameEntity this)
{
	ASSERT(this, "InGameEntity::getGap: null this");

	InGameEntity_setGap(this);
	return this->gap;
}

void InGameEntity_setGap(InGameEntity this)
{
	ASSERT(this, "InGameEntity::setGap: null this");

	if(this->sprites)
	{
		// retrieve the sprite's scale
		Scale scale = __VIRTUAL_CALL_UNSAFE(Scale, Sprite, getScale, __SAFE_CAST(Sprite, VirtualNode_getData(this->sprites->head)));
	
		// retrieve transforming mode
		int bgmapMode = Sprite_getMode(__SAFE_CAST(Sprite, VirtualNode_getData(this->sprites->head)));
	
		// load original gap
		this->gap = this->inGameEntityDefinition->gap;
	
		// if facing to the left... swap left / right gap
		if(__LEFT == this->direction.x && WRLD_AFFINE == bgmapMode)
		{
			this->gap.left 	= this->inGameEntityDefinition->gap.right;
			this->gap.right = this->inGameEntityDefinition->gap.left;
		}
	
		ASSERT(scale.x, "InGameEntity::setGap: 0 scale x");
		ASSERT(scale.y, "InGameEntity::setGap: 0 scale y");

		// scale gap if needed
		if(WRLD_AFFINE != bgmapMode)
		{
			// must scale the gap
			this->gap.left 	= 	FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.left), abs(scale.x)));
			this->gap.right =  	FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.right), abs(scale.x)));
			this->gap.up 	= 	FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.up), abs(scale.y)));
			this->gap.down 	= 	FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.down), abs(scale.y)));
		}
	}
}

// retrieve in game type
u16 InGameEntity_getInGameType(InGameEntity this)
{
	ASSERT(this, "InGameEntity::getInGameType: null this");

	return this->inGameEntityDefinition->inGameType;
}

// does it moves?
bool InGameEntity_moves(InGameEntity this)
{
	ASSERT(this, "InGameEntity::moves: null this");

	return false;
}

// is it moving?
int InGameEntity_isMoving(InGameEntity this)
{
	ASSERT(this, "InGameEntity::isMoving: null this");

	return false;
}

int InGameEntity_getMovementState(InGameEntity this)
{
	ASSERT(this, "InGameEntity::getMovementState: null this");

	return 0;
}

// set direction
void InGameEntity_setDirection(InGameEntity this, Direction direction)
{
	ASSERT(this, "InGameEntity::setDirection: null this");

	this->direction = direction;
}

// get direction
Direction InGameEntity_getDirection(InGameEntity this)
{
	ASSERT(this, "InGameEntity::getDirection: null this");

	// TODO: must be recursive to account for parenting 
	return this->direction;
}

// set shape state
void InGameEntity_setShapeState(InGameEntity this, bool state)
{
	ASSERT(this, "InGameEntity::setShapeState: null this");

	if(this->shape)
	{
		Shape_setActive(this->shape, state);
	}
}
