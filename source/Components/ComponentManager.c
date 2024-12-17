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
#include <BehaviorManager.h>
#include <Collider.h>
#include <CollisionManager.h>
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
friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void ComponentManager::createComponents(SpatialObject owner, ComponentSpec** componentSpecsDirectory[])
{
	for(int16 componentType = 0; componentType < kComponentTypes; componentType++)
	{
		ComponentManager componentManager = ComponentManager::getManager(componentType);

		if(NULL == componentManager)
		{
			continue;
		}

		if(0 < ComponentManager::getCount(componentManager, owner))
		{
			continue;
		}

		ComponentSpec** componentSpecs = componentSpecsDirectory[componentType];
		int16 displacement = ComponentManager::getSpecDisplacementForComponentType(componentType);

		if(NULL != componentSpecs)
		{
			if(sizeof(ComponentSpec*) != displacement)
			{
				ComponentSpec* componentSpec = &((ComponentSpec*)componentSpecs)[0];

				for(; NULL != componentSpec && NULL != componentSpec->allocator; componentSpec += displacement)
				{
					ComponentManager::createComponent(componentManager, owner, componentSpec);
				}
			}
			else
			{
				for(int16 i = 0; NULL != componentSpecs[i] && NULL != componentSpecs[i]->allocator; i++)
				{
					ComponentManager::createComponent(componentManager, owner, componentSpecs[i]);
				}
			}
		}
	}
}
//---------------------------------------------------------------------------------------------------------
static void ComponentManager::destroyComponents(SpatialObject owner, VirtualList components[], uint32 componentType)
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

			if(!isDeleted(components[i]))
			{
				delete components[i];
				components[i] = NULL;
			}

			for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
			{
				Component component = Component::safeCast(node->data);

				if(owner == component->owner)
				{
					ComponentManager::destroyComponent(componentManager, component);
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

		if(!isDeleted(components[componentType]))
		{
			delete components[componentType];
			components[componentType] = NULL;
		}

		for(VirtualNode node = componentManager->components->head; NULL != node; node = node->next)
		{
			Component component = Component::safeCast(node->data);

			if(owner == component->owner)
			{
				ComponentManager::destroyComponent(componentManager, component);
			}
		}
		
	}
}
//---------------------------------------------------------------------------------------------------------
static Component ComponentManager::addComponent(SpatialObject owner, VirtualList components[], ComponentSpec* componentSpec, uint32 componentType)
{
	ComponentManager componentManager = ComponentManager::getManager(componentType);

	if(NULL == componentManager)
	{
		return NULL;
	}

	if(!isDeleted(components[componentType]))
	{
		delete components[componentType];
		components[componentType] = NULL;
	}

	return ComponentManager::createComponent(componentManager, owner, componentSpec);
}
//---------------------------------------------------------------------------------------------------------
static void ComponentManager::removeComponent(VirtualList components[], Component component)
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

	if(!isDeleted(components[componentType]))
	{
		delete components[componentType];
		components[componentType] = NULL;
	}

	ComponentManager::destroyComponent(componentManager, component);
}
//---------------------------------------------------------------------------------------------------------
static void ComponentManager::addComponents(SpatialObject owner, VirtualList components[], ComponentSpec** componentSpecs, uint32 componentType, bool destroyOldComponents)
{
	ComponentManager componentManager = ComponentManager::getManager(componentType);

	if(NULL == componentManager)
	{
		return;
	}

	if(destroyOldComponents)
	{
		ComponentManager::destroyComponents(owner, components, componentType);
	}

	for(int32 i = 0; NULL != componentSpecs[i] && NULL != componentSpecs[i]->allocator; i++)
	{
		ComponentManager::createComponent(componentManager, owner, componentSpecs[i]);
	}
}
//---------------------------------------------------------------------------------------------------------
static VirtualList ComponentManager::getComponents(SpatialObject owner, VirtualList components[], uint32 componentType)
{
	ComponentManager componentManager = ComponentManager::getManager(componentType);

	if(NULL == componentManager)
	{
		return NULL;
	}

	if(NULL == components[componentType])
	{
		components[componentType] = new VirtualList();
	}
	else
	{
		VirtualList::clear(components[componentType]);
	}

	return ComponentManager::doGetComponents(componentManager, owner, components[componentType]);
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

			return ComponentManager::safeCast(VUEngine::getCollisionManager(VUEngine::getInstance()));	
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

	return kComponentTypes;
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
