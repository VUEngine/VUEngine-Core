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
#include <VirtualList.h>

#include "BehaviorManager.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Behavior;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
Behavior BehaviorManager::createComponent(SpatialObject owner, const BehaviorSpec* behaviorSpec)
{
	if(NULL == behaviorSpec)
	{
		return NULL;
	}

	Base::createComponent(this, owner, (ComponentSpec*)behaviorSpec);

	Behavior behavior = ((Behavior (*)(SpatialObject, const BehaviorSpec*)) ((ComponentSpec*)behaviorSpec)->allocator)(owner, behaviorSpec);

	VirtualList::pushBack(this->components, behavior);

	return behavior;
}
//---------------------------------------------------------------------------------------------------------
void BehaviorManager::destroyComponent(SpatialObject owner, Behavior behavior) 
{
	if(isDeleted(behavior))
	{
		return;
	}

	Base::destroyComponent(this, owner, Component::safeCast(behavior));

	VirtualList::removeData(this->components, behavior);

	delete behavior;
}
//---------------------------------------------------------------------------------------------------------
void BehaviorManager::reset()
{	
	BehaviorManager::cleanUp(this);
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void BehaviorManager::constructor()
{
	Base::constructor();
}
//---------------------------------------------------------------------------------------------------------
void BehaviorManager::destructor()
{
	BehaviorManager::cleanUp(this);

	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void BehaviorManager::cleanUp()
{
	if(!isDeleted(this->components))
	{
		VirtualList::deleteData(this->components);
	}
}
//---------------------------------------------------------------------------------------------------------
