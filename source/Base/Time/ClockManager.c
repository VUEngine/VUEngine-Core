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

#include <Clock.h>
#include <DebugConfig.h>
#include <VirtualList.h>

#include "ClockManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ClockManager::reset()
{
	ASSERT(this->clocks, "ClockManager::reset: null clocks list");

	VirtualNode node = this->clocks->head;

	// Update all registered clocks
	for(; node ; node = node->next)
	{
		Clock::reset(node->data);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ClockManager::register(Clock clock)
{
	if(!VirtualList::find(this->clocks, clock))
	{
		VirtualList::pushFront(this->clocks, clock);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ClockManager::unregister(Clock clock)
{
	VirtualList::removeData(this->clocks, clock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ClockManager::update(uint32 elapsedMilliseconds)
{
	ASSERT(this->clocks, "ClockManager::update: null clocks list");

	VirtualNode node = this->clocks->head;

	// Update all registered clocks
	for(; node ; node = node->next)
	{
		Clock::update(node->data, elapsedMilliseconds);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ClockManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Create the clock list
	this->clocks = new VirtualList();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ClockManager::destructor()
{
	VirtualNode node = this->clocks->head;

	// Destroy all registered clocks
	for(; node ; node = node->next)
	{
		Clock::destructor(node->data);
	}

	// Clear my list
	delete this->clocks;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
