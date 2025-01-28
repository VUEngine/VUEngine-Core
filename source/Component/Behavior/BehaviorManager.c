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
friend class VirtualList;
friend class VirtualNode;

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

Behavior BehaviorManager::create(Entity owner, const BehaviorSpec* behaviorSpec)
{
	if(NULL == behaviorSpec)
	{
		return NULL;
	}

	return ((Behavior (*)(Entity, const BehaviorSpec*)) ((ComponentSpec*)behaviorSpec)->allocator)(owner, behaviorSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void BehaviorManager::update()
{
	for(VirtualNode node = this->components->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		Behavior behavior = Behavior::safeCast(node->data);

		NM_ASSERT(!isDeleted(behavior), "BehaviorManager::update: deleted behavior");

		if(behavior->deleteMe)
		{
			VirtualList::removeNode(this->components, node);

			delete behavior;
			continue;
		}
	}
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
