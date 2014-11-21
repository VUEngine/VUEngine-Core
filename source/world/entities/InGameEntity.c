/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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
 
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <InGameEntity.h>
#include <CollisionManager.h>

#ifdef __DEBUG
#include <DirectDraw.h>	
#endif



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the InGameEntity
__CLASS_DEFINITION(InGameEntity);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */




/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// always call these to macros next to each other
__CLASS_NEW_DEFINITION(InGameEntity, __PARAMETERS(InGameEntityDefinition* inGameEntityDefinition, int ID))
__CLASS_NEW_END(InGameEntity, __ARGUMENTS(inGameEntityDefinition, ID));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
void InGameEntity_constructor(InGameEntity this, InGameEntityDefinition* inGameEntityDefinition, int ID){
	
	ASSERT(this, "InGameEntity::constructor: null this");
	ASSERT(inGameEntityDefinition, "InGameEntity::constructor: null definition");
	
	__CONSTRUCT_BASE(Entity, __ARGUMENTS(&inGameEntityDefinition->entityDefinition, ID));
	
	this->inGameEntityDefinition = inGameEntityDefinition;
	
	this->gap = this->inGameEntityDefinition->gap;

	this->direction.x = __RIGHT;
	this->direction.y = __DOWN;
	this->direction.z = __FAR;
	
	this->inGameType = inGameEntityDefinition->inGameType;
	
	this->shape = NULL; 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void InGameEntity_destructor(InGameEntity this){
	
	ASSERT(this, "InGameEntity::destructor: null this");
	
	// better to do it here than forget in other classes
	// unregister the shape for collision detection
	CollisionManager_unregisterShape(CollisionManager_getInstance(), this->shape);
	
	this->shape = NULL;

	// destroy the super objectdirection
	__DESTROY_BASE(Entity);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve gap
Gap InGameEntity_getGap(InGameEntity this){
	
	ASSERT(this, "InGameEntity::getGap: null this");

	InGameEntity_setGap(this);
	return this->gap;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InGameEntity_setGap(InGameEntity this){
	
	ASSERT(this, "InGameEntity::setGap: null this");
	
	// retrieve the sprite's scale
	Scale scale = Sprite_getScale((Sprite)VirtualNode_getData(VirtualList_begin(this->sprites)));
	
	// retrieve transforming mode
	int bgmapMode = Sprite_getMode((Sprite)VirtualNode_getData(VirtualList_begin(this->sprites)));
	
	// load original gap
	this->gap = this->inGameEntityDefinition->gap;
	
	// if facing to the left... swap left / right gap
	if(__LEFT == this->direction.x && WRLD_AFFINE == bgmapMode){
		
		this->gap.left 	= this->inGameEntityDefinition->gap.right;
		this->gap.right = this->inGameEntityDefinition->gap.left;
	}
	
	// scale gap if needed
	if(false && WRLD_AFFINE != bgmapMode){
	
		// must scale the gap
		this->gap.left 	= 	FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.left), abs(scale.x)));
		this->gap.right =  	FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.right), abs(scale.x)));
		this->gap.up 	= 	FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.up), abs(scale.y)));
		this->gap.down 	= 	FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(this->gap.down), abs(scale.y)));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve in game type
int InGameEntity_getInGameType(InGameEntity this){
	
	ASSERT(this, "InGameEntity::getInGameType: null this");

	return this->inGameType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve deep
int InGameEntity_getDeep(InGameEntity this){

	ASSERT(this, "InGameEntity::getDeep: null this");

	return this->inGameEntityDefinition->deep;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// does it moves?
int InGameEntity_moves(InGameEntity this){
	
	ASSERT(this, "InGameEntity::moves: null this");

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is it moving?
int InGameEntity_isMoving(InGameEntity this){
	
	ASSERT(this, "InGameEntity::isMoving: null this");

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set direction
void InGameEntity_setDirection(InGameEntity this, Direction direction){
	
	ASSERT(this, "InGameEntity::setDirection: null this");

	this->direction = direction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get direction
Direction InGameEntity_getDirection(InGameEntity this){
	
	ASSERT(this, "InGameEntity::getDirection: null this");

	return this->direction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set shape state
void InGameEntity_setShapeState(InGameEntity this, int state){
	
	ASSERT(this, "InGameEntity::setShapeState: null this");

	if(this->shape){
		
		Shape_setActive(this->shape, state);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// draw class
void InGameEntity_initialTransform(InGameEntity this, Transformation* environmentTransform){
	
	ASSERT(this, "InGameEntity::transform: null this");

	// call base
	Entity_transform((Entity)this, environmentTransform);

	if(this->shape){
				
		// setup shape
		__VIRTUAL_CALL(void, Shape, positione, this->shape);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve shape
Shape InGameEntity_getShape(InGameEntity this){
	
	ASSERT(this, "InGameEntity::getShape: null this");

	return this->shape;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get elasticiy
fix19_13 InGameEntity_getElasticity(InGameEntity this){
	
	ASSERT(this, "InGameEntity::getElasticity: null this");

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get friction
fix19_13 InGameEntity_getFriction(InGameEntity this){
	
	ASSERT(this, "InGameEntity::getFriction: null this");

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve previous position
VBVec3D InGameEntity_getPreviousPosition(InGameEntity this){
	
	ASSERT(this, "InGameEntity::getPreviousPosition: null this");

	return this->transform.globalPosition;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//set class's local position
void InGameEntity_setLocalPosition(InGameEntity this, VBVec3D position){
	
	ASSERT(this, "Actor::setLocalPosition: null this");

	Container_setLocalPosition((Container)this, position);

	if(this->shape && !Shape_isReady(this->shape)){
				
		// setup shape
		__VIRTUAL_CALL(void, Shape, setup, this->shape);
	}
}

