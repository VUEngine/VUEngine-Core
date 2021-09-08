/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ClockManager.h>
#include <FrameRate.h>
#include <Game.h>
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
