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
__CLASS_NEW_DEFINITION(InanimatedInGameEntity, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition, s16 id, const char* const name)
__CLASS_NEW_END(InanimatedInGameEntity, inanimatedInGameEntityDefinition, id, name);

// class's constructor
void InanimatedInGameEntity_constructor(InanimatedInGameEntity this, InanimatedInGameEntityDefinition* inanimatedInGameEntityDefinition, s16 id, const char* const name)
{
	ASSERT(this, "InanimatedInGameEntity::constructor: null this");
	ASSERT(inanimatedInGameEntityDefinition, "InanimatedInGameEntity::constructor: null definition");

	// construct base object
	__CONSTRUCT_BASE(&inanimatedInGameEntityDefinition->inGameEntityDefinition, id, name);

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

// get elasticiy
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