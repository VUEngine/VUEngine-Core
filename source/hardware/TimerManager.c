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
 * @return		TimerManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void TimerManager::constructor()
{
	Base::constructor();

	this->tcrValue = 0;
	this->milliseconds = 0;
	this->totalMilliseconds = 0;
	this->frequency = __TIMER_100US;
	this->resolution = __TIME_MS(__TIMER_RESOLUTION);

	_timerManager = this;
	_soundManager = SoundManager::getInstance();
}

/**
 * Class destructor
 */
void TimerManager::destructor()
{
	_timerManager = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Reset
 */
void TimerManager::reset()
{
	this->tcrValue = 0;
	this->milliseconds = 0;
	this->totalMilliseconds = 0;
	this->frequency = __TIMER_100US;
	this->resolution = __TIME_MS(__TIMER_RESOLUTION);
}


/**
 * Set timer frequency
 * 
 * @param frequency 	u16
 */
void TimerManager::setFrequency(u16 frequency)
{
	this->frequency = frequency;
}

/**
 * Set timer resolution
 * 
 * @param resolution 	u16
 */
void TimerManager::setResolution(u16 resolution)
{
	this->resolution = resolution;
}

/**
 * Initialize manager
 */
void TimerManager::initialize()
{
	TimerManager::setTimerFrequency(this);
	TimerManager::setTimerResolution(this); 

	TimerManager::clearStat(this);
	TimerManager::enable(this, true);
	TimerManager::enableInterrupt(this, true);
}

/**
 * Enable / disable interrupt
 *
 * @param flag		Bool to enable or disable
 */
void TimerManager::enableInterrupt(bool flag)
{
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
 * @param flag		Bool to enable or disable
 */
void TimerManager::enable(bool flag)
{
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
 */
static void TimerManager::interruptHandler()
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
	}

	// update sounds
	SoundManager::playSounds(SoundManager::getInstance());

	// enable
	TimerManager::enable(_timerManager, true);
}

/**
 * Retrieve the elapsed milliseconds in the current game frame
 *
 * @return			Milliseconds elapsed during the current game frame
 */
u32 TimerManager::getMillisecondsElapsed()
{
	return this->milliseconds;
}

/**
 * Retrieve the total elapsed milliseconds
 *
 * @return			Total elapsed milliseconds
 */
u32 TimerManager::getTotalMillisecondsElapsed()
{
	return this->totalMilliseconds;
}

/**
 * Reset the total milliseconds elapsed
 */
u32 TimerManager::resetMilliseconds()
{
	u32 milliseconds = this->milliseconds;

	this->milliseconds = 0;

	return milliseconds;
}

/**
 * Set Timer's time
 *
 * @param time		New time
 */
void TimerManager::setTimerResolution()
{
	_hardwareRegisters[__TLR] = (this->resolution & 0xFF);
	_hardwareRegisters[__THR] = (this->resolution >> 8);
}

/**
 * Set Timer's frequency
 *
 * @param frequency			New frequency
 */
void TimerManager::setTimerFrequency()
{
	this->tcrValue = (this->tcrValue & 0x0F) | this->frequency;
	_hardwareRegisters[__TCR] = this->tcrValue;
}

/**
 * Retrieve Timer's ZSTAT
 *
 * @return			ZSTAT
 */
int TimerManager::getStat()
{
	return (_hardwareRegisters[__TCR] & __TIMER_ZSTAT);
}

/**
 * Clear Timer's ZSTAT
 */
void TimerManager::clearStat()
{
	_hardwareRegisters[__TCR] = (this->tcrValue | __TIMER_ZCLR);
}

/**
 * Produce a wait
 *
 * @param milliSeconds		Time to wait
 */
void TimerManager::wait(u32 milliSeconds)
{
	// declare as volatile to prevent the compiler to optimize currentMilliseconds away
	// making the last assignment invalid
	volatile u32 currentMilliseconds = this->milliseconds;
	u32 waitStartTime = this->milliseconds;
	volatile u32 *milliseconds = (u32*)&this->milliseconds;

	while ((*milliseconds - waitStartTime) < milliSeconds)
	{
		HardwareManager::halt();
	}

	this->milliseconds = currentMilliseconds;
}

/**
 * Call a method a number of times during a certain lapse
 *
 * @param callTimes			Number of calls to produce during the total duration
 * @param duration			Time that must take the callTimes
 * @param object			Called method's scope
 * @param method			Method to call
 */
void TimerManager::repeatMethodCall(u32 callTimes, u32 duration, Object object, void (*method)(Object, u32))
{
	if(!isDeleted(object) && method)
	{
		// declare as volatile to prevent the compiler to optimize currentMilliseconds away
		// making the last assignment invalid
		volatile u32 currentMilliseconds = this->milliseconds;

		u32 i = 0;

		for(; i < callTimes; i++)
		{
			TimerManager::wait(this, duration / callTimes);

			if(isDeleted(object))
			{
				return;
			}

			method(object, i);
		}

		this->milliseconds = currentMilliseconds;
	}
}
