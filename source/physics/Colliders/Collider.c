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

#include <Collider.h>

#include <CollisionHelper.h>
#include <CollisionManager.h>
#include <Printing.h>
#include <SpatialObject.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __STILL_COLLIDING_CHECK_SIZE_INCREMENT 		__PIXELS_TO_METERS(1)


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param owner
 */
void Collider::constructor(SpatialObject owner, const ColliderSpec* shapeSpec)
{
	// construct base object
	Base::constructor();

	// set the owner
	this->owner = owner;

	// not setup yet
	this->destroyMe = false;
	this->ready = false;
	this->enabled = true;

	this->wireframe = NULL;

	// set flag
	this->checkForCollisions = false;
	this->layers = shapeSpec->layers;
	this->layersToIgnore = shapeSpec->layersToIgnore;
	this->otherColliders = NULL;
	this->registerCollisions = shapeSpec->checkForCollisions;

	this->position = Vector3D::zero();
}

/**
 * Class destructor
 */
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
				Collider::removeEventListener(otherColliderRegistry->collider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderDestroyed, kEventColliderDeleted);
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

/**
 * Reset
 */
void Collider::reset()
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
				Collider::removeEventListener(otherColliderRegistry->collider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderDestroyed, kEventColliderDeleted);
				Collider::removeEventListener(otherColliderRegistry->collider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderChanged);
			}

			delete otherColliderRegistry;
		}

		delete this->otherColliders;
		this->otherColliders = NULL;
	}
}

/**
 * Setup
 *
 * @param layers				uint32
 * @param layersToIgnore		uint32
 */
void Collider::setup(uint32 layers, uint32 layersToIgnore)
{
	this->layers = layers;
	this->layersToIgnore = layersToIgnore;

	if(NULL != this->events)
	{
		Collider::fireEvent(this, kEventColliderChanged);
		NM_ASSERT(!isDeleted(this), "Collider::setup: deleted this during kEventColliderChanged");
	}
}

/**
 * Position
 *
 * @return						Vector3D
 */
Vector3D Collider::getNormal()
{
	return Vector3D::zero();
}

/**
 * Position
 *
 * @param position				Vector3d*
 * @param rotation				Rotation*
 * @param scale					Scale*
 * @param size					Size*
 */
void Collider::transform(const Vector3D* position __attribute__ ((unused)), const Rotation* rotation __attribute__ ((unused)), const Scale* scale __attribute__ ((unused)), const Size* size __attribute__ ((unused)))
{
	if(this->enabled && NULL != this->events)
	{
		Collider::fireEvent(this, kEventColliderChanged);
		NM_ASSERT(!isDeleted(this), "Collider::transformm: deleted this during kEventColliderChanged");
	}

	this->ready = true;

	// TODO: must update the rightbox
	this->position = *position;
}

/**
 * Process enter collision event
 *
 * @param collisionData			Collision data
 */
void Collider::enterCollision(CollisionData* collisionData)
{
	if(SpatialObject::enterCollision(this->owner, &collisionData->collisionInformation))
	{
		OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, collisionData->collisionInformation.otherCollider);

		if(otherColliderRegistry)
		{
			otherColliderRegistry->frictionCoefficient =  SpatialObject::getFrictionCoefficient(collisionData->collisionInformation.otherCollider->owner);
		}
	}
}

/**
 * Process update collision event
 *
 * @param collisionData			Collision data
 */
void Collider::updateCollision(CollisionData* collisionData)
{
	SpatialObject::updateCollision(this->owner, &collisionData->collisionInformation);
}
/**
 * Process exit collision event
 *
 * @param collisionData			Collision data
 */
void Collider::exitCollision(CollisionData* collisionData)
{
	SpatialObject::exitCollision(this->owner, collisionData->collisionInformation.collider, collisionData->shapeNotCollidingAnymore, collisionData->isImpenetrableOtherCollider);
	Collider::unregisterOtherCollider(this, collisionData->shapeNotCollidingAnymore);
}

/**
 * Check if collides with other collider
 *
 * @param collider					collider to check for overlapping
 *
  * @return						CollisionData
 */
