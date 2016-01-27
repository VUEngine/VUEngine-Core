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

#include <string.h>
#include <CollisionsContainerEntity.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the CollisionsContainerEntity
__CLASS_DEFINITION(CollisionsContainerEntity, Entity);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(CollisionsContainerEntity, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, const char* const name)
__CLASS_NEW_END(CollisionsContainerEntity, collisionsContainerEntityDefinition, id, name);


// class's constructor
void CollisionsContainerEntity_constructor(CollisionsContainerEntity this, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, const char* const name)
{
	ASSERT(this, "CollisionsContainerEntity::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(collisionsContainerEntityDefinition, id, name);
}

// class's destructor
void CollisionsContainerEntity_destructor(CollisionsContainerEntity this)
{
	ASSERT(this, "CollisionsContainerEntity::destructor: null this");

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void CollisionsContainerEntity_ready(CollisionsContainerEntity this)
{
	ASSERT(this, "CollisionsContainerEntity::ready: null this");
}

void CollisionsContainerEntity_update(CollisionsContainerEntity this)
{
	ASSERT(this, "CollisionsContainerEntity::update: null this");
}

void CollisionsContainerEntity_transform(CollisionsContainerEntity this, const Transformation* environmentTransform)
{
	ASSERT(this, "CollisionsContainerEntity::transform: null this");
}

void CollisionsContainerEntity_updateVisualRepresentation(CollisionsContainerEntity this)
{
	ASSERT(this, "CollisionsContainerEntity::updateVisualRepresentation: null this");
}

bool CollisionsContainerEntity_handleMessage(CollisionsContainerEntity this, Telegram telegram)
{
	ASSERT(this, "CollisionsContainerEntity::handleMessage: null this");

	return true;
}

bool CollisionsContainerEntity_updateSpritePosition(CollisionsContainerEntity this)
{
	ASSERT(this, "CollisionsContainerEntity::updateSpritePosition: null this");

	return false;
}

bool CollisionsContainerEntity_updateSpriteTransformations(CollisionsContainerEntity this)
{
	ASSERT(this, "CollisionsContainerEntity::updateSpriteTransformations: null this");

	return false;
}

bool CollisionsContainerEntity_handlePropagatedMessage(CollisionsContainerEntity this, int message)
{
	ASSERT(this, "CollisionsContainerEntity::handlePropagatedMessage: null this");

	return false;
}

int CollisionsContainerEntity_passMessage(CollisionsContainerEntity this, int (*propagatedMessageHandler)(Container this, va_list args), va_list args)
{
	ASSERT(this, "CollisionsContainerEntity::passMessage: null this");

	return false;
}
