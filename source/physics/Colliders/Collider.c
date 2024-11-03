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

#include <CollisionHelper.h>
#include <CollisionManager.h>
#include <DebugConfig.h>
#include <Printing.h>
#include <SpatialObject.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "Collider.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __STILL_COLLIDING_CHECK_SIZE_INCREMENT 		__PIXELS_TO_METERS(1)


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Collider::constructor(SpatialObject owner, const ColliderSpec* colliderSpec)
{
	// construct base object
	Base::constructor(owner, colliderSpec);

	// not setup yet
	this->destroyMe = false;
	this->enabled = true;

	this->wireframe = NULL;

	// set flag
	this->checkForCollisions = false;
	this->layers = colliderSpec->layers;
	this->layersToIgnore = colliderSpec->layersToIgnore;
	this->otherColliders = NULL;
	this->registerCollisions = colliderSpec->checkForCollisions;

	this->position = Vector3D::sum(this->transformation->position, Vector3D::getFromPixelVector(colliderSpec->displacement));
	this->invalidPosition = true;
}
//---------------------------------------------------------------------------------------------------------
void Collider::destructor()
{
	// unset owner now
	this->owner = NULL;

	Collider::hide(this);

	if(NULL != this->events)
	{
		Collider::fireEvent(this, kEventColliderDeleted);
		NM_ASSERT(!isDeleted(this), "Collider::destructor: deleted this during kEventColliderDeleted");
	}

	if(NULL != this->otherColliders)
	{
		VirtualNode node = this->otherColliders->head;

		for(; NULL != node; node = node->next)
		{
			OtherColliderRegistry* otherColliderRegistry = (OtherColliderRegistry*)node->data;

			ASSERT(!isDeleted(otherColliderRegistry), "Collider::destructor: dead otherColliderRegistry");

			if(!isDeleted(otherColliderRegistry->collider))
			{
				Collider::removeEventListener(otherColliderRegistry->collider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderDeleted);
				Collider::removeEventListener(otherColliderRegistry->collider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderChanged);
			}

			delete otherColliderRegistry;
		}

		delete this->otherColliders;
		this->otherColliders = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void Collider::enable()
{
	if(!this->enabled)
	{
		Collider::fireEvent(this, kEventColliderChanged);
	}
	
	this->enabled = true;
}
//---------------------------------------------------------------------------------------------------------
void Collider::disable()
{
	if(this->enabled)
	{
		Collider::fireEvent(this, kEventColliderChanged);
	}
	
	this->enabled = false;
}
//---------------------------------------------------------------------------------------------------------
void Collider::setLayers(uint32 layers)
{
	this->layers = layers;
}
//---------------------------------------------------------------------------------------------------------
uint32 Collider::getLayers()
{
	return this->layers;
}
//---------------------------------------------------------------------------------------------------------
void Collider::setLayersToIgnore(uint32 layersToIgnore)
{
	this->layersToIgnore = layersToIgnore;
}
//---------------------------------------------------------------------------------------------------------
uint32 Collider::getLayersToIgnore()
{
	return this->layersToIgnore;
}
//---------------------------------------------------------------------------------------------------------
void Collider::checkCollisions(bool checkCollisions)
{
	this->checkForCollisions = checkCollisions;

	if(checkCollisions)
	{
		Collider::enable(this);
	}
}
//---------------------------------------------------------------------------------------------------------
void Collider::registerCollisions(bool value)
{
	this->registerCollisions = value;
}
//---------------------------------------------------------------------------------------------------------
CollisionResult Collider::collides(Collider collider)
{
	if(isDeleted(this->owner))
	{
		return kNoCollision;
	}

	Collision collision;
	collision.result = kNoCollision;
	collision.collisionInformation.collider = NULL;
	collision.collisionInformation.otherCollider = NULL;

	OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, collider);

	// test if new collision
	if(NULL == otherColliderRegistry)
	{
		// check for new overlap
		CollisionHelper::checkIfOverlap(this, collider, &collision.collisionInformation, 0);

		if(NULL != collision.collisionInformation.collider && 0 != collision.collisionInformation.solutionVector.magnitude)
		{
			// new collision
			collision.result = kEnterCollision;

			if(this->registerCollisions)
			{
				otherColliderRegistry = Collider::registerOtherCollider(this, collider, collision.collisionInformation.solutionVector, false);
			}
		}
	}
	// Impenetrable registered colliding colliders require another test
	// to determine if I'm not colliding against them anymore
	else if(otherColliderRegistry->isImpenetrable && otherColliderRegistry->solutionVector.magnitude)
	{
		CollisionHelper::checkIfOverlap(this, collider, &collision.collisionInformation, __STILL_COLLIDING_CHECK_SIZE_INCREMENT);

		if(collision.collisionInformation.collider == this && collision.collisionInformation.solutionVector.magnitude >= __STILL_COLLIDING_CHECK_SIZE_INCREMENT)
		{
			collision.result = kUpdateCollision;
		}
		else
		{
			collision.result = kExitCollision;
			collision.collisionInformation.collider = this;
			collision.collisionInformation.otherCollider = collider;
		}
	}
	else
	{
		// otherwise make a normal collision test
		CollisionHelper::checkIfOverlap(this, collider, &collision.collisionInformation, 0);

		if(collision.collisionInformation.collider == this && 0 != collision.collisionInformation.solutionVector.magnitude)
		{
			collision.result = kUpdateCollision;
		}
		else
		{
			collision.result = kExitCollision;
			collision.collisionInformation.collider = this;
			collision.collisionInformation.otherCollider = collider;
		}
	}

	switch(collision.result)
	{
		case kEnterCollision:

			Collider::enterCollision(this, &collision);
			break;

		case kUpdateCollision:

			Collider::updateCollision(this, &collision);
			break;

		case kExitCollision:

			Collider::exitCollision(this, &collision);
			break;

		default:
			break;
	}

	return collision.result;
}
//---------------------------------------------------------------------------------------------------------
void Collider::resolveCollision(const CollisionInformation* collisionInformation)
{
	ASSERT(collisionInformation->collider, "Collider::resolveCollision: null collider");
	ASSERT(collisionInformation->otherCollider, "Collider::resolveCollision: null collidingEntities");

	if(isDeleted(this->owner))
	{
		return;
	}

	SolutionVector solutionVector = collisionInformation->solutionVector;

	if(collisionInformation->collider == this && solutionVector.magnitude)
	{
		Collider::displaceOwner(this, Vector3D::scalarProduct(solutionVector.direction, solutionVector.magnitude));

		// need to invalidate solution vectors for other colliding colliders
		//Collider::checkPreviousCollisions(this, collisionInformation->otherCollider);

		if(this->registerCollisions)
		{
			OtherColliderRegistry* otherColliderRegistry = Collider::registerOtherCollider(this, collisionInformation->otherCollider, collisionInformation->solutionVector, true);
			ASSERT(!isDeleted(otherColliderRegistry), "Collider::resolveCollision: dead otherColliderRegistry");
			otherColliderRegistry->frictionCoefficient =  SpatialObject::getFrictionCoefficient(collisionInformation->otherCollider->owner);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
bool Collider::canMoveTowards(Vector3D displacement)
{
	if(!this->otherColliders || NULL == this->otherColliders->head)
	{
		return true;
	}

	bool canMove = true;

	Vector3D normalizedDisplacement = Vector3D::normalize(displacement);

	for(VirtualNode node = this->otherColliders->head; canMove && node; node = node->next)
	{
		OtherColliderRegistry* otherColliderRegistry = (OtherColliderRegistry*)node->data;

		ASSERT(!isDeleted(otherColliderRegistry), "Collider::canMoveTowards: dead otherColliderRegistry");

		if(otherColliderRegistry->isImpenetrable)
		{
			// check if solution is valid
			if(otherColliderRegistry->solutionVector.magnitude)
			{
				fixed_t cosAngle = Vector3D::dotProduct(otherColliderRegistry->solutionVector.direction, normalizedDisplacement);
				canMove &= -(__1I_FIXED - __COLLIDER_ANGLE_TO_PREVENT_DISPLACEMENT) < cosAngle;
			}
		}
	}

	// not colliding anymore
	return canMove;
}
//---------------------------------------------------------------------------------------------------------
void Collider::discardCollisions()
{
	if(NULL != this->otherColliders)
	{
		VirtualNode node = this->otherColliders->head;

		for(; NULL != node; node = node->next)
		{
			OtherColliderRegistry* otherColliderRegistry = (OtherColliderRegistry*)node->data;

			ASSERT(!isDeleted(otherColliderRegistry), "Collider::reset: dead otherColliderRegistry");

			if(!isDeleted(otherColliderRegistry->collider))
			{
				Collider::removeEventListener(otherColliderRegistry->collider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderDeleted);
				Collider::removeEventListener(otherColliderRegistry->collider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderChanged);
			}

			delete otherColliderRegistry;
		}

		delete this->otherColliders;
		this->otherColliders = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
fixed_t Collider::getCollidingFrictionCoefficient()
{
	if(!this->otherColliders)
	{
		return 0;
	}

	fixed_t totalFrictionCoefficient = 0;

	VirtualNode node = this->otherColliders->head;

	for(; NULL != node; node = node->next)
	{
		OtherColliderRegistry* otherColliderRegistry = (OtherColliderRegistry*)node->data;
		ASSERT(!isDeleted(otherColliderRegistry), "Collider::getCollidingFriction: dead otherColliderRegistry");

		ASSERT(otherColliderRegistry->collider, "Collider::getCollidingFriction: null otherCollider");

		if(!isDeleted(otherColliderRegistry->collider->owner))
		{
			totalFrictionCoefficient += otherColliderRegistry->frictionCoefficient;
		}
	}

	return totalFrictionCoefficient;
}
//---------------------------------------------------------------------------------------------------------
void Collider::show()
{
	if(isDeleted(this->wireframe))
	{
		Collider::configureWireframe(this);

		if(!isDeleted(this->wireframe))
		{
			WireframeManager::registerWireframe(WireframeManager::getInstance(), this->wireframe);

			Wireframe::show(this->wireframe);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Collider::hide()
{
	if(!isDeleted(this->wireframe))
	{
		WireframeManager::unregisterWireframe(WireframeManager::getInstance(), this->wireframe);

		delete this->wireframe;
		this->wireframe = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
void Collider::resize(fixed_t sizeDelta __attribute__((unused)))
{}
//---------------------------------------------------------------------------------------------------------
Vector3D Collider::getNormal()
{
	return Vector3D::zero();
}
//---------------------------------------------------------------------------------------------------------
#ifndef __SHIPPING
void Collider::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "SHAPE ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Owner:            ", x, y, NULL);
	Printing::text(Printing::getInstance(), this->owner ? __GET_CLASS_NAME(this->owner) : "No owner", x + 7, y++, NULL);
	Printing::hex(Printing::getInstance(), (int32)this->owner, x + 7, y++, 8, NULL);

	Printing::text(Printing::getInstance(), "Colliding colliders:            ", x, y, NULL);
	Printing::int32(Printing::getInstance(), this->otherColliders ? VirtualList::getCount(this->otherColliders) : 0, x + 21, y++, NULL);
	Printing::text(Printing::getInstance(), "Impenetrable colliders:            ", x, y, NULL);
	Printing::int32(Printing::getInstance(), Collider::getNumberOfImpenetrableOtherColliders(this), x + 21, y++, NULL);
}
#endif
//---------------------------------------------------------------------------------------------------------
bool Collider::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageColliderShow:

			Collider::show(this);
			break;

		case kMessageColliderHide:

			Collider::hide(this);
			break;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Collider::enterCollision(Collision* collision)
{
	if(SpatialObject::enterCollision(this->owner, &collision->collisionInformation))
	{
		OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, collision->collisionInformation.otherCollider);

		if(otherColliderRegistry)
		{
			otherColliderRegistry->frictionCoefficient =  SpatialObject::getFrictionCoefficient(collision->collisionInformation.otherCollider->owner);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Collider::updateCollision(Collision* collision)
{
	SpatialObject::updateCollision(this->owner, &collision->collisionInformation);
}
//---------------------------------------------------------------------------------------------------------
void Collider::exitCollision(Collision* collision)
{
	SpatialObject::exitCollision(this->owner, &collision->collisionInformation);
	Collider::unregisterOtherCollider(this, collision->collisionInformation.otherCollider);
}
//---------------------------------------------------------------------------------------------------------
bool Collider::onOtherColliderChanged(ListenerObject eventFirer)
{
	if(isDeleted(this->owner) || isDeleted(eventFirer))
	{
		return false;
	}

	Collider otherCollider = Collider::safeCast(eventFirer);

	OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, otherCollider);
	NM_ASSERT(!isDeleted(otherColliderRegistry), "Collider::onOtherColliderChanged: dead otherColliderRegistry");

	if(NULL == otherColliderRegistry)
	{
		return false;
	}

	if(Collider::unregisterOtherCollider(this, otherCollider))
	{
		CollisionInformation collisionInformation;
		collisionInformation.collider = this;
		collisionInformation.otherCollider = otherCollider;

		SpatialObject::exitCollision(this->owner, &collisionInformation);
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------
OtherColliderRegistry* Collider::registerOtherCollider(Collider otherCollider, SolutionVector solutionVector, bool isImpenetrable)
{
	if(!this->otherColliders)
	{
		this->otherColliders = new VirtualList();
	}

	bool newEntry = false;
	OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, Collider::safeCast(otherCollider));

	if(NULL == otherColliderRegistry)
	{
		newEntry = true;
		otherColliderRegistry = new OtherColliderRegistry;
	}

	otherColliderRegistry->collider = otherCollider;
	otherColliderRegistry->solutionVector = solutionVector;
	otherColliderRegistry->isImpenetrable = isImpenetrable;
	otherColliderRegistry->frictionCoefficient = 0;

	if(newEntry)
	{
		VirtualList::pushBack(this->otherColliders, otherColliderRegistry);

		Collider::addEventListener(otherCollider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderDeleted);
		Collider::addEventListener(otherCollider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderChanged);
	}

	return otherColliderRegistry;
}
//---------------------------------------------------------------------------------------------------------
bool Collider::unregisterOtherCollider(Collider otherCollider)
{
	ASSERT(!isDeleted(otherCollider), "Collider::removeOtherCollider: dead otherCollider");

	OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, Collider::safeCast(otherCollider));
	NM_ASSERT(!isDeleted(otherColliderRegistry), "Collider::removeOtherCollider: dead otherColliderRegistry");

	if(isDeleted(otherColliderRegistry))
	{
		return false;
	}

	VirtualList::removeData(this->otherColliders, otherColliderRegistry);
	delete otherColliderRegistry;

	if(!isDeleted(otherCollider))
	{
		Collider::removeEventListener(otherCollider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderDeleted);
		Collider::removeEventListener(otherCollider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderChanged);
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------
OtherColliderRegistry* Collider::findOtherColliderRegistry(Collider collider)
{
	ASSERT(collider, "Collider::findOtherColliderRegistry: null collider");

	if(NULL == this->otherColliders || isDeleted(collider))
	{
		return NULL;
	}

	for(VirtualNode node = this->otherColliders->head; NULL != node; node = node->next)
	{
		ASSERT(!isDeleted(node->data), "Collider::findOtherColliderRegistry: deleted registry");

		if(collider == ((OtherColliderRegistry*)node->data)->collider)
		{
			return (OtherColliderRegistry*)node->data;
		}
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------
void Collider::displaceOwner(Vector3D displacement)
{
	// retrieve the colliding spatialObject's position and gap
	Vector3D ownerPosition = * SpatialObject::getPosition(this->owner);

	ownerPosition.x += displacement.x;
	ownerPosition.y += displacement.y;
	ownerPosition.z += displacement.z;

	SpatialObject::setPosition(this->owner, &ownerPosition);
}
//---------------------------------------------------------------------------------------------------------
int32 Collider::getNumberOfImpenetrableOtherColliders()
{
	if(!this->otherColliders)
	{
		return 0;
	}

	int32 count = 0;

	VirtualNode node = this->otherColliders->head;

	for(; NULL != node; node = node->next)
	{
		count += ((OtherColliderRegistry*)node->data)->isImpenetrable ? 1 : 0;
	}

	return count;
}
//---------------------------------------------------------------------------------------------------------
