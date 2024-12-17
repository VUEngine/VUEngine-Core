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
#include <VirtualList.h>

#include "ComponentManager.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Component;
friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ComponentManager::propagateCommand(int32 command, SpatialObject owner, ...)
{
	va_list args;
	va_start(args, owner);

	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Component component = Component::safeCast(node->data);

		if(NULL != owner && owner != component->owner)
		{
			continue;
		}

		Component::handleCommand(component, command, args);
	}

	va_end(args);
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
