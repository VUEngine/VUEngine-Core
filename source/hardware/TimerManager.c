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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <debugConfig.h>
#include <TimerManager.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <SoundManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define TimerManager_ATTRIBUTES																			\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/*  */																							\
		u32 milliseconds;																				\
		/*  */																							\
		u32 totalMilliseconds;																				\
		/*  */																							\
		u8 tcrValue;																					\

/**
 * @class	TimerManager
 * @extends Object
 * @ingroup hardware
 */
__CLASS_DEFINITION(TimerManager, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void TimerManager::constructor(TimerManager this);
static void TimerManager::enableInterrupt(TimerManager this, bool flag);

// use static globals instead of class' members to avoid dereferencing
static TimerManager _timerManager;
static SoundManager _soundManager;

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			TimerManager::getInstance()
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
static void __attribute__ ((noinline)) TimerManager::constructor(TimerManager this)
{
	ASSERT(this, "TimerManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->tcrValue = 0;
	this->milliseconds = 0;
	this->totalMilliseconds = 0;

	_timerManager = this;
	_soundManager = SoundManager::getInstance();
}

/**
 * Class destructor
 *
 * @memberof	TimerManager
 * @public
 *
 * @param this	Function scope
 */
void TimerManager::destructor(TimerManager this)
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
void TimerManager::initialize(TimerManager this)
{
	ASSERT(this, "TimerManager::initialize: null this");

	TimerManager::setFrequency(this, __TIMER_100US);
	TimerManager::setTime(this, __TIME_MS(__TIMER_RESOLUTION));
	TimerManager::clearStat(this);
	TimerManager::enable(this, true);
	TimerManager::enableInterrupt(this, true);
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
static void TimerManager::enableInterrupt(TimerManager this, bool flag)
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
 * Enable / disable timer
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 * @param flag		Bool to enable or disable
 */
void TimerManager::enable(TimerManager this, bool flag)
{
	ASSERT(this, "TimerManager::enable: null this");

	if(flag)
	{
		this->tcrValue |= __TIMER_ENB | __TIMER_INT;
	}
	else
	{
		this->tcrValue &= ~(__TIMER_ENB | __TIMER_INT);
	}

	_hardwareRegisters[__TCR] = this->tcrValue;
}

/**
 * Interrupt handler
 *
 * @memberof		TimerManager
 * @public
 */
void TimerManager::interruptHandler()
{
	//disable
	TimerManager::enable(_timerManager, false);
	TimerManager::clearStat(_timerManager);

#ifdef __ALERT_STACK_OVERFLOW
	HardwareManager::checkStackStatus(HardwareManager::getInstance());
#endif

	// update clocks
	_timerManager->milliseconds += __TIMER_RESOLUTION;
	_timerManager->totalMilliseconds += __TIMER_RESOLUTION;

	// play sounds
	static u32 previousHundredthSecond = 0;
	static u32 currentHundredthSecond = 0;
	currentHundredthSecond += __TIMER_RESOLUTION;

	if(previousHundredthSecond < (u32)(currentHundredthSecond / 10))
	{
		previousHundredthSecond = (u32)(currentHundredthSecond / 10);
		// update sounds
		SoundManager::playSounds(_soundManager);
	}

	// enable
	TimerManager::enable(_timerManager, true);
}

/**
 * Retrieve the elapsed milliseconds in the current game frame
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			Milliseconds elapsed during the current game frame
 */
u32 TimerManager::getMillisecondsElapsed(TimerManager this)
{
	ASSERT(this, "TimerManager::getMillisecondsElapsed: null this");

	return this->milliseconds;
}

/**
 * Retrieve the total elapsed milliseconds
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			Total elapsed milliseconds
 */
u32 TimerManager::getTotalMillisecondsElapsed(TimerManager this)
{
	ASSERT(this, "TimerManager::getTotalMillisecondsElapsed: null this");

	return this->totalMilliseconds;
}

/**
 * Reset the total milliseconds elapsed
 *
 * @memberof		TimerManager
 * @public
 *
 * @param this		Function scope
 */
u32 TimerManager::resetMilliseconds(TimerManager this)
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
void TimerManager::setTime(TimerManager this __attribute__ ((unused)), u16 time)
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
void TimerManager::setFrequency(TimerManager this, int frequency)
{
	ASSERT(this, "TimerManager::setFrequency: null this");

	this->tcrValue = (this->tcrValue & 0x0F) | frequency;

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
int TimerManager::getStat(TimerManager this __attribute__ ((unused)))
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
void TimerManager::clearStat(TimerManager this)
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
void TimerManager::wait(TimerManager this, u32 milliSeconds)
{
	ASSERT(this, "ClockManager::wait: null this");

	// declare as volatile to prevent the compiler to optimize currentMilliseconds away
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
void TimerManager::repeatMethodCall(TimerManager this, u32 callTimes, u32 duration, Object object, void (*method)(Object, u32))
{
	if(object && method)
	{
		// declare as volatile to prevent the compiler to optimize currentMilliseconds away
		// making the last assignment invalid
		volatile u32 currentMilliseconds = this->milliseconds;

		u32 i = 0;

		for(; i < callTimes; i++)
		{
			TimerManager::wait(this, duration / callTimes);
			method(object, i);
		}

		this->milliseconds = currentMilliseconds;
	}
}
