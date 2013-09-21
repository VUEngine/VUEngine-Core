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

#ifndef CLOCK_H_
#define CLOCK_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define Clock_ATTRIBUTES			\
									\
	/* super's attributes */		\
	Object_ATTRIBUTES;				\
									\
	/* time elapsed */				\
	u32 miliSeconds;				\
									\
	/* flag to pause the clock */	\
	int paused;

// declare the virtual methods
#define Clock_METHODS								\
		Object_METHODS								\
		__VIRTUAL_DEC(update);
	


// declare the virtual methods which are redefined
#define Clock_SET_VTABLE(ClassName)					\
		Object_SET_VTABLE(ClassName)				\
		__VIRTUAL_SET(ClassName, Clock, update);


// declare a Clock
__CLASS(Clock);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(Clock);

// class's destructor
void Clock_destructor(Clock this);

// time delay
void Clock_delay(Clock this, int miliSeconds);

// print formated class's attributes's states
void Clock_print(Clock this, int col, int row);

// called on each timer interrupt
void Clock_update(Clock this, u32 ticks);

// function to call in hardware timer interrupt
void Clock_interrupt();

// reset clock's attributes
void Clock_reset(Clock this);

// retrieve clock's miliseconds
int Clock_getMiliSeconds(Clock this);

// retrieve clock's minutes
int Clock_getMinutes(Clock this);

// retrieve clock's seconds
int Clock_getSeconds(Clock this);

// retrieve clock's total elapsed time in seconds
u32 Clock_getTime(Clock this);

// retrieve current elapsed second (fraction) in the current second
int Clock_getTimeInCurrentSecond(Clock this);

// set clock's total elapsed time from seconds paramenters 
void Clock_setTimeInSeconds(Clock this, float totalSeconds);

// set clock's total elapsed time
void Clock_setTime(Clock this, int hours, int minutes, int seconds);

// start the clock
void Clock_start(Clock this);

// stop the clock
void Clock_stop(Clock this);

// pause the clock
void Clock_pause(Clock this, int paused);

// whether the clock is running or not
int Clock_isPaused(Clock this);

#endif /*CLOCK_H_*/
