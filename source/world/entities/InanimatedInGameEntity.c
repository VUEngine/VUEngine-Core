/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InanimatedInGameEntity.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the InanimatedInGameEntity
__CLASS_DEFINITION(InanimatedInGameEntity);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(InanimatedInGameEntity, __PARAMETERS(InanimatedInGameEntityDefinition* backgroundDefinition, s16 ID))
__CLASS_NEW_END(InanimatedInGameEntity, __ARGUMENTS(backgroundDefinition, ID));

// class's constructor
void InanimatedInGameEntity_constructor(InanimatedInGameEntity this, InanimatedInGameEntityDefinition* backgroundDefinition, s16 ID)
{
	ASSERT(this, "InanimatedInGameEntity::constructor: null this");
	ASSERT(backgroundDefinition, "InanimatedInGameEntity::constructor: null definition");

	// construct base object
	__CONSTRUCT_BASE(InGameEntity, __ARGUMENTS(&backgroundDefinition->inGameEntityDefinition, ID));

	// check if register for collision detection
	if (backgroundDefinition->registerShape)
{
		// register a shape for collision detection
		this->shape = CollisionManager_registerShape(CollisionManager_getInstance(), (Entity)this, kCuboid);

		ASSERT(this->shape, "InanimatedInGameEntity::constructor: shape not created");
	}

	this->backgroundDefinition = backgroundDefinition;
}

// class's destructor
void InanimatedInGameEntity_destructor(InanimatedInGameEntity this)
{
	ASSERT(this, "InanimatedInGameEntity::destructor: null this");

	// destroy the super object
	__DESTROY_BASE(InGameEntity);
}

// get elasticiy
fix19_13 InanimatedInGameEntity_getElasticity(InanimatedInGameEntity this)
{
	ASSERT(this, "InanimatedInGameEntity::getElasticity: null this");

	return this->backgroundDefinition->elasticity;
}

// get friction
fix19_13 InanimatedInGameEntity_getFriction(InanimatedInGameEntity this)
{
	ASSERT(this, "InanimatedInGameEntity::getFriction: null this");

	return this->backgroundDefinition->friction;
}
