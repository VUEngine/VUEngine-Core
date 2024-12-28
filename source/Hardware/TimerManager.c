/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <DebugConfig.h>
#include <HardwareManager.h>
#include <ListenerObject.h>
#include <Printing.h>
#include <Profiler.h>
#include <SoundManager.h>
#include <StopwatchManager.h>
#include <SoundTest.h>
#include <VUEngine.h>

#include "TimerManager.h"


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

static TimerManager _timerManager;
static SoundManager _soundManager;
static StopwatchManager _stopwatchManager;


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void TimerManager::interruptHandler()
{
	//disable
#ifndef __ENABLE_PROFILER
	TimerManager::disable(_timerManager);
	TimerManager::clearStat(_timerManager);
#else
	TimerManager::disableInterrupt(_timerManager);

	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeStartInterrupt, NULL);
#endif

	_timerManager->interruptsPerSecond++;

	_timerManager->interruptsPerGameFrame++;

	_timerManager->elapsedMicroseconds += _timerManager->elapsedMicrosecondsPerInterrupt;

	if(_timerManager->elapsedMicroseconds > __MICROSECONDS_PER_MILLISECOND)
	{
		uint32 elapsedMilliseconds = _timerManager->elapsedMicroseconds / __MICROSECONDS_PER_MILLISECOND;

		_timerManager->elapsedMicroseconds = _timerManager->elapsedMicroseconds % __MICROSECONDS_PER_MILLISECOND;

		_timerManager->elapsedMilliseconds += elapsedMilliseconds;

		_timerManager->totalElapsedMilliseconds += elapsedMilliseconds;
	}

	// update sounds
	SoundManager::playSounds(_timerManager->elapsedMicrosecondsPerInterrupt);

	// update Stopwatchs: no use is being done of them so this is commented out for now since it affects PCM playback
	//StopwatchManager::update(_stopwatchManager);

// enable
#ifndef __ENABLE_PROFILER
	TimerManager::enable(_timerManager);
#else
	TimerManager::enableInterrupt(_timerManager);
	Profiler::lap(Profiler::getInstance(), kProfilerLapTypeTimerInterruptProcess, PROCESS_NAME_SOUND_PLAY);
