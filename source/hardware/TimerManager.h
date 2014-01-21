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

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_


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
 * 											DEFINES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//use with 20us timer (range = 0 to 1300)
#define TIME_US(n)		(((n)/20)-1)

//use with 100us timer (range = 0 to 6500, and 0 to 6.5)
#define TIME_MS(n)		(((n)*10)-1)
#define TIME_SEC(n)		(((n)*10000)-1)

#define TIMER_ENB		0x01
#define TIMER_ZSTAT		0x02
#define TIMER_ZCLR		0x04
#define TIMER_INT		0x08
#define TIMER_20US		0x10
#define TIMER_100US		0x00


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */
// declare the virtual methods
#define TimerManager_METHODS							\
		Object_METHODS									\


// declare the virtual methods which are redefined
#define TimerManager_SET_VTABLE(ClassName)						\
		Object_SET_VTABLE(ClassName)								\


__CLASS(TimerManager);

 


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it is a singleton!
TimerManager TimerManager_getInstance();

// class's destructor
void TimerManager_destructor(TimerManager this);

// enable interruptions
void TimerManager_setInterrupt(TimerManager this, int value);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enable timer
void TimerManager_enable(TimerManager this, int value);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get time
u16 TimerManager_getTime(TimerManager this);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sest time
void TimerManager_setTime(TimerManager this, u16 time);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set frequency
void TimerManager_setFrequency(TimerManager this, int frequency);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get stat
int TimerManager_getStat(TimerManager this);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clear stat
void TimerManager_clearStat(TimerManager this);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initialize
void TimerManager_initialize(TimerManager this);

#endif /*TIMER_MANAGER_H_*/
