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

#include <Body.h>
#include <Camera.h>
#include <PhysicalWorld.h>
#include <Collider.h>
#include <State.h>
#include <StateMachine.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "Actor.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualList;
friend class VirtualNode;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Actor::constructor(const ActorSpec* actorSpec, int16 internalId, const char* const name)
{
	// construct base object
	Base::constructor((AnimatedEntitySpec*)&actorSpec->animatedEntitySpec, internalId, name);

	// construct the game state machine
	this->stateMachine = NULL;

	this->body = NULL;

	// create body
	if(actorSpec->createBody)
	{
		Actor::createBody(this, actorSpec->animatedEntitySpec.entitySpec.physicalProperties, actorSpec->axisSubjectToGravity);
	}
}

// class's destructor
void Actor::destructor()
{
	if(!isDeleted(this->body))
	{
		// remove a body
		PhysicalWorld::destroyBody(VUEngine::getPhysicalWorld(_vuEngine), this->body);
		this->body = NULL;
	}

	// destroy state machine
	if(!isDeleted(this->stateMachine))
	{
		delete this->stateMachine;
		this->stateMachine = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Actor::createBody(const PhysicalProperties* physicalProperties, uint16 axisSubjectToGravity)
{
	if(!isDeleted(this->body))
	{
		NM_ASSERT(false, "Actor::createBody: body already created");
		return;
	}

	if(NULL != physicalProperties)
	{
		this->body = PhysicalWorld::createBody(VUEngine::getPhysicalWorld(_vuEngine), SpatialObject::safeCast(this), physicalProperties, axisSubjectToGravity);
	}
	else
	{
		PhysicalProperties defaultActorPhysicalProperties = {__I_TO_FIXED(1), 0, 0, Vector3D::zero(), 0};
		this->body = PhysicalWorld::createBody(VUEngine::getPhysicalWorld(_vuEngine), SpatialObject::safeCast(this), &defaultActorPhysicalProperties, axisSubjectToGravity);
	}

	Body::setPosition(this->body, &this->transformation.position, SpatialObject::safeCast(this));
}

void Actor::initializeStateMachine(State state)
{
	if(isDeleted(this->stateMachine))
	{
		this->stateMachine = new StateMachine(this);
	}

	StateMachine::swapState(this->stateMachine, state);

	this->update = true;
}

void Actor::destroyComponents()
{
	Base::destroyComponents(this);

	// destroy body to prevent any more physical interactions
	if(!isDeleted(this->body))
	{
		// remove a body
		PhysicalWorld::destroyBody(VUEngine::getPhysicalWorld(_vuEngine), this->body);
		this->body = NULL;
	}
}

// set position
void Actor::setPosition(const Vector3D* position)
{
	Base::setPosition(this, position);

	if(!isDeleted(this->body) && Body::getPosition(this->body) != position)
	{
		Body::setPosition(this->body, &this->transformation.position, SpatialObject::safeCast(this));
	}
}

//set class's local position
void Actor::setLocalPosition(const Vector3D* position)
{
	Vector3D displacement = this->localTransformation.position;

	Base::setLocalPosition(this, position);

	displacement.x -= this->localTransformation.position.x;
	displacement.y -= this->localTransformation.position.y;
	displacement.z -= this->localTransformation.position.z;

	this->transformation.position.x -= displacement.x;
	this->transformation.position.y -= displacement.y;
	this->transformation.position.z -= displacement.z;

	if(NULL != this->body)
	{
		Body::setPosition(this->body, &this->transformation.position, SpatialObject::safeCast(this));
	}

	this->transformation.invalid |= (displacement.x ? __X_AXIS: 0) | (displacement.y ? __Y_AXIS: 0) | (displacement.y ? __Z_AXIS: 0);
}

void Actor::setDirection(const Vector3D* direction)
{
	if(NULL == direction)
	{
		return;
	}

	if((uint16)__LOCK_AXIS == ((ActorSpec*)this->entitySpec)->axisForSynchronizationWithBody)
	{
		return;
	}
		
	if(__NO_AXIS == ((ActorSpec*)this->entitySpec)->axisForSynchronizationWithBody)
	{
		NormalizedDirection normalizedDirection = Actor::getNormalizedDirection(this);

		if(0 > direction->x)
		{
			normalizedDirection.x = __LEFT;
		}
		else if(0 < direction->x)
		{
			normalizedDirection.x = __RIGHT;
		}

		if(0 > direction->y)
		{
			normalizedDirection.y = __UP;
		}
		else if(0 < direction->y)
		{
			normalizedDirection.y = __DOWN;
		}

		if(0 > direction->z)
		{
			normalizedDirection.z = __NEAR;
		}
		else if(0 < direction->z)
		{
			normalizedDirection.z = __FAR;
		}

		Actor::setNormalizedDirection(this, normalizedDirection);
	}
	else
	{
		Rotation localRotation = Actor::getRotationFromDirection(this, direction, ((ActorSpec*)this->entitySpec)->axisForSynchronizationWithBody);
		Base::setLocalRotation(this, &localRotation);
	}
}

// execute character's logic
void Actor::update()
{
	// call base
	Base::update(this);

	if(!isDeleted(this->stateMachine))
	{
		StateMachine::update(this->stateMachine);
	}

//	Body::print(this->body, 1, 0);
//	Collider::print(VirtualList::front(this->colliders), 1, 20);
//	Printing::resetCoordinates(Printing::getInstance());
}

// whether changed direction in the last cycle or not
bool Actor::hasChangedDirection(uint16 axis, const Rotation* previousRotation)
{
	if(NULL == previousRotation)
	{
		return false;
	}

	switch(axis)
	{
		case __X_AXIS:

			return this->localTransformation.rotation.x != previousRotation->x;
			break;

		case __Y_AXIS:

			return this->localTransformation.rotation.y != previousRotation->y;
			break;

		case __Z_AXIS:

			return this->localTransformation.rotation.z != previousRotation->z;
			break;
	}

	return false;
}

// change direction over axis
void Actor::changeDirectionOnAxis(uint16 axis)
{
	if(!isDeleted(this->body))
	{
		Actor::setDirection(this, Body::getDirection(this->body));
	}
	else
	{
		NormalizedDirection normalizedDirection = Actor::getNormalizedDirection(this);

		if((__X_AXIS & axis))
		{
			if(__RIGHT == normalizedDirection.x)
			{
				normalizedDirection.x = __LEFT;
			}
			else
			{
				normalizedDirection.x = __RIGHT;
			}
		}

		if((__Y_AXIS & axis))
		{
			if(__UP == normalizedDirection.y)
			{
				normalizedDirection.y = __DOWN;
			}
			else
			{
				normalizedDirection.y = __UP;
			}
		}

		if((__Z_AXIS & axis))
		{
			if(__NEAR == normalizedDirection.z)
			{
				normalizedDirection.x = __FAR;
			}
			else
			{
				normalizedDirection.x = __NEAR;
			}
		}

		Actor::setNormalizedDirection(this, normalizedDirection);
	}
}

// check if gravity must apply to this actor
bool Actor::isSubjectToGravity(Vector3D gravity)
{
	return Actor::canMoveTowards(this, gravity);
}

// check if gravity must apply to this actor
bool Actor::canMoveTowards(Vector3D direction)
{
	fixed_t collisionCheckDistance = __I_TO_FIXED(1);

	Vector3D displacement =
	{
		direction.x ? 0 < direction.x ? collisionCheckDistance : -collisionCheckDistance : 0,
		direction.y ? 0 < direction.y ? collisionCheckDistance : -collisionCheckDistance : 0,
		direction.z ? 0 < direction.z ? collisionCheckDistance : -collisionCheckDistance : 0
	};

	bool canMove = true;

	if(this->colliders)
	{
		VirtualNode node = this->colliders->head;

		for(; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);
			canMove &= Collider::canMoveTowards(collider, displacement, 0);
		}
	}

	return canMove;
}

fixed_t Actor::getBouncinessOnCollision(SpatialObject collidingObject, const Vector3D* collidingObjectNormal __attribute__ ((unused)))
{
	return  SpatialObject::getBounciness(collidingObject);
}

fixed_t Actor::getSurroundingFrictionCoefficient()
{
	fixed_t totalFrictionCoefficient = 0;

	if(!isDeleted(this->colliders))
	{
		VirtualNode node = this->colliders->head;

		for(; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			totalFrictionCoefficient += Collider::getCollidingFrictionCoefficient(collider);
		}
	}

	return totalFrictionCoefficient;
}

fixed_t Actor::getFrictionOnCollision(SpatialObject collidingObject __attribute__ ((unused)), const Vector3D* collidingObjectNormal __attribute__ ((unused)))
{
	return  Actor::getSurroundingFrictionCoefficient(this);
}

bool Actor::registerOtherColliders()
{
	return true;
}

bool Actor::enterCollision(const CollisionInformation* collisionInformation)
{
	ASSERT(collisionInformation->otherCollider, "Actor::enterCollision: otherColliders");

	if(NULL == this->body)
	{
		return false;
	}

	bool returnValue = false;

	if(collisionInformation->collider && collisionInformation->otherCollider)
	{
		if(collisionInformation->solutionVector.magnitude)
		{
			Collider::resolveCollision(collisionInformation->collider, collisionInformation, Actor::registerOtherColliders(this));

			SpatialObject collidingObject = Collider::getOwner(collisionInformation->otherCollider);

			fixed_t bounciness = Actor::getBouncinessOnCollision(this, collidingObject, &collisionInformation->solutionVector.direction);
			fixed_t frictionCoefficient = Actor::getFrictionOnCollision(this, collidingObject, &collisionInformation->solutionVector.direction);

			if(Actor::mustBounce(this))
			{
				Body::bounce(this->body, ListenerObject::safeCast(collisionInformation->otherCollider), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);

				Actor::fireEvent(this, kEventActorBounced);
				NM_ASSERT(!isDeleted(this), "Actor::enterCollision: deleted this during kEventActorBounced");
			}
			else
			{
				uint16 axis = __NO_AXIS;
				axis |= collisionInformation->solutionVector.direction.x ? __X_AXIS : __NO_AXIS;
				axis |= collisionInformation->solutionVector.direction.y ? __Y_AXIS : __NO_AXIS;
				axis |= collisionInformation->solutionVector.direction.z ? __Z_AXIS : __NO_AXIS;
				Actor::stopMovement(this, axis);
			}

			returnValue = true;
		}
	}

	return returnValue;
}

// process a telegram
bool Actor::handleMessage(Telegram telegram)
{
	if(!this->stateMachine || !StateMachine::handleMessage(this->stateMachine, telegram))
	{
		// retrieve message
		int32 message = Telegram::getMessage(telegram);

		if(this->body)
		{
			switch(message)
			{
				case kMessageBodyStartedMoving:

					if(this->collisionsEnabled && NULL != this->colliders)
					{
						Actor::checkCollisions(this, true);
						return true;
					}

					break;

				case kMessageBodyStopped:

					if(__NO_AXIS == Body::getMovementOnAllAxis(this->body) && NULL != this->colliders)
					{
						Actor::checkCollisions(this, false);
					}

					break;

				case kMessageBodyChangedDirection:

					Actor::changeDirectionOnAxis(this, *(int32*)Telegram::getExtraInfo(telegram));
					return true;
					break;
			}
		}
	}

	return false;
}

// retrieve state machine
StateMachine Actor::getStateMachine()
{
	return this->stateMachine;
}

// stop movement completely
void Actor::stopAllMovement()
{
	Actor::stopMovement(this, __ALL_AXIS);
}

// stop movement completely
void Actor::stopMovement(uint16 axis)
{
	if(!isDeleted(this->body))
	{
		Body::stopMovement(this->body, axis);
	}
}

bool Actor::setVelocity(const Vector3D* velocity, bool checkIfCanMove)
{
	ASSERT(this->body, "Actor::applyForce: null body");

	if(!this->body)
	{
		return false;
	}

	if(checkIfCanMove)
	{
		if(!Actor::canMoveTowards(this, *velocity))
		{
			return false;
		}
	}

	Body::moveUniformly(this->body, velocity);

	return true;
}

bool Actor::applyForce(const Vector3D* force, bool checkIfCanMove)
{
	ASSERT(this->body, "Actor::applyForce: null body");

	if(!this->body)
	{
		return false;
	}

	if(checkIfCanMove)
	{
		if(!Actor::canMoveTowards(this, *force))
		{
			return false;
		}
	}

	Body::applyForce(this->body, force);

	return true;
}

// is it moving?
bool Actor::isMoving()
{
	return isDeleted(this->body) ? false : Body::getMovementOnAllAxis(this->body);
}

uint16 Actor::getMovementState()
{
	return Actor::isMoving(this);
}

void Actor::changeEnvironment(Transformation* environmentTransform)
{
	Base::changeEnvironment(this, environmentTransform);

	if(!isDeleted(this->body))
	{
		Body::setPosition(this->body, &this->transformation.position, SpatialObject::safeCast(this));
	}
}

// get bounciness
fixed_t Actor::getBounciness()
{
	PhysicalProperties* physicalProperties = ((ActorSpec*)this->entitySpec)->animatedEntitySpec.entitySpec.physicalProperties;

	return !isDeleted(this->body) ? Body::getBounciness(this->body) : physicalProperties ? physicalProperties->bounciness : 0;
}

// get velocity
const Vector3D* Actor::getVelocity()
{
	return !isDeleted(this->body) ? Body::getVelocity(this->body) : Base::getVelocity(this);
}

const Vector3D* Actor::getDirection()
{
	return !isDeleted(this->body) ? Body::getDirection(this->body) : Base::getDirection(this);
}

fixed_t Actor::getSpeed()
{
	return !isDeleted(this->body) ? Body::getSpeed(this->body) : Base::getSpeed(this);
}

fixed_t Actor::getMaximumSpeed()
{
	return !isDeleted(this->body) ? Body::getMaximumSpeed(this->body) : 0;
}

void Actor::exitCollision(Collider collider  __attribute__ ((unused)), Collider colliderNotCollidingAnymore, bool isColliderImpenetrable)
{
	if(isDeleted(this->body))
	{
		return;
	}

	Body::setSurroundingFrictionCoefficient(this->body,  Actor::getSurroundingFrictionCoefficient(this));

	if(isColliderImpenetrable)
	{
		Body::clearNormal(this->body, ListenerObject::safeCast(colliderNotCollidingAnymore));
	}
}

void Actor::otherColliderOwnerDestroyed(Collider collider __attribute__ ((unused)), Collider colliderNotCollidingAnymore, bool isColliderImpenetrable)
{
	if(isDeleted(this->body))
	{
		return;
	}

	Body::setSurroundingFrictionCoefficient(this->body,  Actor::getSurroundingFrictionCoefficient(this));

	if(isColliderImpenetrable)
	{
		Body::clearNormal(this->body, ListenerObject::safeCast(colliderNotCollidingAnymore));
	}
}

Body Actor::getBody()
{
	return this->body;
}

bool Actor::mustBounce()
{
	return true;
}
