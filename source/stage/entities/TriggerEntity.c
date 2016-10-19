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

#include <TriggerEntity.h>
#include <Game.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the TriggerEntity
__CLASS_DEFINITION(TriggerEntity, InGameEntity);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(TriggerEntity, TriggerEntityDefinition* triggerEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(TriggerEntity, triggerEntityDefinition, id, internalId, name);

// class's constructor
void TriggerEntity_constructor(TriggerEntity this, TriggerEntityDefinition* triggerEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "TriggerEntity::constructor: null this");
	ASSERT(triggerEntityDefinition, "TriggerEntity::constructor: null definition");

	// construct base object
	__CONSTRUCT_BASE(InGameEntity, &triggerEntityDefinition->inGameEntityDefinition, id, internalId, name);

	this->triggerEntityDefinition = triggerEntityDefinition;

	// register a shape for collision detection
	this->shape = CollisionManager_registerShape(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(SpatialObject, this), triggerEntityDefinition->shapeType);

	ASSERT(this->shape, "TriggerEntity::constructor: shape not created");
}

void TriggerEntity_destructor(TriggerEntity this)
{
	ASSERT(this, "TriggerEntity::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// set definition
void TriggerEntity_setDefinition(TriggerEntity this, TriggerEntityDefinition* triggerEntityDefinition)
{
	ASSERT(this, "TriggerEntity::setDefinition: null this");
	ASSERT(triggerEntityDefinition, "TriggerEntity::setDefinition: null definition");

	// save definition
	this->triggerEntityDefinition = triggerEntityDefinition;

	InGameEntity_setDefinition(__SAFE_CAST(InGameEntity, this), &triggerEntityDefinition->inGameEntityDefinition);
}

// does it move?
bool TriggerEntity_moves(TriggerEntity this)
{
	ASSERT(this, "TriggerEntity::moves: null this");

	return this->triggerEntityDefinition->moves;
}