#endif
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void TimerManager::initialize()
{
	TimerManager::disableInterrupt(this);
	TimerManager::setTimerResolution(this);
	TimerManager::clearStat(this);
	TimerManager::resetTimerCounter(this);
	TimerManager::enable(this);
	TimerManager::enableInterrupt(this);
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::reset()
{
	this->tcrValue = 0;
	this->elapsedMilliseconds = 0;
	this->elapsedMicroseconds = 0;
	this->resolution = __TIMER_100US;
	this->targetTimePerInterrupt = 1;
	this->targetTimePerInterrupttUnits = kMS;
	this->interruptsPerGameFrame = 0;
	this->elapsedMicrosecondsPerInterrupt = TimerManager::getTargetTimePerInterruptInUS(this);
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::resetTimerCounter()
{
	uint16 timerCounter = TimerManager::computeTimerCounter(this);

	switch(this->resolution)
	{
		case __TIMER_20US:

			break;

		case __TIMER_100US:

			// Compensate for the difference in speed between 20US and 100US timer resolution
			timerCounter += __TIMER_COUNTER_DELTA;
			break;
	}

	_hardwareRegisters[__TLR] = (timerCounter & 0xFF);
	_hardwareRegisters[__THR] = (timerCounter >> 8);
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::configure(uint16 timerResolution, uint16 targetTimePerInterrupt, uint16 targetTimePerInterrupttUnits)
{
	TimerManager::setResolution(this, timerResolution);
	TimerManager::setTargetTimePerInterruptUnits(this, targetTimePerInterrupttUnits);
	TimerManager::setTargetTimePerInterrupt(this, targetTimePerInterrupt);
	TimerManager::applySettings(this, true);
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::applySettings(bool enable)
{
	TimerManager::disable(this);
	TimerManager::initialize(this);

	if(enable)
	{
		TimerManager::enable(this);
	}
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::enable()
{
	this->tcrValue |= __TIMER_ENB | __TIMER_INT;

	_hardwareRegisters[__TCR] = this->tcrValue;
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::disable()
{
	this->tcrValue &= ~(__TIMER_ENB | __TIMER_INT);

	_hardwareRegisters[__TCR] = this->tcrValue;
}
//---------------------------------------------------------------------------------------------------------
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

	uint32 targetTimePerInterrupt = this->targetTimePerInterrupt;

	switch(this->targetTimePerInterrupttUnits)
	{
		case kUS:
			{
				uint32 residue = targetTimePerInterrupt % TimerManager::getResolutionInUS(this);

				if(targetTimePerInterrupt > residue)
				{
					targetTimePerInterrupt -= residue;
				}
			}
			break;

		case kMS:

			break;

		default:

			ASSERT(false, "SoundTest::setResolution: wrong timer resolution scale");
			break;
	}

	TimerManager::setTargetTimePerInterrupt(this, targetTimePerInterrupt);
}
//---------------------------------------------------------------------------------------------------------
uint16 TimerManager::getResolution()
{
	return this->resolution;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void TimerManager::setTargetTimePerInterrupt(uint16 targetTimePerInterrupt)
{
	int16 minimumTimePerInterrupt = 0;
	int16 maximumTimePerInterrupt = 1000;

	switch(this->targetTimePerInterrupttUnits)
	{
		case kUS:

			minimumTimePerInterrupt = __MINIMUM_TIME_PER_INTERRUPT_US;
			maximumTimePerInterrupt = __MAXIMUM_TIME_PER_INTERRUPT_US;
			break;

		case kMS:

			minimumTimePerInterrupt = __MINIMUM_TIME_PER_INTERRUPT_MS;
			maximumTimePerInterrupt = __MAXIMUM_TIME_PER_INTERRUPT_MS;
			break;

		default:

			ASSERT(false, "TimerManager::setResolution: wrong resolution scale");
			break;
	}

	if((int16)targetTimePerInterrupt < minimumTimePerInterrupt)
	{
		targetTimePerInterrupt = minimumTimePerInterrupt;
	}
	else if((int16)targetTimePerInterrupt > maximumTimePerInterrupt)
	{
		targetTimePerInterrupt = maximumTimePerInterrupt;
	}

	this->targetTimePerInterrupt = targetTimePerInterrupt;
	this->elapsedMicrosecondsPerInterrupt = TimerManager::getTargetTimePerInterruptInUS(this);
}
//---------------------------------------------------------------------------------------------------------
uint16 TimerManager::getTargetTimePerInterrupt()
{
	return this->targetTimePerInterrupt;
}
//---------------------------------------------------------------------------------------------------------
float TimerManager::getTargetTimePerInterruptInMS()
{
	switch(this->targetTimePerInterrupttUnits)
	{
		case kUS:

			return this->targetTimePerInterrupt / (float)__MICROSECONDS_PER_MILLISECOND;
			break;

		case kMS:

			return this->targetTimePerInterrupt;
			break;

		default:

			ASSERT(false, "SoundTest::getTargetTimePerInterruptInMS: wrong timer resolution scale");
			break;
	}

	return 0;
}
//---------------------------------------------------------------------------------------------------------
uint32 TimerManager::getTargetTimePerInterruptInUS()
{
	switch(this->targetTimePerInterrupttUnits)
	{
		case kUS:

			return this->targetTimePerInterrupt;
			break;

		case kMS:

			return this->targetTimePerInterrupt * __MICROSECONDS_PER_MILLISECOND;
			break;

		default:

			ASSERT(false, "SoundTest::getTargetTimePerInterruptInUS: wrong timer resolution scale");
			break;
	}

	return 0;
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::setTargetTimePerInterruptUnits(uint16 targetTimePerInterrupttUnits)
{
	switch(targetTimePerInterrupttUnits)
	{
		case kUS:
		case kMS:

			this->targetTimePerInterrupttUnits = targetTimePerInterrupttUnits;
			break;

		default:

			ASSERT(false, "TimerManager::setTargetTimePerInterruptUnits: wrong resolution scale");
			break;
	}

	TimerManager::setResolution(this, this->resolution);
}
//---------------------------------------------------------------------------------------------------------
uint16 TimerManager::getTargetTimePerInterruptUnits()
{
	return this->targetTimePerInterrupttUnits;
}
//---------------------------------------------------------------------------------------------------------
uint16 TimerManager::getTimerCounter()
{
	return TimerManager::computeTimerCounter(this);
}
//---------------------------------------------------------------------------------------------------------
uint16 TimerManager::getCurrentTimerCounter()
{
	return (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];
}
//---------------------------------------------------------------------------------------------------------
uint16 TimerManager::getMinimumTimePerInterruptStep()
{
	switch(this->targetTimePerInterrupttUnits)
	{
		case kUS:
			return __MINIMUM_TIME_PER_INTERRUPT_US_STEP;
			break;

		case kMS:

			return __MINIMUM_TIME_PER_INTERRUPT_MS_STEP;
			break;
	}

	return 0;
}
//---------------------------------------------------------------------------------------------------------
uint32 TimerManager::getElapsedMilliseconds()
{
	return this->elapsedMilliseconds;
}
//---------------------------------------------------------------------------------------------------------
uint32 TimerManager::getTotalElapsedMilliseconds()
{
	return this->totalElapsedMilliseconds;
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::wait(uint32 milliseconds)
{
	// declare as volatile to prevent the compiler to optimize currentMilliseconds away
	// making the last assignment invalid
	volatile uint32 currentMilliseconds = this->totalElapsedMilliseconds;
	uint32 waitStartTime = this->totalElapsedMilliseconds;
	volatile uint32 *totalElapsedMilliseconds = (uint32*)&this->totalElapsedMilliseconds;

	while ((*totalElapsedMilliseconds - waitStartTime) < milliseconds)
	{
		HardwareManager::halt();
	}

	this->elapsedMilliseconds = currentMilliseconds;
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::repeatMethodCall(uint32 callTimes, uint32 duration, ListenerObject object, void (*method)(ListenerObject, uint32))
{
	if(!isDeleted(object) && method)
	{
		// declare as volatile to prevent the compiler to optimize currentMilliseconds away
		// making the last assignment invalid
		volatile uint32 currentMilliseconds = this->elapsedMilliseconds;

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

		this->elapsedMilliseconds = currentMilliseconds;
	}
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::frameStarted(uint32 elapsedMicroseconds)
{
	this->elapsedMilliseconds = 0;
	this->elapsedMicroseconds = 0;

	if(0 >= this->interruptsPerGameFrame)
	{
		this->elapsedMicrosecondsPerInterrupt = TimerManager::getTargetTimePerInterruptInUS(this);
	}
	else
	{
		this->elapsedMicrosecondsPerInterrupt = elapsedMicroseconds / this->interruptsPerGameFrame;
	}

	this->interruptsPerGameFrame = 0;
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::nextSecondStarted()
{
#ifndef __SHIPPING
#ifdef __SHOW_TIMER_MANAGER_STATUS
	int x = 1;
	int y = 1;
	
	PRINT_TEXT("TIMER STATUS", x, y++);
	PRINT_TEXT("Inter./sec.:          ", x, y);
	PRINT_INT(this->interruptsPerSecond, x + 17, y++);
	PRINT_TEXT("Inter./frm:           ", x, y);
	PRINT_INT(this->interruptsPerSecond / __TARGET_FPS, x + 17, y++);
	PRINT_TEXT("Aver. us/inter.:      ", x, y);
	PRINT_INT(__MICROSECONDS_PER_SECOND / this->interruptsPerSecond, x + 17, y++);
	PRINT_TEXT("Real us/inter.:       ", x, y);
	PRINT_INT(this->elapsedMicrosecondsPerInterrupt, x + 17, y++);
#endif
#endif

	this->interruptsPerSecond = 0;
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "TIMER CONFIG", x, y++, NULL);
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

	switch(this->targetTimePerInterrupttUnits)
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

	Printing::int32(Printing::getInstance(), this->targetTimePerInterrupt, x + 14, y++, NULL);

	Printing::text(Printing::getInstance(), "Timer counter        ", x, y, NULL);
	Printing::int32(Printing::getInstance(), TimerManager::getTimerCounter(this), x + 14, y++, NULL);
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void TimerManager::constructor()
{
	_timerManager = this;
	_soundManager = SoundManager::getInstance();
	_stopwatchManager = StopwatchManager::getInstance();

	// Always explicitly call the base's constructor 
	Base::constructor();

	this->totalElapsedMilliseconds = 0;

	TimerManager::reset(this);
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::destructor()
{
	_timerManager = NULL;

	// allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
uint16 TimerManager::computeTimerCounter()
{
	int16 timerCounter = 0;

	switch(this->targetTimePerInterrupttUnits)
	{
		case kUS:

			timerCounter = __TIME_US(this->targetTimePerInterrupt);
			break;

		case kMS:

			timerCounter = __TIME_MS(this->targetTimePerInterrupt);
			break;

		default:
	
			NM_ASSERT(false, "TimerManager::setTargetTimePerInterruptUnits: wrong resolution scale");
			break;
	}

	return (uint16)(0 >= timerCounter ? 1 : timerCounter);
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::enableInterrupt()
{
	this->tcrValue |= __TIMER_INT;

	_hardwareRegisters[__TCR] = this->tcrValue;
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::disableInterrupt()
{
	this->tcrValue &= ~__TIMER_INT;

	_hardwareRegisters[__TCR] = this->tcrValue;
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::setTimerResolution()
{
	this->tcrValue = (this->tcrValue & 0x0F) | this->resolution;
	_hardwareRegisters[__TCR] = this->tcrValue;
}
//---------------------------------------------------------------------------------------------------------
void TimerManager::clearStat()
{
	_hardwareRegisters[__TCR] = (this->tcrValue | __TIMER_ZCLR);
}
//---------------------------------------------------------------------------------------------------------
