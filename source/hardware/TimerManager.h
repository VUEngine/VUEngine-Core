/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

// use with 100us timer (range = 0 to 6500, and 0 to 6.5)
#define __TIME_MS(n)		(((n)*10)-1)

#define __TIMER_ENB			0x01
#define __TIMER_ZSTAT		0x02
#define __TIMER_ZCLR		0x04
#define __TIMER_INT			0x08
#define __TIMER_100US		0x00


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
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
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

TimerManager TimerManager_getInstance();

void TimerManager_destructor(TimerManager this);
void TimerManager_enable(TimerManager this, bool flag);
u32 TimerManager_getMillisecondsElapsed(TimerManager this);
u32 TimerManager_getTotalMillisecondsElapsed(TimerManager this);
u32 TimerManager_resetMilliseconds(TimerManager this);
void TimerManager_setTime(TimerManager this, u16 time);
void TimerManager_setFrequency(TimerManager this, int frequency);
int TimerManager_getStat(TimerManager this);
void TimerManager_clearStat(TimerManager this);
void TimerManager_initialize(TimerManager this);
void TimerManager_wait(TimerManager this, u32 milliSeconds);
void TimerManager_repeatMethodCall(TimerManager this, u32 callTimes, u32 duration, Object object, void (*method)(Object, u32));


#endif
