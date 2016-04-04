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

#ifndef CLOCK_H_
#define CLOCK_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __EVENT_SECOND_CHANGED	"secondChanged"
#define __EVENT_MINUTE_CHANGED	"minuteChanged"
#define __EVENT_HOUR_CHANGED	"hourChanged"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Clock_ATTRIBUTES																				\
																										\
	/* super's attributes */																			\
	Object_ATTRIBUTES;																					\
																										\
	/* time elapsed */																					\
	u32 milliSeconds;																					\
																										\
	/* register */																						\
	u32 previousSecond;																					\
																										\
	/* register */																						\
	u32 previousMinute;																					\
																										\
	/* flag to pause the clock */																		\
	bool paused;																						\

// declare the virtual methods
#define Clock_METHODS																					\
		Object_METHODS																					\
		__VIRTUAL_DEC(update);																			\

// declare the virtual methods which are redefined
#define Clock_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Clock, update);														\

// declare a Clock
__CLASS(Clock);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Clock);

void Clock_destructor(Clock this);
void Clock_delay(Clock this, int milliSeconds);
void Clock_print(Clock this, int col, int row, const char* font);
void Clock_update(Clock this, u32 ticks);
void Clock_interrupt();
void Clock_reset(Clock this);
u32 Clock_getMilliSeconds(Clock this);
u32 Clock_getSeconds(Clock this);
u32 Clock_getMinutes(Clock this);
u32 Clock_getTime(Clock this);
u32 Clock_getElapsedTime(Clock this);
int Clock_getTimeInCurrentSecond(Clock this);
void Clock_setTimeInSeconds(Clock this, float totalSeconds);
void Clock_setTime(Clock this, int hours, int minutes, int seconds);
void Clock_start(Clock this);
void Clock_stop(Clock this);
void Clock_pause(Clock this, bool paused);
bool Clock_isPaused(Clock this);


#endif