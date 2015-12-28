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

#include <CollisionManager.h>
#include <Circle.h>
#include <Cuboid.h>
#include <InverseCuboid.h>
#include <MessageDispatcher.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CollisionManager_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* a list of shapes registeres */											\
	VirtualList	shapes;															\
																				\
	/* a list of shapes which must detect collisions */							\
	VirtualList	activeShapes;													\
																				\
	/* a list of moving shapes */												\
	VirtualList	movingShapes;													\
																				\
	/* a list of shapes which must be removed */								\
	VirtualList	removedShapes;													\

// define the CollisionManager
__CLASS_DEFINITION(CollisionManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void CollisionManager_constructor(CollisionManager this);

// retrieve shape
Shape SpatialObject_getShape(SpatialObject this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(CollisionManager);

// class's constructor
static void CollisionManager_constructor(CollisionManager this)
{
	ASSERT(this, "CollisionManager::constructor: null this");

	__CONSTRUCT_BASE();

	// create the shape list
	this->shapes = __NEW(VirtualList);
	this->activeShapes = __NEW(VirtualList);
	this->movingShapes = __NEW(VirtualList);
	this->removedShapes = __NEW(VirtualList);
}

// class's destructor
void CollisionManager_destructor(CollisionManager this)
{
	ASSERT(this, "CollisionManager::destructor: null this");
	ASSERT(this->shapes, "CollisionManager::destructor: null shapes");

	// delete the shapes
	VirtualNode node = VirtualList_begin(this->shapes);

	// delete all shapes registered
	for(;node; node = VirtualNode_getNext(node))
	{
		__DELETE(VirtualNode_getData(node));
	}

	// delete lists
	__DELETE(this->shapes);
	__DELETE(this->activeShapes);
	__DELETE(this->movingShapes);
	__DELETE(this->removedShapes);

	// allow a new construct
	__SINGLETON_DESTROY;
}


// register a shape
Shape CollisionManager_registerShape(CollisionManager this, SpatialObject owner, int shapeType)
{
	ASSERT(this, "CollisionManager::registerShape: null this");

	// if the entity is already registered
	Shape shape = CollisionManager_getShape(this, owner);

	if(shape)
	{
		return shape;
	}

	switch(shapeType)
	{
		case kCircle:

			//VirtualList_pushBack(this->shapes, (void*)__NEW(Circle, owner));
			break;

		case kCuboid:

			VirtualList_pushFront(this->shapes, (void*)__NEW(Cuboid, owner));
			break;

		case kInverseCuboid:

			VirtualList_pushFront(this->shapes, (void*)__NEW(InverseCuboid, owner));
			break;
	}

	// return created shape
	return __SAFE_CAST(Shape, VirtualList_front(this->shapes));
}

// remove a shape
void CollisionManager_unregisterShape(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::unregisterShape: null this");

	if(shape)
	{
		// deactivate the shape,
		// will be removed in the next update
		Shape_setActive(shape, false);

		// place in  the removed shapes list
		VirtualList_pushFront(this->removedShapes, (BYTE*)shape);
	}
}

// find a shape given an owner
Shape CollisionManager_getShape(CollisionManager this, SpatialObject owner)
{
	ASSERT(this, "CollisionManager::getShape: null this");
	ASSERT(this->shapes, "CollisionManager::getShape: null shapes");

	VirtualNode node = VirtualList_find(this->shapes, __VIRTUAL_CALL_UNSAFE(const void* const, SpatialObject, getShape, owner));

	return node? __SAFE_CAST(Shape, VirtualNode_getData(node)): NULL;
}

// process removed shapes
void CollisionManager_processRemovedShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::processRemovedShapes: null this");
	ASSERT(this->shapes, "CollisionManager::processRemovedShapes: null shapes");

	VirtualNode node = VirtualList_begin(this->removedShapes);

	if(node)
	{
		for(; node; node = VirtualNode_getNext(node))
		{
			Shape shape = __SAFE_CAST(Shape, VirtualNode_getData(node));

			// remove from the list
			VirtualList_removeElement(this->shapes, (BYTE*) shape);
			VirtualList_removeElement(this->shapes, (BYTE*) this->activeShapes);
			VirtualList_removeElement(this->shapes, (BYTE*) this->movingShapes);

			// delete it
			__DELETE(shape);
		}

		// clear the list
		VirtualList_clear(this->removedShapes);
	}
}

// calculate collisions
void CollisionManager_update(CollisionManager this, fix19_13 elapsedTime)
{
	ASSERT(this, "CollisionManager::update: null this");

	if(!elapsedTime)
	{
		return;
	}
	
	VirtualNode node = VirtualList_begin(this->movingShapes);

	// check the shapes
	for(; node; node = VirtualNode_getNext(node))
	{
		// current to check shape's rectangle
		__VIRTUAL_CALL(void, Shape, position, __SAFE_CAST(Shape, VirtualNode_getData(node)));
	}

	// check the shapes
	node = VirtualList_begin(this->movingShapes);
	for(; node; node = VirtualNode_getNext(node))
	{
		// load the current shape
		Shape shape = __SAFE_CAST(Shape, VirtualNode_getData(node));

		if(!Shape_checkForCollisions(shape))
		{
			continue;
		}

		VirtualList collidingObjects = NULL;

		// the result thrown by the collision algorithm
		int collisionResult = kNoCollision;

		// dont' check again the current shape when processing other movable shapes
		Shape_checked(shape, true);

		VirtualNode nodeForActiveShapes = VirtualList_begin(this->activeShapes);

		// check the shapes
		for(; nodeForActiveShapes; nodeForActiveShapes = VirtualNode_getNext(nodeForActiveShapes))
		{	
			// load the current shape to check against
			Shape shapeToCheck = __SAFE_CAST(Shape, VirtualNode_getData(nodeForActiveShapes));

			// don't compare with current movable shape, when the shape already has been checked
			// and when it is not active
			if(shape != shapeToCheck && !Shape_isChecked(shapeToCheck))
			{
				// check if shapes overlap
				collisionResult = __VIRTUAL_CALL(bool, Shape, overlaps, shape, shapeToCheck);

				if(collisionResult)
				{
					if(!collidingObjects)
					{
						collidingObjects = __NEW(VirtualList);
					}

					// add object to list
					VirtualList_pushFront(collidingObjects, Shape_getOwner(shapeToCheck));
				}
			}
		}

		if(collidingObjects)
		{
			// inform the owner about the collision
			MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, shape), __SAFE_CAST(Object, Shape_getOwner(shape)), kCollision, (void*)collidingObjects);

			__DELETE(collidingObjects);
		}

		collidingObjects = NULL;
	}

	// process removed shapes
	CollisionManager_processRemovedShapes(this);
}

