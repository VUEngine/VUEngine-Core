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
        /* super's attributes */																		\
        Object_ATTRIBUTES;																				\
        /* register clocks */																			\
        VirtualList clocks;																				\
        /* */																							\
        u32 ticks;																						\

// define the manager
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


// class's constructor
static void ClockManager_constructor(ClockManager this)
{
	__CONSTRUCT_BASE(Object);

	// create the clock list
    this->clocks = __NEW(VirtualList);

	this->ticks = 0;
}

// class's destructor
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

// register a clock
void ClockManager_register(ClockManager this, Clock clock)
{
	ASSERT(this, "ClockManager::register: null this");

	VirtualList_pushFront(this->clocks, clock);
}

// remove a clock
void ClockManager_unregister(ClockManager this, Clock clock)
{
	ASSERT(this, "ClockManager::unregister: null this");

	VirtualList_removeElement(this->clocks, clock);
}

// update clocks
void ClockManager_update(ClockManager this, u32 ticksElapsed)
{
	ASSERT(this, "ClockManager::update: null this");
	ASSERT(this->clocks, "ClockManager::update: null clocks list");

	//disable interrupts
	TimerManager_setInterrupt(TimerManager_getInstance(), false);

	static u32 previousSecond = 0;

    VirtualNode node = this->clocks->head;

    // update all registered clocks
    for(; node ; node = node->next)
    {
        Clock_update(__SAFE_CAST(Clock, node->data), ticksElapsed);
    }

	// update tick count
	this->ticks += ticksElapsed;
	u32 currentSecond = (u32)((this->ticks) / __MILLISECONDS_IN_SECOND);

	//if second has changed, set frame rate
    if(previousSecond < currentSecond)
    {
        previousSecond = currentSecond;
    	FrameRate frameRate = FrameRate_getInstance();

#ifdef __DEBUG
    	Printing_text(Printing_getInstance(), "DEBUG MODE", 0, (__SCREEN_HEIGHT >> 3) - 1, NULL);
#endif

#ifdef __PRINT_FRAMERATE
        if(!Game_isInSpecialMode(Game_getInstance()))
        {
            FrameRate_print(frameRate, 0, 0);
        }
#endif

#ifdef __PRINT_MEMORY_POOL_STATUS
        if(!Game_isInSpecialMode(Game_getInstance()))
        {
#ifdef __PRINT_DETAILED_MEMORY_POOL_STATUS
		    MemoryPool_printDetailedUsage(MemoryPool_getInstance(), 30, 1);
#else
            MemoryPool_printResumedUsage(MemoryPool_getInstance(), 40, 1);
#endif
        }
#endif

#ifdef __ALERT_STACK_OVERFLOW
        if(!Game_isInSpecialMode(Game_getInstance()))
        {
            HardwareManager_printStackStatus(HardwareManager_getInstance(), (__SCREEN_WIDTH >> 3) - 10, 0, true);
        }
#endif
        //reset frame rate counters
        FrameRate_reset(frameRate);

#ifdef __PROFILING
        Game_showProfiling(Game_getInstance());
#endif
    }

    //disable interrupts
    TimerManager_setInterrupt(TimerManager_getInstance(), true);
}

// reset clocks
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

	this->ticks = 0;
}
