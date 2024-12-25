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

#include <Ball.h>
#include <Body.h>
#include <ColliderManager.h>
#include <Collider.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "SolidParticle.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Collider;
friend class VirtualList;
friend class VirtualNode;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void SolidParticle::constructor(const SolidParticleSpec* solidParticleSpec)
{
	Base::constructor(&solidParticleSpec->physicalParticleSpec);

	this->solidParticleSpec = solidParticleSpec;

	this->colliderSpec = new ColliderSpec;

	*this->colliderSpec = (ColliderSpec)
	{
		// Component
		{
			// Allocator
			__TYPE(Ball),

			// Component type
			kColliderComponent
		},

		// Displacement
		{__METERS_TO_PIXELS(solidParticleSpec->radius), __METERS_TO_PIXELS(solidParticleSpec->radius), __METERS_TO_PIXELS(solidParticleSpec->radius)},

		// Displacement (x, y, z, p)
		{0, 0, 0, 0},

		// Rotation (x, y, z)
		{0, 0, 0},

		// Scale (x, y, z)
		{1, 1, 1},

		// check for collisions against other colliders
		true,

		/// Layers in which I live
		this->solidParticleSpec->layers,

		/// Layers to ignore when checking for collisions
		this->solidParticleSpec->layersToIgnore,
	};

	// register a collider for collision detection
	this->collider = ColliderManager::createCollider(VUEngine::getColliderManager(VUEngine::getInstance()), SpatialObject::safeCast(this), this->colliderSpec);
	Collider::registerCollisions(this->collider, false);

	// has to set bounciness and friction myself since Particle ignores collisions
	Body::setBounciness(this->body, this->solidParticleSpec->bounciness);
	Body::setFrictionCoefficient(this->body, this->solidParticleSpec->frictionCoefficient);
}
//---------------------------------------------------------------------------------------------------------
void SolidParticle::destructor()
{
	// unregister the collider for collision detection
	ColliderManager::destroyCollider(VUEngine::getColliderManager(VUEngine::getInstance()), this->collider);

	this->collider = NULL;

	if(!isDeleted(this->colliderSpec))
	{
		delete this->colliderSpec;
		this->colliderSpec = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
bool SolidParticle::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageBodyStartedMoving:

			Collider::checkCollisions(this->collider, true);
			return true;
			break;

		case kMessageBodyStopped:

			if(this->solidParticleSpec->disableCollisionOnStop && !Body::getMovementOnAllAxis(this->body))
			{
				Collider::checkCollisions(this->collider, false);
			}
			break;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
fixed_t SolidParticle::getRadius()
{
	return this->solidParticleSpec->radius;
}
//---------------------------------------------------------------------------------------------------------
bool SolidParticle::isSubjectToGravity(Vector3D gravity)
{
	ASSERT(this->collider, "Particle::isSubjectToGravity: null collider");

	fixed_t collisionCheckDistance = __I_TO_FIXED(1);

	Vector3D displacement =
	{
		gravity.x ? 0 < gravity.x ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.y ? 0 < gravity.y ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.z ? 0 < gravity.z ? collisionCheckDistance : -collisionCheckDistance : 0
	};

	return Collider::canMoveTowards(this->collider, displacement);
}
//---------------------------------------------------------------------------------------------------------
uint32 SolidParticle::getInGameType()
{
	return this->solidParticleSpec->inGameType;
}
//---------------------------------------------------------------------------------------------------------
bool SolidParticle::collisionStarts(const CollisionInformation* collisionInformation)
{
	ASSERT(this->body, "SolidParticle::resolveCollision: null body");
	ASSERT(collisionInformation->otherCollider, "SolidParticle::resolveCollision: otherColliders");

	ASSERT(collisionInformation->otherCollider, "SolidParticle::collisionStarts: otherColliders");

	bool returnValue = false;

	if(collisionInformation->collider && collisionInformation->otherCollider)
	{
		if(collisionInformation->solutionVector.magnitude)
		{
			Collider::resolveCollision(collisionInformation->collider, collisionInformation);

			SpatialObject owner = Collider::getOwner(collisionInformation->otherCollider);
			fixed_t frictionCoefficient =  SpatialObject::getFrictionCoefficient(owner);
			fixed_t bounciness =  SpatialObject::getBounciness(owner);

			Body::bounce(this->body, ListenerObject::safeCast(collisionInformation->otherCollider), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);
			returnValue = true;
		}

		if(NULL != this->solidParticleSpec->onCollisionAnimation)
		{
			VisualComponent::play(this->visualComponent, ((ParticleSpec*)this->solidParticleSpec)->animationFunctions, this->solidParticleSpec->onCollisionAnimation, ListenerObject::safeCast(this));
		}
	}

	return returnValue;
}
//---------------------------------------------------------------------------------------------------------
void SolidParticle::collisionEnds(const CollisionInformation* collisionInformation)
{
	ASSERT(this->body, "SolidParticle::collisionEnds: null this");

	if(isDeleted(this->body))
	{
		return;
	}

	if(NULL == collisionInformation || isDeleted(collisionInformation->collider))
	{
		return;
	}

	Body::clearNormal(this->body, ListenerObject::safeCast(collisionInformation->otherCollider));
	Body::setSurroundingFrictionCoefficient(this->body, Collider::getCollidingFrictionCoefficient(collisionInformation->collider));
}
//---------------------------------------------------------------------------------------------------------
void SolidParticle::reset()
{
	Base::reset(this);
	Collider::discardCollisions(this->collider);
}
//---------------------------------------------------------------------------------------------------------
