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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InGameEntity.h>
#include <Prototypes.h>
#ifdef __DEBUG
#include <DirectDraw.h>
#endif


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	InGameEntity
 * @extends Entity
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(InGameEntity, Entity);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(InGameEntity, InGameEntityDefinition* inGameEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(InGameEntity, inGameEntityDefinition, id, internalId, name);

// class's constructor
void InGameEntity_constructor(InGameEntity this, InGameEntityDefinition* inGameEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "InGameEntity::constructor: null this");
	ASSERT(inGameEntityDefinition, "InGameEntity::constructor: null definition");

	__CONSTRUCT_BASE(Entity, &inGameEntityDefinition->entityDefinition, id, internalId, name);

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

// set definition
void InGameEntity_setDefinition(InGameEntity this, InGameEntityDefinition* inGameEntityDefinition)
{
	ASSERT(this, "InGameEntity::setDefinition: null this");
	ASSERT(inGameEntityDefinition, "InGameEntity::setDefinition: null definition");

	// save definition
	this->inGameEntityDefinition = inGameEntityDefinition;

	Entity_setDefinition(__SAFE_CAST(Entity, this), &inGameEntityDefinition->entityDefinition);
}

// retrieve gap
Gap InGameEntity_getGap(InGameEntity this)
{
	ASSERT(this, "InGameEntity::getGap: null this");

	return this->gap;
}

void InGameEntity_calculateGap(InGameEntity this)
{
	ASSERT(this, "InGameEntity::setGap: null this");

	if(this->sprites)
	{
		// retrieve transforming mode
		int bgmapMode = Sprite_getMode(__SAFE_CAST(Sprite, VirtualNode_getData(this->sprites->head)));

		if(__WORLD_AFFINE == bgmapMode)
		{
			// load original gap
			this->gap = this->inGameEntityDefinition->gap;

			// if facing to the left, swap left / right gap
			if(__LEFT == this->direction.x && __WORLD_AFFINE == bgmapMode)
			{
				this->gap.left 	= this->inGameEntityDefinition->gap.right;
				this->gap.right = this->inGameEntityDefinition->gap.left;
			}
		}
		else
		{
			// retrieve the sprite's scale
			Scale scale = this->transform.globalScale;

			ASSERT(scale.x, "InGameEntity::setGap: 0 scale x");
			ASSERT(scale.y, "InGameEntity::setGap: 0 scale y");

			// must scale the gap
			this->gap.left 	= FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.left), __ABS(scale.x)));
			this->gap.right = FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.right), __ABS(scale.x)));
			this->gap.up 	= FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.up), __ABS(scale.y)));
			this->gap.down 	= FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.down), __ABS(scale.y)));
		}
	}
}

// retrieve in game type
u32 InGameEntity_getInGameType(InGameEntity this)
{
	ASSERT(this, "InGameEntity::getInGameType: null this");

	return this->inGameEntityDefinition->inGameType;
}

// does it move?
bool InGameEntity_moves(InGameEntity this __attribute__ ((unused)))
{
	ASSERT(this, "InGameEntity::moves: null this");

	return false;
}

// is it moving?
int InGameEntity_isMoving(InGameEntity this __attribute__ ((unused)))
{
	ASSERT(this, "InGameEntity::isMoving: null this");

	return false;
}

int InGameEntity_getMovementState(InGameEntity this __attribute__ ((unused)))
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
