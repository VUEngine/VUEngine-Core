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

friend class Shape;
friend class Clock;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void CollisionManager::constructor()
{
	Base::constructor();

	// create the shape list
	this->shapes = new VirtualList();
	this->movingShapes = new VirtualList();

	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles = 0;
	this->collisionChecks = 0;
	this->collisions = 0;
}

// class's destructor
void CollisionManager::destructor()
{
	ASSERT(this->shapes, "CollisionManager::destructor: null shapes");

	CollisionManager::reset(this);

	// delete lists
	delete this->shapes;
	delete this->movingShapes;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

// register a shape
Shape CollisionManager::createShape(SpatialObject owner, const ShapeDefinition* shapeDefinition)
{
	// create the shape
	Shape shape = ((Shape (*)(SpatialObject)) shapeDefinition->allocator)(owner);
	Shape::setup(shape, shapeDefinition->layers, shapeDefinition->layersToIgnore);
	Shape::setCheckForCollisions(shape, shapeDefinition->checkForCollisions);

	// register it
	VirtualList::pushFront(this->shapes, shape);

	// return created shape
	return shape;
}

// remove a shape
void CollisionManager::destroyShape(Shape shape)
{
	if(shape && VirtualList::find(this->shapes, shape))
	{
		VirtualList::removeElement(this->shapes, shape);
		VirtualList::removeElement(this->movingShapes, shape);

		// delete it
		delete shape;
	}
}

// calculate collisions
u32 CollisionManager::update(Clock clock)
{
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
		Shape shape = Shape::safeCast(node->data);
		shape->isVisible = true;

		extern const Vector3D* _cameraPosition;
		extern const CameraFrustum* _cameraFrustum;

		RightBox surroundingRightBox =  Shape::getSurroundingRightBox(shape);

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
			Shape::show(shape);
		}
		else
		{
			Shape::hide(shape);
		}
#endif

		shape->moved = false;
	}

	NM_ASSERT(__TOTAL_USABLE_SHAPES >= VirtualList::getSize(this->movingShapes), "CollisionManager::update: too many moving shapes");

	Shape movingShapes[__TOTAL_USABLE_SHAPES];
	Shape activeShapes[__TOTAL_USABLE_SHAPES];

	int movingShapesIndex = 0;
	int activeShapesIndex = 0;

	// check the shapes
	for(movingShapesIndex = 0, node = this->movingShapes->head; node; node = node->next)
	{
		Shape shape = Shape::safeCast(node->data);

		if(shape->isActive)
		{
			movingShapes[movingShapesIndex++] = shape;
		}
	}

	movingShapes[movingShapesIndex] = NULL;

	// check the shapes
	for(activeShapesIndex = 0, node = this->shapes->head; node; node = node->next)
	{
		Shape shape = Shape::safeCast(node->data);

		if(shape->isActive)
		{
			activeShapes[activeShapesIndex++] = shape;
		}
	}

	activeShapes[activeShapesIndex] = NULL;

	// check the shapes
	for(movingShapesIndex = 0; movingShapes[movingShapesIndex]; movingShapesIndex++)
	{
		if(isDeleted(movingShapes[movingShapesIndex]))
		{
			continue;
		}

		// load the current shape
		Shape shape = Shape::safeCast(movingShapes[movingShapesIndex]);

		if(shape->ready && shape->checkForCollisions && shape->isVisible)
		{
			// check the shapes
			for(activeShapesIndex = 0; activeShapes[activeShapesIndex]; activeShapesIndex++)
			{
				if(isDeleted(activeShapes[activeShapesIndex]))
				{
					continue;
				}

				// load the current shape to check against
				Shape shapeToCheck = Shape::safeCast(activeShapes[activeShapesIndex]);

				if(!shapeToCheck->isActive)
				{
					continue;
				}

				// compare only different ready, different shapes against it other if
				// the layer of the shapeToCheck are not excluded by the current shape
				if(shape != shapeToCheck && shapeToCheck->ready && shapeToCheck->isVisible && !(shape->layersToIgnore & shapeToCheck->layers))
				{
					this->lastCycleCollisionChecks++;

					CollisionData collisionData = Shape::collides(shape, shapeToCheck);

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
	CollisionManager::print(this, 25, 1);
#endif

	return returnValue;
}

// unregister all shapes
void CollisionManager::reset()
{
	ASSERT(this->shapes, "CollisionManager::reset: null shapes");

	VirtualNode node = this->shapes->head;

	for(; node; node = node->next)
	{
		// delete it
		delete node->data;
	}

	// empty the lists
	VirtualList::clear(this->shapes);
	VirtualList::clear(this->movingShapes);

	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles = 0;
	this->collisionChecks = 0;
	this->collisions = 0;
}

// inform of a change in the shape
void CollisionManager::shapeStartedMoving(Shape shape)
{
	ASSERT(shape, "CollisionManager::shapeStartedMoving: null shape");

	if(!VirtualList::find(this->movingShapes, shape))
	{
		VirtualList::pushBack(this->movingShapes, shape);
	}
}

// inform of a change in the shape
void CollisionManager::shapeStoppedMoving(Shape shape)
{
	ASSERT(shape, "CollisionManager::shapeStoppedMoving: null shape");

	VirtualList::removeElement(this->movingShapes, shape);
}

// draw shapes
void CollisionManager::showShapes()
{
	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		Shape::show(node->data);
	}
}

// free memory by deleting direct draw Polyhedrons
void CollisionManager::hideShapes()
{//	ASSERT(this->shapes, "CollisionManager::drawShapes: null shapes");

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		Shape::hide(node->data);
	}
}

int CollisionManager::getNumberOfActiveShapes()
{
	int count = 0;

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		Shape shape = Shape::safeCast(node->data);

		if(shape->isActive)
		{
			count++;
		}
	}

	return count;
}

// print status
void CollisionManager::print(int x, int y)
{
	Printing::resetWorldCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "COLLISION MANAGER", x, y++, NULL);
	Printing::text(Printing::getInstance(), "SHAPES", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "  Registered:     ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->shapes), x + 14, y, NULL);
	Printing::text(Printing::getInstance(), "  Active:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), CollisionManager::getNumberOfActiveShapes(this), x + 14, y, NULL);
	Printing::text(Printing::getInstance(), "  Moving:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->movingShapes), x + 14, y++, NULL);

	Printing::text(Printing::getInstance(), "STATISTICS (per cycle)", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Average", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "  Checks:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->checkCycles ? this->collisionChecks / this->checkCycles : 0, x + 14, y, NULL);
	Printing::text(Printing::getInstance(), "  Collisions:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->checkCycles ? this->collisions / this->checkCycles : 0, x + 14, y, NULL);
	Printing::text(Printing::getInstance(), "Last cycle", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "  Checks:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->lastCycleCollisionChecks, x + 14, y, NULL);
	Printing::text(Printing::getInstance(), "  Collisions:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->lastCycleCollisions, x + 14, y, NULL);
}
