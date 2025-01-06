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

#include <DebugConfig.h>
#include <Stopwatch.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "StopwatchManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void StopwatchManager::reset()
{
	StopwatchManager stopwatchManager = StopwatchManager::getInstance();

	VirtualNode node = stopwatchManager->stopwatchs->head;

	// Update all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::reset(node->data);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void StopwatchManager::register(Stopwatch clock)
{
	StopwatchManager stopwatchManager = StopwatchManager::getInstance();

	if(!VirtualList::find(stopwatchManager->stopwatchs, clock))
	{
		VirtualList::pushFront(stopwatchManager->stopwatchs, clock);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void StopwatchManager::unregister(Stopwatch clock)
{
	StopwatchManager stopwatchManager = StopwatchManager::getInstance();

	VirtualList::removeData(stopwatchManager->stopwatchs, clock);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void StopwatchManager::update()
{
	StopwatchManager stopwatchManager = StopwatchManager::getInstance();

	VirtualNode node = stopwatchManager->stopwatchs->head;

	// Update all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::update(node->data);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StopwatchManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Create the clock list
	this->stopwatchs = new VirtualList();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StopwatchManager::destructor()
{
	VirtualNode node = this->stopwatchs->head;

	// Destroy all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::destructor(node->data);
	}

	// Clear my list
	delete this->stopwatchs;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
