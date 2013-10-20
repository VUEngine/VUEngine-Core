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
	
	ASSERT(inGameEntityDefinition, InGameEntity: NULL definition);
	
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
	
	// better to do it here than forget in other classes
	// unregister the shape for collision detection
	CollisionManager_unregisterShape(CollisionManager_getInstance(), this->shape);
	
	this->shape = NULL;

	// destroy the super objectdirection
	__DESTROY_BASE(Entity);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve InGameEntity's friction
fix7_9 InGameEntity_getFrictionFactor(InGameEntity this){

	return 0;
//	return this->inGameEntityDefinition->frictionFactor;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set class's position
void InGameEntity_setLocalPosition(InGameEntity this, VBVec3D position){
	
	// set the position
	Container_setLocalPosition((Container)this, position);
/*
	// calculate the scale	
	Sprite_calculateScale(this->sprite, this->position.z);

	// set sprite's position
	Sprite_setPosition(this->sprite, &this->position);

	// calculate sprite's parallax
	Sprite_calculateParallax(this->sprite, this->position.z);

	//TODO: remove
	// set sprite's position
	Sprite_setPosition(this->sprite, &this->position);

	// scale the sprite
	Sprite_scale(this->sprite);

	// update gap
	InGameEntity_setGap(this);
	
	// update shape for collision detection
	if(this->shape){
		
		CollisionManager_updateShape(CollisionManager_getInstance(), this->shape, &this->position);
	}*/
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve gap
Gap InGameEntity_getGap(InGameEntity this){
	
	InGameEntity_setGap(this);
	return this->gap;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InGameEntity_setGap(InGameEntity this){
	
	// retrieve the sprite's scale
	Scale scale = Sprite_getScale((Sprite)VirtualNode_getData(VirtualList_begin(this->sprites)));
	
	// retrieve rendering mode
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
	
	return this->inGameType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve deep
int InGameEntity_getDeep(InGameEntity this){

	return this->inGameEntityDefinition->deep;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// does it moves?
int InGameEntity_moves(InGameEntity this){
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is it moving?
int InGameEntity_isMoving(InGameEntity this){
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set direction
void InGameEntity_setDirection(InGameEntity this, Direction direction){
	
	this->direction = direction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get direction
Direction InGameEntity_getDirection(InGameEntity this){
	
	return this->direction;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set shape state
void InGameEntity_setShapeState(InGameEntity this, int state){
	
	if(this->shape){
		
		Shape_setActive(this->shape, state);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//render class
void InGameEntity_render(InGameEntity this, Transformation environmentTransform){
	
	// call base
	Entity_render((Entity)this, environmentTransform);

#ifdef __DEBUG
	// draw shape
//	if(this->shape && __VIRTUAL_CALL(int, Entity, updateSpritePosition, (Entity)this)){
		if(this->shape ){
			
		
		//__VIRTUAL_CALL(void, Shape, draw, this->shape);
	}	
#endif
}