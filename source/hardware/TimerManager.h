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

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 											MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// Defines as a pointer to a structure that's not defined here and so is not accessible to the outside world

// declare the virtual methods
#define TimerManager_METHODS(ClassName)																	\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define TimerManager_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(TimerManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

TimerManager TimerManager_getInstance();

void TimerManager_destructor(TimerManager this);
void TimerManager_setInterrupt(TimerManager this, int value);
u32 TimerManager_getMillisecondsElapsed(TimerManager this);
u32 TimerManager_resetMilliseconds(TimerManager this);
void TimerManager_enable(TimerManager this, int value);
void TimerManager_setTime(TimerManager this, u16 time);
void TimerManager_setFrequency(TimerManager this, int frequency);
int TimerManager_getStat(TimerManager this);
void TimerManager_clearStat(TimerManager this);
void TimerManager_initialize(TimerManager this);
void TimerManager_wait(TimerManager this, u32 milliSeconds);
void TimerManager_repeatMethodCall(TimerManager this, u32 callTimes, u32 duration, Object object, void (*method)(Object, u32));


#endif
