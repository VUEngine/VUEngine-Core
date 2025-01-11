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

static void ClockManager::reset()
{
	ClockManager clockManager = ClockManager::getInstance(NULL);

	ASSERT(clockManager->clocks, "ClockManager::reset: null clocks list");

	VirtualNode node = clockManager->clocks->head;

	// Update all registered clocks
	for(; node ; node = node->next)
	{
		Clock::reset(node->data);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ClockManager::register(Clock clock)
{
	ClockManager clockManager = ClockManager::getInstance(NULL);

	if(!VirtualList::find(clockManager->clocks, clock))
	{
		VirtualList::pushFront(clockManager->clocks, clock);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ClockManager::unregister(Clock clock)
{
	ClockManager clockManager = ClockManager::getInstance(NULL);

	VirtualList::removeData(clockManager->clocks, clock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ClockManager::update(uint32 elapsedMilliseconds)
{
	ClockManager clockManager = ClockManager::getInstance(NULL);

	ASSERT(clockManager->clocks, "ClockManager::update: null clocks list");

	VirtualNode node = clockManager->clocks->head;

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
