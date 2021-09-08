/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

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

	Profiler::reset(this);

	_printing = Printing::getInstance();
}

/**
 * Class destructor
 */
void Profiler::destructor()
{
	// allow a new construct
	Base::destructor();
}

void Profiler::reset()
{
	this->lapTypeFlags = 0;
	this->started = false;
	this->timerManager = TimerManager::getInstance();
	this->initialized = false;
	this->previousTimerCounter = 0;
	this->currentProfilingProcess = 0;
	this->printedProcessesNames = false;
	this->timerCounter = TimerManager::getTimerCounter(this->timerManager);
	this->timePerGameFrameInMS = __GAME_FRAME_DURATION;
	this->timeProportion = TimerManager::getTimePerInterruptInMS(this->timerManager) / (float)this->timerCounter;
	this->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;
	this->totalTime = 0;
	this->lastLapIndex = 0;

	for(int i = 0; i < 96; i++)
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

	Printing::setWorldCoordinates(_printing, 0, 0, -8, 2);

	this->started = true;
	this->lapTypeFlags = 0;

	this->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;

	Printing::clear(_printing);
	Printing::text(_printing, "================================================", 0, 27, "Profiler");

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

	Profiler::computeLap(this, "HEADROOM", true);

	Brightness brightness =
	{
		2,
		2,
		6
	};
	
	VIPManager::setupBrightness(VIPManager::getInstance(), &brightness);

	VIPManager::setupBrightnessRepeat(VIPManager::getInstance(), (BrightnessRepeatSpec*)&profileBrightnessRepeatSpec);

	for(int i = 0; i < 96; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = 16;
	}
}

void Profiler::printValue(const char* processName, float elapsedTime, uint8 column)
{
	if(NULL == processName)
	{
		Printing::text(_printing, "<", column, 27, "Profiler");
	}
	else
	{

		Printing::text(_printing, "<", column, 27, "Profiler");

		Printing::setOrientation(_printing, kPrintingOrientationVertical);
		Printing::setDirection(_printing, kPrintingDirectionRTL);
		
		Printing::text(_printing, /*Utilities::toUppercase(*/processName/*)*/, column, 26, "Profiler");
		Printing::float(_printing, elapsedTime, column, 14 + (10 > elapsedTime ? 0 : 1), 2, "Profiler");
		Printing::text(_printing, ":;", column, 10, "Profiler"); // "ms"

		uint8 indicatorRow = 8;

		if(kProfilerLapTypeVIPInterruptProcess & this->lapTypeFlags)
		{
			Printing::text(_printing, ">", column, indicatorRow, "Profiler"); // "(x)"
			indicatorRow--;
			this->lapTypeFlags &= ~kProfilerLapTypeVIPInterruptProcess;
		}

		if(kProfilerLapTypeTimerInterruptProcess & this->lapTypeFlags)
		{
			Printing::text(_printing, "?", column, indicatorRow, "Profiler"); // "(s)"
			indicatorRow--;
			this->lapTypeFlags &= ~kProfilerLapTypeTimerInterruptProcess;
		}

		if(kProfilerLapTypeCommunicationsInterruptProcess & this->lapTypeFlags)
		{
			Printing::text(_printing, "@", column, indicatorRow, "Profiler"); // "(c)"
			indicatorRow--;
			this->lapTypeFlags &= ~kProfilerLapTypeCommunicationsInterruptProcess;
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

	Profiler::computeLap(this, processName, false);
	this->lapTypeFlags |= lapType;
}

void Profiler::computeLap(const char* processName, bool isHeadroom)
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

	TimerManager::enable(this->timerManager, true);

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

	int columnTableEntries = 96 - 2;

	int entries = (int)(((columnTableEntries * gameFrameTimePercentage) / (float)100) + 0.5f) * 4;

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

	int entries = 4;

	for(int i = this->lastLapIndex; i < this->lastLapIndex + entries && i; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = value;
	}

	uint8 printingColumn = this->lastLapIndex / 2;

	Profiler::printValue(this, processName, elapsedTime, printingColumn);
	this->lastLapIndex += entries;
	this->previousTimerCounter = currentTimerCounter;
	this->currentProfilingProcess++;

	if(isHeadroom)
	{
		Profiler::printValue(this, "TOTAL", this->totalTime, 46);
	}

	HardwareManager::enableInterrupts();
}

#endif
