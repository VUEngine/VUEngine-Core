/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <StopwatchManager.h>
#include <FrameRate.h>
#include <HardwareManager.h>
#include <MessageDispatcher.h>
#include <TimerManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			StopwatchManager::getInstance()
 * @memberof	StopwatchManager
 *
 * @return		StopwatchManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void StopwatchManager::constructor()
{
	Base::constructor();

	// create the clock list
	this->stopwatchs = new VirtualList();
}

/**
 * Class destructor
 *
 * @private
 */
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

/**
 * Register a clock
 *
 * @private
 * @param clock Stopwatch to register
 */
void StopwatchManager::register(Stopwatch clock)
{
	if(!VirtualList::find(this->stopwatchs, clock))
	{
		VirtualList::pushFront(this->stopwatchs, clock);
	}
}

/**
 * Un-register a clock
 *
 * @param clock Stopwatch to un-register
 */
void StopwatchManager::unregister(Stopwatch clock)
{
	VirtualList::removeElement(this->stopwatchs, clock);
}

/**
 * Update stopwatchs
 *
 */
void StopwatchManager::update()
{
	VirtualNode node = this->stopwatchs->head;

	// update all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::update(node->data);
	}
}

/**
 * Reset registered stopwatchs
 */
void StopwatchManager::reset()
{
	VirtualNode node = this->stopwatchs->head;

	// update all registered stopwatchs
	for(; node ; node = node->next)
	{
		Stopwatch::reset(node->data);
	}
}