// check if two rectangles overlap
CollisionResult Collider::collides(Collider collider)
{
	if(isDeleted(this->owner))
	{
		return kNoCollision;
	}

	CollisionData collisionData;
	collisionData.result = kNoCollision;
	collisionData.collisionInformation.collider = NULL;
	collisionData.collisionInformation.otherCollider = NULL;
	collisionData.shapeNotCollidingAnymore = NULL;
	collisionData.isImpenetrableOtherCollider = false;
	
	/*
	{
		// result
		kNoCollision,

		// collision information
		{
			// collider
			NULL,
			// colliding collider
			NULL,
			// solution vector
			{
				// direction
				{0, 0, 0},
				// magnitude
				0
			}
		},

		// out-of-collision collider
		NULL,

		// is impenetrable colliding collider
		false,
	};*/

	OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, collider);

	// test if new collision
	if(NULL == otherColliderRegistry)
	{
		// check for new overlap
		CollisionHelper::checkIfOverlap(this, collider, &collisionData.collisionInformation);

		if(NULL != collisionData.collisionInformation.collider && 0 != collisionData.collisionInformation.solutionVector.magnitude)
		{
			// new collision
			collisionData.result = kEnterCollision;

			if(this->registerCollisions)
			{
				otherColliderRegistry = Collider::registerOtherCollider(this, collider, collisionData.collisionInformation.solutionVector, false);
			}
		}
	}
	// impenetrable registered colliding shapes require a another test
	// to determine if I'm not colliding against them anymore
	else if(otherColliderRegistry->isImpenetrable && otherColliderRegistry->solutionVector.magnitude)
	{
		Collider::testForCollision(this, collider, Vector3D::zero(), __STILL_COLLIDING_CHECK_SIZE_INCREMENT, &collisionData.collisionInformation);

		if(collisionData.collisionInformation.collider == this && collisionData.collisionInformation.solutionVector.magnitude >= __STILL_COLLIDING_CHECK_SIZE_INCREMENT)
		{
			collisionData.result = kUpdateCollision;
			collisionData.isImpenetrableOtherCollider = true;
		}
		else
		{
			collisionData.collisionInformation.collider = this;
			collisionData.result = kExitCollision;
			collisionData.isImpenetrableOtherCollider = true;
			collisionData.shapeNotCollidingAnymore = collider;
		}
	}
	else
	{
		// otherwise make a normal collision test
		CollisionHelper::checkIfOverlap(this, collider, &collisionData.collisionInformation);

		if(collisionData.collisionInformation.collider == this && 0 != collisionData.collisionInformation.solutionVector.magnitude)
		{
			collisionData.result = kUpdateCollision;
		}
		else
		{
			collisionData.collisionInformation.collider = this;
			collisionData.result = kExitCollision;
			collisionData.isImpenetrableOtherCollider = otherColliderRegistry->isImpenetrable;
			collisionData.shapeNotCollidingAnymore = collider;
		}
	}

	switch(collisionData.result)
	{
		case kEnterCollision:

			Collider::enterCollision(this, &collisionData);
			break;

		case kUpdateCollision:

			Collider::updateCollision(this, &collisionData);
			break;

		case kExitCollision:

			Collider::exitCollision(this, &collisionData);
			break;

		default:
			break;
	}

	return collisionData.result;
}

/**
 * Check if there is a collision in the magnitude
 *
 * @param displacement		collider displacement
 */
bool Collider::canMoveTowards(Vector3D displacement, fixed_t sizeIncrement __attribute__ ((unused)))
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

/*
void Collider::checkPreviousCollisions(Collider otherCollider)
{
	if(!this->otherColliders)
	{
		return;
	}

	VirtualNode node = this->otherColliders->head;

	for(; NULL != node; node = node->next)
	{
		OtherColliderRegistry* otherColliderRegistry = (OtherColliderRegistry*)node->data;

		ASSERT(!isDeleted(otherColliderRegistry), "Collider::invalidateSolutionVectors: dead otherColliderRegistry");

		if(otherColliderRegistry->isImpenetrable && otherColliderRegistry->collider != otherCollider)
		{
			CollisionInformation collisionInformation =  Collider::testForCollision(this, otherColliderRegistry->collider, Vector3D::zero(), __STILL_COLLIDING_CHECK_SIZE_INCREMENT);

			if(collisionInformation.collider == this && 0 < collisionInformation.solutionVector.magnitude)
			{
				if(collisionInformation.solutionVector.magnitude > __STILL_COLLIDING_CHECK_SIZE_INCREMENT)
				{
					if(Collider::canMoveTowards(this, Vector3D::scalarProduct(otherColliderRegistry->solutionVector.direction, collisionInformation.solutionVector.magnitude), 0))
					{
						Collider::displaceOwner(this, Vector3D::scalarProduct(collisionInformation.solutionVector.direction, collisionInformation.solutionVector.magnitude));
					}
				}
				else if(collisionInformation.solutionVector.magnitude < otherColliderRegistry->solutionVector.magnitude)
				{
					// since I'm not close to that collider anymore, we can discard it
					otherColliderRegistry->solutionVector.magnitude = 0;
				}
			}
			else
			{
				// since I'm not close to that collider anymore, we can discard it
				otherColliderRegistry->solutionVector.magnitude = 0;
			}
		}
	}
}
*/

