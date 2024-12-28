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

#include <Body.h>
#include <Camera.h>
#include <BodyManager.h>
#include <Collider.h>
#include <State.h>
#include <StateMachine.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "Actor.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualList;
friend class VirtualNode;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Actor::constructor(const ActorSpec* actorSpec, int16 internalId, const char* const name)
{
	// construct base object
	Base::constructor((AnimatedEntitySpec*)&actorSpec->animatedEntitySpec, internalId, name);

	// construct the game state machine
	this->stateMachine = NULL;
}
//---------------------------------------------------------------------------------------------------------
void Actor::destructor()
{
	if(!isDeleted(this->body))
	{
		this->body = NULL;
	}

	// destroy state machine
	if(!isDeleted(this->stateMachine))
	{
		delete this->stateMachine;
		this->stateMachine = NULL;
	}


	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
bool Actor::handleMessage(Telegram telegram)
{
	if(!this->stateMachine || !StateMachine::handleMessage(this->stateMachine, telegram))
	{
		int32 message = Telegram::getMessage(telegram);

		if(!isDeleted(this->body))
		{
			switch(message)
			{
				case kMessageBodyStartedMoving:

					Actor::checkCollisions(this, true);
					return true;

					break;

				case kMessageBodyStopped:

					if(__NO_AXIS == Body::getMovementOnAllAxis(this->body))
					{
						Actor::checkCollisions(this, false);
					}

					break;
			}
		}
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
const Vector3D* Actor::getVelocity()
{
	return !isDeleted(this->body) ? Body::getVelocity(this->body) : Base::getVelocity(this);
}
//---------------------------------------------------------------------------------------------------------
fixed_t Actor::getSpeed()
{
	return !isDeleted(this->body) ? Body::getSpeed(this->body) : Base::getSpeed(this);
}
//---------------------------------------------------------------------------------------------------------
fixed_t Actor::getBounciness()
{
	/// PENDING
	return !isDeleted(this->body) ? Body::getBounciness(this->body) : 0;
//	BodySpec* physicalProperties = ((ActorSpec*)this->entitySpec)->animatedEntitySpec.entitySpec.physicalProperties;

//	return !isDeleted(this->body) ? Body::getBounciness(this->body) : physicalProperties ? physicalProperties->bounciness : 0;
}
//---------------------------------------------------------------------------------------------------------
void Actor::setPosition(const Vector3D* position)
{
	Base::setPosition(this, position);

	if(!isDeleted(this->body) && Body::getPosition(this->body) != position)
	{
		Body::setPosition(this->body, &this->transformation.position, SpatialObject::safeCast(this));
	}
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
const Vector3D* Actor::getDirection()
{
	return !isDeleted(this->body) ? Body::getDirection(this->body) : Base::getDirection(this);
}
//---------------------------------------------------------------------------------------------------------
bool Actor::isSubjectToGravity(Vector3D gravity)
{
	return Actor::canMoveTowards(this, gravity);
}
//---------------------------------------------------------------------------------------------------------
bool Actor::collisionStarts(const CollisionInformation* collisionInformation)
{
	ASSERT(collisionInformation->otherCollider, "Actor::collisionStarts: otherColliders");

	if(NULL == this->body)
	{
		return false;
	}

	bool returnValue = false;

	if(collisionInformation->collider && collisionInformation->otherCollider)
	{
		if(collisionInformation->solutionVector.magnitude)
		{
			Collider::resolveCollision(collisionInformation->collider, collisionInformation);

			SpatialObject collidingObject = Collider::getOwner(collisionInformation->otherCollider);

			fixed_t bounciness = Actor::isSensibleToCollidingObjectBouncinessOnCollision(this, collidingObject) ? SpatialObject::getBounciness(collidingObject) : 0;
			fixed_t frictionCoefficient = Actor::isSensibleToCollidingObjectFrictionOnCollision(this, collidingObject) ? Actor::getSurroundingFrictionCoefficient(this) : 0;

			if(Actor::isBouncy(this) && !isDeleted(this->body))
			{
				Body::bounce(this->body, ListenerObject::safeCast(collisionInformation->otherCollider), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);

				Actor::fireEvent(this, kEventActorBounced);
				NM_ASSERT(!isDeleted(this), "Actor::collisionStarts: deleted this during kEventActorBounced");
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
//---------------------------------------------------------------------------------------------------------
void Actor::collisionEnds(const CollisionInformation* collisionInformation)
{
	if(isDeleted(this->body))
	{
		return;
	}

	if(NULL == collisionInformation || isDeleted(collisionInformation->collider))
	{
		return;
	}

	Body::clearNormal(this->body, ListenerObject::safeCast(collisionInformation->otherCollider));
	Body::setSurroundingFrictionCoefficient(this->body,  Actor::getSurroundingFrictionCoefficient(this));
}
//---------------------------------------------------------------------------------------------------------
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

	if(!isDeleted(this->body))
	{
		Body::setPosition(this->body, &this->transformation.position, SpatialObject::safeCast(this));
	}

	this->transformation.invalid |= (displacement.x ? __X_AXIS: 0) | (displacement.y ? __Y_AXIS: 0) | (displacement.y ? __Z_AXIS: 0);
}
//---------------------------------------------------------------------------------------------------------
void Actor::changeEnvironment(Transformation* environmentTransform)
{
	Base::changeEnvironment(this, environmentTransform);

	if(!isDeleted(this->body))
	{
		Body::setPosition(this->body, &this->transformation.position, SpatialObject::safeCast(this));
	}
}
//---------------------------------------------------------------------------------------------------------
void Actor::addedComponent(Component component)
{
	Base::addedComponent(this, component);

	if(kPhysicsComponent == Component::getType(component))
	{
		this->body = Body::safeCast(Object::getCast(component, typeofclass(Body), NULL));

		if(!isDeleted(this->body))
		{
			Body::setPosition(this->body, &this->transformation.position, SpatialObject::safeCast(this));
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Actor::removedComponent(Component component)
{
	Base::removedComponent(this, component);

	if(Body::safeCast(component) == this->body)
	{
		this->body = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
void Actor::update()
{
	// call base
	Base::update(this);

	if(!isDeleted(this->stateMachine))
	{
		StateMachine::update(this->stateMachine);
	}

//	Body::print(this->body, 1, 0);
//	Printing::resetCoordinates(Printing::getInstance());
}
//---------------------------------------------------------------------------------------------------------
void Actor::createStateMachine(State state)
{
	if(isDeleted(this->stateMachine))
	{
		this->stateMachine = new StateMachine(this);
	}

	StateMachine::swapState(this->stateMachine, state);

	this->update = true;
}
//---------------------------------------------------------------------------------------------------------
Body Actor::getBody()
{
	return this->body;
}
//---------------------------------------------------------------------------------------------------------
bool Actor::isMoving()
{
	return isDeleted(this->body) ? false : Body::getMovementOnAllAxis(this->body);
}
//---------------------------------------------------------------------------------------------------------
void Actor::stopAllMovement()
{
	Actor::stopMovement(this, __ALL_AXIS);
}
//---------------------------------------------------------------------------------------------------------
void Actor::stopMovement(uint16 axis)
{
	if(!isDeleted(this->body))
	{
		Body::stopMovement(this->body, axis);
	}
}
//---------------------------------------------------------------------------------------------------------
fixed_t Actor::getMaximumSpeed()
{
	return !isDeleted(this->body) ? Body::getMaximumSpeed(this->body) : 0;
}
//---------------------------------------------------------------------------------------------------------
bool Actor::setVelocity(const Vector3D* velocity, bool checkIfCanMove)
{
	ASSERT(this->body, "Actor::applyForce: null body");

	if(isDeleted(this->body))
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

	Body::setVelocity(this->body, velocity);

	return true;
}
//---------------------------------------------------------------------------------------------------------
bool Actor::applyForce(const Vector3D* force, bool checkIfCanMove)
{
	ASSERT(this->body, "Actor::applyForce: null body");

	if(isDeleted(this->body))
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
//---------------------------------------------------------------------------------------------------------
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

	VirtualList colliders = Actor::getComponents(this, kColliderComponent);

	if(NULL != colliders)
	{
		VirtualNode node = colliders->head;

		for(; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);
			canMove &= Collider::canMoveTowards(collider, displacement);
		}
	}

	return canMove;
}
//---------------------------------------------------------------------------------------------------------
bool Actor::isBouncy()
{
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool Actor::isSensibleToCollidingObjectBouncinessOnCollision(SpatialObject collidingObject __attribute__ ((unused)))
{
	return  true;
}
//---------------------------------------------------------------------------------------------------------
bool Actor::isSensibleToCollidingObjectFrictionOnCollision(SpatialObject collidingObject __attribute__ ((unused)))
{
	return  true;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
Rotation Actor::getRotationFromDirection(const Vector3D* direction, uint8 axis)
{
	Rotation rotation = this->localTransformation.rotation;

	if(__X_AXIS & axis)
	{
		fixed_ext_t z = direction->z;

		if(direction->x)
		{
			z = Math::squareRootFixed(__FIXED_EXT_MULT(direction->x, direction->x) + __FIXED_EXT_MULT(direction->z, direction->z));

			z = 0 > direction->z ? -z : z;
		}

		rotation.x = __I_TO_FIXED(Math::getAngle(__FIXED_TO_FIX7_9(direction->y), __FIXED_TO_FIX7_9(z))) - __QUARTER_ROTATION_DEGREES;
	}
	
	if(__Y_AXIS & axis)
	{
		fixed_ext_t x = direction->x;

		if(direction->y)
		{
			x = Math::squareRootFixed(__FIXED_EXT_MULT(direction->y, direction->y) + __FIXED_EXT_MULT(direction->x, direction->x));

			x = 0 > direction->x ? -x : x;
		}

		rotation.y = __I_TO_FIXED(Math::getAngle(__FIXED_TO_FIX7_9((direction->z)), __FIXED_TO_FIX7_9(x)));
	}

	if(__Z_AXIS & axis)
	{
		fixed_ext_t y = direction->y;

		if(direction->z)
		{
			y = Math::squareRootFixed(__FIXED_EXT_MULT(direction->z, direction->z) + __FIXED_EXT_MULT(direction->y, direction->y));

			y = 0 > direction->y ? -y : y;
		}

		rotation.z = __I_TO_FIXED(Math::getAngle(__FIXED_TO_FIX7_9((direction->x)), __FIXED_TO_FIX7_9(y)));
	}

	if(__X_AXIS & axis)
	{
		if(__QUARTER_ROTATION_DEGREES < rotation.z)
		{
			rotation.x = rotation.x - __HALF_ROTATION_DEGREES;
		}
	}

	if(__Y_AXIS & axis)
	{
		if(__QUARTER_ROTATION_DEGREES < rotation.x)
		{
			rotation.y = rotation.y - __HALF_ROTATION_DEGREES;
		}
	}

	if(__Z_AXIS & axis)
	{
		if(__QUARTER_ROTATION_DEGREES < rotation.y)
		{
			rotation.z = rotation.z - __HALF_ROTATION_DEGREES;
		}
	}

	return Rotation::clamp(rotation.x, rotation.y, rotation.z);	
}
//---------------------------------------------------------------------------------------------------------
fixed_t Actor::getSurroundingFrictionCoefficient()
{
	fixed_t totalFrictionCoefficient = 0;

	VirtualList colliders = Actor::getComponents(this, kColliderComponent);

	if(NULL != colliders)
	{
		VirtualNode node = colliders->head;

		for(; NULL != node; node = node->next)
		{
			Collider collider = Collider::safeCast(node->data);

			totalFrictionCoefficient += Collider::getCollidingFrictionCoefficient(collider);
		}
	}

	return totalFrictionCoefficient;
}
//---------------------------------------------------------------------------------------------------------
