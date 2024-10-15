/**
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

#include <Clock.h>
#include <DebugConfig.h>
#include <VirtualList.h>

#include "ClockManager.h"


//=========================================================================================================
// CLASS'S DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS'S PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ClockManager::register(Clock clock)
{
	if(!VirtualList::find(this->clocks, clock))
	{
		VirtualList::pushFront(this->clocks, clock);
	}
}
//---------------------------------------------------------------------------------------------------------

void ClockManager::unregister(Clock clock)
{
	VirtualList::removeElement(this->clocks, clock);
}
//---------------------------------------------------------------------------------------------------------
void ClockManager::update(uint32 elapsedMilliseconds)
{
	ASSERT(this->clocks, "ClockManager::update: null clocks list");

	VirtualNode node = this->clocks->head;

	// update all registered clocks
	for(; node ; node = node->next)
	{
		Clock::update(node->data, elapsedMilliseconds);
	}
}
//---------------------------------------------------------------------------------------------------------
void ClockManager::reset()
{
	ASSERT(this->clocks, "ClockManager::reset: null clocks list");

	VirtualNode node = this->clocks->head;

	// update all registered clocks
	for(; node ; node = node->next)
	{
		Clock::reset(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS'S PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ClockManager::constructor()
{
	Base::constructor();

	// create the clock list
	this->clocks = new VirtualList();
}
//---------------------------------------------------------------------------------------------------------
void ClockManager::destructor()
{
	VirtualNode node = this->clocks->head;

	// destroy all registered clocks
	for(; node ; node = node->next)
	{
		Clock::destructor(node->data);
	}

	// clear my list
	delete this->clocks;

	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