/**
 * Retrieve the position
 *
 * @return Vector3D			Collider's position
 */
Vector3D Collider::getPosition()
{
	return this->position;
}

/**
 * Set position
 *
 * @param position				Vector3d*
 */
void Collider::setPosition(const Vector3D* position)
{
	// TODO: must update the rightbox
	this->position = *position;
}

/**
 * Displace owner
 *
 * @param displacement		Displacement to apply to owner
 */
void Collider::displaceOwner(Vector3D displacement)
{
	// retrieve the colliding spatialObject's position and gap
	Vector3D ownerPosition = * SpatialObject::getPosition(this->owner);

	ownerPosition.x += displacement.x;
	ownerPosition.y += displacement.y;
	ownerPosition.z += displacement.z;

	SpatialObject::setPosition(this->owner, &ownerPosition);
}

/**
 * Solve the collision by moving owner
 */
void Collider::resolveCollision(const CollisionInformation* collisionInformation, bool registerOtherCollider)
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

		// need to invalidate solution vectors for other colliding shapes
		//Collider::checkPreviousCollisions(this, collisionInformation->otherCollider);

		if(registerOtherCollider)
		{
			OtherColliderRegistry* otherColliderRegistry = Collider::registerOtherCollider(this, collisionInformation->otherCollider, collisionInformation->solutionVector, true);
			ASSERT(!isDeleted(otherColliderRegistry), "Collider::resolveCollision: dead otherColliderRegistry");
			otherColliderRegistry->frictionCoefficient =  SpatialObject::getFrictionCoefficient(collisionInformation->otherCollider->owner);
		}
	}
}

/**
 * Retrieve owner
 *
 * @return		Owning SpatialObject
 */
SpatialObject Collider::getOwner()
{
	return this->owner;
}

/**
 * Is enabled?
 *
 * @return		Enabled status
 */
bool Collider::isEnabled()
{
	return this->enabled;
}

/**
 * Make this collider to test collision against other shapes
 *
 * @param activate
 */
void Collider::activeCollisionChecks(bool activate)
{
	Collider::setCheckForCollisions(this, activate);

	if(activate && !this->enabled)
	{
		Collider::enable(this, activate);
	}
}

/**
 * Enable / disable
 *
 * @param enable
 */
void Collider::enable(bool enable)
{
	if(this->enabled != enable)
	{
		if(!enable)
		{
			Collider::fireEvent(this, kEventColliderChanged);
		}
	}
	
	this->enabled = enable;
}

/**
 * Has been configured?
 *
 * @return		Configured status
 */
bool Collider::isReady()
{
	return this->ready;
}

/**
 * Set configured flag
 *
 * @param ready
 */
void Collider::setReady(bool ready)
{
	this->ready = ready;
}

/**
 * Set flag
 *
 * @param checkForCollisions
 */
void Collider::setCheckForCollisions(bool checkForCollisions)
{
	this->checkForCollisions = checkForCollisions;
}

/**
 * Get flag
 *
 * @return		Collision check status
 */
bool Collider::checkForCollisions()
{
	return this->checkForCollisions;
}

/**
 * Register colliding collider from the lists
 *
 * @private
 * @param otherCollider	Colliding collider to register
 */
OtherColliderRegistry* Collider::registerOtherCollider(Collider otherCollider, SolutionVector solutionVector, bool isImpenetrable)
{
	if(!this->otherColliders)
	{
		this->otherColliders = new VirtualList();
	}

	bool newEntry = false;
	OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, Collider::safeCast(otherCollider));

	if(!otherColliderRegistry)
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

		Collider::addEventListener(otherCollider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderDestroyed, kEventColliderDeleted);
		Collider::addEventListener(otherCollider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderChanged);
	}

	return otherColliderRegistry;
}

/**
 * Remove colliding collider from the lists
 *
 * @private
 * @param otherCollider	Colliding collider to remove
 */
bool Collider::unregisterOtherCollider(Collider otherCollider)
{
	ASSERT(!isDeleted(otherCollider), "Collider::removeOtherCollider: dead otherCollider");

	OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, Collider::safeCast(otherCollider));

	if(!otherColliderRegistry)
	{
		return false;
	}

	ASSERT(!isDeleted(otherColliderRegistry), "Collider::removeOtherCollider: dead otherColliderRegistry");
	VirtualList::removeElement(this->otherColliders, otherColliderRegistry);
	delete otherColliderRegistry;

	if(!isDeleted(otherCollider))
	{
		Collider::removeEventListener(otherCollider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderDestroyed, kEventColliderDeleted);
		Collider::removeEventListener(otherCollider, ListenerObject::safeCast(this), (EventListener)Collider::onOtherColliderChanged, kEventColliderChanged);
	}

	return true;
}

