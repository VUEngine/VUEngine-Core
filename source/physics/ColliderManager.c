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

#include "ColliderManager.h"


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
Collider ColliderManager::createComponent(SpatialObject owner, const ColliderSpec* colliderSpec)
{
	if(NULL == colliderSpec)
	{
		return NULL;
	}

	Base::createComponent(this, owner, (ComponentSpec*)colliderSpec);

	return ColliderManager::createCollider(this, owner, colliderSpec);
}
//---------------------------------------------------------------------------------------------------------
void ColliderManager::destroyComponent(SpatialObject owner, Collider collider) 
{
	if(isDeleted(collider))
	{
		return;
	}

	Base::destroyComponent(this, owner, Component::safeCast(collider));

	ColliderManager::destroyCollider(this, collider);
}
//---------------------------------------------------------------------------------------------------------
void ColliderManager::reset()
{
	ASSERT(this->components, "ColliderManager::reset: null colliders");

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
void ColliderManager::purgeDestroyedColliders()
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
uint32 ColliderManager::update()
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
				NM_ASSERT(false, "ColliderManager::update: added a collider as a response to a collision");
				node = this->components->head;
			}
		}
	}

#ifdef __SHOW_PHYSICS_PROFILING
	_collisionChecks += _lastCycleCollisionChecks;
	_collisions += _lastCycleCollisions;

	ColliderManager::print(this, 25, 1);
#endif

	return returnValue;
}
Collider ColliderManager::createCollider(SpatialObject owner, const ColliderSpec* colliderSpec)
{
	if(NULL == colliderSpec)
	{
		return NULL;
	}

	ColliderManager::purgeDestroyedColliders(this);

	Collider collider = ((Collider (*)(SpatialObject, const ColliderSpec*)) ((ComponentSpec*)colliderSpec)->allocator)(owner, colliderSpec);

	this->dirty = true;

	VirtualList::pushFront(this->components, collider);

	return collider;
}
//---------------------------------------------------------------------------------------------------------
void ColliderManager::destroyCollider(Collider collider)
{
	if(!isDeleted(collider))
	{
#ifndef __ENABLE_PROFILER
		NM_ASSERT(NULL != VirtualList::find(this->components, collider), "ColliderManager::destroyCollider: non registerd collider");
#endif
		collider->deleteMe = true;
	}
}
//---------------------------------------------------------------------------------------------------------
void ColliderManager::setCheckCollidersOutOfCameraRange(bool value)
{
	this->checkCollidersOutOfCameraRange = value;
}
//---------------------------------------------------------------------------------------------------------
void ColliderManager::showColliders()
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
void ColliderManager::hideColliders()
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
void ColliderManager::print(int32 x, int32 y)
{
	Printing::resetCoordinates(Printing::getInstance());

	Printing::text(Printing::getInstance(), "COLLISION MANAGER", x, y++, NULL);
	Printing::text(Printing::getInstance(), "COLLIDERS", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "REGISTERED:     ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->components), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "ENABLED:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), ColliderManager::getNumberOfEnabledColliders(this), x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "MOVING:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), ColliderManager::getNumberOfMovingEnabledColliders(this), x + 12, y++, NULL);

#ifdef __SHOW_PHYSICS_PROFILING
	Printing::text(Printing::getInstance(), "STATISTICS (PER CYCLE)", x, ++y, NULL);
	y++;
	Printing::text(Printing::getInstance(), "AVERAGE", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "CHECKS:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _checkCycles ? _collisionChecks / _checkCycles : 0, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "COLLISIONS:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _checkCycles ? _collisions / _checkCycles : 0, x + 12, y++, NULL);
	Printing::text(Printing::getInstance(), "LAST CYCLE", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "PRODUCTS:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _lastCycleCheckProducts, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "CHECKS:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _lastCycleCollisionChecks, x + 12, y, NULL);
	Printing::text(Printing::getInstance(), "COLLISIONS:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), _lastCycleCollisions, x + 12, y, NULL);
#endif
}
#endif
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ColliderManager::constructor()
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
void ColliderManager::destructor()
{
	ASSERT(this->components, "ColliderManager::destructor: null colliders");

	if(!isDeleted(this->components))
	{
		VirtualList::deleteData(this->components);
	}
	
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
int32 ColliderManager::getNumberOfEnabledColliders()
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
int32 ColliderManager::getNumberOfMovingEnabledColliders()
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
