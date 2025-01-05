/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <Body.h>
#include <Collider.h>
#include <ColliderManager.h>
#include <ComponentManager.h>
#include <Printing.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "Entity.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->components = NULL;
	this->transformation.position = Vector3D::zero();
	this->transformation.rotation = Rotation::zero();
	this->transformation.scale = Scale::unit();
	this->body = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::destructor()
{
	this->body = NULL;

	Entity::clearComponentLists(this, kComponentTypes);
	Entity::destroyComponents(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageBodyStartedMoving:

			Entity::checkCollisions(this, true);
			return true;
			break;

		case kMessageBodyStopped:

			if(NULL == this->body || __NO_AXIS == Body::getMovementOnAllAxis(this->body))
			{
				Entity::checkCollisions(this, false);
			}
			break;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::clearComponentLists(uint32 componentType)
{
	if(NULL == this->components)
	{
		return;
	}

	if(kComponentTypes <= componentType)
	{
		for(int16 i = 0; i < kComponentTypes; i++)
		{
			if(NULL != this->components[i])
			{
				delete this->components[i];
				this->components[i] = NULL;
			}
		}

		delete this->components;
		this->components = NULL;
	}
	else if(NULL != this->components[componentType])
	{
		delete this->components[componentType];
		this->components[componentType] = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Component Entity::addComponent(const ComponentSpec* componentSpec)
{
	return ComponentManager::addComponent(this, componentSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::removeComponent(Component component)
{
	if(NULL == component)
	{
		return;
	}

	ComponentManager::removeComponent(this, component);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::addComponents(ComponentSpec** componentSpecs, uint32 componentType)
{
	ComponentManager::addComponents(this, componentSpecs, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::removeComponents(uint32 componentType)
{
	ComponentManager::removeComponents(this, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Component Entity::getComponentAtIndex(uint32 componentType, int16 componentIndex)
{
	return ComponentManager::getComponentAtIndex(this, componentType, componentIndex);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualList Entity::getComponents(uint32 componentType)
{
	return ComponentManager::getComponents(this, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::getComponentsOfClass(ClassPointer classPointer, VirtualList components, uint32 componentType)
{
	return ComponentManager::getComponentsOfClass(this, classPointer, components, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 Entity::getComponentsCount(uint32 componentType)
{
	return ComponentManager::getComponentsCount(this, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::resetComponents()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cComponentCommandReset, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Transformation* Entity::getTransformation()
{
	return &this->transformation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* Entity::getPosition()
{
	return &this->transformation.position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Rotation* Entity::getRotation()
{
	return &this->transformation.rotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Scale* Entity::getScale()
{
	return &this->transformation.scale;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Body Entity::getBody()
{
	return this->body;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::isMoving()
{
	return isDeleted(this->body) ? false : Body::getMovementOnAllAxis(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::stopAllMovement()
{
	Entity::stopMovement(this, __ALL_AXIS);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::stopMovement(uint16 axis)
{
	if(!isDeleted(this->body))
	{
		Body::stopMovement(this->body, axis);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::setVelocity(const Vector3D* velocity, bool checkIfCanMove)
{
	ASSERT(this->body, "Entity::applyForce: null body");

	if(isDeleted(this->body))
	{
		return false;
	}

	if(checkIfCanMove)
	{
		if(!Entity::canMoveTowards(this, *velocity))
		{
			return false;
		}
	}

	Body::setVelocity(this->body, velocity);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* Entity::getVelocity()
{
	if(isDeleted(this->body))
	{
		static Vector3D dummyVelocity = {0, 0, 0};

		return &dummyVelocity;
	}

	return Body::getVelocity(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Entity::getSpeed()
{
	if(isDeleted(this->body))
	{
		return 0;
	}

	return Body::getSpeed(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Entity::getMaximumSpeed()
{
	return !isDeleted(this->body) ? Body::getMaximumSpeed(this->body) : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Entity::getBounciness()
{
	if(isDeleted(this->body))
	{
		return 0;
	}

	return Body::getBounciness(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Entity::getFrictionCoefficient()
{
	if(isDeleted(this->body))
	{
		return 0;
	}

	return Body::getFrictionCoefficient(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::enableCollisions()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cComponentCommandEnable, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::disableCollisions()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cComponentCommandDisable, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::checkCollisions(bool active)
{
	ColliderManager::propagateCommand
	(
		VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandCheckCollisions, this, (uint32)active
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::registerCollisions(bool value)
{
	ColliderManager::propagateCommand
	(
		VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandRegisterCollisions, this, (uint32)value
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setCollidersLayers(uint32 layers)
{
	ColliderManager::propagateCommand
	(
		VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandSetLayers, this, (uint32)layers
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Entity::getCollidersLayers()
{
	uint32 collidersLayers = 0;

	VirtualList colliders = Entity::getComponents(this, kColliderComponent);

	for(VirtualNode node = colliders->head; NULL != node; node = node->next)
	{
		Collider collider = Collider::safeCast(node->data);

		collidersLayers |= Collider::getLayers(collider);
	}

	return collidersLayers;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setCollidersLayersToIgnore(uint32 layersToIgnore)
{
	ColliderManager::propagateCommand
	(
		VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandSetLayersToIgnore, this, (uint32)layersToIgnore
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Entity::getCollidersLayersToIgnore()
{
	uint32 collidersLayersToIgnore = 0;

	VirtualList colliders = Entity::getComponents(this, kColliderComponent);

	for(VirtualNode node = colliders->head; NULL != node; node = node->next)
	{
		Collider collider = Collider::safeCast(node->data);

		collidersLayersToIgnore |= Collider::getLayersToIgnore(collider);
	}

	return collidersLayersToIgnore;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::showColliders()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandShow, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::hideColliders()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandHide, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::createComponents(ComponentSpec** componentSpecs)
{
	if(NULL == componentSpecs || 0 < ComponentManager::getComponentsCount(this, kComponentTypes))
	{
		// Components creation must happen only once in the spatial object's life cycle,
		// But deferred instantiation can cause multiple calls to this method.
		return;
	}

	Entity::addComponents(this, componentSpecs, kComponentTypes);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::destroyComponents()
{
	ComponentManager::removeComponents(this, kComponentTypes);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::addedComponent(Component component)
{
	switch(Component::getType(component))
	{
		case kSpriteComponent:
		case kWireframeComponent:
		{
			if(Entity::overrides(this, calculateSize))
			{
				Entity::calculateSize(this);
			}

			break;
		}

		case kPhysicsComponent:
		{
			NM_ASSERT(NULL == this->body, "Entity::addedComponent: adding multiple bodies");

			if(NULL != this->body)
			{
				return;
			}

			this->body = Body::safeCast(Object::getCast(component, typeofclass(Body), NULL));

			if(!isDeleted(this->body))
			{
				Body::setPosition(this->body, &this->transformation.position, Entity::safeCast(this));
			}

			break;
		}
	}	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::removedComponent(Component component __attribute__((unused)))
{
	switch(Component::getType(component))
	{
		case kSpriteComponent:
		case kWireframeComponent:
		{
			if(Entity::overrides(this, calculateSize))
			{
				Entity::calculateSize(this);
			}

			break;
		}

		case kPhysicsComponent:
		{
			if(Body::safeCast(component) == this->body)
			{
				this->body = NULL;
			}

			break;
		}
	}	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::show()
{
	VisualComponent::propagateCommand(cVisualComponentCommandShow, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::hide()
{
	VisualComponent::propagateCommand(cVisualComponentCommandHide, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setTransparency(uint8 transparency)
{
	VisualComponent::propagateCommand(cVisualComponentCommandSetTransparency, this, (uint32)transparency);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::calculateSize()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Entity::getRadius()
{
	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setPosition(const Vector3D* position)
{
	this->transformation.position = *position;

	if(!isDeleted(this->body) && Body::getPosition(this->body) != position)
	{
		Body::setPosition(this->body, &this->transformation.position, Entity::safeCast(this));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setRotation(const Rotation* rotation)
{
	this->transformation.rotation = *rotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setScale(const Scale* scale)
{
	this->transformation.scale = *scale;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setDirection(const Vector3D* direction __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* Entity::getDirection()
{
	if(isDeleted(this->body))
	{
		static Vector3D dummyDirection = {0, 0, 0};

		return &dummyDirection;
	}

	return Body::getDirection(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::applyForce(const Vector3D* force, bool checkIfCanMove)
{
	NM_ASSERT(NULL != this->body, "Entity::applyForce: null body");

	if(isDeleted(this->body))
	{
		return false;
	}

	if(checkIfCanMove)
	{
		if(!Entity::canMoveTowards(this, *force))
		{
			return false;
		}
	}

	Body::applyForce(this->body, force);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::canMoveTowards(Vector3D direction)
{
	fixed_t collisionCheckDistance = __PIXELS_TO_METERS(4);

	Vector3D displacement =
	{
		direction.x ? 0 < direction.x ? collisionCheckDistance : -collisionCheckDistance : 0,
		direction.y ? 0 < direction.y ? collisionCheckDistance : -collisionCheckDistance : 0,
		direction.z ? 0 < direction.z ? collisionCheckDistance : -collisionCheckDistance : 0
	};

	bool canMove = true;

	VirtualList colliders = Entity::getComponents(this, kColliderComponent);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::isSensibleToCollidingObjectBouncinessOnCollision(Entity collidingEntity __attribute__ ((unused)))
{
	return  true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::isSensibleToCollidingObjectFrictionOnCollision(Entity collidingEntity __attribute__ ((unused)))
{
	return  true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::isSubjectToGravity(Vector3D gravity)
{
	if(isDeleted(this->body))
	{
		return false;
	}

	if(__NO_AXIS == Body::getAxisSubjectToGravity(this->body))
	{
		return false;
	}

	return Entity::canMoveTowards(this, gravity);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Entity::collisionStarts(const CollisionInformation* collisionInformation)
{
	ASSERT(collisionInformation->otherCollider, "Entity::collisionStarts: otherColliders");

	ASSERT(NULL != collisionInformation->otherCollider, "Particle::collisionStarts: otherColliders");

	bool returnValue = false;

	if(collisionInformation->collider && collisionInformation->otherCollider)
	{
		if(collisionInformation->solutionVector.magnitude)
		{
			Collider::resolveCollision(collisionInformation->collider, collisionInformation);

			Entity collidingEntity = Collider::getOwner(collisionInformation->otherCollider);

			fixed_t bounciness = 
				Entity::isSensibleToCollidingObjectBouncinessOnCollision(this, collidingEntity) ? 
				Entity::getBounciness(collidingEntity) : 0;

			fixed_t frictionCoefficient = 
				Entity::isSensibleToCollidingObjectFrictionOnCollision(this, collidingEntity) ? 
				Entity::getSurroundingFrictionCoefficient(this) : 0;

			if(!isDeleted(this->body))
			{
				Body::bounce
				(
					this->body, 
					ListenerObject::safeCast(collisionInformation->otherCollider), 
					collisionInformation->solutionVector.direction, 
					frictionCoefficient, 
					bounciness
				);
			}

			returnValue = true;
		}
	}

	return returnValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::collisionPersists(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::collisionEnds(const CollisionInformation* collisionInformation)
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
	Body::setSurroundingFrictionCoefficient(this->body,  Entity::getSurroundingFrictionCoefficient(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Entity::getInGameType()
{
	return kTypeNone;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t Entity::getSurroundingFrictionCoefficient()
{
	fixed_t totalFrictionCoefficient = 0;

	VirtualList colliders = Entity::getComponents(this, kColliderComponent);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
