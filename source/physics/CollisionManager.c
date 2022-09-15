/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
	VirtualList::pushBack(this->activeForCollisionCheckingShapes, shape);

	// register it
	VirtualList::pushFront(this->shapes, shape);

	// return created shape
	return shape;
}

// remove a shape
void CollisionManager::destroyShape(Shape shape)
{
	if(!isDeleted(shape))
	{
		NM_ASSERT(NULL != VirtualList::find(this->shapes, shape), "CollisionManager::destroyShape: non registerd shape");
		shape->destroyMe = true;
	}
}

// calculate collisions
uint32 CollisionManager::update(Clock clock)
{
	if(clock->paused)
	{
		return false;
	}

	uint32 returnValue = false;

	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles++;

	// check the shapes
	for(VirtualNode auxNode = this->shapes->head, auxNextNode = NULL; auxNode; auxNode = auxNextNode)
	{
		auxNextNode = auxNode->next;

		// load the current shape to check against
		Shape shape = Shape::safeCast(auxNode->data);

		if(isDeleted(shape) || shape->destroyMe)
		{
			VirtualList::removeNode(this->shapes, auxNode);
			VirtualList::removeElement(this->activeForCollisionCheckingShapes, shape);

			delete shape;
			continue;
		}

		shape->moved = false;

		// compare only different shapes against each other if
		// the layers of the shapeToCheck are not excluded by the current shape
		if(!shape->enabled || !shape->ready)
		{
			shape->isVisible = false;
			continue;
		}

		shape->isVisible = true;

		extern const Vector3D* _cameraPosition;

		// not ready for collision checks if out of the camera
		if(!this->checkShapesOutOfCameraRange)
		{
			extern const Rotation* _cameraInvertedRotation;
			Vector3D relativePosition = Vector3D::rotate(Vector3D::getRelativeToCamera(Shape::getPosition(shape)), *_cameraInvertedRotation);
			PixelVector position2D = Vector3D::projectToPixelVector(relativePosition, 0);

			// check x visibility
			if((position2D.x < _cameraFrustum->x0) || (position2D.x > _cameraFrustum->x1))
			{
				shape->isVisible = false;
				continue;
			}

			if((position2D.y < _cameraFrustum->y0) || (position2D.y > _cameraFrustum->y1))
			{
				shape->isVisible = false;
				continue;
			}

			if((position2D.z < _cameraFrustum->z0) || (position2D.z > _cameraFrustum->z1))
			{
				shape->isVisible = false;
				continue;
			}			
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
	}

	// check the shapes
	for(VirtualNode node = this->activeForCollisionCheckingShapes->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		Shape shape = Shape::safeCast(node->data);

		if(!shape->checkForCollisions)
		{
			VirtualList::removeNode(this->activeForCollisionCheckingShapes, node);
			continue;
		}

		if(!shape->isVisible)
		{
			continue;
		}

		Vector3D shapePosition = Shape::getPosition(shape);

		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Shape shapeToCheck = Shape::safeCast(node->data);

			if(!shapeToCheck->isVisible)
			{
				continue;
			}

			if(0 == (shape->layersToIgnore & shapeToCheck->layers))
			{
				if(shape->owner == shapeToCheck->owner)
				{
					continue;
				}

				fixed_ext_t distanceVectorSquareLength = Vector3D::squareLength(Vector3D::get(Shape::getPosition(shapeToCheck), shapePosition));

				if(__FIXED_SQUARE(__SHAPE_MAXIMUM_SIZE) >= distanceVectorSquareLength)
				{
#ifdef __SHOW_PHYSICS_PROFILING
					this->lastCycleCollisionChecks++;
#endif

#ifdef __SHOW_PHYSICS_PROFILING
					// check if shapes overlap
					if(kNoCollision != Shape::collides(shape, shapeToCheck))
					{
						this->lastCycleCollisions++;
					}
#else
					Shape::collides(shape, shapeToCheck);
#endif
				}
			}
			
		}
	}

#ifdef __SHOW_PHYSICS_PROFILING
	this->collisionChecks += this->lastCycleCollisionChecks;
	this->collisions += this->lastCycleCollisions;

	CollisionManager::print(this, 25, 1);
#endif

	return returnValue;
}

// unregister all shapes
void CollisionManager::reset()
{
	ASSERT(this->shapes, "CollisionManager::reset: null shapes");

	VirtualNode node = this->shapes->head;

	for(; NULL != node; node = node->next)
	{
		delete node->data;
	}

	// empty the lists
	VirtualList::clear(this->shapes);
	VirtualList::clear(this->activeForCollisionCheckingShapes);

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
		}
	}
}

// draw shapes
void CollisionManager::showShapes()
{
	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; NULL != node; node = node->next)
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
	for(; NULL != node; node = node->next)
	{
		Shape::hide(node->data);
	}
}

int32 CollisionManager::getNumberOfactiveForCollisionCheckingShapes()
{
	int32 count = 0;

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; NULL != node; node = node->next)
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
void CollisionManager::print(int32 x, int32 y)
{
	Printing::resetCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "COLLISION MANAGER", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Shapes", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Registered:     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->shapes), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Active:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), CollisionManager::getNumberOfactiveForCollisionCheckingShapes(this), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Moving:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->activeForCollisionCheckingShapes), x + 12, y++, NULL);

	Printing::text(Printing::getInstance(), "Statistics (per cycle)", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Average", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Checks:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->checkCycles ? this->collisionChecks / this->checkCycles : 0, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Collisions:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->checkCycles ? this->collisions / this->checkCycles : 0, x + 12, y++, NULL);
	Printing::text(Printing::getInstance(), "Last cycle", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Checks:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->lastCycleCollisionChecks, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Collisions:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->lastCycleCollisions, x + 12, y, NULL);
}
