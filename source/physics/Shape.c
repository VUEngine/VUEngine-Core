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

#include <Shape.h>
#include <CollisionManager.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the Shape
__CLASS_DEFINITION(Shape);


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
// class's constructor
void Shape_constructor(Shape this, InGameEntity owner){

	ASSERT(this, "Shape::constructor: null this");

	// this is an abstract class so must initialize the vtable here
	// since this class does not have an allocator
	__SET_CLASS(Shape);	

	// construct base object
	__CONSTRUCT_BASE(Object);

	// set the owner	
	this->owner = owner;
	
	// do I move?
	this->moves = __VIRTUAL_CALL(int, InGameEntity, moves, owner);
	
	// not checked yet
	this->checked = false;
	
	// not setup yet
	this->ready = false;
	
	// set flag
	this->checkForCollisions = true;
	
	Shape_setActive(this, true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Shape_destructor(Shape this){

	ASSERT(this, "Shape::destructor: null this");

	// destroy the super object
	__DESTROY_BASE(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve owner
InGameEntity Shape_getOwner(Shape this){
	
	ASSERT(this, "Shape::getOwner: null this");

	return this->owner;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set active
void Shape_setActive(Shape this, int active){
	
	ASSERT(this, "Shape::setActive: null this");

	if(active) {
		
		CollisionManager_shapeBecameActive(CollisionManager_getInstance(), this);
	}
	else {
		CollisionManager_shapeBecameInactive(CollisionManager_getInstance(), this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is active?
int Shape_isActive(Shape this){
	
	ASSERT(this, "Shape::isActive: null this");

	return false;
	//TODO 
//	return CollisionManager_isShapeActive(CollisionManager_getInstance(), this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// do I move?
int Shape_moves(Shape this){
	
	ASSERT(this, "Shape::moves: null this");

	return this->moves;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// has been checked
int Shape_isChecked(Shape this){
	
	ASSERT(this, "Shape::isChecked: null this");

	return this->checked;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set check status
void Shape_checked(Shape this, int checked){
	
	ASSERT(this, "Shape::checked: null this");

	this->checked = checked;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// has been configured?
int Shape_isReady(Shape this){
	
	return this->ready;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set flag
void Shape_setCheckForCollisions(Shape this, int checkForCollisions){
	
	this->checkForCollisions = checkForCollisions;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get flag
int Shape_checkForCollisions(Shape this){
	
	return this->checkForCollisions;
}
