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

#include <string.h>

#include <Body.h>
#include <Collider.h>
#include <ColliderManager.h>
#include <ComponentManager.h>
#include <Printing.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "SpatialObject.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void SpatialObject::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// set position
	this->transformation.position = Vector3D::zero();

	// set rotation
	this->transformation.rotation = Rotation::zero();

	// set scale
	this->transformation.scale = Scale::unit();

	this->components = NULL;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::destructor()
{
	if(!isDeleted(this->components))
	{
		for(int16 i = 0; i < kComponentTypes; i++)
		{
			if(!isDeleted(this->components[i]))
			{
				delete this->components[i];
			}
		}

		delete this->components;
		this->components = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::clearComponentLists(uint32 componentType)
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
//---------------------------------------------------------------------------------------------------------
Component SpatialObject::addComponent(ComponentSpec* componentSpec)
{
	Component component = ComponentManager::addComponent(this, componentSpec);

	SpatialObject::calculateSize(this);

	return component;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::removeComponent(Component component)
{
	if(NULL == component)
	{
		return;
	}

	ComponentManager::removeComponent(this, component);

	SpatialObject::calculateSize(this);
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::addComponents(ComponentSpec** componentSpecs, uint32 componentType)
{
	ComponentManager::addComponents(this, componentSpecs, componentType);

	SpatialObject::calculateSize(this);
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::removeComponents(uint32 componentType)
{
	ComponentManager::removeComponents(this, componentType);

	SpatialObject::calculateSize(this);
}
//---------------------------------------------------------------------------------------------------------
Component SpatialObject::getComponentAtIndex(uint32 componentType, int16 componentIndex)
{
	return ComponentManager::getComponentAtIndex(this, componentType, componentIndex);
}
//---------------------------------------------------------------------------------------------------------
VirtualList SpatialObject::getComponents(uint32 componentType)
{
	return ComponentManager::getComponents(this, componentType);
}
//---------------------------------------------------------------------------------------------------------
bool SpatialObject::getComponentsOfClass(ClassPointer classPointer, VirtualList components, uint32 componentType)
{
	return ComponentManager::getComponentsOfClass(this, classPointer, components, componentType);
}
//---------------------------------------------------------------------------------------------------------
uint16 SpatialObject::getComponentsCount(uint32 componentType)
{
	return ComponentManager::getComponentsCount(this, componentType);
}
//---------------------------------------------------------------------------------------------------------
const Transformation* SpatialObject::getTransformation()
{
	return &this->transformation;
}
//---------------------------------------------------------------------------------------------------------
const Vector3D* SpatialObject::getPosition()
{
	return &this->transformation.position;
}
//---------------------------------------------------------------------------------------------------------
const Rotation* SpatialObject::getRotation()
{
	return &this->transformation.rotation;
}
//---------------------------------------------------------------------------------------------------------
const Scale* SpatialObject::getScale()
{
	return &this->transformation.scale;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::enableCollisions()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cComponentCommandEnable, SpatialObject::safeCast(this));
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::disableCollisions()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cComponentCommandDisable, SpatialObject::safeCast(this));
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::checkCollisions(bool active)
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandCheckCollisions, SpatialObject::safeCast(this), (uint32)active);
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::registerCollisions(bool value)
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandRegisterCollisions, SpatialObject::safeCast(this), (uint32)value);
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setCollidersLayers(uint32 layers)
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandSetLayers, SpatialObject::safeCast(this), (uint32)layers);
}
//---------------------------------------------------------------------------------------------------------
uint32 SpatialObject::getCollidersLayers()
{
	uint32 collidersLayers = 0;

	VirtualList colliders = SpatialObject::getComponents(this, kColliderComponent);

	for(VirtualNode node = colliders->head; NULL != node; node = node->next)
	{
		Collider collider = Collider::safeCast(node->data);

		collidersLayers |= Collider::getLayers(collider);
	}

	return collidersLayers;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setCollidersLayersToIgnore(uint32 layersToIgnore)
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandSetLayersToIgnore, SpatialObject::safeCast(this), (uint32)layersToIgnore);
}
//---------------------------------------------------------------------------------------------------------
uint32 SpatialObject::getCollidersLayersToIgnore()
{
	uint32 collidersLayersToIgnore = 0;

	VirtualList colliders = SpatialObject::getComponents(this, kColliderComponent);

	for(VirtualNode node = colliders->head; NULL != node; node = node->next)
	{
		Collider collider = Collider::safeCast(node->data);

		collidersLayersToIgnore |= Collider::getLayersToIgnore(collider);
	}

	return collidersLayersToIgnore;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::showColliders()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandShow, SpatialObject::safeCast(this));
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::hideColliders()
{
	ColliderManager::propagateCommand(VUEngine::getColliderManager(_vuEngine), cColliderComponentCommandHide, SpatialObject::safeCast(this));
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::createComponents(ComponentSpec** componentSpecs)
{
	if(NULL == componentSpecs || 0 < ComponentManager::getComponentsCount(this, kComponentTypes))
	{
		// Components creation must happen only once in the spatial object's life cycle,
		// but deferred instantiation can cause multiple calls to this method.
		return;
	}

	SpatialObject::addComponents(this, componentSpecs, kComponentTypes);
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::destroyComponents()
{
	ComponentManager::removeComponents(this, kComponentTypes);
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::addedComponent(Component component __attribute__((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::removedComponent(Component component __attribute__((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::calculateSize()
{}
//---------------------------------------------------------------------------------------------------------
fixed_t SpatialObject::getRadius()
{
	return 0;
}
//---------------------------------------------------------------------------------------------------------
const Vector3D* SpatialObject::getVelocity()
{
	static Vector3D dummyVelocity = {0, 0, 0};

	return &dummyVelocity;
}
//---------------------------------------------------------------------------------------------------------
fixed_t SpatialObject::getSpeed()
{
	return 0;
}
//---------------------------------------------------------------------------------------------------------
fixed_t SpatialObject::getBounciness()
{
	Body body = Body::safeCast(ComponentManager::getComponentAtIndex(SpatialObject::safeCast(this), kPhysicsComponent, 0));

	if(isDeleted(body))
	{
		return 0;
	}

	return Body::getBounciness(body);
}
//---------------------------------------------------------------------------------------------------------
fixed_t SpatialObject::getFrictionCoefficient()
{
	Body body = Body::safeCast(ComponentManager::getComponentAtIndex(SpatialObject::safeCast(this), kPhysicsComponent, 0));

	if(isDeleted(body))
	{
		return 0;
	}

	return Body::getFrictionCoefficient(body);
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setPosition(const Vector3D* position)
{
	this->transformation.position = *position;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setRotation(const Rotation* rotation)
{
	this->transformation.rotation = *rotation;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setScale(const Scale* scale)
{
	this->transformation.scale = *scale;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::setDirection(const Vector3D* direction __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
const Vector3D* SpatialObject::getDirection()
{
	static Vector3D dummyDirection = {0, 0, 0};

	return &dummyDirection;
}
//---------------------------------------------------------------------------------------------------------
bool SpatialObject::isSubjectToGravity(Vector3D gravity __attribute__ ((unused)))
{
	return false;
}
//---------------------------------------------------------------------------------------------------------
uint32 SpatialObject::getInGameType()
{
	return kTypeNone;
}
//---------------------------------------------------------------------------------------------------------
bool SpatialObject::collisionStarts(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{
	return false;
}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::collisionPersists(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
void SpatialObject::collisionEnds(const CollisionInformation* collisionInformation __attribute__ ((unused)))
{}
//---------------------------------------------------------------------------------------------------------
