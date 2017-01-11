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

#include <CollisionManager.h>
#include <Circle.h>
#include <Cuboid.h>
#include <InverseCuboid.h>
#include <MessageDispatcher.h>
#include <HardwareManager.h>
#include <VirtualList.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CollisionManager_ATTRIBUTES												                        \
        /* super's attributes */													                    \
        Object_ATTRIBUTES															                    \
        /* a list of registered shapes */											                    \
        VirtualList	shapes;															                    \
        /* a list of shapes which must detect collisions */							                    \
        VirtualList	activeShapes;													                    \
        /* a list of moving shapes */												                    \
        VirtualList	movingShapes;													                    \
        /* a list of shapes which must be removed */								                    \
        VirtualList	removedShapes;													                    \
        /* a list of shapes which became inactive */								                    \
        VirtualList	inactiveShapes;													                    \
        /* flag to block removals when traversing the shapes list */				                    \
        bool checkingCollisions;													                    \

// define the CollisionManager
__CLASS_DEFINITION(CollisionManager, Object);

__CLASS_FRIEND_DEFINITION(Shape);
__CLASS_FRIEND_DEFINITION(Clock);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

Shape SpatialObject_getShape(SpatialObject this);
static void CollisionManager_processInactiveShapes(CollisionManager this);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(CollisionManager)
__CLASS_NEW_END(CollisionManager);

// class's constructor
void CollisionManager_constructor(CollisionManager this)
{
	ASSERT(this, "CollisionManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	// create the shape list
	this->shapes = __NEW(VirtualList);
	this->activeShapes = __NEW(VirtualList);
	this->movingShapes = __NEW(VirtualList);
	this->removedShapes = __NEW(VirtualList);
	this->inactiveShapes = __NEW(VirtualList);

	this->checkingCollisions = false;
}

// class's destructor
void CollisionManager_destructor(CollisionManager this)
{
	ASSERT(this, "CollisionManager::destructor: null this");
	ASSERT(this->shapes, "CollisionManager::destructor: null shapes");

	// delete the shapes
	VirtualNode node = this->shapes->head;

	// delete all shapes registered
	for(;node; node = node->next)
	{
		__DELETE(node->data);
	}

	// delete lists
	__DELETE(this->shapes);
	__DELETE(this->activeShapes);
	__DELETE(this->movingShapes);
	__DELETE(this->removedShapes);
	__DELETE(this->inactiveShapes);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
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

	VirtualNode node = VirtualList_find(this->shapes, __VIRTUAL_CALL(SpatialObject, getShape, owner));

	return node ? __SAFE_CAST(Shape, node->data) : NULL;
}

// process removed shapes
void CollisionManager_processRemovedShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::processRemovedShapes: null this");
	ASSERT(this->shapes, "CollisionManager::processRemovedShapes: null shapes");

	VirtualNode node = this->removedShapes->head;

    for(; node; node = node->next)
    {
        Shape shape = __SAFE_CAST(Shape, node->data);

        // remove from the list
        VirtualList_removeElement(this->shapes, shape);
        VirtualList_removeElement(this->activeShapes, shape);
        VirtualList_removeElement(this->movingShapes, shape);
        VirtualList_removeElement(this->inactiveShapes, shape);

        // delete it
        __DELETE(shape);
    }

    // clear the list
    VirtualList_clear(this->removedShapes);
}

// process inactive shapes
static void CollisionManager_processInactiveShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::processInactiveShapes: null this");
	ASSERT(this->inactiveShapes, "CollisionManager::processInactiveShapes: null inactiveShapes");

	VirtualNode node = this->inactiveShapes->head;

	for(; node; node = node->next)
	{
		Shape shape = __SAFE_CAST(Shape, node->data);

		// remove from the list
		VirtualList_removeElement(this->activeShapes, shape);
		VirtualList_removeElement(this->movingShapes, shape);
	}

	// clear the list
	VirtualList_clear(this->inactiveShapes);
}

// calculate collisions
u32 CollisionManager_update(CollisionManager this, Clock clock)
{
	ASSERT(this, "CollisionManager::update: null this");

	if(clock->paused)
	{
		return false;
	}

	// process removed shapes
	CollisionManager_processRemovedShapes(this);

	CollisionManager_processInactiveShapes(this);

	this->checkingCollisions = true;

	u32 returnValue = false;

	// check the shapes
	VirtualNode node = this->movingShapes->head;

	for(; node; node = node->next)
	{
		// load the current shape
		Shape shape = __SAFE_CAST(Shape, node->data);

		if(shape->checkForCollisions)
		{
            VirtualList collidingObjects = NULL;

            // the result thrown by the collision algorithm
            int collisionResult = kNoCollision;

            // don't check the current shape again when processing other movable shapes
            Shape_checked(shape, true);

            VirtualNode nodeForActiveShapes = this->activeShapes->head;

            // check the shapes
            for(; nodeForActiveShapes; nodeForActiveShapes = nodeForActiveShapes->next)
            {
                // load the current shape to check against
                Shape shapeToCheck = __SAFE_CAST(Shape, nodeForActiveShapes->data);

                // don't compare with current movable shape, when the shape already has been checked
                // and when it is not active
                if(shape != shapeToCheck && !shapeToCheck->checked)
                {
                    // check if shapes overlap
                    collisionResult = __VIRTUAL_CALL(Shape, overlaps, shape, shapeToCheck);

                    if(collisionResult)
                    {
                        if(!collidingObjects)
                        {
                            collidingObjects = __NEW(VirtualList);
                        }

                        // add object to list
                        VirtualList_pushFront(collidingObjects, shapeToCheck->owner);
                    }
                }
            }

            if(collidingObjects)
            {
                // inform the owner about the collision
               returnValue |= MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, shape), __SAFE_CAST(Object, shape->owner), kCollision, (void*)collidingObjects);

                __DELETE(collidingObjects);
            }

            collidingObjects = NULL;
        }
	}

	this->checkingCollisions = false;

    return returnValue;
}

