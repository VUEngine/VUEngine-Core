/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

/**
 * @class	ClockManager
 * @extends Object
 * @ingroup base
 */
implements ClockManager : Object;
friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//class's constructor
static void ClockManager::constructor(ClockManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			ClockManager::getInstance()
 * @memberof	ClockManager
 * @public
 *
 * @return		ClockManager instance
 */
__SINGLETON(ClockManager);

/**
 * Class constructor
 *
 * @memberof	ClockManager
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) ClockManager::constructor(ClockManager this)
{
	ASSERT(this, "ClockManager::constructor: null this");

	Base::constructor();

	// create the clock list
	this->clocks = __NEW(VirtualList);
}

/**
 * Class destructor
 *
 * @memberof	ClockManager
 * @private
 *
 * @param this	Function scope
 */
void ClockManager::destructor(ClockManager this)
{
	ASSERT(this, "ClockManager::destructor: null this");

	VirtualNode node = this->clocks->head;

	// destroy all registered clocks
	for(; node ; node = node->next)
	{
		Clock::destructor(__SAFE_CAST(Clock, node->data));
	}

	// clear my list
	__DELETE(this->clocks);

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Register a clock
 *
 * @memberof	ClockManager
 * @private
 *
 * @param this	Function scope
 * @param clock Clock to register
 */
void ClockManager::register(ClockManager this, Clock clock)
{
	ASSERT(this, "ClockManager::register: null this");

	if(!VirtualList::find(this->clocks, clock))
	{
		VirtualList::pushFront(this->clocks, clock);
	}
}

/**
 * Un-register a clock
 *
 * @memberof	ClockManager
 * @private
 *
 * @param this	Function scope
 * @param clock Clock to un-register
 */
void ClockManager::unregister(ClockManager this, Clock clock)
{
	ASSERT(this, "ClockManager::unregister: null this");

	VirtualList::removeElement(this->clocks, clock);
}

/**
 * Update clocks
 *
 * @memberof					ClockManager
 * @private
 *
 * @param this					Function scope
 * @param millisecondsElapsed	Milliseconds elapsed between calls
 */
void ClockManager::update(ClockManager this, u32 millisecondsElapsed)
{
	ASSERT(this, "ClockManager::update: null this");
	ASSERT(this->clocks, "ClockManager::update: null clocks list");

	VirtualNode node = this->clocks->head;

	// update all registered clocks
	for(; node ; node = node->next)
	{
		Clock::update(__SAFE_CAST(Clock, node->data), millisecondsElapsed);
	}
}

/**
 * Reset registered clocks
 *
 * @memberof			ClockManager
 * @private
 *
 * @param this			Function scope
 */
void ClockManager::reset(ClockManager this)
{
	ASSERT(this, "ClockManager::reset: null this");
	ASSERT(this->clocks, "ClockManager::reset: null clocks list");

	VirtualNode node = this->clocks->head;

	// update all registered clocks
	for(; node ; node = node->next)
	{
		Clock::reset(__SAFE_CAST(Clock, node->data));
	}
}
