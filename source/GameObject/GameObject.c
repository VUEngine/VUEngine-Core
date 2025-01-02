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

#include "GameObject.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->components = NULL;
	this->transformation.position = Vector3D::zero();
	this->transformation.rotation = Rotation::zero();
	this->transformation.scale = Scale::unit();
	this->body = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::destructor()
{
	this->body = NULL;

	GameObject::clearComponentLists(this, kComponentTypes);
	GameObject::destroyComponents(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kMessageBodyStartedMoving:

			GameObject::checkCollisions(this, true);
			return true;
			break;

		case kMessageBodyStopped:

			if(NULL == this->body || __NO_AXIS == Body::getMovementOnAllAxis(this->body))
			{
				GameObject::checkCollisions(this, false);
			}
			break;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::clearComponentLists(uint32 componentType)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Component GameObject::addComponent(const ComponentSpec* componentSpec)
{
	return ComponentManager::addComponent(this, componentSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::removeComponent(Component component)
{
	if(NULL == component)
	{
		return;
	}

	ComponentManager::removeComponent(this, component);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::addComponents(ComponentSpec** componentSpecs, uint32 componentType)
{
	ComponentManager::addComponents(this, componentSpecs, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::removeComponents(uint32 componentType)
{
	ComponentManager::removeComponents(this, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Component GameObject::getComponentAtIndex(uint32 componentType, int16 componentIndex)
{
	return ComponentManager::getComponentAtIndex(this, componentType, componentIndex);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualList GameObject::getComponents(uint32 componentType)
{
	return ComponentManager::getComponents(this, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::getComponentsOfClass(ClassPointer classPointer, VirtualList components, uint32 componentType)
{
	return ComponentManager::getComponentsOfClass(this, classPointer, components, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 GameObject::getComponentsCount(uint32 componentType)
{
	return ComponentManager::getComponentsCount(this, componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::resetComponents()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cComponentCommandReset, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

const Transformation* GameObject::getTransformation()
{
	return &this->transformation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* GameObject::getPosition()
{
	return &this->transformation.position;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

const Rotation* GameObject::getRotation()
{
	return &this->transformation.rotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

const Scale* GameObject::getScale()
{
	return &this->transformation.scale;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Body GameObject::getBody()
{
	return this->body;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::isMoving()
{
	return isDeleted(this->body) ? false : Body::getMovementOnAllAxis(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::stopAllMovement()
{
	GameObject::stopMovement(this, __ALL_AXIS);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::stopMovement(uint16 axis)
{
	if(!isDeleted(this->body))
	{
		Body::stopMovement(this->body, axis);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::setVelocity(const Vector3D* velocity, bool checkIfCanMove)
{
	ASSERT(this->body, "GameObject::applyForce: null body");

	if(isDeleted(this->body))
	{
		return false;
	}

	if(checkIfCanMove)
	{
		if(!GameObject::canMoveTowards(this, *velocity))
		{
			return false;
		}
	}

	Body::setVelocity(this->body, velocity);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* GameObject::getVelocity()
{
	Body body = Body::safeCast(ComponentManager::getComponentAtIndex(this, kPhysicsComponent, 0));

	if(isDeleted(body))
	{
		static Vector3D dummyVelocity = {0, 0, 0};

		return &dummyVelocity;
	}

	return Body::getVelocity(body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t GameObject::getSpeed()
{
	Body body = Body::safeCast(ComponentManager::getComponentAtIndex(this, kPhysicsComponent, 0));

	if(isDeleted(body))
	{
		return 0;
	}

	return Body::getSpeed(body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t GameObject::getMaximumSpeed()
{
	return !isDeleted(this->body) ? Body::getMaximumSpeed(this->body) : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t GameObject::getBounciness()
{
	Body body = Body::safeCast(ComponentManager::getComponentAtIndex(this, kPhysicsComponent, 0));

	if(isDeleted(body))
	{
		return 0;
	}

	return Body::getBounciness(body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t GameObject::getFrictionCoefficient()
{
	Body body = Body::safeCast(ComponentManager::getComponentAtIndex(this, kPhysicsComponent, 0));

	if(isDeleted(body))
	{
		return 0;
	}

	return Body::getFrictionCoefficient(body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::enableCollisions()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cComponentCommandEnable, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::disableCollisions()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cComponentCommandDisable, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::checkCollisions(bool active)
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandCheckCollisions, this, (uint32)active);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::registerCollisions(bool value)
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandRegisterCollisions, this, (uint32)value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::setCollidersLayers(uint32 layers)
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandSetLayers, this, (uint32)layers);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 GameObject::getCollidersLayers()
{
	uint32 collidersLayers = 0;

	VirtualList colliders = GameObject::getComponents(this, kColliderComponent);

	for(VirtualNode node = colliders->head; NULL != node; node = node->next)
	{
		Collider collider = Collider::safeCast(node->data);

		collidersLayers |= Collider::getLayers(collider);
	}

	return collidersLayers;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::setCollidersLayersToIgnore(uint32 layersToIgnore)
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandSetLayersToIgnore, this, (uint32)layersToIgnore);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 GameObject::getCollidersLayersToIgnore()
{
	uint32 collidersLayersToIgnore = 0;

	VirtualList colliders = GameObject::getComponents(this, kColliderComponent);

	for(VirtualNode node = colliders->head; NULL != node; node = node->next)
	{
		Collider collider = Collider::safeCast(node->data);

		collidersLayersToIgnore |= Collider::getLayersToIgnore(collider);
	}

	return collidersLayersToIgnore;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::showColliders()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandShow, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::hideColliders()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandHide, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::createComponents(ComponentSpec** componentSpecs)
{
	if(NULL == componentSpecs || 0 < ComponentManager::getComponentsCount(this, kComponentTypes))
	{
		// Components creation must happen only once in the spatial object's life cycle,
		// but deferred instantiation can cause multiple calls to this method.
		return;
	}

	GameObject::addComponents(this, componentSpecs, kComponentTypes);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::destroyComponents()
{
	ComponentManager::removeComponents(this, kComponentTypes);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::addedComponent(Component component)
{
	switch(Component::getType(component))
	{
		case kSpriteComponent:
		case kWireframeComponent:
		{
			if(GameObject::overrides(this, calculateSize))
			{
				GameObject::calculateSize(this);
			}

			break;
		}

		case kPhysicsComponent:
		{
			NM_ASSERT(NULL == this->body, "GameObject::addedComponent: adding multiple bodies");

			if(NULL != this->body)
			{
				return;
			}

			this->body = Body::safeCast(Object::getCast(component, typeofclass(Body), NULL));

			if(!isDeleted(this->body))
			{
				Body::setPosition(this->body, &this->transformation.position, GameObject::safeCast(this));
			}

			break;
		}
	}	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::removedComponent(Component component __attribute__((unused)))
{
	switch(Component::getType(component))
	{
		case kSpriteComponent:
		case kWireframeComponent:
		{
			if(GameObject::overrides(this, calculateSize))
			{
				GameObject::calculateSize(this);
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::show()
{
	VisualComponent::propagateCommand(cVisualComponentCommandShow, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::hide()
{
	VisualComponent::propagateCommand(cVisualComponentCommandHide, this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::setTransparency(uint8 transparency)
{
	VisualComponent::propagateCommand(cVisualComponentCommandSetTransparency, this, (uint32)transparency);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::calculateSize()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t GameObject::getRadius()
{
	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::setPosition(const Vector3D* position)
{
	this->transformation.position = *position;

	if(!isDeleted(this->body) && Body::getPosition(this->body) != position)
	{
		Body::setPosition(this->body, &this->transformation.position, GameObject::safeCast(this));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::setRotation(const Rotation* rotation)
{
	this->transformation.rotation = *rotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::setScale(const Scale* scale)
{
	this->transformation.scale = *scale;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::setDirection(const Vector3D* direction __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

const Vector3D* GameObject::getDirection()
{
	Body body = Body::safeCast(ComponentManager::getComponentAtIndex(this, kPhysicsComponent, 0));

	if(isDeleted(body))
	{
		static Vector3D dummyDirection = {0, 0, 0};

		return &dummyDirection;
	}

	return Body::getDirection(this->body);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::applyForce(const Vector3D* force, bool checkIfCanMove)
{
	NM_ASSERT(NULL != this->body, "GameObject::applyForce: null body");

	if(isDeleted(this->body))
	{
		return false;
	}

	if(checkIfCanMove)
	{
		if(!GameObject::canMoveTowards(this, *force))
		{
			return false;
		}
	}

	Body::applyForce(this->body, force);

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::canMoveTowards(Vector3D direction)
{
	fixed_t collisionCheckDistance = __I_TO_FIXED(1);

	Vector3D displacement =
	{
		direction.x ? 0 < direction.x ? collisionCheckDistance : -collisionCheckDistance : 0,
		direction.y ? 0 < direction.y ? collisionCheckDistance : -collisionCheckDistance : 0,
		direction.z ? 0 < direction.z ? collisionCheckDistance : -collisionCheckDistance : 0
	};

	bool canMove = true;

	VirtualList colliders = GameObject::getComponents(this, kColliderComponent);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::isSensibleToCollidingObjectBouncinessOnCollision(GameObject collidingObject __attribute__ ((unused)))
{
	return  true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::isSensibleToCollidingObjectFrictionOnCollision(GameObject collidingObject __attribute__ ((unused)))
{
	return  true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::collisionStarts(const CollisionInformation* collisionInformation)
{
	ASSERT(collisionInformation->otherCollider, "GameObject::collisionStarts: otherColliders");

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

			GameObject collidingObject = Collider::getOwner(collisionInformation->otherCollider);

			fixed_t bounciness = GameObject::isSensibleToCollidingObjectBouncinessOnCollision(this, collidingObject) ? GameObject::getBounciness(collidingObject) : 0;
			fixed_t frictionCoefficient = GameObject::isSensibleToCollidingObjectFrictionOnCollision(this, collidingObject) ? GameObject::getSurroundingFrictionCoefficient(this) : 0;

			if(!isDeleted(this->body))
			{
				Body::bounce(this->body, ListenerObject::safeCast(collisionInformation->otherCollider), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);
			}
			else
			{
				uint16 axis = __NO_AXIS;
				axis |= collisionInformation->solutionVector.direction.x ? __X_AXIS : __NO_AXIS;
				axis |= collisionInformation->solutionVector.direction.y ? __Y_AXIS : __NO_AXIS;
				axis |= collisionInformation->solutionVector.direction.z ? __Z_AXIS : __NO_AXIS;
				GameObject::stopMovement(this, axis);
			}

			returnValue = true;
		}
	}

	return returnValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::collisionPersists(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::collisionEnds(const CollisionInformation* collisionInformation)
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
	Body::setSurroundingFrictionCoefficient(this->body,  GameObject::getSurroundingFrictionCoefficient(this));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 GameObject::getInGameType()
{
	return kTypeNone;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t GameObject::getSurroundingFrictionCoefficient()
{
	fixed_t totalFrictionCoefficient = 0;

	VirtualList colliders = GameObject::getComponents(this, kColliderComponent);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
