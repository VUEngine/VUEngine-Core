/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with timerManager source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <HardwareManager.h>
#include <ListenerObject.h>
#include <Printing.h>
#include <Profiler.h>
#include <SoundManager.h>
#include <StopwatchManager.h>

#include "TimerManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::interruptHandler()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	//disable
#ifndef __ENABLE_PROFILER
	TimerManager::disable();
	TimerManager::clearStat();
#else
	TimerManager::disableInterrupt();

	Profiler::lap(kProfilerLapTypeStartInterrupt, NULL);
#endif

	timerManager->interruptsPerSecond++;

	timerManager->interruptsPerGameFrame++;

	timerManager->elapsedMicroseconds += timerManager->elapsedMicrosecondsPerInterrupt;

	if(timerManager->elapsedMicroseconds > __MICROSECONDS_PER_MILLISECOND)
	{
		uint32 elapsedMilliseconds = timerManager->elapsedMicroseconds / __MICROSECONDS_PER_MILLISECOND;

		timerManager->elapsedMicroseconds = timerManager->elapsedMicroseconds % __MICROSECONDS_PER_MILLISECOND;

		timerManager->elapsedMilliseconds += elapsedMilliseconds;

		timerManager->totalElapsedMilliseconds += elapsedMilliseconds;
	}

	// Update sounds
	SoundManager::playSounds(timerManager->elapsedMicrosecondsPerInterrupt);

	// Update Stopwatchs: no use is being done of them so timerManager is commented out for now since it affects PCM playback
	//StopwatchManager::update();

// enable
#ifndef __ENABLE_PROFILER
	TimerManager::enable();
