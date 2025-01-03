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

#include <Component.h>
#include <Behavior.h>
#include <Body.h>
#include <BehaviorManager.h>
#include <Collider.h>
#include <ColliderManager.h>
#include <Printing.h>
#include <GameObject.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <VirtualList.h>
#include <VUEngine.h>
#include <Wireframe.h>
#include <WireframeManager.h>

#include "ComponentManager.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Component;
friend class GameObject;
friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __MAXIMUM_NUMBER_OF_COMPONENTS		10


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Component ComponentManager::addComponent(GameObject owner, const ComponentSpec* componentSpec)
{
	ComponentManager componentManager = ComponentManager::getManager(componentSpec->componentType);

	if(NULL == componentManager)
	{
		return NULL;
	}

	Component component = ComponentManager::createComponent(componentManager, owner, componentSpec);

	if(!isDeleted(component))
	{
		GameObject::addedComponent(owner, component);
	}

	return component;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ComponentManager::removeComponent(GameObject owner, Component component)
{
	uint32 componentType = ComponentManager::getComponentType(component);

	if(kComponentTypes <= componentType)
	{
		return;
	}

	ComponentManager componentManager = ComponentManager::getManager(componentType);

	if(NULL == componentManager)
	{
		return;
	}

	if(!isDeleted(component))
	{
		GameObject::removedComponent(owner, component);
	}

	ComponentManager::destroyComponent(componentManager, owner, component);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ComponentManager::addComponents(GameObject owner, ComponentSpec** componentSpecs, uint32 componentType)
{
	for(int32 i = 0; NULL != componentSpecs[i] && NULL != componentSpecs[i]->allocator; i++)
	{
		if(kComponentTypes <= componentType || componentSpecs[i]->componentType == componentType)
		{
			ComponentManager::addComponent(owner, componentSpecs[i]);
		}
		
#ifndef __RELEASE
		if(__MAXIMUM_NUMBER_OF_COMPONENTS < i)
		{
			Printing::setDebugMode(Printing::getInstance());
			Printing::clear(Printing::getInstance());
			Printing::text(Printing::getInstance(), "Component specs array: ", 1, 26, NULL);
			Printing::hex(Printing::getInstance(), (uint32)componentSpecs, 1, 27, 8, NULL);
			Error::triggerException("ComponentManager::addComponents: Non terminated component specs array", NULL);	
		}
#endif
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ComponentManager::removeComponents(GameObject owner, uint32 componentType)
{
	if(kComponentTypes <= componentType)
	{
		for(int16 i = 0; i < kComponentTypes; i++)
		{
			ComponentManager componentManager = ComponentManager::getManager(i);

			if(NULL == componentManager)
			{
				continue;
			}

			for(VirtualNode node = componentManager->components->head, nextNode = NULL; NULL != node; node = nextNode)
			{
				nextNode = node->next;
		
				Component component = Component::safeCast(node->data);

				if(owner == component->owner)
				{
					GameObject::removedComponent(owner, component);

					ComponentManager::destroyComponent(componentManager, owner, component);
				}
			}	
		}
	}
	else
	{
		ComponentManager componentManager = ComponentManager::getManager(componentType);

		if(NULL == componentManager)
		{
			return;
		}

		for(VirtualNode node = componentManager->components->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;
	
			Component component = Component::safeCast(node->data);

			if(owner == component->owner)
			{
				GameObject::removedComponent(owner, component);

				ComponentManager::destroyComponent(componentManager, owner, component);
			}
		}	
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Component ComponentManager::getComponentAtIndex(GameObject owner, uint32 componentType, int16 componentIndex)
{
	if(kComponentTypes <= componentType || 0 > componentIndex)
	{
		return NULL;
	}

	ComponentManager componentManager = ComponentManager::getManager(componentType);

	if(NULL == componentManager)
	{
		return NULL;
	}

	for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
	{
		Component component = Component::safeCast(node->data);

		if(owner == component->owner)
		{
			if(0 == componentIndex--)
			{
				return component;
			}
		}
	}

	return NULL;
}


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static VirtualList ComponentManager::getComponents(GameObject owner, uint32 componentType)
{
	if(NULL == owner->components)
	{
		owner->components = 
			(VirtualList*)
			(
				(uint32)MemoryPool::allocate(sizeof(VirtualList) * kComponentTypes + __DYNAMIC_STRUCT_PAD) + __DYNAMIC_STRUCT_PAD
			);

		for(int16 i = 0; i < kComponentTypes; i++)
		{
			owner->components[i] = NULL;
		}
	}

	if(NULL == owner->components[componentType])
	{
		owner->components[componentType] = new VirtualList();
	}
	else
	{
		return owner->components[componentType];
	}

	ComponentManager componentManager = ComponentManager::getManager(componentType);

	if(NULL == componentManager)
	{
		return NULL;
	}

	return ComponentManager::doGetComponents(componentManager, owner, owner->components[componentType]);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool ComponentManager::getComponentsOfClass(GameObject owner, ClassPointer classPointer, VirtualList components, uint32 componentType)
{
	if(kComponentTypes <= componentType)
	{
		return false;
	}

	ComponentManager componentManager = ComponentManager::getManager(componentType);

	if(NULL == componentManager)
	{
		return false;
	}

	for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
	{
		Component component = Component::safeCast(node->data);

		if(owner != component->owner)
		{
			continue;
		}

		if(!classPointer || Object::getCast(component, classPointer, NULL))
		{
			VirtualList::pushBack(components, component);
		}
	}

	if(NULL != components->head)
	{
		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 ComponentManager::getComponentsCount(GameObject owner, uint32 componentType)
{
	uint16 count = 0;

	if(kComponentTypes <= componentType)
	{
		for(int16 i = 0; i < kComponentTypes; i++)
		{
			ComponentManager componentManager = ComponentManager::getManager(i);

			if(NULL == componentManager)
			{
				continue;
			}

			for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
			{
				Component component = Component::safeCast(node->data);

				if(owner == component->owner)
				{
					count++;
				}
			}
		}
	}
	else
	{
		ComponentManager componentManager = ComponentManager::getManager(componentType);

		if(NULL == componentManager)
		{
			return 0;
		}

		for(VirtualNode node = componentManager->components->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;
	
			Component component = Component::safeCast(node->data);

			if(owner == component->owner)
			{
				count++;
			}
		}	
	}

	return count;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static ComponentManager ComponentManager::getManager(uint32 componentType)
{
	if(kComponentTypes <= componentType)
	{
		return NULL;
	}

	switch (componentType)
	{
		case kColliderComponent:

			return ComponentManager::safeCast(VUEngine::getColliderManager(VUEngine::getInstance()));	
			break;

		case kSpriteComponent:

			return ComponentManager::safeCast(SpriteManager::getInstance());
			break;

		case kWireframeComponent:

			return ComponentManager::safeCast(WireframeManager::getInstance());
			break;

		case kBehaviorComponent:

			return ComponentManager::safeCast(BehaviorManager::getInstance());
			break;

		case kPhysicsComponent:

			return ComponentManager::safeCast(VUEngine::getBodyManager(VUEngine::getInstance()));	
			break;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 ComponentManager::getComponentType(Component component)
{
	if(isDeleted(component))
	{
		return kComponentTypes;
	}

	if(NULL == component->componentSpec)
	{
		if(__GET_CAST(Collider, component))
		{
			return kColliderComponent;
		}
		
		if(__GET_CAST(Sprite, component))
		{
			return kSpriteComponent;
		}
		
		if(__GET_CAST(Wireframe, component))
		{
			return kWireframeComponent;
		}
		
		if(__GET_CAST(Behavior, component))
		{
			return kBehaviorComponent;
		}

		if(__GET_CAST(Body, component))
		{
			return kPhysicsComponent;
		}
	}

	return component->componentSpec->componentType;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ComponentManager::cleanOwnerComponentLists(GameObject owner, uint32 componentType)
{
	if(NULL != owner->components && NULL != owner->components[componentType])
	{
		delete owner->components[componentType];
		owner->components[componentType] = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————



//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ComponentManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->components = new VirtualList();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ComponentManager::destructor()
{
	if(!isDeleted(this->components))
	{
		VirtualList::deleteData(this->components);
		delete this->components;
		this->components = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ComponentManager::propagateCommand(int32 command, GameObject owner, ...)
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Component component = Component::safeCast(node->data);

		if(NULL != owner && owner != component->owner)
		{
			continue;
		}

		va_list args;
		va_start(args, owner);

		Component::handleCommand(component, command, args);

		va_end(args);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 ComponentManager::getCount(GameObject owner)
{
	uint16 count = 0;

	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Component component = Component::safeCast(node->data);

		if(NULL != owner && owner != component->owner)
		{
			continue;
		}
		
		count++;
	}

	return count;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Component ComponentManager::createComponent(GameObject owner, const ComponentSpec* componentSpec)
{
	if(kComponentTypes <= componentSpec->componentType)
	{
		return NULL;
	}

	ComponentManager::cleanOwnerComponentLists(owner, componentSpec->componentType);

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ComponentManager::destroyComponent(GameObject owner, Component component) 
{
	if(isDeleted(component))
	{
		return;
	}

	if(NULL == component->componentSpec || kComponentTypes <= component->componentSpec->componentType)
	{
		return;
	}

	ComponentManager::cleanOwnerComponentLists(owner, component->componentSpec->componentType);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualList ComponentManager::doGetComponents(GameObject owner, VirtualList components)
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Component component = Component::safeCast(node->data);

		if(owner == component->owner)
		{
			VirtualList::pushBack(components, component);
		}
	}

	return components;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ComponentManager::isAnyVisible(GameObject owner __attribute((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

