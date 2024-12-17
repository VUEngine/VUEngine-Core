/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Clock.h>
#include <DebugConfig.h>
#include <Printing.h>
#include <SpatialObject.h>
#include <Collider.h>
#include <VirtualList.h>

#include "CollisionManager.h"


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

friend class Collider;
friend class Clock;
friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __TOTAL_USABLE_SHAPES		128


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

#ifdef __SHOW_PHYSICS_PROFILING
/// Counters for debugging
static uint16 _lastCycleCheckProducts;
static uint16 _lastCycleCollisionChecks;
static uint16 _lastCycleCollisions;
static uint16 _collisionChecks;
static uint16 _collisions;
static uint16 _checkCycles;
#endif


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
Collider CollisionManager::createComponent(SpatialObject owner, const ColliderSpec* colliderSpec)
{
	return CollisionManager::createCollider(this, owner, colliderSpec);
}
//---------------------------------------------------------------------------------------------------------
void CollisionManager::destroyComponent(Collider collider)
{
	CollisionManager::destroyCollider(this, collider);
}
//---------------------------------------------------------------------------------------------------------
void CollisionManager::reset()
{
	ASSERT(this->components, "CollisionManager::reset: null colliders");

	VirtualList::deleteData(this->components);

	this->dirty = false;

#ifdef __SHOW_PHYSICS_PROFILING
	_lastCycleCheckProducts = 0;
	_lastCycleCollisionChecks = 0;
	_lastCycleCollisions = 0;
	_checkCycles = 0;
	_collisionChecks = 0;
	_collisions = 0;
#endif
}
//---------------------------------------------------------------------------------------------------------
void CollisionManager::purgeDestroyedColliders()
{
	for(VirtualNode auxNode = this->components->head, auxNextNode = NULL; auxNode; auxNode = auxNextNode)
	{
		auxNextNode = auxNode->next;

		// load the current collider to check against
		Collider collider = Collider::safeCast(auxNode->data);

		if(collider->deleteMe)
		{
			VirtualList::removeNode(this->components, auxNode);

			delete collider;
		}
	}	
}
//---------------------------------------------------------------------------------------------------------
uint32 CollisionManager::update()
{
	uint32 returnValue = false;

#ifdef __SHOW_PHYSICS_PROFILING
	_lastCycleCheckProducts = 0;
	_lastCycleCollisionChecks = 0;
	_lastCycleCollisions = 0;
	_checkCycles++;
#endif

	this->dirty = false;

	for(VirtualNode auxNode = this->components->head, auxNextNode = NULL; NULL != auxNode; auxNode = auxNextNode)
	{
		auxNextNode = auxNode->next;

		// load the current collider to check against
		Collider collider = Collider::safeCast(auxNode->data);

#ifndef __RELEASE
		if(isDeleted(collider) || collider->deleteMe)
#else
		if(collider->deleteMe)
#endif
		{
			VirtualList::removeNode(this->components, auxNode);

			delete collider;
			continue;
		}

			collider->invalidPosition = true;
	}

	// check the colliders
	for(VirtualNode auxNode = this->components->head; NULL != auxNode; auxNode = auxNode->next)
	{
		// load the current collider to check against
		Collider collider = Collider::safeCast(auxNode->data);

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

		if(collider->invalidPosition)
		{
			Vector3D displacement = Vector3D::rotate(Vector3D::getFromPixelVector(((ColliderSpec*)collider->componentSpec)->displacement), collider->transformation->rotation);
			collider->position = Vector3D::sum(collider->transformation->position, displacement);
			collider->invalidPosition = false;
		}

		Vector3D colliderPosition = collider->transformation->position;

		for(VirtualNode node = this->components->head; NULL != node; node = node->next)
		{
			Collider colliderToCheck = Collider::safeCast(node->data);

			if(!colliderToCheck->enabled || __NON_TRANSFORMED == colliderToCheck->transformation->invalid)
			{
				continue;
			}

#ifdef __SHOW_PHYSICS_PROFILING
			_lastCycleCheckProducts++;
#endif

			if(0 != (collider->layersToIgnore & colliderToCheck->layers))
			{
				continue;
			}

			if(collider->owner == colliderToCheck->owner)
			{
				continue;
			}

			fixed_ext_t distanceVectorSquareLength = Vector3D::squareLength(Vector3D::get(colliderToCheck->transformation->position, colliderPosition));

			if(__FIXED_SQUARE(__COLLIDER_MAXIMUM_SIZE) < distanceVectorSquareLength)
			{
				continue;
			}

#ifdef __SHOW_PHYSICS_PROFILING
			_lastCycleCollisionChecks++;
#endif

			if(colliderToCheck->invalidPosition)
			{
				Vector3D displacement = Vector3D::rotate(Vector3D::getFromPixelVector(((ColliderSpec*)colliderToCheck->componentSpec)->displacement), colliderToCheck->transformation->rotation);				
				colliderToCheck->position = Vector3D::sum(colliderToCheck->transformation->position, displacement);
				colliderToCheck->invalidPosition = false;
			}

#ifdef __SHOW_PHYSICS_PROFILING
			if(kNoCollision != Collider::collides(collider, colliderToCheck))
			{
				_lastCycleCollisions++;
			}
#else
			Collider::collides(collider, colliderToCheck);
#endif
			if(this->dirty)
			{
				NM_ASSERT(false, "CollisionManager::update: added a collider as a response to a collision");
				node = this->components->head;
			}
		}
	}

#ifdef __SHOW_PHYSICS_PROFILING
	_collisionChecks += _lastCycleCollisionChecks;
	_collisions += _lastCycleCollisions;

	CollisionManager::print(this, 25, 1);
#endif

	return returnValue;
}
Collider CollisionManager::createCollider(SpatialObject owner, const ColliderSpec* colliderSpec)
{
	NM_ASSERT(!(NULL == colliderSpec || NULL == colliderSpec->allocator), "CollisionManager::createCollider: invalid collider spec");

	if(NULL == colliderSpec || NULL == colliderSpec->allocator)
	{
		return NULL;
	}

	CollisionManager::purgeDestroyedColliders(this);

	Collider collider = ((Collider (*)(SpatialObject, const ColliderSpec*)) colliderSpec->allocator)(owner, colliderSpec);

	this->dirty = true;

	VirtualList::pushFront(this->components, collider);

	return collider;
}
//---------------------------------------------------------------------------------------------------------
void CollisionManager::destroyCollider(Collider collider)
{
	if(!isDeleted(collider))
	{
#ifndef __ENABLE_PROFILER
		NM_ASSERT(NULL != VirtualList::find(this->components, collider), "CollisionManager::destroyCollider: non registerd collider");
#endif
		collider->deleteMe = true;
	}
}
//---------------------------------------------------------------------------------------------------------
void CollisionManager::setCheckCollidersOutOfCameraRange(bool value)
{
	this->checkCollidersOutOfCameraRange = value;
}
//---------------------------------------------------------------------------------------------------------
void CollisionManager::showColliders()
{
	// comparing against the other colliders
	VirtualNode node = this->components->head;

	// check the colliders
	for(; NULL != node; node = node->next)
	{
		Collider::show(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------
void CollisionManager::hideColliders()
{
	// comparing against the other colliders
	VirtualNode node = this->components->head;

	// check the colliders
	for(; NULL != node; node = node->next)
	{
		Collider::hide(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------
#ifndef __SHIPPING
void CollisionManager::print(int32 x, int32 y)
{
	Printing::resetCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "COLLISION MANAGER", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Colliders", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Registered:     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->components), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Enabled:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), CollisionManager::getNumberOfEnabledColliders(this), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Moving:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), CollisionManager::getNumberOfMovingEnabledColliders(this), x + 12, y++, NULL);

#ifdef __SHOW_PHYSICS_PROFILING
	Printing::text(Printing::getInstance(), "Statistics (per cycle)", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Average", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Checks:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _checkCycles ? _collisionChecks / _checkCycles : 0, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Collisions:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _checkCycles ? _collisions / _checkCycles : 0, x + 12, y++, NULL);
	Printing::text(Printing::getInstance(), "Last cycle", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "Products:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _lastCycleCheckProducts, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Checks:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _lastCycleCollisionChecks, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "Collisions:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _lastCycleCollisions, x + 12, y, NULL);
#endif
}
#endif
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void CollisionManager::constructor()
{
#ifdef __SHOW_PHYSICS_PROFILING
	_lastCycleCheckProducts = 0;
	_lastCycleCollisionChecks = 0;
	_lastCycleCollisions = 0;
	_checkCycles = 0;
	_collisionChecks = 0;
	_collisions = 0;
#endif

	Base::constructor();

	this->checkCollidersOutOfCameraRange = false;
	this->dirty = false;
}
//---------------------------------------------------------------------------------------------------------
void CollisionManager::destructor()
{
	ASSERT(this->components, "CollisionManager::destructor: null colliders");

	CollisionManager::reset(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
int32 CollisionManager::getNumberOfEnabledColliders()
{
	int32 count = 0;

	// comparing against the other colliders
	VirtualNode node = this->components->head;

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
//---------------------------------------------------------------------------------------------------------
int32 CollisionManager::getNumberOfMovingEnabledColliders()
{
	int32 count = 0;

	// comparing against the other colliders
	VirtualNode node = this->components->head;

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
//---------------------------------------------------------------------------------------------------------
