/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
	this->activeForCollisionCheckingShapes = new VirtualList();
	this->clearActiveForCollisionCheckingShapes = true;

	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles = 0;
	this->collisionChecks = 0;
	this->collisions = 0;
	this->checkShapesOutOfCameraRange = false;
}

// class's destructor
void CollisionManager::destructor()
{
	ASSERT(this->shapes, "CollisionManager::destructor: null shapes");

	CollisionManager::reset(this);

	// delete lists
	delete this->shapes;
	delete this->activeForCollisionCheckingShapes;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

// register a shape
Shape CollisionManager::createShape(SpatialObject owner, const ShapeSpec* shapeSpec)
{
	// create the shape
	Shape shape = ((Shape (*)(SpatialObject)) shapeSpec->allocator)(owner);
	Shape::setup(shape, shapeSpec->layers, shapeSpec->layersToIgnore);
	Shape::setCheckForCollisions(shape, shapeSpec->checkForCollisions);

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
		VirtualList::removeElement(this->activeForCollisionCheckingShapes, shape);
		this->clearActiveForCollisionCheckingShapes = true;

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

	static VirtualList activeForCollisionCheckingShapes = NULL;

	if(NULL == activeForCollisionCheckingShapes)
	{
		activeForCollisionCheckingShapes = new VirtualList();
	}
	else if(this->clearActiveForCollisionCheckingShapes)
	{
		VirtualList::clear(activeForCollisionCheckingShapes);
		VirtualList::copy(activeForCollisionCheckingShapes, this->activeForCollisionCheckingShapes);
	}

	this->clearActiveForCollisionCheckingShapes = false;

	// check the shapes
	for(VirtualNode auxNode = this->shapes->head; auxNode; auxNode = auxNode->next)
	{
		// load the current shape to check against
		Shape shapeToCheck = Shape::safeCast(auxNode->data);

		// compare only different ready, different shapes against it other if
		// the layer of the shapeToCheck are not excluded by the current shape
		if(isDeleted(shapeToCheck) || !shapeToCheck->enabled || !shapeToCheck->ready)
		{
			continue;
		}

		shapeToCheck->isVisible = true;

		extern const Vector3D* _cameraPosition;

		// not ready for collision checks if out of the camera
		if(!this->checkShapesOutOfCameraRange)
		{
			if(
				shapeToCheck->rightBox.x0 - _cameraPosition->x > __SCREEN_WIDTH_METERS ||
				shapeToCheck->rightBox.x1 - _cameraPosition->x < 0 ||
				shapeToCheck->rightBox.y0 - _cameraPosition->y > __SCREEN_HEIGHT_METERS ||
				shapeToCheck->rightBox.y1 - _cameraPosition->y < 0
			)
			{
				shapeToCheck->isVisible = false;
			}
		}

		shapeToCheck->moved = false;

		if(!shapeToCheck->isVisible)
		{
			continue;
		}

#ifdef __DRAW_SHAPES
		if(shapeToCheck->enabled && shapeToCheck->isVisible)
		{
			Shape::show(shapeToCheck);
		}
		else
		{
			Shape::hide(shapeToCheck);
		}
#endif
		// check the shapes
		for(VirtualNode node = activeForCollisionCheckingShapes->head; node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			if(isDeleted(shape) || !shape->enabled || (__COLLISION_ALL_LAYERS == shape->layersToIgnore))
			{
				continue;
			}

			// load the current shape
			if(!shape->ready || !shape->checkForCollisions || !shape->isVisible || shape == shapeToCheck || (shape->layersToIgnore & shapeToCheck->layers))
			{
				continue;
			}

			this->lastCycleCollisionChecks++;

			CollisionData collisionData = Shape::collides(shape, shapeToCheck);

			// check if shapes overlap
			if(kNoCollision != collisionData.result)
			{
				this->lastCycleCollisions++;
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
	VirtualList::clear(this->activeForCollisionCheckingShapes);
	this->clearActiveForCollisionCheckingShapes = true;

	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles = 0;
	this->collisionChecks = 0;
	this->collisions = 0;
}

// inform of a change in the shape
void CollisionManager::activeCollisionCheckForShape(Shape shape, bool activate)
{
	ASSERT(shape, "CollisionManager::activeCollisionCheckForShape: null shape");

	if(activate)
	{
		Shape::enable(shape, true);

		if(!VirtualList::find(this->activeForCollisionCheckingShapes, shape))
		{
			VirtualList::pushBack(this->activeForCollisionCheckingShapes, shape);
			this->clearActiveForCollisionCheckingShapes = true;
		}
	}
	else
	{
		VirtualList::removeElement(this->activeForCollisionCheckingShapes, shape);
		this->clearActiveForCollisionCheckingShapes = true;
	}
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

int CollisionManager::getNumberOfactiveForCollisionCheckingShapes()
{
	int count = 0;

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		Shape shape = Shape::safeCast(node->data);

		if(shape->enabled)
		{
			count++;
		}
	}

	return count;
}

void CollisionManager::setCheckShapesOutOfCameraRange(bool value)
{
	this->checkShapesOutOfCameraRange = value;
}


// print status
void CollisionManager::print(int x, int y)
{
	Printing::resetCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "COLLISION MANAGER", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Shapes", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Registered:     ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->shapes), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Active:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), CollisionManager::getNumberOfactiveForCollisionCheckingShapes(this), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Moving:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->activeForCollisionCheckingShapes), x + 12, y++, NULL);

	Printing::text(Printing::getInstance(), "Statistics (per cycle)", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Average", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Checks:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->checkCycles ? this->collisionChecks / this->checkCycles : 0, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Collisions:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->checkCycles ? this->collisions / this->checkCycles : 0, x + 12, y++, NULL);
	Printing::text(Printing::getInstance(), "Last cycle", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Checks:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->lastCycleCollisionChecks, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Collisions:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), this->lastCycleCollisions, x + 12, y, NULL);
}
