/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <debugConfig.h>
#include <TimerManager.h>
#include <HardwareManager.h>
#include <ClockManager.h>
#include <SoundManager.h>
#include <StopwatchManager.h>
#include <SoundTest.h>
#include <Game.h>
#include <Profiler.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// use static globals instead of class' members to avoid dereferencing
static TimerManager _timerManager;
static SoundManager _soundManager;
static StopwatchManager _stopwatchManager;


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
	this->microseconds = 0;
	this->totalMilliseconds = 0;
	this->resolution = __TIMER_100US;
	this->timePerInterrupt = 1;
	this->timePerInterruptUnits = kMS;
	this->minimumTimePerInterruptUS = __MINIMUM_TIME_PER_INTERRUPT_US;
	this->maximumTimePerInterruptUS = __MAXIMUM_TIME_PER_INTERRUPT_US;
	this->minimumTimePerInterruptMS = __MINIMUM_TIME_PER_INTERRUPT_MS;
	this->maximumTimePerInterruptMS = __MAXIMUM_TIME_PER_INTERRUPT_MS;
	this->interruptsPerGameFrame = 0;
	this->microsecondsPerInterrupt = TimerManager::getTimePerInterruptInUS(this);


	_timerManager = this;
	_soundManager = SoundManager::getInstance();
	_stopwatchManager = StopwatchManager::getInstance();
}

/**
 *
  Class destructor
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
	this->microseconds = 0;
	this->totalMilliseconds = 0;
	this->resolution = __TIMER_100US;
	this->timePerInterrupt = 1;
	this->timePerInterruptUnits = kMS;
	this->minimumTimePerInterruptUS = __MINIMUM_TIME_PER_INTERRUPT_US;
	this->maximumTimePerInterruptUS = __MAXIMUM_TIME_PER_INTERRUPT_US;
	this->minimumTimePerInterruptMS = __MINIMUM_TIME_PER_INTERRUPT_MS;
	this->maximumTimePerInterruptMS = __MAXIMUM_TIME_PER_INTERRUPT_MS;
	this->interruptsPerGameFrame = 0;
	this->microsecondsPerInterrupt = TimerManager::getTimePerInterruptInUS(this);
}

/**
 * Get resolution in US
 *
 * @return resolution in us	uint16
 
 */
uint16 TimerManager::getResolutionInUS()
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
 * @return frequency	uint16
 */
uint16 TimerManager::getResolution()
{
	return this->resolution;
}

/**
 * Set timer resolution
 *
 * @param resolution 	uint16
 */
void TimerManager::setResolution(uint16 resolution)
{
	switch(resolution)
	{
		case __TIMER_20US:

			this->resolution = resolution;
			break;

		case __TIMER_100US:

			this->resolution = resolution;
			break;

		default:

			NM_ASSERT(false, "TimerManager::setResolution: wrong timer resolution");

			this->resolution =  __TIMER_20US;
			break;
	}

	this->minimumTimePerInterruptUS = __MINIMUM_TIME_PER_INTERRUPT_US;

	uint32 timePerInterrupt = this->timePerInterrupt;

	switch(this->timePerInterruptUnits)
	{
		case kUS:
			{
				uint32 residue = timePerInterrupt % TimerManager::getResolutionInUS(this);

				if(timePerInterrupt > residue)
				{
					timePerInterrupt -= residue;
				}
			}
			break;

		case kMS:

			break;

		default:

			ASSERT(false, "SoundTest::setResolution: wrong timer resolution scale");
			break;
	}

	TimerManager::setTimePerInterrupt(this, timePerInterrupt);
}

/**
 * Get applied timer counter
 *
 * @return timer counter 	uint16
 */
uint16 TimerManager::getTimerCounter()
{
	return TimerManager::computeTimerCounter(this);
}

/**
 * Get target time per interrupt
 *
 * @return timePerInterrupt 	uint16
 */
uint16 TimerManager::getTimePerInterrupt()
{
	return this->timePerInterrupt;
}

float TimerManager::getTimePerInterruptInMS()
{
	switch(this->timePerInterruptUnits)
	{
		case kUS:

			return this->timePerInterrupt / (float)__MICROSECONDS_PER_MILLISECOND;
			break;

		case kMS:

			return this->timePerInterrupt;
			break;

		default:

			ASSERT(false, "SoundTest::getTimePerInterruptInMS: wrong timer resolution scale");
			break;
	}

	return 0;
}

