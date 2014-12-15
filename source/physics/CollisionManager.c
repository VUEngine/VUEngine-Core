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

#include <CollisionManager.h>
#include <Circle.h>
#include <Cuboid.h>
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
__CLASS_DEFINITION(CollisionManager);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void CollisionManager_constructor(CollisionManager this);

// retrieve shape
Shape Entity_getShape(Entity this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(CollisionManager);

// class's constructor
static void CollisionManager_constructor(CollisionManager this)
{
	ASSERT(this, "CollisionManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

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
	for (;node; node = VirtualNode_getNext(node))
{
		__DELETE((Shape)VirtualNode_getData(node));
	}

	// delete lists
	__DELETE(this->shapes);
	__DELETE(this->activeShapes);
	__DELETE(this->movingShapes);
	__DELETE(this->removedShapes);

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}


// register a shape
Shape CollisionManager_registerShape(CollisionManager this, Entity owner, int shapeType)
{
	ASSERT(this, "CollisionManager::registerShape: null this");

	// if the entity is already registered
	Shape shape = CollisionManager_getShape(this, owner);

	if (shape)
	{
		return shape;
	}

	switch (shapeType)
	{
		case kCircle:

			//VirtualList_pushBack(this->shapes, (void*)__NEW(Circle, __ARGUMENTS(owner)));
			break;

		case kCuboid:

			VirtualList_pushFront(this->shapes, (void*)__NEW(Cuboid, __ARGUMENTS(owner)));
			break;
	}

	// return created shape
	return (Shape)VirtualList_front(this->shapes);
}

// remove a shape
void CollisionManager_unregisterShape(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::unregisterShape: null this");

	if (shape)
	{
		// deactivate the shape,
		// will be removed in the next update
		Shape_setActive(shape, false);

		// place in  the removed shapes list
		VirtualList_pushFront(this->removedShapes, (BYTE*)shape);
	}
}

// find a shape given an owner
Shape CollisionManager_getShape(CollisionManager this, Entity owner)
{
	ASSERT(this, "CollisionManager::getShape: null this");
	ASSERT(this->shapes, "CollisionManager::getShape: null shapes");

	return (Shape)VirtualList_find(this->shapes, (const void* const)Entity_getShape(owner));
}

// process removed shapes
void CollisionManager_processRemovedShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::processRemovedShapes: null this");
	ASSERT(this->shapes, "CollisionManager::processRemovedShapes: null shapes");

	VirtualNode node = VirtualList_begin(this->removedShapes);

	if (node)
	{
		for (; node; node = VirtualNode_getNext(node))
		{
			Shape shape = (Shape)VirtualNode_getData(node);

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
int CollisionManager_update(CollisionManager this)
{
	ASSERT(this, "CollisionManager::update: null this");

	int thereWhereCollisions = false;

	VirtualNode node = VirtualList_begin(this->movingShapes);

	// check the shapes
	for (; node; node = VirtualNode_getNext(node))
	{
		// current to check shape's rectangle
		__VIRTUAL_CALL(void, Shape, positione, (Shape)VirtualNode_getData(node));
	}

	// check the shapes
	node = VirtualList_begin(this->movingShapes);
	for (; node; node = VirtualNode_getNext(node))
	{
		// load the current shape
		Shape shape = (Shape)VirtualNode_getData(node);

		if (!Shape_checkForCollisions(shape))
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
		for (; nodeForActiveShapes; nodeForActiveShapes = VirtualNode_getNext(nodeForActiveShapes))
		{
			// load the current shape to check against
			Shape shapeToCheck = (Shape)VirtualNode_getData(nodeForActiveShapes);

			// don't compare with current movable shape,
			// when the shape already has been checked
			// and when it is not active
			if (shape != shapeToCheck && !Shape_isChecked(shapeToCheck))
			{
				// check if shapes overlap
				collisionResult = __VIRTUAL_CALL(int, Shape, overlaps, shape, __ARGUMENTS(shapeToCheck));

				if (collisionResult)
				{
					if (!collidingObjects)
					{
						collidingObjects = __NEW(VirtualList);
					}

					// add object to list
					VirtualList_pushFront(collidingObjects, (void*)Shape_getOwner(shapeToCheck));
				}
			}
		}

		if (collidingObjects)
		{
			thereWhereCollisions = true;

			// inform the owner about the collision
			MessageDispatcher_dispatchMessage(0, (Object)shape, (Object)Shape_getOwner(shape), kCollision, (void*)collidingObjects);

			__DELETE(collidingObjects);
		}

		collidingObjects = NULL;
	}

	// process removed shapes
	CollisionManager_processRemovedShapes(this);

	return thereWhereCollisions;
}

// unregister all shapes
void CollisionManager_reset(CollisionManager this)
{
	ASSERT(this, "CollisionManager::reset: null this");
	ASSERT(this->shapes, "CollisionManager::reset: null shapes");

	VirtualNode node = VirtualList_begin(this->shapes);

	for (; node; node = VirtualNode_getNext(node))
	{
		// delete it
		__DELETE((Shape)VirtualNode_getData(node));
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
	ASSERT(this, "CollisionManager::shapeChangedState: null this");

	ASSERT(shape, "CollisionManager::shapeChangedState: null shape");

	CollisionManager_shapeBecameActive(this, shape);

	if (!VirtualList_find(this->movingShapes, shape))
	{
//		Printing_text("CollisionManager::shapeStartedMoving", 10, 10);
		VirtualList_pushBack(this->movingShapes, shape);
	}
}

// inform of a change in the shape
void CollisionManager_shapeStopedMoving(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::shapeChangedState: null this");

	ASSERT(shape, "CollisionManager::shapeChangedState: null shape");

//	Printing_text("CollisionManager::shapeStopedMoving", 10, 10);
	VirtualList_removeElement(this->movingShapes, shape);
}

// inform of a change in the shape
void CollisionManager_shapeBecameActive(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::shapeBecameActive: null this");

	ASSERT(shape, "CollisionManager::shapeBecameActive: null shape");

	if (!VirtualList_find(this->activeShapes, shape))
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
	for (; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Shape, draw, (Shape)VirtualNode_getData(node));
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
	for (; node; node = VirtualNode_getNext(node))
	{
		__VIRTUAL_CALL(void, Shape, deleteDirectDrawData, (Shape)VirtualNode_getData(node));
	}
}

// print status
void CollisionManager_print(CollisionManager this, int x, int y)
{
	ASSERT(this, "CollisionManager::print: null this");

	Printing_text("COLLISION SHAPES", x, y++);
	Printing_text("Registered shapes: ", x, ++y);
	Printing_int(VirtualList_getSize(this->shapes), x + 19, y);
	Printing_text("Active shapes: ", x, ++y);
	Printing_int(VirtualList_getSize(this->activeShapes), x + 19, y);
	Printing_text("Moving shapes: ", x, ++y);
	Printing_int(VirtualList_getSize(this->movingShapes), x + 19, y);
}