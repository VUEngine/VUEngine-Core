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

#include <TimerManager.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <SoundManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define TimerManager_ATTRIBUTES																			\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /*  */																							\
        u32 ticks;																					    \
        /*  */																							\
        u8 tcrValue;																					\

// define the TimerManager
__CLASS_DEFINITION(TimerManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void TimerManager_constructor(TimerManager this);


// use static globals instead of class' members to avoid dereferencing
static TimerManager _timerManager;
static SoundManager _soundManager;

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(TimerManager);

// class's constructor
static void __attribute__ ((noinline)) TimerManager_constructor(TimerManager this)
{
	ASSERT(this, "TimerManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->tcrValue = 0;
	this->ticks = 0;

	_timerManager = this;
	_soundManager = SoundManager_getInstance();
}

// class's destructor
void TimerManager_destructor(TimerManager this)
{
	ASSERT(this, "TimerManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// enable interruptions
void TimerManager_setInterrupt(TimerManager this, int value)
{
	ASSERT(this, "TimerManager::setInterrupt: null this");

	if(value)
	{
		this->tcrValue |= __TIMER_INT;
	}
	else
	{
		this->tcrValue &= ~__TIMER_INT;
	}

	_hardwareRegisters[__TCR] = this->tcrValue;
}

// timer's interrupt handler
void TimerManager_interruptHandler(void)
{
	//disable interrupts
	TimerManager_setInterrupt(_timerManager, false);

#ifdef __ALERT_STACK_OVERFLOW
	HardwareManager_checkStackStatus(HardwareManager_getInstance());
#endif

	// update clocks
	_timerManager->ticks += __TIMER_RESOLUTION;

    // play sounds
	static u32 previousHundredthSecond = 0;
	static u32 currentHundredthSecond = 0;
	currentHundredthSecond += __TIMER_RESOLUTION;

    if(previousHundredthSecond < (u32)(currentHundredthSecond / 10))
    {
        previousHundredthSecond = (u32)(currentHundredthSecond / 10);
	    // update sounds
    	SoundManager_playSounds(_soundManager);
    }

	// enable interrupts
	TimerManager_setInterrupt(_timerManager, true);
}

u32 TimerManager_getTicks(TimerManager this)
{
	ASSERT(this, "TimerManager::getTicks: null this");

    return this->ticks;
}

u32 TimerManager_getAndResetTicks(TimerManager this)
{
	ASSERT(this, "TimerManager::getTicks: null this");

    u32 ticks = this->ticks;

    this->ticks = 0;

    return ticks;
}

// enable timer
void TimerManager_enable(TimerManager this, int value)
{
	ASSERT(this, "TimerManager::enable: null this");

	if(value)
	{
		this->tcrValue |= __TIMER_ENB;
	}
	else
	{
		this->tcrValue &= ~__TIMER_ENB;
	}

	_hardwareRegisters[__TCR] = this->tcrValue;
}

// get time
u16 TimerManager_getTime(TimerManager this __attribute__ ((unused)))
{
	ASSERT(this, "TimerManager::getTime: null this");

	return (_hardwareRegisters[__TLR] | (_hardwareRegisters[__THR] << 8));
}

// sest time
void TimerManager_setTime(TimerManager this __attribute__ ((unused)), u16 time)
{
	ASSERT(this, "TimerManager::setTime: null this");

	_hardwareRegisters[__TLR] = (time & 0xFF);
	_hardwareRegisters[__THR] = (time >> 8);
}

// set frequency
void TimerManager_setFrequency(TimerManager this, int frequency)
{
	ASSERT(this, "TimerManager::setFrequency: null this");

	if(frequency)
	{
		this->tcrValue |= __TIMER_20US;
	}
	else
	{
		this->tcrValue &= ~__TIMER_20US;
	}

	_hardwareRegisters[__TCR] = this->tcrValue;
}


// get stat
int TimerManager_getStat(TimerManager this __attribute__ ((unused)))
{
	ASSERT(this, "TimerManager::getStat: null this");

	return (_hardwareRegisters[__TCR] & __TIMER_ZSTAT);
}

// clear stat
void TimerManager_clearStat(TimerManager this)
{
	ASSERT(this, "TimerManager::clearStat: null this");

	_hardwareRegisters[__TCR] = (this->tcrValue | __TIMER_ZCLR);
}

// initialize
void TimerManager_initialize(TimerManager this)
{
	ASSERT(this, "TimerManager::initialize: null this");

	//setup timer interrupts
	HardwareManager_setInterruptLevel(HardwareManager_getInstance(), 0);
	//setup timer
	TimerManager_setFrequency(this, __TIMER_100US);
	TimerManager_setTime(this, __TIME_MS(__TIMER_RESOLUTION));
	TimerManager_clearStat(this);
	TimerManager_setInterrupt(this, true);
	TimerManager_enable(this, true);
}

// produce a wait
void TimerManager_wait(TimerManager this, u32 milliSeconds)
{
	ASSERT(this, "ClockManager::wait: null this");

    u32 currentTicks = this->ticks;
	u32 waitStartTime = this->ticks;
    u32 volatile *ticks = (u32*)&this->ticks;

    while ((*ticks - waitStartTime) < milliSeconds);

    this->ticks = currentTicks;
}

void TimerManager_repeatMethodCall(TimerManager this, u32 callTimes, u32 duration, Object object, void (*method)(Object, u32))
{
    if(object && method)
    {
        u32 currentTicks = this->ticks;

        u32 i = 0;

        for(; i < callTimes; i++)
        {
            TimerManager_wait(this, __TIMER_RESOLUTION * duration / callTimes);
            method(object, i);
        }

        this->ticks = currentTicks;
    }
}