uint32 TimerManager::getTimePerInterruptInUS()
{
	switch(this->timePerInterruptUnits)
	{
		case kUS:

			return this->timePerInterrupt;
			break;

		case kMS:

			return this->timePerInterrupt * __MICROSECONDS_PER_MILLISECOND;
			break;

		default:

			ASSERT(false, "SoundTest::getTimePerInterruptInUS: wrong timer resolution scale");
			break;
	}

	return 0;
}

/**
 * Set target time per interrupt
 *
 * @param timePerInterrupt 	uint16
 */
void TimerManager::setTimePerInterrupt(uint16 timePerInterrupt)
{
	int16 minimumTimePerInterrupt = TimerManager::getMinimumTimePerInterruptStep(this);
	int16 maximumTimePerInterrupt = 1000;

	switch(this->timePerInterruptUnits)
	{
		case kUS:

			maximumTimePerInterrupt = this->maximumTimePerInterruptUS;
			break;

		case kMS:

			maximumTimePerInterrupt = this->maximumTimePerInterruptMS;
			break;

		default:

			ASSERT(false, "TimerManager::setResolution: wrong resolution scale");
			break;
	}

	if((int16)timePerInterrupt < minimumTimePerInterrupt)
	{
		timePerInterrupt = minimumTimePerInterrupt;
	}
	else if((int16)timePerInterrupt > maximumTimePerInterrupt)
	{
		timePerInterrupt = maximumTimePerInterrupt;
	}

	this->timePerInterrupt = timePerInterrupt;
	this->microsecondsPerInterrupt = TimerManager::getTimePerInterruptInUS(this);
}

/**
 * Get target time per interrupt units
 *
 * @return scale 	uint16
 */
uint16 TimerManager::getTimePerInterruptUnits()
{
	return this->timePerInterruptUnits;
}

/**
 * Set target time per interrupt units
 *
 * @param timePerInterruptUnits 	uint16
 */
void TimerManager::setTimePerInterruptUnits(uint16 timePerInterruptUnits)
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

uint16 TimerManager::getMinimumTimePerInterruptStep()
{
	switch(this->timePerInterruptUnits)
	{
		case kUS:
			return this->minimumTimePerInterruptUS;
			break;

		case kMS:

			return this->minimumTimePerInterruptMS;
			break;
	}

	return 0;
}

