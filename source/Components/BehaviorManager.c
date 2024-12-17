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
void BehaviorManager::reset()
{	
	BehaviorManager::cleanUp(this);
}
//---------------------------------------------------------------------------------------------------------
Behavior BehaviorManager::createComponent(SpatialObject owner, const BehaviorSpec* behaviorSpec)
{
	NM_ASSERT(NULL != behaviorSpec, "BehaviorManager::createBehavior: null behaviorSpec");
	NM_ASSERT(NULL != behaviorSpec->allocator, "BehaviorManager::createBehavior: no behavior allocator");

	Behavior behavior = ((Behavior (*)(SpatialObject, const BehaviorSpec*)) behaviorSpec->allocator)(owner, (BehaviorSpec*)behaviorSpec);
	ASSERT(!isDeleted(behavior), "BehaviorManager::createBehavior: failed creating behavior");

	VirtualList::pushBack(this->components, behavior);

	return behavior;
}
//---------------------------------------------------------------------------------------------------------
void BehaviorManager::destroyComponent(Behavior behavior)
{
	NM_ASSERT(!isDeleted(behavior), "BehaviorManager::destroyBehavior: trying to dispose dead behavior");
	NM_ASSERT(__GET_CAST(Behavior, behavior), "BehaviorManager::destroyBehavior: trying to dispose a non behavior");

	if(isDeleted(behavior))
	{
		return;
	}

	VirtualList::removeData(this->components, behavior);

	delete behavior;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void BehaviorManager::constructor()
{
	// construct base object
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
