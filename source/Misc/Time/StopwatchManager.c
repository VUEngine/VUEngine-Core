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

#include <Stopwatch.h>
#include <DebugConfig.h>
#include <Singleton.h>
#include <VirtualList.h>

#include "StopwatchManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void StopwatchManager::reset()
{
	if(isDeleted(this->stopwatches))
	{
		return;
	}

	for(VirtualNode node = this->stopwatches->head; node ; node = node->next)
	{
		Stopwatch::reset(node->data);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void StopwatchManager::register(Stopwatch stopwatch)
{
	if(isDeleted(this->stopwatches))
	{
		return;
	}

	if(!VirtualList::find(this->stopwatches, stopwatch))
	{
		VirtualList::pushFront(this->stopwatches, stopwatch);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void StopwatchManager::unregister(Stopwatch stopwatch)
{
	if(isDeleted(this->stopwatches))
	{
		return;
	}

	VirtualList::removeData(this->stopwatches, stopwatch);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void StopwatchManager::update()
{
	if(isDeleted(this->stopwatches))
	{
		return;
	}

	for(VirtualNode node = this->stopwatches->head; node ; node = node->next)
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

	// Create the stopwatch list
	this->stopwatches = new VirtualList();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void StopwatchManager::destructor()
{
	if(!isDeleted(this->stopwatches))
	{
		VirtualList stopwatches = this->stopwatches;
		this->stopwatches = NULL;

		VirtualList::deleteData(stopwatches);
		delete stopwatches;

		delete this->stopwatches;
	}


	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