uint16 TimerManager::computeTimerCounter()
{
	uint16 timerCounter = 0;

	switch(this->timePerInterruptUnits)
	{
		case kUS:

			timerCounter = __TIME_US(this->timePerInterrupt) - (__TIMER_20US == this->resolution ? 1 : 0);
			break;

		case kMS:

			timerCounter = __TIME_MS(this->timePerInterrupt);
			break;

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
	TimerManager::configureTimerCounter(this);

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

void TimerManager::nextFrameStarted(uint32 elapsedMicroseconds)
{
	// reset timer
	TimerManager::resetMilliseconds(this);

	if(0 >= this->interruptsPerGameFrame)
	{
		this->microsecondsPerInterrupt = TimerManager::getTimePerInterruptInUS(this);
	}
	else
	{
		this->microsecondsPerInterrupt = elapsedMicroseconds / this->interruptsPerGameFrame;
	}

	this->interruptsPerGameFrame = 0;
}

#ifdef __SHOW_TIMER_MANAGER_STATUS
void TimerManager::nextSecondStarted()
{
	TimerManager::printStatus(this, 1, 10);

	this->interruptsPerSecond = 0;
}
#endif

void TimerManager::printStatus(int32 x, int32 y)
{
	PRINT_INT(this->interruptsPerSecond, x + 22, y);
	PRINT_INT(this->microsecondsPerInterrupt, x + 22, ++y);
/*	PRINT_TEXT("TIMER MANAGER", x, y++);

	PRINT_TEXT("Interrupts/second:          ", x, ++y);
	PRINT_INT(this->interruptsPerSecond, x + 22, y);
	PRINT_TEXT("Interrupts/frame:          ", x, ++y);
	PRINT_INT(this->interruptsPerSecond / __TARGET_FPS, x + 22, y);
	PRINT_TEXT("Average us/interrupt:          ", x, ++y);
	PRINT_INT(__MICROSECONDS_PER_SECOND / this->interruptsPerSecond, x + 22, y);
	PRINT_TEXT("Real us/interrupt:          ", x, ++y);
	PRINT_INT(this->microsecondsPerInterrupt, x + 22, y);
	*/
}

/**
 * Interrupt handler
 */
static void TimerManager::interruptHandler()
{
	//disable
#ifndef __ENABLE_PROFILER
	TimerManager::enable(_timerManager, false);
	TimerManager::clearStat(_timerManager);
#else
	TimerManager::enableInterrupt(_timerManager, false);
#endif

#ifdef __SHOW_TIMER_MANAGER_STATUS
	_timerManager->interruptsPerSecond++;
#endif

	_timerManager->interruptsPerGameFrame++;

	_timerManager->microseconds += _timerManager->microsecondsPerInterrupt;

	if(_timerManager->microseconds > __MICROSECONDS_PER_MILLISECOND)
	{
		uint32 elapsedMilliseconds = _timerManager->microseconds / __MICROSECONDS_PER_MILLISECOND;

		_timerManager->microseconds = _timerManager->microseconds % __MICROSECONDS_PER_MILLISECOND;

		_timerManager->milliseconds += elapsedMilliseconds;

#ifdef __SOUND_TEST
		if(Game::isInSoundTest(Game::getInstance()))
		{
			SoundManager::printPlaybackTime(SoundManager::getInstance());
		}
#endif
	}

	// update sounds
	SoundManager::playSounds(_timerManager->microsecondsPerInterrupt);

	// update Stopwatchs: no use is being done of them so this is commented out for now since it affects PCM playback
	//StopwatchManager::update(_stopwatchManager);

// enable
#ifndef __ENABLE_PROFILER
	TimerManager::enable(_timerManager, true);
#else
	TimerManager::enableInterrupt(_timerManager, true);
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeTimerInterruptProcess, PROCESS_NAME_SOUND_PLAY);
#endif
}

/**
 * Retrieve the elapsed milliseconds in the current game frame
 *
 * @return			Milliseconds elapsed during the current game frame
 */
uint32 TimerManager::getMillisecondsElapsed()
{
	return this->milliseconds;
}

/**
 * Retrieve the total elapsed milliseconds
 *
 * @return			Total elapsed milliseconds
 */
uint32 TimerManager::getTotalMillisecondsElapsed()
{
	return this->totalMilliseconds;
}

/**
 * Reset the total milliseconds elapsed
 */
uint32 TimerManager::resetMilliseconds()
{
	uint32 milliseconds = this->milliseconds;

	this->milliseconds = 0;
	this->microseconds = 0;
	this->totalMilliseconds += milliseconds;

	return milliseconds;
}

/**
 * Set Timer's time
 *
 * @param time		New time
 */
void TimerManager::configureTimerCounter()
{
	uint16 timerCounter = TimerManager::computeTimerCounter(this);
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
int32 TimerManager::getStat()
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
void TimerManager::wait(uint32 milliSeconds)
{
	// declare as volatile to prevent the compiler to optimize currentMilliseconds away
	// making the last assignment invalid
	volatile uint32 currentMilliseconds = this->milliseconds;
	uint32 waitStartTime = this->milliseconds;
	volatile uint32 *milliseconds = (uint32*)&this->milliseconds;

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
void TimerManager::repeatMethodCall(uint32 callTimes, uint32 duration, Object object, void (*method)(Object, uint32))
{
	if(!isDeleted(object) && method)
	{
		// declare as volatile to prevent the compiler to optimize currentMilliseconds away
		// making the last assignment invalid
		volatile uint32 currentMilliseconds = this->milliseconds;

		uint32 i = 0;

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

void TimerManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "TIMER", x, y++, NULL);
	y++;

	switch(this->resolution)
	{
		case __TIMER_20US:

			Printing::text(Printing::getInstance(), "Resolution    20 US ", x, y++, NULL);
			break;

		case __TIMER_100US:

			Printing::text(Printing::getInstance(), "Resolution    100 US ", x, y++, NULL);
			break;

		default:

			Printing::text(Printing::getInstance(), "Resolution    ?      ", x, y++, NULL);
			break;
	}

	switch(this->timePerInterruptUnits)
	{
		case kUS:

			Printing::text(Printing::getInstance(), "US/interrupt        ", x, y, NULL);
			break;

		case kMS:

			Printing::text(Printing::getInstance(), "MS/interrupt        ", x, y, NULL);
			break;

		default:

			Printing::text(Printing::getInstance(), " ?/interrupt        ", x, y, NULL);
			break;
	}

	Printing::int32(Printing::getInstance(), this->timePerInterrupt, x + 14, y++, NULL);

	Printing::text(Printing::getInstance(), "Timer counter               ", x, y, NULL);
	Printing::int32(Printing::getInstance(), TimerManager::computeTimerCounter(this), x + 14, y++, NULL);
}