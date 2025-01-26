/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <HardwareManager.h>
#include <ListenerObject.h>
#include <Printer.h>
#include <Profiler.h>
#include <Singleton.h>
#include <SoundManager.h>
#include <StopwatchManager.h>
#include <VUEngine.h>

#include "TimerManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __MAXIMUM_TIME_PER_INTERRUPT_US 			(1.3f * 1000)
#define __MINIMUM_TIME_PER_INTERRUPT_MS				1
#define __MAXIMUM_TIME_PER_INTERRUPT_MS 			49

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::interruptHandler()
{
	TimerManager timerManager = TimerManager::getInstance();

	//disable
#ifndef __ENABLE_PROFILER
	TimerManager::disable(timerManager);
	TimerManager::clearStat(timerManager);
#else
	TimerManager::disableInterrupt(timerManager);

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
	SoundManager::playSounds(SoundManager::getInstance(), timerManager->elapsedMicrosecondsPerInterrupt);

	// Update Stopwatchs: no use is being done of them so this is commented out for now since it affects PCM playback
	//StopwatchManager::update();

// enable
#ifndef __ENABLE_PROFILER
	TimerManager::enable();
#else
	TimerManager::enableInterrupt(timerManager);
	Profiler::lap(kProfilerLapTypeTimerInterruptProcess, PROCESS_NAME_SOUND_PLAY);
#endif
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
	TimerManager timerManager = TimerManager::getInstance();

	TimerManager::disable();
	TimerManager::initialize(timerManager);

	if(enable)
	{
		TimerManager::enable();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::enable()
{
	TimerManager timerManager = TimerManager::getInstance();

	timerManager->tcrValue |= __TIMER_ENB | __TIMER_INT;

	_hardwareRegisters[__TCR] = timerManager->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::disable()
{
	TimerManager timerManager = TimerManager::getInstance();

	timerManager->tcrValue &= ~(__TIMER_ENB | __TIMER_INT);

	_hardwareRegisters[__TCR] = timerManager->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::resetTimerCounter()
{
	TimerManager timerManager = TimerManager::getInstance();

	uint16 timerCounter = TimerManager::computeTimerCounter(timerManager);

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

static void TimerManager::setResolution(uint16 resolution)
{
	TimerManager timerManager = TimerManager::getInstance();

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
				uint32 residue = targetTimePerInterrupt % TimerManager::getResolutionInUS(timerManager);

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
	TimerManager timerManager = TimerManager::getInstance();

	return timerManager->resolution;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 TimerManager::getResolutionInUS()
{
	TimerManager timerManager = TimerManager::getInstance();

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
	TimerManager timerManager = TimerManager::getInstance();

	int16 minimumTimePerInterrupt = 0;
	int16 maximumTimePerInterrupt = 1000;

	switch(timerManager->targetTimePerInterrupttUnits)
	{
		case kUS:

			minimumTimePerInterrupt = 
				TimerManager::getResolutionInUS(timerManager) + TimerManager::getResolutionInUS(timerManager) * __TIMER_COUNTER_DELTA;
			maximumTimePerInterrupt = __MAXIMUM_TIME_PER_INTERRUPT_US;
			break;

		case kMS:

			minimumTimePerInterrupt = __MINIMUM_TIME_PER_INTERRUPT_MS;
			maximumTimePerInterrupt = __MAXIMUM_TIME_PER_INTERRUPT_MS;
			break;

		default:

			ASSERT(false, "TimerManager::setTargetTimePerInterrupt: wrong resolution scale");
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
	timerManager->elapsedMicrosecondsPerInterrupt = TimerManager::getTargetTimePerInterruptInUS(timerManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 TimerManager::getTargetTimePerInterrupt()
{
	TimerManager timerManager = TimerManager::getInstance();

	return timerManager->targetTimePerInterrupt;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static float TimerManager::getTargetTimePerInterruptInMS()
{
	TimerManager timerManager = TimerManager::getInstance();

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

uint32 TimerManager::getTargetTimePerInterruptInUS()
{
	TimerManager timerManager = TimerManager::getInstance();

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
	TimerManager timerManager = TimerManager::getInstance();

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

uint16 TimerManager::getTargetTimePerInterruptUnits()
{
	TimerManager timerManager = TimerManager::getInstance();

	return timerManager->targetTimePerInterrupttUnits;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 TimerManager::getTimerCounter()
{
	TimerManager timerManager = TimerManager::getInstance();

	return TimerManager::computeTimerCounter(timerManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 TimerManager::getCurrentTimerCounter()
{
	return (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 TimerManager::getMinimumTimePerInterruptStep()
{
	TimerManager timerManager = TimerManager::getInstance();

	switch(timerManager->targetTimePerInterrupttUnits)
	{
		case kUS:
			return TimerManager::getResolutionInUS();
			break;

		case kMS:

			return 1;
			break;
	}

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 TimerManager::getElapsedMilliseconds()
{
	TimerManager timerManager = TimerManager::getInstance();

	return timerManager->elapsedMilliseconds;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 TimerManager::getTotalElapsedMilliseconds()
{
	TimerManager timerManager = TimerManager::getInstance();

	return timerManager->totalElapsedMilliseconds;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::wait(uint32 milliseconds)
{
	TimerManager timerManager = TimerManager::getInstance();

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
	TimerManager timerManager = TimerManager::getInstance();

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

static void TimerManager::print(int32 x, int32 y)
{
	TimerManager timerManager = TimerManager::getInstance();

	Printer::text("TIMER CONFIG", x, y++, NULL);
	y++;

	switch(timerManager->resolution)
	{
		case __TIMER_20US:

			Printer::text("Resolution    20 US ", x, y++, NULL);
			break;

		case __TIMER_100US:

			Printer::text("Resolution    100 US ", x, y++, NULL);
			break;

		default:

			Printer::text("Resolution    ?      ", x, y++, NULL);
			break;
	}

	switch(timerManager->targetTimePerInterrupttUnits)
	{
		case kUS:

			Printer::text("US/interrupt        ", x, y, NULL);
			break;

		case kMS:

			Printer::text("MS/interrupt        ", x, y, NULL);
			break;

		default:

			Printer::text(" ?/interrupt        ", x, y, NULL);
			break;
	}

	Printer::int32(timerManager->targetTimePerInterrupt, x + 14, y++, NULL);

	Printer::text("Timer counter        ", x, y, NULL);
	Printer::int32(TimerManager::getTimerCounter(timerManager), x + 14, y++, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TimerManager::printInterruptStats(int x, int y)
{
	TimerManager timerManager = TimerManager::getInstance();

	PRINT_TEXT("TIMER STATUS", x, y++);
	PRINT_TEXT("Inter./sec.:          ", x, y);
	PRINT_INT(timerManager->interruptsPerSecond, x + 17, y++);
	PRINT_TEXT("Inter./frm:           ", x, y);
	PRINT_INT(timerManager->interruptsPerSecond / __TARGET_FPS, x + 17, y++);
	PRINT_TEXT("Aver. us/inter.:      ", x, y);
	PRINT_INT(__MICROSECONDS_PER_SECOND / timerManager->interruptsPerSecond, x + 17, y++);
	PRINT_TEXT("Real us/inter.:       ", x, y);
	PRINT_INT(timerManager->elapsedMicrosecondsPerInterrupt, x + 17, y++);

	timerManager->interruptsPerSecond = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TimerManager::reset()
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void TimerManager::frameStarted(uint32 elapsedMicroseconds)
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

	TimerManager::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TimerManager::destructor()
{
	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TimerManager::initialize()
{
	TimerManager::disableInterrupt(this);
	TimerManager::setTimerResolution(this);
	TimerManager::clearStat(this);
	TimerManager::resetTimerCounter(this);
	TimerManager::enable(this);
	TimerManager::enableInterrupt(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TimerManager::enableInterrupt()
{
	this->tcrValue |= __TIMER_INT;

	_hardwareRegisters[__TCR] = this->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TimerManager::disableInterrupt()
{
	this->tcrValue &= ~__TIMER_INT;

	_hardwareRegisters[__TCR] = this->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TimerManager::setTimerResolution()
{
	this->tcrValue = (this->tcrValue & 0x0F) | this->resolution;
	_hardwareRegisters[__TCR] = this->tcrValue;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TimerManager::clearStat()
{
	_hardwareRegisters[__TCR] = (this->tcrValue | __TIMER_ZCLR);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