// unregister all shapes
void CollisionManager_reset(CollisionManager this)
{
	ASSERT(this, "CollisionManager::reset: null this");
	ASSERT(this->shapes, "CollisionManager::reset: null shapes");

	VirtualNode node = VirtualList_begin(this->shapes);

	for(; node; node = VirtualNode_getNext(node))
	{
		// delete it
		__DELETE(VirtualNode_getData(node));
	}

	// empty the lists
	VirtualList_clear(this->shapes);
	VirtualList_clear(this->activeShapes);
	VirtualList_clear(this->movingShapes);
	VirtualList_clear(this->removedShapes);
}


// inform of a change in the shape
void CollisionManager_shapeStartedMoving(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::shapeStartedMoving: null this");
	ASSERT(shape, "CollisionManager::shapeStartedMoving: null shape");

	CollisionManager_shapeBecameActive(this, shape);

	if(!VirtualList_find(this->movingShapes, shape))
	{
		VirtualList_pushBack(this->movingShapes, shape);
	}
}

// inform of a change in the shape
void CollisionManager_shapeStoppedMoving(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::shapeStoppedMoving: null this");
	ASSERT(shape, "CollisionManager::shapeStoppedMoving: null shape");

	VirtualList_removeElement(this->movingShapes, shape);
	
	// make sure other moving shapes test for collisions against it
//	Shape_checked(shape, false);
}

// inform of a change in the shape
void CollisionManager_shapeBecameActive(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::shapeBecameActive: null this");

	ASSERT(shape, "CollisionManager::shapeBecameActive: null shape");

	if(!VirtualList_find(this->activeShapes, shape))
	{
		VirtualList_pushBack(this->activeShapes, shape);
	}
}

// inform of a change in the shape
void CollisionManager_shapeBecameInactive(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::shapeChangedState: null this");

	ASSERT(shape, "CollisionManager::shapeChangedState: null shape");

	VirtualList_removeElement(this->activeShapes, shape);
	VirtualList_removeElement(this->movingShapes, shape);
}
// draw shapes
void CollisionManager_drawShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::drawShapes: null this");

	// comparing against the other shapes
	VirtualNode node = VirtualList_begin(this->shapes);

	// check the shapes
	for(; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Shape, draw, VirtualNode_getData(node));
	}
}

// free memory by deleting direct draw polygons
void CollisionManager_flushShapesDirectDrawData(CollisionManager this)
{
	ASSERT(this, "CollisionManager::drawShapes: null this");
//	ASSERT(this->shapes, "CollisionManager::drawShapes: null shapes");

	// comparing against the other shapes
	VirtualNode node = VirtualList_begin(this->shapes);

	// check the shapes
	for(; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Shape, deleteDirectDrawData, VirtualNode_getData(node));
	}
}

// print status
void CollisionManager_print(CollisionManager this, int x, int y)
{
	ASSERT(this, "CollisionManager::print: null this");

	Printing_text(Printing_getInstance(), "COLLISION SHAPES", x, y++, NULL);
	Printing_text(Printing_getInstance(), "Registered shapes: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->shapes), x + 19, y, NULL);
	Printing_text(Printing_getInstance(), "Active shapes: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->activeShapes), x + 19, y, NULL);
	Printing_text(Printing_getInstance(), "Moving shapes: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->movingShapes), x + 19, y, NULL);
}