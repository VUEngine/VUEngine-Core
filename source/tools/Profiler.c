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
#include <Game.h>
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
	this->previousTimerCounter = 0;
	this->currentProfilingProcess = 0;
	this->printedProcessesNames = false;
	this->timerCounter = TimerManager::getTimerCounter(this->timerManager);
	this->timePerGameFrameInMS = Game::getGameFrameDuration(Game::getInstance());
	this->timeProportion = TimerManager::getTimePerInterruptInMS(this->timerManager) / (float)this->timerCounter;
	this->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;
	this->totalTime = 0;
	this->lastLapIndex = 0;

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

void Profiler::start()
{
	if(!this->initialized)
	{
		return;
	}

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
	this->previousTimerCounter = this->timerCounter;
	this->totalTime = 0;
	this->lastLapIndex = 0;

	TimerManager::enable(this->timerManager, false);
	TimerManager::configureTimerCounter(this->timerManager);
	TimerManager::enable(this->timerManager, true);
}

void Profiler::end()
{
	if(!this->initialized || __ENABLE_PROFILER_SKIP_FRAMES != this->skipFrames)
	{
		return;
	}

	if(!this->started)
	{
		return;
	}

	this->started = false;

	Profiler::computeLap(this, "HEADROOM", kProfilerLapTypeNormalProcess, true);

	Brightness brightness =
	{
		2,
		2,
		6
	};
	
	VIPManager::setupBrightness(VIPManager::getInstance(), &brightness);

	VIPManager::setupBrightnessRepeat(VIPManager::getInstance(), (BrightnessRepeatSpec*)&profileBrightnessRepeatSpec);

	for(int32 i = 0; i < 96; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = 16;
	}

	Profiler::print(this);
}

void Profiler::print()
{
	Printing::setWorldCoordinates(_printing, 0, 0, -8, 2);
	Printing::clear(_printing);
	Printing::text(_printing, "================================================", 0, 27, "Profiler");

	uint32 previousLapType = kProfilerLapTypeNormalProcess;

	for(VirtualNode node = VirtualList::begin(this->laps); NULL != node; node = VirtualNode::getNext(node))
	{
		Lap* lap = (Lap*)VirtualNode::getData(node);
		Profiler::printValue(this, lap, previousLapType); 
		previousLapType = lap->lapType;
	}
}

void Profiler::registerLap(const char* processName, float elapsedTime, uint32 lapType, uint8 column)
{
	Lap* lap = new Lap;

	lap->processName = NULL == processName ? "NO NAME" : processName;
	lap->elapsedTime = elapsedTime;
	lap->lapType = lapType;
	lap->column = column;

	VirtualList::pushBack(this->laps, lap);
}

void Profiler::printValue(Lap* lap, uint32 previousLapType)
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
		Printing::float(_printing, lap->elapsedTime, lap->column, 14 + (10 > lap->elapsedTime ? 0 : 1), 2, "Profiler");
		Printing::text(_printing, ":;", lap->column, 10, "Profiler"); // "ms"

		uint8 indicatorRow = 8;

		if(kProfilerLapTypeVIPInterruptProcess & previousLapType)
		{
			Printing::text(_printing, ">", lap->column, indicatorRow, "Profiler"); // "(x)"
			indicatorRow--;
		}

		if(kProfilerLapTypeTimerInterruptProcess & previousLapType)
		{
			Printing::text(_printing, "?", lap->column, indicatorRow, "Profiler"); // "(s)"
			indicatorRow--;
		}

		if(kProfilerLapTypeCommunicationsInterruptProcess & previousLapType)
		{
			Printing::text(_printing, "@", lap->column, indicatorRow, "Profiler"); // "(c)"
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

	Profiler::computeLap(this, processName, lapType, false);
}

void Profiler::computeLap(const char* processName, uint32 lapType, bool isHeadroom)
{
	if(!this->started && !isHeadroom)
	{
		return;
	}

	if(!this->initialized || __ENABLE_PROFILER_SKIP_FRAMES != this->skipFrames)
	{
		return;
	}

	HardwareManager::disableInterrupts();

	TimerManager::enable(this->timerManager, false);
	uint16 currentTimerCounter = (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];

	if(this->previousTimerCounter < currentTimerCounter)
	{
		this->previousTimerCounter += this->timerCounter;
	}

	float elapsedTime = this->timePerGameFrameInMS - this->totalTime;

	if(!isHeadroom)
	{
		elapsedTime = (this->previousTimerCounter - currentTimerCounter) * this->timeProportion;
		this->totalTime += elapsedTime;
	}

/*
	float gameFrameTimePercentage = (elapsedTime * 100) / this->timePerGameFrameInMS;

	int32 columnTableEntries = 96 - 2;

	int32 entries = (int32)(((columnTableEntries * gameFrameTimePercentage) / (float)100) + 0.5f) * 4;

	entries = (entries + (entries % 8)) / 4;

	if(2 > entries)
	{
		entries = 2;
	}
*/
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

	uint8 printingColumn = this->lastLapIndex / 2;

	Profiler::registerLap(this, processName, elapsedTime, lapType, printingColumn);

	this->lastLapIndex += entries;
	this->previousTimerCounter = currentTimerCounter;
	this->currentProfilingProcess++;

	if(isHeadroom)
	{
		Profiler::registerLap(this, "TOTAL", this->totalTime, lapType, 46);
	}

	TimerManager::enable(this->timerManager, true);

	HardwareManager::enableInterrupts();
}

#endif
