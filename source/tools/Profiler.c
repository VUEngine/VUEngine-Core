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
	this->xpend = false;
	this->playedMIDISounds = false;
	this->processedCommunications = false;
	this->started = false;
	this->timerManager = TimerManager::getInstance();
	this->initialized = false;
	this->previousTimerCounter = 0;
	this->currentProfilingProcess = 0;
	this->printedProcessesNames = false;
	this->timerCounter = TimerManager::getTimerCounter(this->timerManager);
	this->timePerGameFrameInMS = __MILLISECONDS_PER_SECOND / __TARGET_FPS;
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

	this->started = true;
	this->xpend = false;
	this->playedMIDISounds = false;
	this->processedCommunications = false;

	this->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;

	Printing::clear(_printing);
	Printing::text(_printing, "================================================", 0, 27, "Profiler");

	this->printedProcessesNames = true;

	Profiler::printValue(this, "TOTAL", this->totalTime, (this->totalTime * 100) / this->timePerGameFrameInMS, 47);

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

	Profiler::finalLap(this, "HEADROOM");

	__SET_BRIGHT(2, 2, 2);

	VIPManager::setupBrightnessRepeat(VIPManager::getInstance(), (BrightnessRepeatSpec*)&profileBrightnessRepeatSpec);

	for(int i = 0; i < 96; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = 16;
	}

	this->started = false;
}

void Profiler::xpend()
{
	this->xpend = true;
}

void Profiler::playedMIDISounds()
{
	this->playedMIDISounds = true;
}

void Profiler::processedCommunications()
{
	this->processedCommunications = true;
}

void Profiler::printValue(const char* processName, float elapsedTime, float gameFrameTimePercentage, u8 column)
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

		Printing::setOrientation(_printing, kPrintingOrientationVertical);
		Printing::setDirection(_printing, kPrintingDirectionRTL);
		Printing::float(_printing, elapsedTime, column, 14 + (elapsedTime >= 10), "Profiler");

		Printing::setOrientation(_printing, kPrintingOrientationVertical);
		Printing::setDirection(_printing, kPrintingDirectionRTL);
		Printing::text(_printing, ":;", column, 11, "Profiler"); // "ms"

		u8 indicatorRow = 9;

		if(this->xpend)
		{
			Printing::text(_printing, ">", column, indicatorRow, "Profiler"); // "(x)"
			indicatorRow--;
			this->xpend = false;
		}

		if(this->playedMIDISounds)
		{
			Printing::text(_printing, "?", column, indicatorRow, "Profiler"); // "(s)"
			indicatorRow--;
			this->playedMIDISounds = false;
		}

		if(this->processedCommunications)
		{
			Printing::text(_printing, "@", column, indicatorRow, "Profiler"); // "(c)"
			indicatorRow--;
			this->processedCommunications = false;
		}

/*
		Printing::setOrientation(_printing, kPrintingOrientationVertical);
		Printing::setDirection(_printing, kPrintingDirectionRTL);
		Printing::float(_printing, gameFrameTimePercentage, column, 7 + (gameFrameTimePercentage >= 10) + (gameFrameTimePercentage >= 100), "Profiler");

		Printing::setOrientation(_printing, kPrintingOrientationVertical);
		Printing::setDirection(_printing, kPrintingDirectionRTL);
		Printing::text(_printing, "/", column, 4, "Profiler"); // "%"
*/
	}
}

void Profiler::lap(const char* processName)
{
	if(!this->started)
	{
		return;
	}

	if(!this->initialized || __ENABLE_PROFILER_SKIP_FRAMES != this->skipFrames)
	{
		return;
	}

	TimerManager::enable(this->timerManager, false);
	u16 currentTimerCounter = (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];

	TimerManager::enable(this->timerManager, true);

	if(this->previousTimerCounter < currentTimerCounter)
	{
		this->previousTimerCounter += this->timerCounter;
	}

	u32 elapsedTicks = this->previousTimerCounter - currentTimerCounter;
	float elapsedTime = elapsedTicks * this->timeProportion;
	float gameFrameTimePercentage = (elapsedTime * 100) / this->timePerGameFrameInMS;

	this->totalTime += elapsedTime;

	int columnTableEntries = 96 - 2;
	u8 value = 0;

	if(this->currentProfilingProcess % 2)
	{
		value = 6;
	}
	else
	{
		value = 16;
	}

	int entries = (int)(((columnTableEntries * gameFrameTimePercentage) / (float)100) + 0.5f) * 4;

	entries = (entries + (entries % 8)) / 4;

	if(2 > entries)
	{
		entries = 2;
	}

	for(int i = this->lastLapIndex; i < this->lastLapIndex + entries && i < columnTableEntries; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = value;
	}

	u8 printingColumn = this->lastLapIndex / 2;

	Profiler::printValue(this, processName, elapsedTime, gameFrameTimePercentage, printingColumn);

	this->lastLapIndex += entries;

	this->previousTimerCounter = currentTimerCounter;
	this->currentProfilingProcess++;
}

void Profiler::finalLap(const char* processName)
{
	if(!this->started)
	{
		return;
	}

	if(!this->initialized || __ENABLE_PROFILER_SKIP_FRAMES != this->skipFrames)
	{
		return;
	}

	TimerManager::enable(this->timerManager, false);
	u16 currentTimerCounter = (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];

	TimerManager::enable(this->timerManager, true);

	if(this->previousTimerCounter < currentTimerCounter)
	{
		this->previousTimerCounter += this->timerCounter;
	}

	float elapsedTime = 20 - this->totalTime;
	float gameFrameTimePercentage = (elapsedTime * 100) / this->timePerGameFrameInMS;

	int columnTableEntries = 96 - 2;
	u8 value = 0;

	if(this->currentProfilingProcess % 2)
	{
		value = 6;
	}
	else
	{
		value = 16;
	}

	int entries = (int)(((columnTableEntries * gameFrameTimePercentage) / (float)100) + 0.5f) * 4;

	entries = (entries + (entries % 8)) / 4;

	if(2 > entries)
	{
		entries = 2;
	}

	for(int i = this->lastLapIndex; i < this->lastLapIndex + entries && i < columnTableEntries; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = value;
	}

	u8 printingColumn = this->lastLapIndex / 2;

	Profiler::printValue(this, processName, elapsedTime, gameFrameTimePercentage, printingColumn);

	this->lastLapIndex += entries;

	this->previousTimerCounter = currentTimerCounter;
	this->currentProfilingProcess++;
}

#endif