#else
	TimerManager::enableInterrupt();
	Profiler::lap(kProfilerLapTypeTimerInterruptProcess, PROCESS_NAME_SOUND_PLAY);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::initialize()
{
	TimerManager::disableInterrupt();
	TimerManager::setTimerResolution();
	TimerManager::clearStat();
	TimerManager::resetTimerCounter();
	TimerManager::enable();
	TimerManager::enableInterrupt();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::reset()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	timerManager->tcrValue = 0;
	timerManager->elapsedMilliseconds = 0;
	timerManager->elapsedMicroseconds = 0;
	timerManager->resolution = __TIMER_100US;
	timerManager->targetTimePerInterrupt = 1;
	timerManager->targetTimePerInterrupttUnits = kMS;
	timerManager->interruptsPerGameFrame = 0;
	timerManager->elapsedMicrosecondsPerInterrupt = TimerManager::getTargetTimePerInterruptInUS();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::resetTimerCounter()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	uint16 timerCounter = TimerManager::computeTimerCounter();

	switch(timerManager->resolution)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::configure(uint16 timerResolution, uint16 targetTimePerInterrupt, uint16 targetTimePerInterrupttUnits)
{
	TimerManager::setResolution(timerResolution);
	TimerManager::setTargetTimePerInterruptUnits(targetTimePerInterrupttUnits);
	TimerManager::setTargetTimePerInterrupt(targetTimePerInterrupt);
	TimerManager::applySettings(true);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::applySettings(bool enable)
{
	TimerManager::disable();
	TimerManager::initialize();

	if(enable)
	{
		TimerManager::enable();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::enable()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	timerManager->tcrValue |= __TIMER_ENB | __TIMER_INT;

	_hardwareRegisters[__TCR] = timerManager->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::disable()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	timerManager->tcrValue &= ~(__TIMER_ENB | __TIMER_INT);

	_hardwareRegisters[__TCR] = timerManager->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::setResolution(uint16 resolution)
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	switch(resolution)
	{
		case __TIMER_20US:

			timerManager->resolution = resolution;
			break;

		case __TIMER_100US:

			timerManager->resolution = resolution;
			break;

		default:

			NM_ASSERT(false, "TimerManager::setResolution: wrong timer resolution");

			timerManager->resolution =  __TIMER_20US;
			break;
	}

	uint32 targetTimePerInterrupt = timerManager->targetTimePerInterrupt;

	switch(timerManager->targetTimePerInterrupttUnits)
	{
		case kUS:
			{
				uint32 residue = targetTimePerInterrupt % TimerManager::getResolutionInUS();

				if(targetTimePerInterrupt > residue)
				{
					targetTimePerInterrupt -= residue;
				}
			}
			break;

		case kMS:

			break;

		default:

			ASSERT(false, "TimerManager::setResolution: wrong timer resolution scale");
			break;
	}

	TimerManager::setTargetTimePerInterrupt(targetTimePerInterrupt);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 TimerManager::getResolution()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	return timerManager->resolution;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 TimerManager::getResolutionInUS()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	switch(timerManager->resolution)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::setTargetTimePerInterrupt(uint16 targetTimePerInterrupt)
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	int16 minimumTimePerInterrupt = 0;
	int16 maximumTimePerInterrupt = 1000;

	switch(timerManager->targetTimePerInterrupttUnits)
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

	timerManager->targetTimePerInterrupt = targetTimePerInterrupt;
	timerManager->elapsedMicrosecondsPerInterrupt = TimerManager::getTargetTimePerInterruptInUS();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 TimerManager::getTargetTimePerInterrupt()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	return timerManager->targetTimePerInterrupt;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static float TimerManager::getTargetTimePerInterruptInMS()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	switch(timerManager->targetTimePerInterrupttUnits)
	{
		case kUS:

			return timerManager->targetTimePerInterrupt / (float)__MICROSECONDS_PER_MILLISECOND;
			break;

		case kMS:

			return timerManager->targetTimePerInterrupt;
			break;

		default:

			ASSERT(false, "TimerManager::getTargetTimePerInterruptInMS: wrong timer resolution scale");
			break;
	}

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 TimerManager::getTargetTimePerInterruptInUS()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	switch(timerManager->targetTimePerInterrupttUnits)
	{
		case kUS:

			return timerManager->targetTimePerInterrupt;
			break;

		case kMS:

			return timerManager->targetTimePerInterrupt * __MICROSECONDS_PER_MILLISECOND;
			break;

		default:

			ASSERT(false, "TimerManager::getTargetTimePerInterruptInUS: wrong timer resolution scale");
			break;
	}

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::setTargetTimePerInterruptUnits(uint16 targetTimePerInterrupttUnits)
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	switch(targetTimePerInterrupttUnits)
	{
		case kUS:
		case kMS:

			timerManager->targetTimePerInterrupttUnits = targetTimePerInterrupttUnits;
			break;

		default:

			ASSERT(false, "TimerManager::setTargetTimePerInterruptUnits: wrong resolution scale");
			break;
	}

	TimerManager::setResolution(timerManager->resolution);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 TimerManager::getTargetTimePerInterruptUnits()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	return timerManager->targetTimePerInterrupttUnits;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 TimerManager::getTimerCounter()
{
	return TimerManager::computeTimerCounter();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 TimerManager::getCurrentTimerCounter()
{
	return (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 TimerManager::getMinimumTimePerInterruptStep()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	switch(timerManager->targetTimePerInterrupttUnits)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 TimerManager::getElapsedMilliseconds()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	return timerManager->elapsedMilliseconds;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 TimerManager::getTotalElapsedMilliseconds()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	return timerManager->totalElapsedMilliseconds;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::wait(uint32 milliseconds)
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	// Declare as volatile to prevent the compiler to optimize currentMilliseconds away
	// Making the last assignment invalid
	volatile uint32 currentMilliseconds = timerManager->totalElapsedMilliseconds;
	uint32 waitStartTime = timerManager->totalElapsedMilliseconds;
	volatile uint32 *totalElapsedMilliseconds = (uint32*)&timerManager->totalElapsedMilliseconds;

	while ((*totalElapsedMilliseconds - waitStartTime) < milliseconds)
	{
		HardwareManager::halt();
	}

	timerManager->elapsedMilliseconds = currentMilliseconds;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::repeatMethodCall(uint32 callTimes, uint32 duration, ListenerObject object, void (*method)(ListenerObject, uint32))
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	if(!isDeleted(object) && method)
	{
		// Declare as volatile to prevent the compiler to optimize currentMilliseconds away
		// Making the last assignment invalid
		volatile uint32 currentMilliseconds = timerManager->elapsedMilliseconds;

		uint32 i = 0;

		for(; i < callTimes; i++)
		{
			TimerManager::wait(duration / callTimes);

			if(isDeleted(object))
			{
				return;
			}

			method(object, i);
		}

		timerManager->elapsedMilliseconds = currentMilliseconds;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::frameStarted(uint32 elapsedMicroseconds)
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	timerManager->elapsedMilliseconds = 0;
	timerManager->elapsedMicroseconds = 0;

	if(0 >= timerManager->interruptsPerGameFrame)
	{
		timerManager->elapsedMicrosecondsPerInterrupt = TimerManager::getTargetTimePerInterruptInUS();
	}
	else
	{
		timerManager->elapsedMicrosecondsPerInterrupt = elapsedMicroseconds / timerManager->interruptsPerGameFrame;
	}

	timerManager->interruptsPerGameFrame = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::nextSecondStarted()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

#ifndef __SHIPPING
#ifdef __SHOW_TIMER_MANAGER_STATUS
	int x = 1;
	int y = 1;
	
	PRINT_TEXT("TIMER STATUS", x, y++);
	PRINT_TEXT("Inter./sec.:          ", x, y);
	PRINT_INT(timerManager->interruptsPerSecond, x + 17, y++);
	PRINT_TEXT("Inter./frm:           ", x, y);
	PRINT_INT(timerManager->interruptsPerSecond / __TARGET_FPS, x + 17, y++);
	PRINT_TEXT("Aver. us/inter.:      ", x, y);
	PRINT_INT(__MICROSECONDS_PER_SECOND / timerManager->interruptsPerSecond, x + 17, y++);
	PRINT_TEXT("Real us/inter.:       ", x, y);
	PRINT_INT(timerManager->elapsedMicrosecondsPerInterrupt, x + 17, y++);
#endif
#endif

	timerManager->interruptsPerSecond = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::print(int32 x, int32 y)
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	Printing::text("TIMER CONFIG", x, y++, NULL);
	y++;

	switch(timerManager->resolution)
	{
		case __TIMER_20US:

			Printing::text("Resolution    20 US ", x, y++, NULL);
			break;

		case __TIMER_100US:

			Printing::text("Resolution    100 US ", x, y++, NULL);
			break;

		default:

			Printing::text("Resolution    ?      ", x, y++, NULL);
			break;
	}

	switch(timerManager->targetTimePerInterrupttUnits)
	{
		case kUS:

			Printing::text("US/interrupt        ", x, y, NULL);
			break;

		case kMS:

			Printing::text("MS/interrupt        ", x, y, NULL);
			break;

		default:

			Printing::text(" ?/interrupt        ", x, y, NULL);
			break;
	}

	Printing::int32(timerManager->targetTimePerInterrupt, x + 14, y++, NULL);

	Printing::text("Timer counter        ", x, y, NULL);
	Printing::int32(TimerManager::getTimerCounter(), x + 14, y++, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 TimerManager::computeTimerCounter()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	int16 timerCounter = 0;

	switch(timerManager->targetTimePerInterrupttUnits)
	{
		case kUS:

			timerCounter = __TIME_US(timerManager->targetTimePerInterrupt);
			break;

		case kMS:

			timerCounter = __TIME_MS(timerManager->targetTimePerInterrupt);
			break;

		default:
	
			NM_ASSERT(false, "TimerManager::setTargetTimePerInterruptUnits: wrong resolution scale");
			break;
	}

	return (uint16)(0 >= timerCounter ? 1 : timerCounter);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::enableInterrupt()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	timerManager->tcrValue |= __TIMER_INT;

	_hardwareRegisters[__TCR] = timerManager->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::disableInterrupt()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	timerManager->tcrValue &= ~__TIMER_INT;

	_hardwareRegisters[__TCR] = timerManager->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::setTimerResolution()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	timerManager->tcrValue = (timerManager->tcrValue & 0x0F) | timerManager->resolution;
	_hardwareRegisters[__TCR] = timerManager->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::clearStat()
{
	TimerManager timerManager = TimerManager::getInstance(NULL);

	_hardwareRegisters[__TCR] = (timerManager->tcrValue | __TIMER_ZCLR);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TimerManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->totalElapsedMilliseconds = 0;

	TimerManager::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TimerManager::destructor()
{
	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
