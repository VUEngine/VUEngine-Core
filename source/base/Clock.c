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

#include <Clock.h>
#include <ClockManager.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the Clock
__CLASS_DEFINITION(Clock);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void Clock_constructor(Clock this);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// always call these to macros next to each other
__CLASS_NEW_DEFINITION(Clock)
__CLASS_NEW_END(Clock);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Clock_constructor(Clock this){

	__CONSTRUCT_BASE(Object);
	
	// initialize time
	this->miliSeconds = 0;
	
	// initialize state
	this->paused = true;
	
	// register clock
	ClockManager_register(ClockManager_getInstance(), this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Clock_destructor(Clock this){
	
	// unregister the clock
	ClockManager_unregister(ClockManager_getInstance(), this);
	
	// destroy the super object
	__DESTROY_BASE(Object);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// time delay
void Clock_delay(Clock this, int miliSeconds){
	
	u32 time = this->miliSeconds;
	
	if(this->paused){
		
		return;
	}	
	else{
		
		u32 volatile *clockTime = (u32 *)&this->miliSeconds;
	
		while((*clockTime - time) < miliSeconds);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print formated class's attributes's states
void Clock_print(Clock this, int col, int row){

	int minutes = Clock_getMinutes(this);
	int seconds = Clock_getSeconds(this) - minutes * 60;
	
	int minutesPosition = col;
	int secondsPosition = col + 3;

	//print minutes
	if(minutes < 10){

		vbjPrintText("0", minutesPosition, row);
		minutesPosition++;
	}
	
	vbjPrintInt(minutes, minutesPosition, row);
	
	// print divisor
	vbjPrintText(":", secondsPosition - 1, row);
	
	//print seconds
	if(seconds < 10){

		vbjPrintText("0", secondsPosition, row);
		secondsPosition++;
	}

	vbjPrintInt(seconds, secondsPosition, row);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// called on each timer interrupt
void Clock_update(Clock this, u32 ticks){
	
	// increase count
	if(!this->paused){

		//calculate miliseconds
		this->miliSeconds += ticks;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset clock's attributes
void Clock_reset(Clock this){		

	this->miliSeconds = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve clock's miliseconds
int Clock_getMiliSeconds(Clock this){
	
	return this->miliSeconds;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve clock's minutes
int Clock_getMinutes(Clock this){
	
	return this->miliSeconds / (1000 * 60);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//retrieve clock's seconds
int Clock_getSeconds(Clock this){
	
	return this->miliSeconds / 1000 ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve clock's total elapsed time in seconds
u32 Clock_getTime(Clock this){
	
	return this->miliSeconds;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve current elapsed miliseconds in the current second
int Clock_getTimeInCurrentSecond(Clock this){
	
	return 1000 * (this->miliSeconds * 0.001f - F_FLOOR(this->miliSeconds * 0.001f));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set clock's total elapsed time from seconds paramenters 
void Clock_setTimeInSeconds(Clock this, float totalSeconds){
	
	this->miliSeconds = totalSeconds * 1000;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// start the clock
void Clock_start(Clock this){
	
	//Clock_reset(this);
	
	this->paused = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stop the clock
void Clock_stop(Clock this){
	
	Clock_reset(this);
	
	this->paused = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pause the clock
void Clock_pause(Clock this, int paused){
	
	this->paused = paused;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// whether the clock is running or not
int Clock_isPaused(Clock this){
	
	return this->paused;
}

