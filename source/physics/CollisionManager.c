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

#include <CollisionManager.h>
#include <Circle.h>
#include <Cuboid.h>
#include <MessageDispatcher.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define CollisionManager_ATTRIBUTES										\
																		\
	/* super's attributes */											\
	Object_ATTRIBUTES;													\
																		\
	/* a list of shapes which must detect collisions */					\
	VirtualList	shapes;													\
																		\
	/* a list of shapes which must be removed */						\
	VirtualList	removedShapes;											\
																		\
	/* flag to know if shapes must be prepared */						\
	int selectShapesToCheck:1;
	

// define the CollisionManager
__CLASS_DEFINITION(CollisionManager);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void CollisionManager_constructor(CollisionManager this);

// precalculate movable shape's position before doing collision detection on them
static void CollisionManager_selectShapesToCheck(CollisionManager this);

// only process shapes which move and are active
Shape shapes[__MAX_SHAPES_PER_LEVEL] = {NULL};
Shape shapesToCheck[__MAX_SHAPES_PER_LEVEL] = {NULL};

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */




//////////////////////////////////////////////////////////////////////////////////////////////////////////////

__SINGLETON(CollisionManager);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void CollisionManager_constructor(CollisionManager this){
	
	__CONSTRUCT_BASE(Object);

	// create the shape list
	this->shapes = __NEW(VirtualList);
	
	this->removedShapes = __NEW(VirtualList);

	this->selectShapesToCheck = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void CollisionManager_destructor(CollisionManager this){

	// delete the shapes
	VirtualNode node = VirtualList_begin(this->shapes);
	
	// delete all shapes registered
	for(;node; node = VirtualNode_getNext(node)){
		
		__DELETE((Shape)VirtualNode_getData(node));
	}
	
	// delete list
	__DELETE(this->shapes);
	
	// delete the list of removed shapes
	__DELETE(this->removedShapes);
	
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// register a shape
Shape CollisionManager_registerShape(CollisionManager this, InGameEntity owner, int shapeType){

	// if the entity is already registered
	Shape shape = CollisionManager_getShape(this, owner);
	
	if(shape){
		
		return shape;
	}	
	
	switch(shapeType){
	
		case kCircle:
			
			//VirtualList_pushBack(this->shapes, (void*)__NEW(Circle, __ARGUMENTS(owner)));			
			break;

		case kCuboid:

			VirtualList_pushFront(this->shapes, (void*)__NEW(Cuboid, __ARGUMENTS(owner)));
			break;
	}
	
	// must prepare shapes in the next update
	this->selectShapesToCheck = true;
	
	// return created shape
	return (Shape)VirtualList_front(this->shapes);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remove a shape
void CollisionManager_unregisterShape(CollisionManager this, Shape shape){

	if(shape){
		
		// deactivate teh shape,
		// will be removed in the next update
		Shape_setActive(shape, false);
		
		// place in  the removed shapes list
		VirtualList_pushFront(this->removedShapes, (BYTE*)shape);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// find a shape given an owner
Shape CollisionManager_getShape(CollisionManager this, InGameEntity owner){

	VirtualNode node = VirtualList_begin(this->shapes);
	
	for(; node; node = VirtualNode_getNext(node)){

		// current shape
		Shape shape = (Shape)VirtualNode_getData(node);
		
		// check if current shape's owner is the same as the entity calling this method
		if(owner == Shape_getOwner(shape) && Shape_isActive(shape)){

			return shape;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process removed shapes
void CollisionManager_processRemovedShapes(CollisionManager this){

	VirtualNode node = VirtualList_begin(this->removedShapes);

	if(node){
		
		for(; node; node = VirtualNode_getNext(node)){
	
			Shape shape = (Shape)VirtualNode_getData(node);
			
			// remove from the list
			VirtualList_removeElement(this->shapes, (BYTE*) shape);
				
			// delete it
			__DELETE(shape);
		}
	
		// clear the list
		VirtualList_clear(this->removedShapes);
		
		// must prepare shapes in the next update
		this->selectShapesToCheck = true;		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// precalculate movable shape's position before doing collision detection on them
static void CollisionManager_selectShapesToCheck(CollisionManager this){
	
	VirtualNode node = NULL;
	
	int i = 0;
	int j = 0;
	
	this->selectShapesToCheck = false;

	// prepare shapes which move 
	// this will place the shape in the owner's position
	for(node = VirtualList_begin(this->shapes); node; node = VirtualNode_getNext(node)){

		// load the current shape
		Shape shape = (Shape)VirtualNode_getData(node);
		
		// first check it shape needs setup
		if(!Shape_isReady(shape)){
			
			// setup shape
			__VIRTUAL_CALL(void, Shape, setup, shape);
		}

		// only check entities which are active
		if(Shape_isActive(shape)){
		
			// and moves 
			if(Shape_moves(shape)){
				
				// feed the array of movable shapes to check for collisions
				shapes[i++] = shape;
			}
			
			// feed the array of all shapes
			shapesToCheck[j++] = shape;
		}
	}
	
	shapes[i] = NULL;
	shapesToCheck[j] = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate collisions
void CollisionManager_update(CollisionManager this){
	
	int i = 0;
	int j = 0;
	
	// check if must select shapes to process
	if(this->selectShapesToCheck){
		
		CollisionManager_selectShapesToCheck(this);	
	}

	// check the shapes
	for(i = 0; shapes[i] && i < __MAX_SHAPES_PER_LEVEL; i++){

		// current to check shape's rectangle 
		__VIRTUAL_CALL(void, Shape, positione, shapes[i]);
	}

	// check the shapes
	for(i = 0; shapes[i] && i < __MAX_SHAPES_PER_LEVEL; i++){

		// if shape is active to be processed
		if(Shape_isActive(shapes[i])){

			VirtualList collidingObjects = NULL;

			// load the current shape
			Shape shape = shapes[i];
			
			// get owner
			InGameEntity owner = Shape_getOwner(shape);

			// the result thrown by the collision algorithm
			int collisionResult = kNoCollision;

			// determine over which axis is moving
			int axisMovement = __VIRTUAL_CALL(int, InGameEntity, isMoving, owner);
	
			// if owner is moving
			if(axisMovement){
			
				// dont' check again the current shape when processing other movable shapes
				Shape_checked(shape, true);
			}
			
			// comparing against the other shapes
			for(j = 0; shapesToCheck[j] && j < __MAX_SHAPES_PER_LEVEL; j++){
				
				// load the current shape to check against
				Shape shapeToCheck = shapesToCheck[j];
				
				// don't compare with current movable shape, 
				// when the shape already has been checked 
				// and when it is not active
				if(shape != shapeToCheck && !Shape_isChecked(shapeToCheck) && Shape_isActive(shapeToCheck)){
					
					// check if shapes overlap
					collisionResult = __VIRTUAL_CALL(int, Shape, overlaps, shape, __ARGUMENTS(shapeToCheck));

					if(collisionResult){
						
						if(!collidingObjects) {
							
							collidingObjects = __NEW(VirtualList);
						}

						// add object to list
						VirtualList_pushFront(collidingObjects, (void*)Shape_getOwner(shapeToCheck));
					}
				}
			}
			
			if(collidingObjects){

				// inform the owner about the collision
				MessageDispatcher_dispatchMessage(0, (Object)shape, (Object)owner, kCollision, (void*)collidingObjects);

				__DELETE(collidingObjects);
			}
			
			collidingObjects = NULL;
		}
	}
	
	// process removed shapes
	CollisionManager_processRemovedShapes(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update a shape
void CollisionManager_updateShape(CollisionManager this, Shape shape, const VBVec3D* const position){

	__VIRTUAL_CALL(void, Shape, setup, shape, __ARGUMENTS(position));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unregister all shapes
void CollisionManager_reset(CollisionManager this){

	VirtualNode node = VirtualList_begin(this->shapes);

	for(; node; node = VirtualNode_getNext(node)){

		// delete it
		__DELETE((Shape)VirtualNode_getData(node));
	}
	
	// empty the lists
	VirtualList_clear(this->shapes);	
	VirtualList_clear(this->removedShapes);	

	// must prepare shapes in the next update
	this->selectShapesToCheck = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if an entity has been registered
int CollisionManager_isEntityRegistered(CollisionManager this, InGameEntity owner){

	VirtualNode node = VirtualList_begin(this->shapes);
	
	for(; node; node = VirtualNode_getNext(node)){

		// current shape
		Shape shape = (Shape)VirtualNode_getData(node);
		
		// check if current shape's owner is the same as the entity calling this method
		if(owner == Shape_getOwner(shape)){
			
			// check if shape is active.... maybe a shape must be removed
			// and a new entity has been loaded in the same memory location
			// as the owner of the found shape
			return Shape_isActive(shape);
		}
	}
	
	return false;
}