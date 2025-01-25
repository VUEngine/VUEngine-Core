/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with profiler source code.
 */

#ifdef __ENABLE_PROFILER

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <HardwareManager.h>
#include <Printer.h>
#include <Singleton.h>
#include <TimerManager.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <VUEngine.h>

#include "Profiler.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __ENABLE_PROFILER_SKIP_FRAMES				5

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

typedef struct Lap
{
	const char* processName;
	float elapsedTime;
	uint32 lapType;
	uint32 interruptFlags;
	uint8 column;
} Lap;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static BrightnessRepeatSpec profileBrightnessRepeatSpec =
{
	// Mirror spec?
	false,

	// Brightness repeat values
	{
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	}
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::initialize()
{
	Profiler profiler = Profiler::getInstance();

	Profiler::reset();
	Printer::resetCoordinates();

	profiler->initialized = true;

	_vipRegisters[__GPLT0] = 0x50;
	_vipRegisters[__GPLT1] = 0x50;
	_vipRegisters[__GPLT2] = 0x50;
	_vipRegisters[__GPLT3] = 0x50;
	_vipRegisters[__JPLT0] = 0x50;
	_vipRegisters[__JPLT1] = 0x50;
	_vipRegisters[__JPLT2] = 0x50;
	_vipRegisters[__JPLT3] = 0x50;

	_vipRegisters[0x30 | __PRINTING_PALETTE] = 0xE0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::reset()
{
	Profiler profiler = Profiler::getInstance();

	VirtualList::deleteData(profiler->laps);
	
	profiler->started = false;
	profiler->initialized = false;
	profiler->currentProfilingProcess = 0;
	profiler->printedProcessesNames = false;
	profiler->timerCounter = TimerManager::getTimerCounter();
	profiler->timePerGameFrameInMS = VUEngine::getGameFrameDuration();
	profiler->timeProportion = TimerManager::getTargetTimePerInterruptInMS() / (float)profiler->timerCounter;
	profiler->skipFrames = 1;
	profiler->lastCycleTotalTime = 0;
	profiler->totalTime = 0;
	profiler->lastLapIndex = 0;
	profiler->previousTimerCounter = 0;
	profiler->cycles = 0;

	for(int32 i = 0; i < 96; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = 16;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::start()
{
	Profiler profiler = Profiler::getInstance();

	if(!profiler->initialized)
	{
		return;
	}

	Profiler::end();

	if(0 < --profiler->skipFrames)
	{
		return;
	}

	if(profiler->started)
	{
		return;
	}

	VirtualList::deleteData(profiler->laps);

	profiler->started = true;
	profiler->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;
	profiler->printedProcessesNames = true;
	profiler->currentProfilingProcess = 0;
	profiler->lastCycleTotalTime = 0;
	profiler->lastLapIndex = 0;
	profiler->interruptFlags = 0;

	TimerManager::disable();
	TimerManager::resetTimerCounter();
	TimerManager::enable();
	Profiler::wait(1000);

	profiler->previousTimerCounter = TimerManager::getCurrentTimerCounter();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::end()
{
	Profiler profiler = Profiler::getInstance();

	if(profiler->started)
	{
		Profiler::computeLap("HEADROOM", kProfilerLapTypeNormalProcess, true);

		VIPManager::configureBrightnessRepeat((BrightnessRepeatSpec*)&profileBrightnessRepeatSpec);

		for(int32 i = 0; i < 96; i++)
		{
			profileBrightnessRepeatSpec.brightnessRepeat[i] = 16;
		}

		Profiler::print();
		profiler->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;
		profiler->timePerGameFrameInMS = VUEngine::getGameFrameDuration();
	}

	profiler->started = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::lap(uint32 lapType, const char* processName)
{
	Profiler profiler = Profiler::getInstance();

	if(!profiler->started)
	{
		return;
	}

	if(kProfilerLapTypeNormalProcess != lapType)
	{
		if(kProfilerLapTypeStartInterrupt == lapType)
		{
			profiler->previousTimerCounter = TimerManager::getCurrentTimerCounter();
			return;
		}
		else
		{
			profiler->interruptFlags |= lapType;
		}
	}

	Profiler::computeLap(processName, lapType, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::wait(int16 delay)
{
	// Needed to give the timer enough time to reset its registers before profiler method is called again
	volatile int16 dummy = delay;

	while(0 < --dummy);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::registerLap(const char* processName, float elapsedTime, uint32 lapType, uint8 column)
{
	Profiler profiler = Profiler::getInstance();

	Lap* lap = new Lap;

	NM_ASSERT(0 <= elapsedTime, "Profiler::registerLap: negative elapsed time")

	lap->processName = NULL == processName ? "NO NAME" : processName;
	lap->elapsedTime = elapsedTime;
	lap->lapType = lapType;
	lap->column = column;
	lap->interruptFlags = profiler->interruptFlags;
	profiler->interruptFlags = 0;

	VirtualList::pushBack(profiler->laps, lap);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::computeLap(const char* processName, uint32 lapType, bool isHeadroom)
{
	Profiler profiler = Profiler::getInstance();

	HardwareManager::suspendInterrupts();

	TimerManager::disable();

	uint16 currentTimerCounter = TimerManager::getCurrentTimerCounter();

	if(profiler->previousTimerCounter < currentTimerCounter)
	{
		profiler->previousTimerCounter += profiler->timerCounter;
	}

	float elapsedTime = profiler->timePerGameFrameInMS - profiler->lastCycleTotalTime;

	if(0 > elapsedTime)
	{
		return;
	}

	if(!isHeadroom)
	{
		elapsedTime = (profiler->previousTimerCounter - currentTimerCounter) * profiler->timeProportion;
		profiler->lastCycleTotalTime += elapsedTime;
	}

	uint8 value = 0;

	if(profiler->currentProfilingProcess % 2)
	{
		value = 6;
	}
	else
	{
		value = 16;
	}

	int32 entries = 4;

	for(int32 i = profiler->lastLapIndex; i < profiler->lastLapIndex + entries && i; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = value;
	}

	uint8 printingColumn = profiler->lastLapIndex >> 1;

	Profiler::registerLap(processName, elapsedTime, lapType, printingColumn + 1);

	profiler->lastLapIndex += entries;
	profiler->currentProfilingProcess++;
	profiler->previousTimerCounter = currentTimerCounter;

	if(isHeadroom)
	{
		Profiler::registerLap("TOTAL", profiler->lastCycleTotalTime, lapType, 46);
		profiler->cycles++;
		profiler->totalTime += profiler->lastCycleTotalTime;
		Profiler::registerLap("AVERAGE", profiler->totalTime / profiler->cycles, lapType, 47);		
	}

	TimerManager::enable();

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::print()
{
	Profiler profiler = Profiler::getInstance();

	Printer::resetCoordinates();
	Printer::setWorldCoordinates(0, 0, -64, +3);
	Printer::clear();
	Printer::text("================================================", 0, 27, "Profiler");

	for(VirtualNode node = VirtualList::begin(profiler->laps); NULL != node; node = VirtualNode::getNext(node))
	{
		Lap* lap = (Lap*)VirtualNode::getData(node);
		Profiler::printValue(lap); 
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Profiler::printValue(Lap* lap)
{
	if(NULL == lap->processName)
	{
		Printer::text("<", lap->column, 27, "Profiler");
	}
	else
	{
		Printer::text("<", lap->column, 27, "Profiler");

		Printer::setOrientation(kPrintingOrientationVertical);
		Printer::setTextDirection(kPrintingDirectionRTL);
		
		Printer::text(/*Utilities::toUppercase(*/lap->processName/*)*/, lap->column, 26, "Profiler");
		Printer::float(lap->elapsedTime, lap->column, 13 + (10 < lap->elapsedTime ? 1 : 0), 2, "Profiler");
		Printer::text(":;", lap->column, 10, "Profiler"); // "ms"

		uint8 indicatorRow = 7;

		if(kProfilerLapTypeVIPInterruptFRAMESTARTProcess & lap->interruptFlags)
		{
			Printer::text("F", lap->column, indicatorRow, "Profiler"); // "(x)"
			indicatorRow--;
		}

		if(kProfilerLapTypeVIPInterruptGAMESTARTProcess & lap->interruptFlags)
		{
			Printer::text("G", lap->column, indicatorRow, "Profiler"); // "(x)"
			indicatorRow--;
		}

		if(kProfilerLapTypeVIPInterruptXPENDProcess & lap->interruptFlags)
		{
			Printer::text("X", lap->column, indicatorRow, "Profiler"); // "(x)"
			indicatorRow--;
		}

		if(kProfilerLapTypeTimerInterruptProcess & lap->interruptFlags)
		{
			Printer::text("T", lap->column, indicatorRow, "Profiler"); // "(s)"
			indicatorRow--;
		}

		if(kProfilerLapTypeCommunicationsInterruptProcess & lap->interruptFlags)
		{
			Printer::text("C", lap->column, indicatorRow, "Profiler"); // "(c)"
			indicatorRow--;
		}

		Printer::setOrientation(kPrintingOrientationHorizontal);
		Printer::setTextDirection(kPrintingDirectionLTR);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->laps = new VirtualList();

	Profiler::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::destructor()
{
	VirtualList::deleteData(this->laps);

	delete this->laps;
	this->laps = NULL;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif