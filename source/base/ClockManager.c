/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Printing.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define ClockManager_ATTRIBUTES																			\
		Object_ATTRIBUTES																				\
		/**
		 * @var VirtualList	clocks
		 * @brief			registered clocks
		 * @memberof		ClockManager
		 */																								\
		VirtualList clocks;																				\

/**
 * @class	ClockManager
 * @extends Object
 * @ingroup base
 */
__CLASS_DEFINITION(ClockManager, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//class's constructor
static void ClockManager_constructor(ClockManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			ClockManager_getInstance()
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
static void __attribute__ ((noinline)) ClockManager_constructor(ClockManager this)
{
	__CONSTRUCT_BASE(Object);

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
void ClockManager_destructor(ClockManager this)
{
	ASSERT(this, "ClockManager::destructor: null this");

	VirtualNode node = this->clocks->head;

	// destroy all registered clocks
	for(; node ; node = node->next)
	{
		Clock_destructor(__SAFE_CAST(Clock, node->data));
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
void ClockManager_register(ClockManager this, Clock clock)
{
	ASSERT(this, "ClockManager::register: null this");

	if(!VirtualList_find(this->clocks, clock))
	{
		VirtualList_pushFront(this->clocks, clock);
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
void ClockManager_unregister(ClockManager this, Clock clock)
{
	ASSERT(this, "ClockManager::unregister: null this");

	VirtualList_removeElement(this->clocks, clock);
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
void ClockManager_update(ClockManager this, u32 millisecondsElapsed)
{
	ASSERT(this, "ClockManager::update: null this");
	ASSERT(this->clocks, "ClockManager::update: null clocks list");

	VirtualNode node = this->clocks->head;

	// update all registered clocks
	for(; node ; node = node->next)
	{
		Clock_update(__SAFE_CAST(Clock, node->data), millisecondsElapsed);
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
void ClockManager_reset(ClockManager this)
{
	ASSERT(this, "ClockManager::reset: null this");
	ASSERT(this->clocks, "ClockManager::reset: null clocks list");

	VirtualNode node = this->clocks->head;

	// update all registered clocks
	for(; node ; node = node->next)
	{
		Clock_reset(__SAFE_CAST(Clock, node->data));
	}
}
