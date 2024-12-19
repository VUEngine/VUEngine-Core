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

#include <Component.h>
#include <Behavior.h>
#include <Body.h>
#include <BehaviorManager.h>
#include <Collider.h>
#include <ColliderManager.h>
#include <SpatialObject.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <VirtualList.h>
#include <VUEngine.h>
#include <Wireframe.h>
#include <WireframeManager.h>

#include "ComponentManager.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Component;
friend class SpatialObject;
friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static Component ComponentManager::addComponent(SpatialObject owner, ComponentSpec* componentSpec)
{
	ComponentManager componentManager = ComponentManager::getManager(componentSpec->componentType);

	if(NULL == componentManager)
	{
		return NULL;
	}

	return ComponentManager::createComponent(componentManager, owner, componentSpec);
}
//---------------------------------------------------------------------------------------------------------
static void ComponentManager::removeComponent(SpatialObject owner, Component component)
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

	ComponentManager::destroyComponent(componentManager, owner, component);
}
//---------------------------------------------------------------------------------------------------------
static void ComponentManager::addComponents(SpatialObject owner, ComponentSpec** componentSpecs)
{
	for(int32 i = 0; NULL != componentSpecs[i] && NULL != componentSpecs[i]->allocator; i++)
	{
		ComponentManager componentManager = ComponentManager::getManager(componentSpecs[i]->componentType);

		if(NULL == componentManager)
		{
			return;
		}

		ComponentManager::createComponent(componentManager, owner, componentSpecs[i]);
	}
}
//---------------------------------------------------------------------------------------------------------
static void ComponentManager::removeComponents(SpatialObject owner, uint32 componentType)
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

			for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
			{
				Component component = Component::safeCast(node->data);

				if(owner == component->owner)
				{
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

		for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
		{
			Component component = Component::safeCast(node->data);

			if(owner == component->owner)
			{
				ComponentManager::destroyComponent(componentManager, owner, component);
			}
		}
		
	}
}
//---------------------------------------------------------------------------------------------------------
static VirtualList ComponentManager::getComponents(SpatialObject owner, uint32 componentType)
{
	ComponentManager componentManager = ComponentManager::getManager(componentType);

	if(NULL == componentManager)
	{
		return NULL;
	}

	if(NULL == owner->components[componentType])
	{
		owner->components[componentType] = new VirtualList();
	}
	else
	{
		VirtualList::clear(owner->components[componentType]);
	}

	return ComponentManager::doGetComponents(componentManager, owner, owner->components[componentType]);
}
//---------------------------------------------------------------------------------------------------------
static bool ComponentManager::getComponentsOfClass(SpatialObject owner, ClassPointer classPointer, VirtualList components, uint32 componentType)
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
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
static int16 ComponentManager::getSpecDisplacementForComponentType(uint32 componentType)
{
	switch (componentType)
	{
		case kColliderComponent:

			return sizeof(ColliderSpec) >> 2;
			break;

		case kSpriteComponent:
		case kWireframeComponent:
		case kBehaviorComponent:

			return sizeof(ComponentSpec*);
			break;
	}

	return 1;
}
//---------------------------------------------------------------------------------------------------------


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ComponentManager::propagateCommand(int32 command, SpatialObject owner, ...)
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
//---------------------------------------------------------------------------------------------------------
uint16 ComponentManager::getCount(SpatialObject owner)
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
//---------------------------------------------------------------------------------------------------------
Component ComponentManager::createComponent(SpatialObject owner, const ComponentSpec* componentSpec)
{
	if(kComponentTypes <= componentSpec->componentType)
	{
		return NULL;
	}

	if(!isDeleted(owner->components[ componentSpec->componentType]))
	{
		delete owner->components[ componentSpec->componentType];
		owner->components[ componentSpec->componentType] = NULL;
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------
void ComponentManager::destroyComponent(SpatialObject owner, Component component) 
{
	if(isDeleted(component))
	{
		return;
	}

	if(NULL == component->componentSpec || kComponentTypes <= component->componentSpec->componentType)
	{
		return;
	}

	if(!isDeleted(owner->components[ component->componentSpec->componentType]))
	{
		delete owner->components[ component->componentSpec->componentType];
		owner->components[ component->componentSpec->componentType] = NULL;
	}
}
//---------------------------------------------------------------------------------------------------------
VirtualList ComponentManager::doGetComponents(SpatialObject owner, VirtualList components)
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
//---------------------------------------------------------------------------------------------------------
bool ComponentManager::isAnyVisible(SpatialObject owner __attribute((unused)))
{
	return false;
}
//---------------------------------------------------------------------------------------------------------


//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ComponentManager::constructor()
{
	Base::constructor();

	this->components = new VirtualList();
}
//---------------------------------------------------------------------------------------------------------
void ComponentManager::destructor()
{
	if(!isDeleted(this->components))
	{
		VirtualList::deleteData(this->components);
		delete this->components;
		this->components = NULL;
	}

	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
