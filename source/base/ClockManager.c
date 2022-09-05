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

#include <ClockManager.h>
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
 * @fn			ClockManager::getInstance()
 * @memberof	ClockManager
 *
 * @return		ClockManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void ClockManager::constructor()
{
	Base::constructor();

	// create the clock list
	this->clocks = new VirtualList();
}

/**
 * Class destructor
 *
 * @private
 */
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

/**
 * Register a clock
 *
 * @private
 * @param clock Clock to register
 */
void ClockManager::register(Clock clock)
{
	if(!VirtualList::find(this->clocks, clock))
	{
		VirtualList::pushFront(this->clocks, clock);
	}
}

/**
 * Un-register a clock
 *
 * @param clock Clock to un-register
 */
void ClockManager::unregister(Clock clock)
{
	VirtualList::removeElement(this->clocks, clock);
}

/**
 * Update clocks
 *
 * @param millisecondsElapsed	Milliseconds elapsed between calls
 */
void ClockManager::update(uint32 millisecondsElapsed)
{
	ASSERT(this->clocks, "ClockManager::update: null clocks list");

	VirtualNode node = this->clocks->head;

	// update all registered clocks
	for(; node ; node = node->next)
	{
		Clock::update(node->data, millisecondsElapsed);
	}
}

/**
 * Reset registered clocks
 */
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
