/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Camera.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											MACROS
//---------------------------------------------------------------------------------------------------------
#define __TOTAL_USABLE_SHAPES		128


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CollisionManager_ATTRIBUTES																		\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* a list of registered shapes */																\
		VirtualList	shapes;																				\
		/* a list of moving shapes */																	\
		VirtualList	movingShapes;																		\
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
	this->movingShapes = __NEW(VirtualList);

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
	__DELETE(this->movingShapes);

	// destroy the super object
	// must always be called at the end of the destructor
	Base_destructor();
}

// register a shape
Shape CollisionManager_createShape(CollisionManager this, SpatialObject owner, const ShapeDefinition* shapeDefinition)
{
	ASSERT(this, "CollisionManager::createShape: null this");

	// create the shape
	Shape shape = ((Shape (*)(SpatialObject)) shapeDefinition->allocator)(owner);
	Shape_setup(shape, shapeDefinition->layers, shapeDefinition->layersToIgnore);
	Shape_setCheckForCollisions(shape, shapeDefinition->checkForCollisions);

	// register it
	VirtualList_pushFront(this->shapes, shape);

	// return created shape
	return shape;
}

// remove a shape
void CollisionManager_destroyShape(CollisionManager this, Shape shape)
{
	ASSERT(this, "CollisionManager::destroyShape: null this");

	if(shape && VirtualList_find(this->shapes, shape))
	{
		VirtualList_removeElement(this->shapes, shape);
		VirtualList_removeElement(this->movingShapes, shape);

		// delete it
		__DELETE(shape);
	}
}

// calculate collisions
u32 CollisionManager_update(CollisionManager this, Clock clock)
{
	ASSERT(this, "CollisionManager::update: null this");

	if(clock->paused)
	{
		return false;
	}

	u32 returnValue = false;

	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles++;

	// cull off outside of camera bounds shapes
	VirtualNode node = this->shapes->head;

	for(; node; node = node->next)
	{
		// load the current shape
		Shape shape = __SAFE_CAST(Shape, node->data);
		shape->isVisible = true;

		extern const Vector3D* _cameraPosition;
		extern const CameraFrustum* _cameraFrustum;

		RightBox surroundingRightBox =  Shape_getSurroundingRightBox(shape);

		// not ready for collision checks if out of the camera
		if(
			surroundingRightBox.x0 - _cameraPosition->x > __I_TO_FIX10_6(_cameraFrustum->x1) ||
			surroundingRightBox.x1 - _cameraPosition->x < __I_TO_FIX10_6(_cameraFrustum->x0) ||
			surroundingRightBox.y0 - _cameraPosition->y > __I_TO_FIX10_6(_cameraFrustum->y1) ||
			surroundingRightBox.y1 - _cameraPosition->y < __I_TO_FIX10_6(_cameraFrustum->y0)
		)
		{
			shape->isVisible = false;
		}

#ifdef __DRAW_SHAPES
		if(shape->isActive && shape->isVisible)
		{
			Shape_show(shape);
		}
		else
		{
			Shape_hide(shape);
		}
#endif

		shape->moved = false;
	}

	NM_ASSERT(__TOTAL_USABLE_SHAPES >= VirtualList_getSize(this->movingShapes), "CollisionManager::update: too many moving shapes");

	Shape movingShapes[__TOTAL_USABLE_SHAPES];
	Shape activeShapes[__TOTAL_USABLE_SHAPES];

	int movingShapesIndex = 0;
	int activeShapesIndex = 0;

	// check the shapes
	for(movingShapesIndex = 0, node = this->movingShapes->head; node; node = node->next)
	{
		Shape shape = __SAFE_CAST(Shape, node->data);

		if(shape->isActive)
		{
			movingShapes[movingShapesIndex++] = shape;
		}
	}

	movingShapes[movingShapesIndex] = NULL;

	// check the shapes
	for(activeShapesIndex = 0, node = this->shapes->head; node; node = node->next)
	{
		Shape shape = __SAFE_CAST(Shape, node->data);

		if(shape->isActive)
		{
			activeShapes[activeShapesIndex++] = shape;
		}
	}

	activeShapes[activeShapesIndex] = NULL;

	// check the shapes
	for(movingShapesIndex = 0; movingShapes[movingShapesIndex]; movingShapesIndex++)
	{
		if(!__IS_OBJECT_ALIVE(movingShapes[movingShapesIndex]))
		{
			continue;
		}

		// load the current shape
		Shape shape = __SAFE_CAST(Shape, movingShapes[movingShapesIndex]);

		if(shape->ready && shape->checkForCollisions && shape->isVisible)
		{
			// check the shapes
			for(activeShapesIndex = 0; activeShapes[activeShapesIndex]; activeShapesIndex++)
			{
				if(!__IS_OBJECT_ALIVE(activeShapes[activeShapesIndex]))
				{
					continue;
				}

				// load the current shape to check against
				Shape shapeToCheck = __SAFE_CAST(Shape, activeShapes[activeShapesIndex]);

				if(!shapeToCheck->isActive)
				{
					continue;
				}

				// compare only different ready, different shapes against it other if
				// the layer of the shapeToCheck are not excluded by the current shape
				if(shape != shapeToCheck && shapeToCheck->ready && shapeToCheck->isVisible && !(shape->layersToIgnore & shapeToCheck->layers))
				{
					this->lastCycleCollisionChecks++;

					CollisionData collisionData = Shape_collides(shape, shapeToCheck);

					// check if shapes overlap
					if(kNoCollision != collisionData.result)
					{
						this->lastCycleCollisions++;
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

	VirtualNode node = this->shapes->head;

	for(; node; node = node->next)
	{
		// delete it
		__DELETE(node->data);
	}

	// empty the lists
	VirtualList_clear(this->shapes);
	VirtualList_clear(this->movingShapes);

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

// draw shapes
void CollisionManager_showShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::drawShapes: null this");

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		Shape_show(__SAFE_CAST(Shape, node->data));
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
		Shape_hide(__SAFE_CAST(Shape, node->data));
	}
}

int CollisionManager_getNumberOfActiveShapes(CollisionManager this)
{
	ASSERT(this, "CollisionManager::getNumberOfActiveShapes: null this");

	int count = 0;

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		Shape shape = __SAFE_CAST(Shape, node->data);

		if(shape->isActive)
		{
			count++;
		}
	}

	return count;
}

// print status
void CollisionManager_print(CollisionManager this, int x, int y)
{
	ASSERT(this, "CollisionManager::print: null this");

	Printing_resetWorldCoordinates(Printing_getInstance());

	Printing_text(Printing_getInstance(), "COLLISION MANAGER", x, y++, NULL);
	Printing_text(Printing_getInstance(), "SHAPES", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "  Registered:     ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->shapes), x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "  Active:          ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), CollisionManager_getNumberOfActiveShapes(this), x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "  Moving:          ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->movingShapes), x + 14, y++, NULL);

	Printing_text(Printing_getInstance(), "STATISTICS (per cycle)", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "Average", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "  Checks:          ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->checkCycles ? this->collisionChecks / this->checkCycles : 0, x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "  Collisions:      ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->checkCycles ? this->collisions / this->checkCycles : 0, x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "Last cycle", x, ++y, NULL);
	Printing_text(Printing_getInstance(), "  Checks:          ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->lastCycleCollisionChecks, x + 14, y, NULL);
	Printing_text(Printing_getInstance(), "  Collisions:      ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), this->lastCycleCollisions, x + 14, y, NULL);
}
