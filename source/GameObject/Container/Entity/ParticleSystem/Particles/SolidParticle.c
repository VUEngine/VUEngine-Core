/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Ball.h>
#include <Body.h>
#include <ColliderManager.h>
#include <Collider.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "SolidParticle.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Collider;
friend class VirtualList;
friend class VirtualNode;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void SolidParticle::constructor(const SolidParticleSpec* solidParticleSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(&solidParticleSpec->physicalParticleSpec);

	this->solidParticleSpec = solidParticleSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void SolidParticle::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SolidParticle::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageBodyStartedMoving:

			SolidParticle::checkCollisions(this, true);
			return true;
			break;

		case kMessageBodyStopped:

			if(!Body::getMovementOnAllAxis(this->body))
			{
				SolidParticle::checkCollisions(this, false);
			}
			break;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SolidParticle::isSubjectToGravity(Vector3D gravity)
{
	Collider collider = Collider::safeCast(SolidParticle::getComponentAtIndex(this, kColliderComponent, 0));

	fixed_t collisionCheckDistance = __I_TO_FIXED(1);

	Vector3D displacement =
	{
		gravity.x ? 0 < gravity.x ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.y ? 0 < gravity.y ? collisionCheckDistance : -collisionCheckDistance : 0,
		gravity.z ? 0 < gravity.z ? collisionCheckDistance : -collisionCheckDistance : 0
	};

	if(NULL == collider)
	{
		return Base::isSubjectToGravity(this, gravity);
	}

	return Collider::canMoveTowards(collider, displacement);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 SolidParticle::getInGameType()
{
	return this->solidParticleSpec->inGameType;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

			GameObject owner = Collider::getOwner(collisionInformation->otherCollider);
			fixed_t frictionCoefficient =  GameObject::getFrictionCoefficient(owner);
			fixed_t bounciness =  GameObject::getBounciness(owner);

			Body::bounce
			(
				this->body, 
				ListenerObject::safeCast(collisionInformation->otherCollider), 
				collisionInformation->solutionVector.direction, 
				frictionCoefficient, 
				bounciness
			);

			returnValue = true;
		}

		if(NULL != this->solidParticleSpec->onCollisionAnimation)
		{
			SolidParticle::playAnimation(this, ((ParticleSpec*)this->solidParticleSpec)->animationFunctions, this->solidParticleSpec->onCollisionAnimation);
		}
	}

	return returnValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

