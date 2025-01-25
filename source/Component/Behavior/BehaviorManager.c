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
#include <VirtualList.h>

#include "BehaviorManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Behavior;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 BehaviorManager::getType()
{
	return kBehaviorComponent;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BehaviorManager::enable()
{
	Base::enable(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BehaviorManager::disable()
{
	Base::disable(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Behavior BehaviorManager::instantiateComponent(Entity owner, const BehaviorSpec* behaviorSpec)
{
	if(NULL == behaviorSpec)
	{
		return NULL;
	}

	Base::instantiateComponent(this, owner, (ComponentSpec*)behaviorSpec);

	Behavior behavior = ((Behavior (*)(Entity, const BehaviorSpec*)) ((ComponentSpec*)behaviorSpec)->allocator)(owner, behaviorSpec);

	VirtualList::pushBack(this->components, behavior);

	return behavior;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BehaviorManager::deinstantiateComponent(Entity owner, Behavior behavior) 
{
	if(isDeleted(behavior))
	{
		return;
	}

	Base::deinstantiateComponent(this, owner, Component::safeCast(behavior));

	VirtualList::removeData(this->components, behavior);

	delete behavior;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BehaviorManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BehaviorManager::destructor()
{
	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
