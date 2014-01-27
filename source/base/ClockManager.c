/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <ClockManager.h>
#include <FrameRate.h>
#include <Game.h>
#include <SoundManager.h>
#include <HardwareManager.h>



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define ClockManager_ATTRIBUTES				\
											\
	/* super's attributes */				\
	Object_ATTRIBUTES;						\
											\
	/* register clocks */					\
	VirtualList clocks;						\
											\
	/* */									\
	u32 ticks;

// define the manager
__CLASS_DEFINITION(ClockManager);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//class's constructor
static void ClockManager_constructor(ClockManager this);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ClockManager.c
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

__SINGLETON(ClockManager);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void ClockManager_constructor(ClockManager this){
	
	__CONSTRUCT_BASE(Object);

	// create the clock list
	this->clocks = NULL;
	
	this->ticks = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void ClockManager_destructor(ClockManager this){

	VirtualNode node = VirtualList_begin(this->clocks); 
	
	// destroy all registered clocks 
	for(; node ; node = VirtualNode_getNext(node)){
		
		Clock_destructor((Clock)VirtualNode_getData(node));		
	}

	// clear my liest
	VirtualList_clear(this->clocks);

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// register a clock
void ClockManager_register(ClockManager this, Clock clock){
	
	if(!this->clocks) {
		
		this->clocks = __NEW(VirtualList);
	}
	
	VirtualList_pushFront(this->clocks, clock);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// remove a clock
void ClockManager_unregister(ClockManager this, Clock clock){
	
	VirtualList_removeElement(this->clocks, clock);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update clocks
void ClockManager_update(ClockManager this, u32 ticksElapsed){

	u32 previousSecond = this->ticks / 1000;

	if(this->clocks) {
		
		VirtualNode node = VirtualList_begin(this->clocks);
		
		// update all registered clocks 
		for(; node ; node = VirtualNode_getNext(node)){
			
			Clock_update((Clock)VirtualNode_getData(node), ticksElapsed);
		}
	}
	
	
	// update tick count
	this->ticks += ticksElapsed;
	
    //if second has changed, set frame rate 
    if(previousSecond != (this->ticks / 1000)){
    	
#ifdef __DEBUG_0
    		
//	    	FrameRate_print(FrameRate_getInstance(), 0, 0);
	    	// get stack pointer
    		//Printing_hex(HW_REGS[SCR], 38, 4);
    		//Printing_hex(HardwareManager_readKeypad(HardwareManager_getInstance()), 38, 5);
	    	//Clock_print(_clock, 40, 0);
	    	//Clock_print(_inGameClock, 38, 0);
	    	//MemoryPool_printMemUsage(MemoryPool_getInstance(),0,0);
	    	//Game_printClassSizes(0, 0);
	    	//ParamTableManager_print(ParamTableManager_getInstance(),0,10);
	    	//GameWorld_printListsSize(GameEngine_getAuxGameWorld(GameEngine_getInstance()),0, 5);
#endif
			
			//reset frame rate counters
			FrameRate_reset(FrameRate_getInstance());		
    }	
    
    // Play background music 
    SoundManager_playBGM(SoundManager_getInstance());
    
    // Play sound effects 
    SoundManager_playFxSounds(SoundManager_getInstance());
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update clocks
void ClockManager_reset(ClockManager this){

	VirtualNode node = VirtualList_begin(this->clocks);
	// update all registered clocks 
	for(; node ; node = VirtualNode_getNext(node)){
		
		Clock_reset((Clock)VirtualNode_getData(node));
	}

	this->ticks = 0;
}