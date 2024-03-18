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

#include <Clock.h>
#include <DebugConfig.h>
#include <Printing.h>
#include <SpatialObject.h>
#include <Collider.h>
#include <VirtualList.h>

#include "CollisionManager.h"


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
	this->colliders = new VirtualList();

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
	ASSERT(this->colliders, "CollisionManager::destructor: null colliders");

	CollisionManager::reset(this);

	delete this->colliders;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void CollisionManager::purgeDestroyedColliders()
{
	for(VirtualNode auxNode = this->colliders->head, auxNextNode = NULL; auxNode; auxNode = auxNextNode)
	{
		auxNextNode = auxNode->next;

		// load the current collider to check against
		Collider colliderToCheck = Collider::safeCast(auxNode->data);

		if(colliderToCheck->destroyMe)
		{
			VirtualList::removeNode(this->colliders, auxNode);

			delete colliderToCheck;
		}
	}	
}

// register a collider
Collider CollisionManager::createCollider(SpatialObject owner, const ColliderSpec* colliderSpec)
{
	NM_ASSERT(!(NULL == colliderSpec || NULL == colliderSpec->allocator), "CollisionManager::createCollider: invalid collider spec");

	if(NULL == colliderSpec || NULL == colliderSpec->allocator)
	{
		return NULL;
	}

	CollisionManager::purgeDestroyedColliders(this);

	// create the collider
	Collider collider = ((Collider (*)(SpatialObject, const ColliderSpec*)) colliderSpec->allocator)(owner, colliderSpec);

	this->dirty = true;

	// register it
	VirtualList::pushFront(this->colliders, collider);

	// return created collider
	return collider;
}

// remove a collider
void CollisionManager::destroyCollider(Collider collider)
{
	if(!isDeleted(collider))
	{
#ifndef __ENABLE_PROFILER
		NM_ASSERT(NULL != VirtualList::find(this->colliders, collider), "CollisionManager::destroyCollider: non registerd collider");
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

	// check the colliders
	for(VirtualNode auxNode = this->colliders->head, auxNextNode = NULL; auxNode; auxNode = auxNextNode)
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
			VirtualList::removeNode(this->colliders, auxNode);

			delete collider;
			continue;
		}

	#ifdef __DRAW_SHAPES
		if(collider->enabled)
		{
			Collider::show(collider);
		}
		else
		{
			Collider::hide(collider);
		}
	#endif

		if(!(collider->enabled && collider->checkForCollisions) || __NON_TRANSFORMED == collider->transformation->invalid)
		{
			continue;
		}

		Vector3D colliderPosition = collider->transformation->position;

		for(VirtualNode node = this->colliders->head; NULL != node; node = node->next)
		{
			Collider colliderToCheck = Collider::safeCast(node->data);

			if(!(colliderToCheck->enabled) || __NON_TRANSFORMED == colliderToCheck->transformation->invalid)
			{
				continue;
			}

#ifdef __SHOW_PHYSICS_PROFILING
			this->lastCycleCheckProducts++;
#endif

			if(0 == (collider->layersToIgnore & colliderToCheck->layers))
			{
				if(collider->owner == colliderToCheck->owner)
				{
					continue;
				}

				fixed_ext_t distanceVectorSquareLength = Vector3D::squareLength(Vector3D::get(colliderToCheck->transformation->position, colliderPosition));

				if(__FIXED_SQUARE(__COLLIDER_MAXIMUM_SIZE) >= distanceVectorSquareLength)
				{
#ifdef __SHOW_PHYSICS_PROFILING
					this->lastCycleCollisionChecks++;
#endif

#ifdef __SHOW_PHYSICS_PROFILING
					// check if colliders overlap
					if(kNoCollision != Collider::collides(collider, colliderToCheck))
					{
						this->lastCycleCollisions++;
					}
#else
					Collider::collides(collider, colliderToCheck);
#endif
					if(this->dirty)
					{
						node = this->colliders->head;
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

// unregister all colliders
void CollisionManager::reset()
{
	ASSERT(this->colliders, "CollisionManager::reset: null colliders");

	VirtualList::deleteData(this->colliders);

	this->lastCycleCheckProducts = 0;
	this->lastCycleCollisionChecks = 0;
	this->lastCycleCollisions = 0;
	this->checkCycles = 0;
	this->collisionChecks = 0;
	this->collisions = 0;
	this->dirty = false;
}


// draw colliders
void CollisionManager::showColliders()
{
	// comparing against the other colliders
	VirtualNode node = this->colliders->head;

	// check the colliders
	for(; NULL != node; node = node->next)
	{
		Collider::show(node->data);
	}
}

// free memory by deleting direct draw Polyhedrons
void CollisionManager::hideColliders()
{
	// comparing against the other colliders
	VirtualNode node = this->colliders->head;

	// check the colliders
	for(; NULL != node; node = node->next)
	{
		Collider::hide(node->data);
	}
}

int32 CollisionManager::getNumberOfEnabledColliders()
{
	int32 count = 0;

	// comparing against the other colliders
	VirtualNode node = this->colliders->head;

	// check the colliders
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

	// comparing against the other colliders
	VirtualNode node = this->colliders->head;

	// check the colliders
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

#ifndef __SHIPPING
void CollisionManager::print(int32 x, int32 y)
{
	Printing::resetCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "COLLISION MANAGER", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Colliders", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Registered:     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->colliders), x + 12, y, NULL);
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
#endif
