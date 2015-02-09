/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ClockManager.h>
#include <FrameRate.h>
#include <Game.h>
#include <SoundManager.h>
#include <HardwareManager.h>
#include <MessageDispatcher.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define ClockManager_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* register clocks */														\
	VirtualList clocks;															\
																				\
	/* */																		\
	u32 ticks;

// define the manager
__CLASS_DEFINITION(ClockManager, Object);


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
	__CONSTRUCT_BASE();

	// create the clock list
	this->clocks = NULL;

	this->ticks = 0;
}


// class's destructor
void ClockManager_destructor(ClockManager this)
{
	ASSERT(this, "ClockManager::destructor: null this");

	VirtualNode node = VirtualList_begin(this->clocks);

	// destroy all registered clocks
	for (; node ; node = VirtualNode_getNext(node))
	{
		Clock_destructor(__UPCAST(Clock, VirtualNode_getData(node)));
	}

	// clear my list
	VirtualList_clear(this->clocks);

	// allow a new construct
	__SINGLETON_DESTROY;
}


// register a clock
void ClockManager_register(ClockManager this, Clock clock)
{
	ASSERT(this, "ClockManager::register: null this");

	if (!this->clocks)
	{
		this->clocks = __NEW(VirtualList);
	}

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

	u32 previousSecond = this->ticks / __MILLISECONDS_IN_SECOND;

	if (this->clocks)
	{
		VirtualNode node = VirtualList_begin(this->clocks);

		// update all registered clocks
		for (; node ; node = VirtualNode_getNext(node))
		{
			Clock_update(__UPCAST(Clock, VirtualNode_getData(node)), ticksElapsed);
		}
	}

	// update tick count
	this->ticks += ticksElapsed;
	
    //if second has changed, set frame rate 
    if (previousSecond != (this->ticks / __MILLISECONDS_IN_SECOND))
    {
    		FrameRate frameRate = FrameRate_getInstance();
    		
#ifdef __PRINT_FRAMERATE
    		bool printFrameRate = !Game_isInSpecialMode(Game_getInstance());
    		int y = 0;
#ifdef __DEBUG
    		Printing_text(Printing_getInstance(), "DEBUG MODE", 0, 0);
    		y = 1;
#endif 	    		
	    	if (printFrameRate)
	    	{
	    		FrameRate_print(frameRate, 0, y);
	    	}
#endif
	    	//reset frame rate counters
			FrameRate_reset(frameRate);

			// no need to track this, so prevent a very unlikely overflow
	    	this->ticks = 0;
    }	

    // Play background music
    SoundManager_playBGM(SoundManager_getInstance());

    // Play sound effects
    SoundManager_playFxSounds(SoundManager_getInstance());
}

// update clocks
void ClockManager_reset(ClockManager this)
{
	ASSERT(this, "ClockManager::reset: null this");
	ASSERT(this->clocks, "ClockManager::reset: null clocks list");

	VirtualNode node = VirtualList_begin(this->clocks);

	// update all registered clocks
	for (; node ; node = VirtualNode_getNext(node))
	{
		Clock_reset(__UPCAST(Clock, VirtualNode_getData(node)));
	}

	this->ticks = 0;
}