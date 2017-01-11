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
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InanimatedInGameEntity.h>
#include <Game.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the InanimatedInGameEntity
__CLASS_DEFINITION(InanimatedInGameEntity, InGameEntity);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(InanimatedInGameEntity, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(InanimatedInGameEntity, inanimatedInGameEntityDefinition, id, internalId, name);

// class's constructor
void InanimatedInGameEntity_constructor(InanimatedInGameEntity this, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "InanimatedInGameEntity::constructor: null this");
	ASSERT(inanimatedInGameEntityDefinition, "InanimatedInGameEntity::constructor: null definition");

	// construct base object
	__CONSTRUCT_BASE(InGameEntity, &inanimatedInGameEntityDefinition->inGameEntityDefinition, id, internalId, name);

	// check if register for collision detection
	if(inanimatedInGameEntityDefinition->registerShape)
	{
		// register a shape for collision detection
		this->shape = CollisionManager_registerShape(Game_getCollisionManager(Game_getInstance()), __SAFE_CAST(SpatialObject, this), kCuboid);

		ASSERT(this->shape, "InanimatedInGameEntity::constructor: shape not created");
	}

	this->inanimatedInGameEntityDefinition = inanimatedInGameEntityDefinition;
}

// class's destructor
void InanimatedInGameEntity_destructor(InanimatedInGameEntity this)
{
	ASSERT(this, "InanimatedInGameEntity::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// set definition
void InanimatedInGameEntity_setDefinition(InanimatedInGameEntity this, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition)
{
	ASSERT(this, "InanimatedInGameEntity::setDefinition: null this");
	ASSERT(inanimatedInGameEntityDefinition, "InanimatedInGameEntity::setDefinition: null definition");

	// save definition
	this->inanimatedInGameEntityDefinition = inanimatedInGameEntityDefinition;

	InGameEntity_setDefinition(__SAFE_CAST(InGameEntity, this), &inanimatedInGameEntityDefinition->inGameEntityDefinition);
}

// get elasticity
fix19_13 InanimatedInGameEntity_getElasticity(InanimatedInGameEntity this)
{
	ASSERT(this, "InanimatedInGameEntity::getElasticity: null this");

	return this->inanimatedInGameEntityDefinition->elasticity;
}

// get friction
fix19_13 InanimatedInGameEntity_getFriction(InanimatedInGameEntity this)
{
	ASSERT(this, "InanimatedInGameEntity::getFriction: null this");

	return this->inanimatedInGameEntityDefinition->friction;
}
