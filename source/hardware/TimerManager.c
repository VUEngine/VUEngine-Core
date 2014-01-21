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

#include <TimerManager.h>
#include <HardwareManager.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define TimerManager_ATTRIBUTES											\
																		\
	/* super's attributes */											\
	Object_ATTRIBUTES;													\
																		\
	/*  */																\
	u8 tcrValue;														\
	

// define the TimerManager
__CLASS_DEFINITION(TimerManager);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void TimerManager_constructor(TimerManager this);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */




//////////////////////////////////////////////////////////////////////////////////////////////////////////////

__SINGLETON(TimerManager);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void TimerManager_constructor(TimerManager this){
	
	__CONSTRUCT_BASE(Object);
	
	this->tcrValue = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void TimerManager_destructor(TimerManager this){

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enable interruptions
void TimerManager_setInterrupt(TimerManager this, int value){

	if (value){ 
		
		this->tcrValue |= TIMER_INT;
	}
	else {
		
		this->tcrValue &= ~TIMER_INT;
	}
	
	HW_REGS[TCR] = this->tcrValue;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enable timer
void TimerManager_enable(TimerManager this, int value) {
	
	if (value){
		
		this->tcrValue |= TIMER_ENB;
	}		
	else{
		
		this->tcrValue &= ~TIMER_ENB;		
	}
	
	HW_REGS[TCR] = this->tcrValue;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get time
u16 TimerManager_getTime(TimerManager this) {

	return (HW_REGS[TLR] | (HW_REGS[THR] << 8));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sest time
void TimerManager_setTime(TimerManager this, u16 time) {
	
	HW_REGS[TLR] = (time & 0xFF);
	HW_REGS[THR] = (time >> 8);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set frequency
void TimerManager_setFrequency(TimerManager this, int frequency) {

	if (frequency){
		
		this->tcrValue |= TIMER_20US;
	}
	else {
		this->tcrValue &= ~TIMER_20US;
	}
	
	HW_REGS[TCR] = this->tcrValue;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get stat
int TimerManager_getStat(TimerManager this) {

	return (HW_REGS[TCR] & TIMER_ZSTAT);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clear stat
void TimerManager_clearStat(TimerManager this) {
	
	HW_REGS[TCR] = (this->tcrValue | TIMER_ZCLR);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initialize
void TimerManager_initialize(TimerManager this) {

	//setup timer interrupts
	HardwareManager_setInterruptLevel(HardwareManager_getInstance(), 0);
	//setup timer
	TimerManager_setFrequency(this, TIMER_100US);
	TimerManager_setTime(this, TIME_MS(__TIMER_RESOLUTION));
	TimerManager_clearStat(this);
	TimerManager_setInterrupt(this, 1);
	TimerManager_enable(this, 1);
}

