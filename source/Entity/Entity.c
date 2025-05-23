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
#include <ComponentManager.h>
#include <Printer.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>

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
	this->isVisible = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::destructor()
{
	this->body = NULL;

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
		{
			Entity::checkCollisions(this, true);
			return true;
		}
		
		case kMessageBodyStopped:
		{
			if(NULL == this->body || __NO_AXIS == Body::getMovementOnAllAxis(this->body))
			{
				Entity::checkCollisions(this, false);
			}
			
			break;
		}
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
	if(NULL == this->components)
	{
		this->components = 
			(VirtualList*)
			(
				(uint32)MemoryPool::allocate(sizeof(VirtualList) * kComponentTypes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD
			);

		for(int16 i = 0; i < kComponentTypes; i++)
		{
			this->components[i] = NULL;
		}
	}

	if(NULL == this->components[componentType])
	{
		this->components[componentType] = new VirtualList();

		ComponentManager::getComponents(this, componentType, this->components[componentType]);
	}
	else
	{
		return this->components[componentType];
	}

	return this->components[componentType];
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
	ComponentManager::propagateCommand(cComponentCommandReset, this, kComponentTypes);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setVisible()
{
	// Only I can put down this flag
	this->isVisible = true;
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
	ComponentManager::propagateCommand(cComponentCommandEnable, this, kColliderComponent);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::disableCollisions()
{
	ComponentManager::propagateCommand(cComponentCommandDisable, this, kColliderComponent);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::checkCollisions(bool active)
{
	ComponentManager::propagateCommand(cColliderComponentCommandCheckCollisions, this, kColliderComponent, (uint32)active);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::registerCollisions(bool value)
{
	ComponentManager::propagateCommand(cColliderComponentCommandRegisterCollisions, this, kColliderComponent, (uint32)value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setCollidersLayers(uint32 layers)
{
	ComponentManager::propagateCommand(cColliderComponentCommandSetLayers, this, kColliderComponent, (uint32)layers);
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
	ComponentManager::propagateCommand(cColliderComponentCommandSetLayersToIgnore, this, (uint32)layersToIgnore);
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
	ComponentManager::propagateCommand(cColliderComponentCommandShow, this, kColliderComponent);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::hideColliders()
{
	ComponentManager::propagateCommand(cColliderComponentCommandHide, this, kColliderComponent);
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

	ComponentManager::createComponents(this, componentSpecs);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::destroyComponents()
{
	Entity::clearComponentLists(this, kComponentTypes);
	ComponentManager::destroyComponents(this);
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
	ComponentManager::propagateCommand(cVisualComponentCommandShow, this, kSpriteComponent);
	ComponentManager::propagateCommand(cVisualComponentCommandShow, this, kWireframeComponent);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::hide()
{
	ComponentManager::propagateCommand(cVisualComponentCommandHide, this, kSpriteComponent);
	ComponentManager::propagateCommand(cVisualComponentCommandHide, this, kWireframeComponent);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Entity::setTransparency(uint8 transparency)
{
	ComponentManager::propagateCommand(cVisualComponentCommandSetTransparency, this, kSpriteComponent, (uint32)transparency);
	ComponentManager::propagateCommand(cVisualComponentCommandSetTransparency, this, kWireframeComponent, (uint32)transparency);
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
	this->transformation.rotation = Rotation::clamp(rotation->x, rotation->y, rotation->z);

	if(!isDeleted(this->body))
	{
		Vector3D direction = Vector3D::zero();

		if(0 != this->transformation.rotation.x)
		{
			direction.x = 0;
			direction.y = __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(this->transformation.rotation.x)));
			direction.z = __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(this->transformation.rotation.x)));
		
			Body::setDirection(this->body, &direction);
		}
		else if(0 != this->transformation.rotation.y)
		{
			direction.x = __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(this->transformation.rotation.y)));
			direction.y = 0;
			direction.z = __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(this->transformation.rotation.y)));
	
			Body::setDirection(this->body, &direction);
		}
		else if(0 != this->transformation.rotation.z)
		{
			direction.x = __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(this->transformation.rotation.z)));
			direction.y = __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(this->transformation.rotation.z)));
			direction.z = 0;

			Body::setDirection(this->body, &direction);
		}
	}
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
