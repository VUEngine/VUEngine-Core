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

#include <Clock.h>
#include <Printing.h>
#include <SpatialObject.h>
#include <Collider.h>
#include <VirtualList.h>

#include <DebugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											MACROS
//---------------------------------------------------------------------------------------------------------
#define __TOTAL_USABLE_SHAPES		128


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Collider;
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

	// create the collider list
	this->shapes = new VirtualList();

	this->lastCycleCheckProducts = 0;
	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles = 0;
	this->collisionChecks = 0;
	this->collisions = 0;
	this->checkCollidersOutOfCameraRange = false;
	this->dirty = false;
}

// class's destructor
void CollisionManager::destructor()
{
	ASSERT(this->shapes, "CollisionManager::destructor: null shapes");

	CollisionManager::reset(this);

	delete this->shapes;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void CollisionManager::purgeDestroyedColliders()
{
	for(VirtualNode auxNode = this->shapes->head, auxNextNode = NULL; auxNode; auxNode = auxNextNode)
	{
		auxNextNode = auxNode->next;

		// load the current collider to check against
		Collider shapeToCheck = Collider::safeCast(auxNode->data);

		if(shapeToCheck->destroyMe)
		{
			VirtualList::removeNode(this->shapes, auxNode);

			delete shapeToCheck;
		}
	}	
}

// register a collider
Collider CollisionManager::createCollider(SpatialObject owner, const ColliderSpec* shapeSpec)
{
	NM_ASSERT(!(NULL == shapeSpec || NULL == shapeSpec->allocator), "CollisionManager::createCollider: invalid collider spec");

	if(NULL == shapeSpec || NULL == shapeSpec->allocator)
	{
		return NULL;
	}

	CollisionManager::purgeDestroyedColliders(this);

	// create the collider
	Collider collider = ((Collider (*)(SpatialObject, const ColliderSpec*)) shapeSpec->allocator)(owner, shapeSpec);

	this->dirty = true;

	// register it
	VirtualList::pushFront(this->shapes, collider);

	// return created collider
	return collider;
}

// remove a collider
void CollisionManager::destroyCollider(Collider collider)
{
	if(!isDeleted(collider))
	{
#ifndef __ENABLE_PROFILER
		NM_ASSERT(NULL != VirtualList::find(this->shapes, collider), "CollisionManager::destroyCollider: non registerd collider");
#endif
		collider->destroyMe = true;
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

#ifdef __SHOW_PHYSICS_PROFILING
	this->lastCycleCheckProducts = 0;
	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles++;
#endif

	this->dirty = false;

	// check the shapes
	for(VirtualNode auxNode = this->shapes->head, auxNextNode = NULL; auxNode; auxNode = auxNextNode)
	{
		auxNextNode = auxNode->next;

		// load the current collider to check against
		Collider collider = Collider::safeCast(auxNode->data);

#ifndef __RELEASE
		if(isDeleted(collider) || collider->destroyMe)
#else
		if(collider->destroyMe)
#endif
		{
			VirtualList::removeNode(this->shapes, auxNode);

			delete collider;
			continue;
		}

	#ifdef __DRAW_SHAPES
		if(collider->enabled && collider->ready)
		{
			Collider::show(collider);
		}
		else
		{
			Collider::hide(collider);
		}
	#endif

		if(!(collider->enabled && collider->ready && collider->checkForCollisions))
		{
			continue;
		}

		Vector3D shapePosition = collider->position;

		for(VirtualNode node = this->shapes->head; NULL != node; node = node->next)
		{
			Collider shapeToCheck = Collider::safeCast(node->data);

			if(!(shapeToCheck->enabled && shapeToCheck->ready))
			{
				continue;
			}

#ifdef __SHOW_PHYSICS_PROFILING
			this->lastCycleCheckProducts++;
#endif

			if(0 == (collider->layersToIgnore & shapeToCheck->layers))
			{
				if(collider->owner == shapeToCheck->owner)
				{
					continue;
				}

				fixed_ext_t distanceVectorSquareLength = Vector3D::squareLength(Vector3D::get(shapeToCheck->position, shapePosition));

				if(__FIXED_SQUARE(__COLLIDER_MAXIMUM_SIZE) >= distanceVectorSquareLength)
				{
#ifdef __SHOW_PHYSICS_PROFILING
					this->lastCycleCollisionChecks++;
#endif

#ifdef __SHOW_PHYSICS_PROFILING
					// check if shapes overlap
					if(kNoCollision != Collider::collides(collider, shapeToCheck))
					{
						this->lastCycleCollisions++;
					}
#else
					Collider::collides(collider, shapeToCheck);
#endif
					if(this->dirty)
					{
						node = this->shapes->head;
					}
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

	VirtualList::deleteData(this->shapes);

	this->lastCycleCheckProducts = 0;
	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles = 0;
	this->collisionChecks = 0;
	this->collisions = 0;
	this->dirty = false;
}


// draw shapes
void CollisionManager::showColliders()
{
	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; NULL != node; node = node->next)
	{
		Collider::show(node->data);
	}
}

// free memory by deleting direct draw Polyhedrons
void CollisionManager::hideColliders()
{
	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; NULL != node; node = node->next)
	{
		Collider::hide(node->data);
	}
}

int32 CollisionManager::getNumberOfEnabledColliders()
{
	int32 count = 0;

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; NULL != node; node = node->next)
	{
		Collider collider = Collider::safeCast(node->data);

		if(collider->enabled)
		{
			count++;
		}
	}

	return count;
}

int32 CollisionManager::getNumberOfMovingEnabledColliders()
{
	int32 count = 0;

	// comparing against the other shapes
	VirtualNode node = this->shapes->head;

	// check the shapes
	for(; NULL != node; node = node->next)
	{
		Collider collider = Collider::safeCast(node->data);

		if(collider->enabled && collider->checkForCollisions)
		{
			count++;
		}
	}

	return count;
}

void CollisionManager::setCheckCollidersOutOfCameraRange(bool value)
{
	this->checkCollidersOutOfCameraRange = value;
}


// print status
void CollisionManager::print(int32 x, int32 y)
{
	Printing::resetCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "COLLISION MANAGER", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Colliders", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Registered:     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->shapes), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Enabled:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), CollisionManager::getNumberOfEnabledColliders(this), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Moving:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), CollisionManager::getNumberOfMovingEnabledColliders(this), x + 12, y++, NULL);

	Printing::text(Printing::getInstance(), "Statistics (per cycle)", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Average", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Checks:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->checkCycles ? this->collisionChecks / this->checkCycles : 0, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Collisions:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->checkCycles ? this->collisions / this->checkCycles : 0, x + 12, y++, NULL);
	Printing::text(Printing::getInstance(), "Last cycle", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Products:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->lastCycleCheckProducts, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Checks:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->lastCycleCollisionChecks, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Collisions:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->lastCycleCollisions, x + 12, y, NULL);
}
