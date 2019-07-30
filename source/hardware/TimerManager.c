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
	this->resolution = __TIMER_100US;
	this->timePerInterrupt = 1;
	this->timePerInterruptUnits = kMS;

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
	this->resolution = __TIMER_100US;
	this->timePerInterrupt = 1;
	this->timePerInterruptUnits = kMS;
}

/**
 * Get resolution in US
 * 
 * @return resolution in us	u16
 */
u16 TimerManager::getResolutionInUS()
{
	switch(this->resolution)
	{
		case __TIMER_20US:

			return 20;
			break;

		case __TIMER_100US:

			return 100;
			break;

		default:

			ASSERT(false, "TimerManager::getResolutionInUS: wrong timer resolution");

			break;
	}

	return 0;
}

/**
 * Get timer frequency
 * 
 * @return frequency	u16
 */
u16 TimerManager::getResolution()
{
	return this->resolution;
}

/**
 * Set timer resolution
 * 
 * @param resolution 	u16
 */
void TimerManager::setResolution(u16 resolution)
{
	switch(resolution)
	{
		case __TIMER_20US:
		case __TIMER_100US:

			this->resolution = resolution;
			break;

		default:

			NM_ASSERT(false, "TimerManager::setResolution: wrong timer resolution");
			
			this->resolution =  __TIMER_20US;
			break;
	}
}

/**
 * Get applied timer counter
 * 
 * @return timer counter 	u16
 */
u16 TimerManager::getTimerCounter()
{
	return TimerManager::computeTimerCounter(this);
}

/**
 * Get target time per interrupt
 * 
 * @return timePerInterrupt 	u16
 */
u16 TimerManager::getTimePerInterrupt()
{
	return this->timePerInterrupt;
}

/**
 * Set target time per interrupt
 * 
 * @param timePerInterrupt 	u16
 */
void TimerManager::setTimePerInterrupt(u16 timePerInterrupt)
{
	u16 minimumTimePerInterrupt = 0;
	u16 maximumTimePerInterrupt = 0;

	switch(this->timePerInterruptUnits)
	{
		case kUS:

			minimumTimePerInterrupt = 100;
			maximumTimePerInterrupt = 1300;
			break;

		case kMS:

			minimumTimePerInterrupt = 1;
			maximumTimePerInterrupt = 6500;
			break;

		case kSEC:
			
			minimumTimePerInterrupt = 1;
			maximumTimePerInterrupt = 6;
			break;
		
		default:

			ASSERT(false, "TimerManager::setResolution: wrong resolution scale");
			break;
	}

	if(timePerInterrupt < minimumTimePerInterrupt)
	{
		timePerInterrupt = minimumTimePerInterrupt;
	}
	else if(timePerInterrupt > maximumTimePerInterrupt)
	{
		timePerInterrupt = maximumTimePerInterrupt;
	}

	this->timePerInterrupt = timePerInterrupt;
}

/**
 * Get target time per interrupt units
 * 
 * @return scale 	u16
 */
u16 TimerManager::getTimePerInterruptUnits()
{
	return this->timePerInterruptUnits;
}

/**
 * Set target time per interrupt units
 * 
 * @param timePerInterruptUnits 	u16
 */
void TimerManager::setTimePerInterruptUnits(u16 timePerInterruptUnits)
{
	switch(timePerInterruptUnits)
	{
		case kUS:
		case kMS:
			
			this->timePerInterruptUnits = timePerInterruptUnits;
			break;
		
		default:

			ASSERT(false, "TimerManager::setTimePerInterruptUnits: wrong resolution scale");
			break;
	}
	
	TimerManager::setResolution(this, this->resolution);
}

u16 TimerManager::computeTimerCounter()
{
	u16 timerCounter = 0;

	u16 timerResolutionUS = TimerManager::getResolutionInUS(TimerManager::getInstance());


	switch(this->timePerInterruptUnits)
	{
		case kUS:

			timerCounter = __TIME_US(this->timePerInterrupt * 100 / timerResolutionUS);
			break;

		case kMS:

			timerCounter = __TIME_MS(this->timePerInterrupt * 100 / timerResolutionUS);
			break;

		case kSEC:

			timerCounter = __TIME_SEC(this->timePerInterrupt * 100 / timerResolutionUS);
			break;

		default:

			NM_ASSERT(false, "TimerManager::setTimePerInterruptUnits: wrong resolution scale");
			break;
	}

	return timerCounter;
}


/**
 * Initialize manager
 */
void TimerManager::initialize()
{
	TimerManager::enableInterrupt(this, false);
	TimerManager::setTimerResolution(this);
	TimerManager::setTimerCounter(this); 

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

	// update MIDI sounds
	SoundManager::playMIDISounds(SoundManager::getInstance());

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
void TimerManager::setTimerCounter()
{
	u16 timerCounter = TimerManager::computeTimerCounter(this);
	_hardwareRegisters[__TLR] = (timerCounter & 0xFF);
	_hardwareRegisters[__THR] = (timerCounter >> 8);
}

/**
 * Set Timer's resolution
 *
 * @param resolution			New resolution
 */
void TimerManager::setTimerResolution()
{
	TimerManager::enable(this, false);

	this->tcrValue = (this->tcrValue & 0x0F) | this->resolution;
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

void TimerManager::print(int x, int y)
{
	PRINT_TEXT("TIMER", x, y++);
	
	switch(this->resolution)
	{
		case __TIMER_20US:

			PRINT_TEXT("Resolution    : 20 US ", x, y++);
			break;

		case __TIMER_100US:

			PRINT_TEXT("Resolution    : 100 US ", x, y++);
			break;

		default:

			PRINT_TEXT("Resolution    : ?      ", x, y++);
			break;
	}

	switch(this->timePerInterruptUnits)
	{
		case kUS:

			PRINT_TEXT("Counter Units : US ", x, y++);
			break;

		case kMS:

			PRINT_TEXT("Counter Units : MS ", x, y++);
			break;

		default:

			PRINT_TEXT("Counter Units : ?  ", x, y++);
			break;
	}

	PRINT_TEXT("Timer counter :      ", x, y);
	PRINT_INT(TimerManager::computeTimerCounter(this), x + 16, y++);

	PRINT_TEXT("Time step     :         ", x, y);
	PRINT_INT(this->timePerInterrupt, x + 16, y++);

	PRINT_TEXT("US per inter  :         ", x, y);
	PRINT_INT((TimerManager::computeTimerCounter(this) + 1) * TimerManager::getResolutionInUS(this), x + 16, y++);
}