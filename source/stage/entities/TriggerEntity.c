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

#include <TriggerEntity.h>
#include <Game.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	TriggerEntity
 * @extends InGameEntity
 */
__CLASS_DEFINITION(TriggerEntity, InGameEntity);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
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

