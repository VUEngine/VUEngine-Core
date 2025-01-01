/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __ENABLE_PROFILER


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <HardwareManager.h>
#include <Printing.h>
#include <TimerManager.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <VUEngine.h>

#include "Profiler.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __ENABLE_PROFILER_SKIP_FRAMES				5


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

typedef struct Lap
{
	const char* processName;
	float elapsedTime;
	uint32 lapType;
	uint32 interruptFlags;
	uint8 column;
} Lap;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


static BrightnessRepeatSpec profileBrightnessRepeatSpec =
{
	// mirror spec?
	false,

	// brightness repeat values
	{
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	}
};

static Printing _printing = NULL;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::initialize()
{
	Profiler::reset(this);
	Printing::resetCoordinates(Printing::getInstance());

	this->initialized = true;

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::reset()
{
	VirtualList::deleteData(this->laps);
	
	this->started = false;
	this->timerManager = TimerManager::getInstance();
	this->initialized = false;
	this->currentProfilingProcess = 0;
	this->printedProcessesNames = false;
	this->timerCounter = TimerManager::getTimerCounter(this->timerManager);
	this->timePerGameFrameInMS = VUEngine::getGameFrameDuration(VUEngine::getInstance());
	this->timeProportion = TimerManager::getTargetTimePerInterruptInMS(this->timerManager) / (float)this->timerCounter;
	this->skipFrames = 1;
	this->lastCycleTotalTime = 0;
	this->totalTime = 0;
	this->lastLapIndex = 0;
	this->previousTimerCounter = 0;
	this->cycles = 0;

	for(int32 i = 0; i < 96; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = 16;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::start()
{
	if(!this->initialized)
	{
		return;
	}

	Profiler::end(this);

	if(0 < --this->skipFrames)
	{
		return;
	}

	if(this->started)
	{
		return;
	}

	VirtualList::deleteData(this->laps);

	this->started = true;
	this->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;
	this->printedProcessesNames = true;
	this->currentProfilingProcess = 0;
	this->lastCycleTotalTime = 0;
	this->lastLapIndex = 0;
	this->interruptFlags = 0;

	TimerManager::disable(this->timerManager);
	TimerManager::resetTimerCounter(this->timerManager);
	TimerManager::enable(this->timerManager);
	Profiler::wait(this, 1000);

	this->previousTimerCounter = TimerManager::getCurrentTimerCounter(this->timerManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::end()
{
	if(this->started)
	{
		Profiler::computeLap(this, "HEADROOM", kProfilerLapTypeNormalProcess, true);

		VIPManager::setupBrightnessRepeat(VIPManager::getInstance(), (BrightnessRepeatSpec*)&profileBrightnessRepeatSpec);

		for(int32 i = 0; i < 96; i++)
		{
			profileBrightnessRepeatSpec.brightnessRepeat[i] = 16;
		}

		Profiler::print(this);
		this->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;
		this->timePerGameFrameInMS = VUEngine::getGameFrameDuration(VUEngine::getInstance());
	}

	this->started = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::lap(uint32 lapType, const char* processName)
{
	if(!this->started)
	{
		return;
	}

	if(kProfilerLapTypeNormalProcess != lapType)
	{
		if(kProfilerLapTypeStartInterrupt == lapType)
		{
			this->previousTimerCounter = TimerManager::getCurrentTimerCounter(this->timerManager);
			return;
		}
		else
		{
			this->interruptFlags |= lapType;
		}
	}

	Profiler::computeLap(this, processName, lapType, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->laps = new VirtualList();

	Profiler::reset(this);

	_printing = Printing::getInstance();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::destructor()
{
	VirtualList::deleteData(this->laps);

	delete this->laps;
	this->laps = NULL;

	// allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::wait(int16 delay)
{
	// Needed to give the timer enough time to reset its registers before this method is called again
	volatile int16 dummy = delay;

	while(0 < --dummy);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::registerLap(const char* processName, float elapsedTime, uint32 lapType, uint8 column)
{
	Lap* lap = new Lap;

	NM_ASSERT(0 <= elapsedTime, "Profiler::registerLap: negative elapsed time")

	lap->processName = NULL == processName ? "NO NAME" : processName;
	lap->elapsedTime = elapsedTime;
	lap->lapType = lapType;
	lap->column = column;
	lap->interruptFlags = this->interruptFlags;
	this->interruptFlags = 0;

	VirtualList::pushBack(this->laps, lap);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::computeLap(const char* processName, uint32 lapType, bool isHeadroom)
{
	HardwareManager::suspendInterrupts();

	TimerManager::disable(this->timerManager);

	uint16 currentTimerCounter = TimerManager::getCurrentTimerCounter(this->timerManager);

	if(this->previousTimerCounter < currentTimerCounter)
	{
		this->previousTimerCounter += this->timerCounter;
	}

	float elapsedTime = this->timePerGameFrameInMS - this->lastCycleTotalTime;

	if(0 > elapsedTime)
	{
		return;
	}

	if(!isHeadroom)
	{
		elapsedTime = (this->previousTimerCounter - currentTimerCounter) * this->timeProportion;
		this->lastCycleTotalTime += elapsedTime;
	}

	uint8 value = 0;

	if(this->currentProfilingProcess % 2)
	{
		value = 6;
	}
	else
	{
		value = 16;
	}

	int32 entries = 4;

	for(int32 i = this->lastLapIndex; i < this->lastLapIndex + entries && i; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = value;
	}

	uint8 printingColumn = this->lastLapIndex >> 1;

	Profiler::registerLap(this, processName, elapsedTime, lapType, printingColumn + 1);

	this->lastLapIndex += entries;
	this->currentProfilingProcess++;
	this->previousTimerCounter = currentTimerCounter;

	if(isHeadroom)
	{
		Profiler::registerLap(this, "TOTAL", this->lastCycleTotalTime, lapType, 46);
		this->cycles++;
		this->totalTime += this->lastCycleTotalTime;
		Profiler::registerLap(this, "AVERAGE", this->totalTime / this->cycles, lapType, 47);		
	}

	TimerManager::enable(this->timerManager);

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::print()
{
	Printing::resetCoordinates(_printing);
	Printing::setWorldCoordinates(_printing, 0, 0, -64, -3);
	Printing::clear(_printing);
	Printing::text(_printing, "================================================", 0, 27, "Profiler");

	for(VirtualNode node = VirtualList::begin(this->laps); NULL != node; node = VirtualNode::getNext(node))
	{
		Lap* lap = (Lap*)VirtualNode::getData(node);
		Profiler::printValue(this, lap); 
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Profiler::printValue(Lap* lap)
{
	if(NULL == lap->processName)
	{
		Printing::text(_printing, "<", lap->column, 27, "Profiler");
	}
	else
	{
		Printing::text(_printing, "<", lap->column, 27, "Profiler");

		Printing::setOrientation(_printing, kPrintingOrientationVertical);
		Printing::setDirection(_printing, kPrintingDirectionRTL);
		
		Printing::text(_printing, /*Utilities::toUppercase(*/lap->processName/*)*/, lap->column, 26, "Profiler");
		Printing::float(_printing, lap->elapsedTime, lap->column, 13 + (10 < lap->elapsedTime ? 1 : 0), 2, "Profiler");
		Printing::text(_printing, ":;", lap->column, 10, "Profiler"); // "ms"

		uint8 indicatorRow = 7;

		if(kProfilerLapTypeVIPInterruptFRAMESTARTProcess & lap->interruptFlags)
		{
			Printing::text(_printing, "F", lap->column, indicatorRow, "Profiler"); // "(x)"
			indicatorRow--;
		}

		if(kProfilerLapTypeVIPInterruptGAMESTARTProcess & lap->interruptFlags)
		{
			Printing::text(_printing, "G", lap->column, indicatorRow, "Profiler"); // "(x)"
			indicatorRow--;
		}

		if(kProfilerLapTypeVIPInterruptXPENDProcess & lap->interruptFlags)
		{
			Printing::text(_printing, "X", lap->column, indicatorRow, "Profiler"); // "(x)"
			indicatorRow--;
		}

		if(kProfilerLapTypeTimerInterruptProcess & lap->interruptFlags)
		{
			Printing::text(_printing, "T", lap->column, indicatorRow, "Profiler"); // "(s)"
			indicatorRow--;
		}

		if(kProfilerLapTypeCommunicationsInterruptProcess & lap->interruptFlags)
		{
			Printing::text(_printing, "C", lap->column, indicatorRow, "Profiler"); // "(c)"
			indicatorRow--;
		}

		Printing::setOrientation(_printing, kPrintingOrientationHorizontal);
		Printing::setDirection(_printing, kPrintingDirectionLTR);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


#endif