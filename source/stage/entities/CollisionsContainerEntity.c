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

#include <string.h>
#include <CollisionsContainerEntity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the CollisionsContainerEntity
__CLASS_DEFINITION(CollisionsContainerEntity, Entity);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(CollisionsContainerEntity, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(CollisionsContainerEntity, collisionsContainerEntityDefinition, id, internalId, name);


// class's constructor
void CollisionsContainerEntity_constructor(CollisionsContainerEntity this, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "CollisionsContainerEntity::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(Entity, collisionsContainerEntityDefinition, id, internalId, name);
}

// class's destructor
void CollisionsContainerEntity_destructor(CollisionsContainerEntity this)
{
	ASSERT(this, "CollisionsContainerEntity::destructor: null this");

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void CollisionsContainerEntity_ready(CollisionsContainerEntity this __attribute__ ((unused)), u32 recursive __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::ready: null this");
}

void CollisionsContainerEntity_update(CollisionsContainerEntity this __attribute__ ((unused)), u32 elapsedTime __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::update: null this");
}

void CollisionsContainerEntity_transform(CollisionsContainerEntity this __attribute__ ((unused)), const Transformation* environmentTransform __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::transform: null this");
}

void CollisionsContainerEntity_updateVisualRepresentation(CollisionsContainerEntity this __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::updateVisualRepresentation: null this");
}

bool CollisionsContainerEntity_handleMessage(CollisionsContainerEntity this __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::handleMessage: null this");

	return true;
}

bool CollisionsContainerEntity_updateSpritePosition(CollisionsContainerEntity this __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::updateSpritePosition: null this");

	return false;
}

bool CollisionsContainerEntity_updateSpriteRotation(CollisionsContainerEntity this __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::updateSpriteRotation: null this");

	return false;
}

bool CollisionsContainerEntity_updateSpriteScale(CollisionsContainerEntity this __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::updateSpriteScale: null this");

	return false;
}

bool CollisionsContainerEntity_handlePropagatedMessage(CollisionsContainerEntity this __attribute__ ((unused)), int message __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::handlePropagatedMessage: null this");

	return false;
}

int CollisionsContainerEntity_passMessage(CollisionsContainerEntity this __attribute__ ((unused)), int (*propagatedMessageHandler)(Container this, va_list args) __attribute__ ((unused)), va_list args __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::passMessage: null this");

	return false;
}
