/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <TimerManager.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <SoundManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define TimerManager_ATTRIBUTES																			\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/*  */																							\
		u32 milliseconds;																				\
		/*  */																							\
		u8 tcrValue;																					\

/**
 * @class	TimerManager
 * @extends Object
 * @ingroup hardware
 */
__CLASS_DEFINITION(TimerManager, Object);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

// use with 20us timer (range = 0 to 1300)
#define __TIME_US(n)		(((n)*50)-1)

// use with 100us timer (range = 0 to 6500, and 0 to 6.5)
#define __TIME_MS(n)		(((n)*10)-1)
#define __TIME_SEC(n)		(((n)*10000)-1)

#define __TIMER_ENB			0x01
#define __TIMER_ZSTAT		0x02
#define __TIMER_ZCLR		0x04
#define __TIMER_INT			0x08
#define __TIMER_20US		0x10
#define __TIMER_100US		0x00

#if __TIMER_FREQUENCY == __TIMER_20US
#define __TIMER_RESOLUTION_FUNCTION __TIME_US
#else
#if __TIMER_FREQUENCY == __TIMER_100US
#define __TIMER_RESOLUTION_FUNCTION __TIME_MS
#endif
#endif


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void TimerManager_constructor(TimerManager this);


// use static globals instead of class' members to avoid dereferencing
static TimerManager _timerManager;
static SoundManager _soundManager;

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			TimerManager_getInstance()
 * @memberof	TimerManager
 * @public
 *
 * @return		TimerManager instance
 */
__SINGLETON(TimerManager);

/**
 * Class constructor
 *
 * @memberof	TimerManager
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) TimerManager_constructor(TimerManager this)
{
	ASSERT(this, "TimerManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->tcrValue = 0;
	this->milliseconds = 0;

	_timerManager = this;
	_soundManager = SoundManager_getInstance();
}

/**
 * Class destructor
 *
 * @memberof	TimerManager
 * @public
 *
 * @param this	Function scope
 */
void TimerManager_destructor(TimerManager this)
{
	ASSERT(this, "TimerManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Initialize manager
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 */
void TimerManager_initialize(TimerManager this)
{
	ASSERT(this, "TimerManager::initialize: null this");

	//setup timer interrupts
	HardwareManager_setInterruptLevel(HardwareManager_getInstance(), 0);
	//setup timer
	TimerManager_setFrequency(this, __TIMER_FREQUENCY);
	TimerManager_setTime(this, __TIMER_RESOLUTION_FUNCTION(__TIMER_RESOLUTION));
	TimerManager_clearStat(this);
	TimerManager_enable(this, true);
}

/**
 * Enable / disable interrupt
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 * @param flag		Bool to enable or disable
 */
void TimerManager_enable(TimerManager this, bool flag)
{
	ASSERT(this, "TimerManager::enable: null this");

	if(flag)
	{
		this->tcrValue |= __TIMER_INT;
	}
	else
	{
		this->tcrValue &= ~__TIMER_INT;
	}

	_hardwareRegisters[__TCR] = this->tcrValue;
}

/**
 * Interrupt handler
 *
 * @memberof		TimerManager
 * @public
 */
void TimerManager_interruptHandler(void)
{
	//disable interrupts
	TimerManager_enable(_timerManager, false);

#ifdef __ALERT_STACK_OVERFLOW
	HardwareManager_checkStackStatus(HardwareManager_getInstance());
#endif

	// update clocks
	_timerManager->milliseconds += __TIMER_RESOLUTION;

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
	TimerManager_enable(_timerManager, true);
}

/**
 * Retrieve the total milliseconds elapsed
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			Total milliseconds elapsed
 */
u32 TimerManager_getMillisecondsElapsed(TimerManager this)
{
	ASSERT(this, "TimerManager::getMillisecondsElapsed: null this");

	return this->milliseconds;
}

/**
 * Reset the total milliseconds elapsed
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 */
u32 TimerManager_resetMilliseconds(TimerManager this)
{
	ASSERT(this, "TimerManager::resetMilliseconds: null this");

	u32 milliseconds = this->milliseconds;

	this->milliseconds = 0;

	return milliseconds;
}

/**
 * Set Timer's time
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 * @param time		New time
 */
void TimerManager_setTime(TimerManager this __attribute__ ((unused)), u16 time)
{
	ASSERT(this, "TimerManager::setTime: null this");

	_hardwareRegisters[__TLR] = (time & 0xFF);
	_hardwareRegisters[__THR] = (time >> 8);
}

/**
 * Set Timer's frequency
 *
 * @memberof				TimerManager
 * @public
 *
 * @param this				Function scope
 * @param frequency			New frequency
 */
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

/**
 * Retrieve Timer's ZSTAT
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			ZSTAT
 */
int TimerManager_getStat(TimerManager this __attribute__ ((unused)))
{
	ASSERT(this, "TimerManager::getStat: null this");

	return (_hardwareRegisters[__TCR] & __TIMER_ZSTAT);
}

/**
 * Clear Timer's ZSTAT
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 */
void TimerManager_clearStat(TimerManager this)
{
	ASSERT(this, "TimerManager::clearStat: null this");

	_hardwareRegisters[__TCR] = (this->tcrValue | __TIMER_ZCLR);
}

/**
 * Produce a wait
 *
 * @memberof				TimerManager
 * @public
 *
 * @param this				Function scope
 * @param milliSeconds		Time to wait
 */
void TimerManager_wait(TimerManager this, u32 milliSeconds)
{
	ASSERT(this, "ClockManager::wait: null this");

	// declare as volatile to prevent the compiler to optimize currentTicks away
	// making the last assignment invalid
	volatile u32 currentMilliseconds = this->milliseconds;
	u32 waitStartTime = this->milliseconds;
	volatile u32 *milliseconds = (u32*)&this->milliseconds;

	while ((*milliseconds - waitStartTime) < milliSeconds);

	this->milliseconds = currentMilliseconds;
}

/**
 * Call a method a number of times during a certain lapse
 *
 * @memberof				TimerManager
 * @public
 *
 * @param this				Function scope
 * @param callTimes			Number of calls to produce during the total duration
 * @param duration			Time that must take the callTimes
 * @param object			Called method's scope
 * @param method			Method to call
 */
void TimerManager_repeatMethodCall(TimerManager this, u32 callTimes, u32 duration, Object object, void (*method)(Object, u32))
{
	if(object && method)
	{
		// declare as volatile to prevent the compiler to optimize currentMilliseconds away
		// making the last assignment invalid
		volatile u32 currentMilliseconds = this->milliseconds;

		u32 i = 0;

		for(; i < callTimes; i++)
		{
			TimerManager_wait(this, __TIMER_RESOLUTION * duration / callTimes);
			method(object, i);
		}

		this->milliseconds = currentMilliseconds;
	}
}