/**
 * Collider destroying listener
 *
 * @private
 * @param eventFirer		Destroyed collider
 */
void Collider::onOtherColliderDestroyed(ListenerObject eventFirer)
{
	if(isDeleted(this->owner))
	{
		return;
	}

	Collider shapeNotCollidingAnymore = Collider::safeCast(eventFirer);

	OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, shapeNotCollidingAnymore);
	ASSERT(otherColliderRegistry, "Collider::onOtherColliderDestroyed: onOtherColliderDestroyed not found");

	if(!otherColliderRegistry)
	{
		return;
	}

	bool isImpenetrable = otherColliderRegistry->isImpenetrable;

	if(Collider::unregisterOtherCollider(this, shapeNotCollidingAnymore))
	{
		SpatialObject::otherColliderOwnerDestroyed(this->owner, this, shapeNotCollidingAnymore, isImpenetrable);
	}
}

/**
 * Collider changed listener
 *
 * @private
 * @param eventFirer		Changed collider
 */
void Collider::onOtherColliderChanged(ListenerObject eventFirer)
{
	if(isDeleted(this->owner))
	{
		return;
	}

	Collider shapeNotCollidingAnymore = Collider::safeCast(eventFirer);

	Collider::registerOtherCollider(this, shapeNotCollidingAnymore, (SolutionVector){{0, 0, 0}, 0}, true);

	OtherColliderRegistry* otherColliderRegistry = Collider::findOtherColliderRegistry(this, shapeNotCollidingAnymore);
	ASSERT(!isDeleted(otherColliderRegistry), "Collider::removeOtherCollider: dead otherColliderRegistry");

	bool isImpenetrable = otherColliderRegistry->isImpenetrable;

	if(Collider::unregisterOtherCollider(this, shapeNotCollidingAnymore))
	{
		SpatialObject::exitCollision(this->owner, this, shapeNotCollidingAnymore, isImpenetrable);
	}
}

/**
 * Get OtherColliderRegistry
 *
 * @private
 * @param collider	Collider to find
 * @return		OtherColliderRegistry*
 */
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

/**
 * Get total friction of colliding shapes
 *
 * @return				The sum of friction coefficients
 */
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

uint32 Collider::getLayers()
{
	return this->layers;
}

void Collider::setLayers(uint32 layers)
{
	this->layers = layers;
}

uint32 Collider::getLayersToIgnore()
{
	return this->layersToIgnore;
}

void Collider::setLayersToIgnore(uint32 layersToIgnore)
{
	this->layersToIgnore = layersToIgnore;
}

void Collider::registerCollisions(bool value)
{
	this->registerCollisions = value;
}

// show me
void Collider::show()
{
	if(isDeleted(this->wireframe))
	{
		Collider::configureWireframe(this);

		if(!isDeleted(this->wireframe))
		{
			WireframeManager::registerWireframe(WireframeManager::getInstance(), this->wireframe);

			Wireframe::setup(this->wireframe, &this->position, NULL, NULL, false);
			Wireframe::show(this->wireframe);
		}
	}
}

// hide polyhedron
void Collider::hide()
{
	if(!isDeleted(this->wireframe))
	{
		WireframeManager::unregisterWireframe(WireframeManager::getInstance(), this->wireframe);

		// delete the Polyhedron
		delete this->wireframe;
		this->wireframe = NULL;
	}
}

void Collider::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "SHAPE ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Owner:            ", x, y, NULL);
	Printing::text(Printing::getInstance(), this->owner ? __GET_CLASS_NAME(this->owner) : "No owner", x + 7, y++, NULL);
	Printing::hex(Printing::getInstance(), (int32)this->owner, x + 7, y++, 8, NULL);

	Printing::text(Printing::getInstance(), "Colliding shapes:            ", x, y, NULL);
	Printing::int32(Printing::getInstance(), this->otherColliders ? VirtualList::getSize(this->otherColliders) : 0, x + 21, y++, NULL);
	Printing::text(Printing::getInstance(), "Impenetrable shapes:            ", x, y, NULL);
	Printing::int32(Printing::getInstance(), Collider::getNumberOfImpenetrableOtherColliders(this), x + 21, y++, NULL);
}
