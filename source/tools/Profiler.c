/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __ENABLE_PROFILER


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Profiler.h>
#include <VUEngine.h>
#include <HardwareManager.h>
#include <TimerManager.h>
#include <Utilities.h>
#include <VIPManager.h>
#include <debugConfig.h>

#define __ENABLE_PROFILER_SKIP_FRAMES				10


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

typedef struct Lap
{
	const char* processName;
	float elapsedTime;
	uint32 lapType;
	uint32 interruptFlags;
	uint8 column;
} Lap;


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


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			Profiler::getInstance()
 * @memberof	Profiler
 * @public
 * @return		Profiler instance
 */


/**
 * Class constructor
 *
 * @private
 */
void Profiler::constructor()
{
	Base::constructor();

	this->laps = new VirtualList();

	Profiler::reset(this);

	_printing = Printing::getInstance();
}

/**
 * Class destructor
 */
void Profiler::destructor()
{
	VirtualList::deleteData(this->laps);

	delete this->laps;
	this->laps = NULL;

	// allow a new construct
	Base::destructor();
}

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
	this->timeProportion = TimerManager::getTimePerInterruptInMS(this->timerManager) / (float)this->timerCounter;
	this->skipFrames = 1;
	this->totalTime = 0;
	this->lastLapIndex = 0;
	this->previousTimerCounter = 0;

	for(int32 i = 0; i < 96; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = 16;
	}
}

/**
 * Initialize manager
 */
void Profiler::initialize()
{
	Profiler::reset(this);
	Printing::resetCoordinates(Printing::getInstance());

	this->initialized = true;
	/**/
	_vipRegisters[__GPLT0] = 0b01010000;
	_vipRegisters[__GPLT1] = 0b01010000;
	_vipRegisters[__GPLT2] = 0b01010000;
	_vipRegisters[__GPLT3] = 0b01010000;
	_vipRegisters[__JPLT0] = 0b01010000;
	_vipRegisters[__JPLT1] = 0b01010000;
	_vipRegisters[__JPLT2] = 0b01010000;
	_vipRegisters[__JPLT3] = 0b01010000;

	_vipRegisters[0x30 | __PRINTING_PALETTE] = 0b11100000;
	/**/
}

void Profiler::wait(int16 delay)
{
	// Needed to give the timer enough time to reset its registers before this method is called again
	volatile int16 dummy = delay;

	while(0 < --dummy);
}

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
	this->totalTime = 0;
	this->lastLapIndex = 0;
	this->interruptFlags = 0;

	TimerManager::enable(this->timerManager, false);
	TimerManager::configureTimerCounter(this->timerManager);
	TimerManager::enable(this->timerManager, true);
	Profiler::wait(this, 1000);

	this->previousTimerCounter = TimerManager::getCurrentTimerCounter(this->timerManager);
}

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
	}

	this->started = false;
}

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

void Profiler::print()
{
	Printing::resetCoordinates(_printing);
	Printing::setWorldCoordinates(_printing, 0, 0, -8, 2);
	Printing::clear(_printing);
	Printing::text(_printing, "================================================", 0, 27, "Profiler");

	for(VirtualNode node = VirtualList::begin(this->laps); NULL != node; node = VirtualNode::getNext(node))
	{
		Lap* lap = (Lap*)VirtualNode::getData(node);
		Profiler::printValue(this, lap); 
	}
}

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

void Profiler::lap(uint32 lapType, const char* processName)
{
	if(!this->started)
	{
		return;
	}

	if(kProfilerLapTypeNormalProcess != lapType)
	{
		this->interruptFlags |= lapType;
		return;
	}

	Profiler::computeLap(this, processName, lapType, false);
}

void Profiler::computeLap(const char* processName, uint32 lapType, bool isHeadroom)
{
	HardwareManager::suspendInterrupts();

	TimerManager::enable(this->timerManager, false);
	uint16 currentTimerCounter = TimerManager::getCurrentTimerCounter(this->timerManager);

	if(this->previousTimerCounter < currentTimerCounter)
	{
		this->previousTimerCounter += this->timerCounter;
	}

	float elapsedTime = this->timePerGameFrameInMS - this->totalTime;

	if(0 > elapsedTime)
	{
		return;
	}

	if(!isHeadroom)
	{
		elapsedTime = (this->previousTimerCounter - currentTimerCounter) * this->timeProportion;
		this->totalTime += elapsedTime;
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

	Profiler::registerLap(this, processName, elapsedTime, lapType, printingColumn);

	this->lastLapIndex += entries;
	this->currentProfilingProcess++;
	this->previousTimerCounter = currentTimerCounter;

	if(isHeadroom)
	{
		Profiler::registerLap(this, "TOTAL", this->totalTime, lapType, 46);
	}

	TimerManager::enable(this->timerManager, true);

	HardwareManager::resumeInterrupts();
}

#endif
