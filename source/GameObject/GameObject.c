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
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::destructor()
{
	GameObject::clearComponentLists(this, kComponentTypes);
	GameObject::destroyComponents(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
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
	Component component = ComponentManager::addComponent(this, componentSpec);

	GameObject::calculateSize(this);

	return component;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::removeComponent(Component component)
{
	if(NULL == component)
	{
		return;
	}

	ComponentManager::removeComponent(this, component);

	GameObject::calculateSize(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::addComponents(ComponentSpec** componentSpecs, uint32 componentType)
{
	ComponentManager::addComponents(this, componentSpecs, componentType);

	GameObject::calculateSize(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::removeComponents(uint32 componentType)
{
	ComponentManager::removeComponents(this, componentType);

	GameObject::calculateSize(this);
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

void GameObject::addedComponent(Component component __attribute__((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::removedComponent(Component component __attribute__((unused)))
{}

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

const Vector3D* GameObject::getVelocity()
{
	static Vector3D dummyVelocity = {0, 0, 0};

	return &dummyVelocity;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

fixed_t GameObject::getSpeed()
{
	return 0;
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

void GameObject::setPosition(const Vector3D* position)
{
	this->transformation.position = *position;
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
	static Vector3D dummyDirection = {0, 0, 0};

	return &dummyDirection;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool GameObject::collisionStarts(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::collisionPersists(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void GameObject::collisionEnds(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 GameObject::getInGameType()
{
	return kTypeNone;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
