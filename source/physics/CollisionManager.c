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
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CollisionManager.h>
#include <MessageDispatcher.h>
#include <HardwareManager.h>
#include <VirtualList.h>
#include <Screen.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CollisionManager_ATTRIBUTES																		\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* a list of registered shapes */																\
		VirtualList	shapes;																				\
		/* a list of shapes which must detect collisions */												\
		VirtualList	activeShapes;																		\
		/* a list of moving shapes */																	\
		VirtualList	movingShapes;																		\
		/* a list of shapes which must be removed */													\
		VirtualList	removedShapes;																		\
		/* a list of shapes which became inactive */													\
		VirtualList	inactiveShapes;																		\
		/* flag to block removals when traversing the shapes list */									\
		bool checkingCollisions;																		\
		/* counters for statistics */																	\
		u32 lastCycleCollisionChecks;																	\
		u32 lastCycleCollisions;																		\
		u32 collisionChecks;																			\
		u32 collisions;																					\
		u32 checkCycles;																				\

/**
 * @class	CollisionManager
 * @extends Object
 * @ingroup physics
 */
__CLASS_DEFINITION(CollisionManager, Object);
__CLASS_FRIEND_DEFINITION(Shape);
__CLASS_FRIEND_DEFINITION(Clock);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void CollisionManager_processInactiveShapes(CollisionManager this);

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
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
	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles = 0;
	this->collisionChecks = 0;
	this->collisions = 0;
}

// class's destructor
void CollisionManager_destructor(CollisionManager this)
{
	ASSERT(this, "CollisionManager::destructor: null this");
	ASSERT(this->shapes, "CollisionManager::destructor: null shapes");

	CollisionManager_reset(this);

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
Shape CollisionManager_createShape(CollisionManager this, SpatialObject owner, const ShapeDefinition* shapeDefinition)
{
	ASSERT(this, "CollisionManager::createShape: null this");

	// create the shape
	Shape shape = ((Shape (*)(SpatialObject)) shapeDefinition->allocator)(owner);
	Shape_setup(shape, shapeDefinition->layers, shapeDefinition->layersToIgnore);
	Shape_setCheckForCollisions(shape, shapeDefinition->checkForCollisions);
	Shape_setActive(shape, true);

	// register it
	VirtualList_pushFront(this->shapes, shape);

	// return created shape
	return shape;
}

// remove a shape
void CollisionManager_destroyShape(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::destroyShape: null this");

	if(shape && !VirtualList_find(this->removedShapes, shape))
	{
		// deactivate the shape,
		// will be removed in the next update
		CollisionManager_shapeBecameInactive(this, shape);

		// place in the removed shapes list
		VirtualList_pushFront(this->removedShapes, shape);
	}
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

	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles++;

	// cull off outside of screen bounds shapes
	VirtualNode node = this->activeShapes->head;

	for(; node; node = node->next)
	{
		// load the current shape
		Shape shape = __SAFE_CAST(Shape, node->data);
		shape->isVisible = true;

		extern const Vector3D* _screenPosition;
		extern const CameraFrustum* _cameraFrustum;

		RightBox surroundingRightBox = __VIRTUAL_CALL(Shape, getSurroundingRightBox, shape);

		// not ready for collision checks if out of the screen
		if(
			surroundingRightBox.x0 - _screenPosition->x > __I_TO_FIX19_13(_cameraFrustum->x1) ||
			surroundingRightBox.x1 - _screenPosition->x < __I_TO_FIX19_13(_cameraFrustum->x0) ||
			surroundingRightBox.y0 - _screenPosition->y > __I_TO_FIX19_13(_cameraFrustum->y1) ||
			surroundingRightBox.y1 - _screenPosition->y < __I_TO_FIX19_13(_cameraFrustum->y0)
		)
		{
			shape->isVisible = false;
		}
	}

	// check the shapes
	for(node = this->movingShapes->head; node; node = node->next)
	{
		// load the current shape
		Shape shape = __SAFE_CAST(Shape, node->data);

		if(shape->ready && shape->checkForCollisions && shape->isVisible)
		{
			VirtualNode nodeForActiveShapes = this->activeShapes->head;

			// check the shapes
			for(; nodeForActiveShapes; nodeForActiveShapes = nodeForActiveShapes->next)
			{
				// load the current shape to check against
				Shape shapeToCheck = __SAFE_CAST(Shape, nodeForActiveShapes->data);

				// compare only different ready, different shapes against it other if
				// the layer of the shapeToCheck are not excluded by the current shape
				if(shape != shapeToCheck && shapeToCheck->ready && shapeToCheck->isVisible && !(shape->layersToIgnore & shapeToCheck->layers))
				{
					this->lastCycleCollisionChecks++;

					// check if shapes overlap
					if(Shape_collides(shape, shapeToCheck))
					{
						this->lastCycleCollisions++;
						returnValue = true;
					}
				}
			}
		}
	}

	this->collisionChecks += this->lastCycleCollisionChecks;
	this->collisions += this->lastCycleCollisions;

#ifdef __SHOW_PHYSICS_PROFILING
	CollisionManager_print(this, 25, 1);
#endif

	return returnValue;
}

// unregister all shapes
void CollisionManager_reset(CollisionManager this)
{
	ASSERT(this, "CollisionManager::reset: null this");
	ASSERT(this->shapes, "CollisionManager::reset: null shapes");

	CollisionManager_processRemovedShapes(this);

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

	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles = 0;
	this->collisionChecks = 0;
	this->collisions = 0;
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
void CollisionManager_showShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::drawShapes: null this");

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(Shape, show, node->data);
	}
}

// free memory by deleting direct draw Polyhedrons
void CollisionManager_hideShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::drawShapes: null this");
//	ASSERT(this->shapes, "CollisionManager::drawShapes: null shapes");

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(Shape, hide, node->data);
	}
}

// print status
void CollisionManager_print(CollisionManager this, int x, int y)
{
	ASSERT(this, "CollisionManager::print: null this");

	Printing_text(Printing_getInstance(), "COLLISION MANAGER", x, y++, NULL);
	Printing_text(Printing_getInstance(), "SHAPES", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "  Registered:     ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->shapes), x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "  Active:          ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->activeShapes), x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "  Moving:          ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->movingShapes), x + 14, y++, NULL);

	Printing_text(Printing_getInstance(), "STATISTICS (per cycle)", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "Average", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "  Checks:          ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->collisionChecks / this->checkCycles, x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "  Collisions:      ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->collisions / this->checkCycles, x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "Last cycle", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "  Checks:          ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->lastCycleCollisionChecks, x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "  Collisions:      ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->lastCycleCollisions, x + 14, y, NULL);
}
