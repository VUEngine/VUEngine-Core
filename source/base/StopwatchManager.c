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

#include <DebugConfig.h>
#include <Stopwatch.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "StopwatchManager.h"


//=========================================================================================================
// CLASS'S DECLARATIONS
//=========================================================================================================

friend class VirtualList;
friend class VirtualNode;


//=========================================================================================================
// CLASS'S PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void StopwatchManager::reset()
{
	VirtualNode node = this->stopwatchs->head;

	// update all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::reset(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------
void StopwatchManager::register(Stopwatch clock)
{
	if(!VirtualList::find(this->stopwatchs, clock))
	{
		VirtualList::pushFront(this->stopwatchs, clock);
	}
}
//---------------------------------------------------------------------------------------------------------
void StopwatchManager::unregister(Stopwatch clock)
{
	VirtualList::removeData(this->stopwatchs, clock);
}
//---------------------------------------------------------------------------------------------------------
void StopwatchManager::update()
{
	VirtualNode node = this->stopwatchs->head;

	// update all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::update(node->data);
	}
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS'S PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void StopwatchManager::constructor()
{
	Base::constructor();

	// create the clock list
	this->stopwatchs = new VirtualList();
}
//---------------------------------------------------------------------------------------------------------
void StopwatchManager::destructor()
{
	VirtualNode node = this->stopwatchs->head;

	// destroy all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::destructor(node->data);
	}

	// clear my list
	delete this->stopwatchs;

	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
