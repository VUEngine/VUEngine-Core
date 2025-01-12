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
#include <Entity.h>
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
friend class Entity;
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

static Component ComponentManager::createComponent(Entity owner, const ComponentSpec* componentSpec)
{
	if(NULL == componentSpec)
	{
		return NULL;
	}

	ComponentManager componentManager = ComponentManager::getManager(componentSpec->componentType);

	if(NULL == componentManager)
	{
		return NULL;
	}

	Component component = ComponentManager::instantiateComponent(componentManager, owner, componentSpec);

	if(!isDeleted(component) && !isDeleted(owner))
	{
		Entity::addedComponent(owner, component);
	}

	return component;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ComponentManager::destroyComponent(Entity owner, Component component)
{
	if(isDeleted(component))
	{
		return;
	}

	if(owner != component->owner)
	{
		return;
	}

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

	if(!isDeleted(owner))
	{
		Entity::removedComponent(owner, component);
	}

	ComponentManager::deinstantiateComponent(componentManager, owner, component);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Component ComponentManager::addComponent(Entity owner, const ComponentSpec* componentSpec)
{
	if(isDeleted(owner))
	{
		return NULL;
	}

	return ComponentManager::createComponent(owner, componentSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ComponentManager::removeComponent(Entity owner, Component component)
{
	if(isDeleted(component))
	{
		return;
	}

	if(owner != component->owner)
	{
		return;
	}

	ComponentManager::destroyComponent(owner, component);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ComponentManager::addComponents(Entity owner, ComponentSpec** componentSpecs, uint32 componentType)
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
			Printing::setDebugMode();
			Printing::clear();
			Printing::text("Component specs array: ", 1, 26, NULL);
			Printing::hex((uint32)componentSpecs, 1, 27, 8, NULL);
			Error::triggerException("ComponentManager::addComponents: Non terminated component specs array", NULL);	
		}
#endif
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ComponentManager::removeComponents(Entity owner, uint32 componentType)
{
	void removeComponents(ComponentManager componentManager)
	{
		if(NULL == componentManager)
		{
			return;
		}

		for(VirtualNode node = componentManager->components->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;
	
			Component component = Component::safeCast(node->data);

			if(!component->deleteMe && owner == component->owner)
			{
				ComponentManager::removeComponent(owner, component);
			}
		}	
	}

	if(kComponentTypes <= componentType)
	{
		for(int16 i = 0; i < kComponentTypes; i++)
		{
			removeComponents(ComponentManager::getManager(i));
		}
	}
	else
	{		
		removeComponents(ComponentManager::getManager(componentType));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Component ComponentManager::getComponentAtIndex(Entity owner, uint32 componentType, int16 componentIndex)
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

		if(!component->deleteMe && owner == component->owner)
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

static VirtualList ComponentManager::getComponents(Entity owner, uint32 componentType)
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

	for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
	{
		Component component = Component::safeCast(node->data);

		if(owner == component->owner)
		{
			VirtualList::pushBack(owner->components[componentType], component);
		}
	}

	return owner->components[componentType];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool ComponentManager::getComponentsOfClass(Entity owner, ClassPointer classPointer, VirtualList components, uint32 componentType)
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

		if(owner != component->owner || component->deleteMe)
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

static uint16 ComponentManager::getComponentsCount(Entity owner, uint32 componentType)
{
	uint16 getCount(ComponentManager componentManager)
	{
		if(NULL == componentManager)
		{
			return 0;
		}

		uint16 count = 0;

		for(VirtualNode node = componentManager->components->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;
	
			Component component = Component::safeCast(node->data);

			if(owner == component->owner && !component->deleteMe)
			{
				count++;
			}
		}

		return count;
	}

	uint16 count = 0;

	if(kComponentTypes <= componentType)
	{
		for(int16 i = 0; i < kComponentTypes; i++)
		{
			count += getCount(ComponentManager::getManager(i));
		}
	}
	else
	{
		count = getCount(ComponentManager::getManager(componentType));
	}

	return count;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ComponentManager::propagateCommand(int32 command, Entity owner, uint32 componentType, ...)
{
	void propagateCommand(ComponentManager componentManager, va_list args)
	{
		for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
		{
			Component component = Component::safeCast(node->data);

			if(NULL != owner && owner != component->owner)
			{
				continue;
			}

			Component::handleCommand(component, command, args);
		}
	}

	if(kComponentTypes <= componentType)
	{
		for(int16 i = 0; i < kComponentTypes; i++)
		{
			va_list args;
			va_start(args, componentType);

			propagateCommand(ComponentManager::getManager(i), args);

			va_end(args);
		}
	}
	else
	{		
		va_list args;
		va_start(args, componentType);
	
		propagateCommand(ComponentManager::getManager(componentType), args);

		va_end(args);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 ComponentManager::getCount(Entity owner, uint32 componentType)
{
	if(kComponentTypes <= componentType)
	{
		return 0;
	}

	int16 count = 0;

	ComponentManager componentManager = ComponentManager::getManager(componentType);

	if(NULL == componentManager)
	{
		return 0;
	}

	for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
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

static bool ComponentManager::calculateRightBox(Entity owner, RightBox* rightBox)
{
	bool modified = false;

	if(NULL == owner)
	{
		return false;
	}

	for(int16 i = 0; i < kComponentTypes; i++)
	{
		ComponentManager componentManager = ComponentManager::getManager(i);

		if(NULL == componentManager || !ComponentManager::overrides(componentManager, isAnyVisible))
		{
			continue;
		}

		modified |= ComponentManager::getRightBoxFromComponents(componentManager, owner, rightBox);
	}

	return modified;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool ComponentManager::isAnyCompomentVisible(Entity owner)
{
	if(NULL == owner)
	{
		return false;
	}

	for(int16 i = 0; i < kComponentTypes; i++)
	{
		ComponentManager componentManager = ComponentManager::getManager(i);

		if(NULL == componentManager || !ComponentManager::overrides(componentManager, isAnyVisible))
		{
			continue;
		}

		if(ComponentManager::isAnyVisible(componentManager, owner))
		{
			return true;
		}
	}

	return false;
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
		case kSpriteComponent:

			return ComponentManager::safeCast(SpriteManager::getInstance());
			break;

		case kColliderComponent:

			return ComponentManager::safeCast(VUEngine::getColliderManager(VUEngine::getInstance()));	
			break;

		case kPhysicsComponent:

			return ComponentManager::safeCast(VUEngine::getBodyManager(VUEngine::getInstance()));	
			break;

		case kWireframeComponent:

			return ComponentManager::safeCast(WireframeManager::getInstance());
			break;

		case kBehaviorComponent:

			return ComponentManager::safeCast(BehaviorManager::getInstance());
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

static void ComponentManager::cleanOwnerComponentLists(Entity owner, uint32 componentType)
{
	if(NULL == owner)
	{
		return;
	}
	
	if(NULL != owner->components && NULL != owner->components[componentType])
	{
		delete owner->components[componentType];
		owner->components[componentType] = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool ComponentManager::getRightBoxFromComponents(ComponentManager componentMananager, Entity owner, RightBox* rightBox)
{
	if(NULL == rightBox)
	{
		return false;
	}

	bool modified = false;

	for(VirtualNode node = componentMananager->components->head; node; node = node->next)
	{
		Component component = Component::safeCast(node->data);

		if(owner != component->owner)
		{
			continue;
		}

		modified = true;

		RightBox componentRightBox = Component::getRightBox(component);

		NM_ASSERT(componentRightBox.x0 < componentRightBox.x1, "ComponentManager::getRightBoxFromComponents: 0 width");
		NM_ASSERT(componentRightBox.y0 < componentRightBox.y1, "ComponentManager::getRightBoxFromComponents: 0 height");
		NM_ASSERT(componentRightBox.z0 < componentRightBox.z1, "ComponentManager::getRightBoxFromComponents: 0 depth");

		if(rightBox->x0 > componentRightBox.x0)
		{
			rightBox->x0 = componentRightBox.x0;
		}

		if(rightBox->x1 < componentRightBox.x1)
		{
			rightBox->x1 = componentRightBox.x1;
		}

		if(rightBox->y0 > componentRightBox.y0)
		{
			rightBox->y0 = componentRightBox.y0;
		}

		if(rightBox->y1 < componentRightBox.y1)
		{
			rightBox->y1 = componentRightBox.y1;
		}

		if(rightBox->z0 > componentRightBox.z0)
		{
			rightBox->z0 = componentRightBox.z0;
		}

		if(rightBox->z1 < componentRightBox.z1)
		{
			rightBox->z1 = componentRightBox.z1;
		}
	}

	return modified;
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

Component ComponentManager::instantiateComponent(Entity owner, const ComponentSpec* componentSpec)
{
	if(kComponentTypes <= componentSpec->componentType)
	{
		return NULL;
	}

	ComponentManager::cleanOwnerComponentLists(owner, componentSpec->componentType);

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ComponentManager::deinstantiateComponent(Entity owner, Component component) 
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

bool ComponentManager::isAnyVisible(Entity owner __attribute((unused)))
{
	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
