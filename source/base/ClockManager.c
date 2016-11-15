/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
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
// 											CLASS'S DEFINITION
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
 */
__CLASS_DEFINITION(ClockManager, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//class's constructor
static void ClockManager_constructor(ClockManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

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

	VirtualList_pushFront(this->clocks, clock);
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
 * @memberof			ClockManager
 * @private
 *
 * @param this			Function scope
 * @param ticksElapsed	Miliseconds elapsed between calls
 */
void ClockManager_update(ClockManager this, u32 ticksElapsed)
{
	ASSERT(this, "ClockManager::update: null this");
	ASSERT(this->clocks, "ClockManager::update: null clocks list");

	VirtualNode node = this->clocks->head;

	// update all registered clocks
	for(; node ; node = node->next)
	{
		Clock_update(__SAFE_CAST(Clock, node->data), ticksElapsed);
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