// unregister all shapes
void CollisionManager_reset(CollisionManager this)
{
	ASSERT(this, "CollisionManager::reset: null this");
	ASSERT(this->shapes, "CollisionManager::reset: null shapes");

	VirtualNode node = this->shapes->head;

	for(; node; node = node->next)
	{
		// delete it
		__DELETE(node->data);
	}

	// empty the lists
	VirtualList_clear(this->shapes);
	VirtualList_clear(this->activeShapes);
	VirtualList_clear(this->inactiveShapes);
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

	if(VirtualList_find(this->activeShapes, shape))
	{
	    return;
    }

    VirtualList_pushFront(this->activeShapes, shape);
    VirtualList_removeElement(this->inactiveShapes, shape);
}

// inform of a change in the shape
void CollisionManager_shapeBecameInactive(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::shapeChangedState: null this");

	ASSERT(shape, "CollisionManager::shapeChangedState: null shape");

	if(!this->checkingCollisions)
	{
		VirtualList_removeElement(this->activeShapes, shape);
		VirtualList_removeElement(this->movingShapes, shape);
		return;
	}

    VirtualList_pushBack(this->inactiveShapes, shape);
}

// draw shapes
void CollisionManager_drawShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::drawShapes: null this");

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(Shape, draw, node->data);
	}
}

// free memory by deleting direct draw polygons
void CollisionManager_flushShapesDirectDrawData(CollisionManager this)
{
	ASSERT(this, "CollisionManager::drawShapes: null this");
//	ASSERT(this->shapes, "CollisionManager::drawShapes: null shapes");

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(Shape, deleteDirectDrawData, node->data);
	}
}

// check if gravity must apply to this actor
SpatialObject CollisionManager_searchNextObjectOfCollision(CollisionManager this, const Shape shape, VBVec3D direction)
{
	ASSERT(this, "CollisionManager::searchNextShapeOfCollision: null this");

	VBVec3D displacement =
    {
    	direction.x ? 0 < direction.x ? __1I_FIX19_13 : ITOFIX19_13(-1) : 0,
		direction.y ? 0 < direction.y ? __1I_FIX19_13 : ITOFIX19_13(-1) : 0,
		direction.z ? 0 < direction.z ? __1I_FIX19_13 : ITOFIX19_13(-1) : 0
	};

	if(0 == __ABS(direction.x) + __ABS(direction.y) + __ABS(direction.z))
	{
		return NULL;
	}

	SpatialObject collidingObject = NULL;

	do
	{
		VirtualNode nodeForActiveShapes = this->activeShapes->head;

		// check the shapes
		for(; nodeForActiveShapes; nodeForActiveShapes = nodeForActiveShapes->next)
		{
			// load the current shape to check against
			Shape shapeToCheck = __SAFE_CAST(Shape, nodeForActiveShapes->data);

			if(shape == shapeToCheck)
			{
				continue;
			}

			NM_ASSERT(VirtualList_getSize(this->activeShapes), "CollisionManager::searchNextShapeOfCollision: 0 active shapes");

			// check if shapes overlap
			if(__VIRTUAL_CALL(Shape, testIfCollision, shape, __SAFE_CAST(SpatialObject, shapeToCheck->owner), displacement))
			{
				collidingObject = shapeToCheck->owner;
				break;
			}
		}

		displacement.x += 0 < direction.x? __1I_FIX19_13: ITOFIX19_13(-1);
		displacement.y += 0 < direction.y? __1I_FIX19_13: ITOFIX19_13(-1);
		displacement.z += 0 < direction.z? __1I_FIX19_13: ITOFIX19_13(-1);
	}
	while(!collidingObject && ITOFIX19_13(__SCREEN_WIDTH) > __ABS(displacement.x) && ITOFIX19_13(__SCREEN_HEIGHT) > __ABS(displacement.y) && ITOFIX19_13(__SCREEN_WIDTH) > __ABS(displacement.z));

	NM_ASSERT(collidingObject, "CollisionManager::searchNextShapeOfCollision: 0 active shapes");
	return collidingObject;
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
