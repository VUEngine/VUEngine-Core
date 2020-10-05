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
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}
};

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
	this->timerManager = TimerManager::getInstance();
	this->initialized = false;
	this->previousTimerCounter = 0;
	this->currentProfilingProcess = 0;
	this->printedProcessesNames = false;
	this->timerCounter = TimerManager::getTimerCounter(this->timerManager);
	this->timePerInterruptInMS = TimerManager::getTimePerInterruptInMS(this->timerManager);
	this->timeProportion = this->timePerInterruptInMS / (float)this->timerCounter;
	this->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;
	this->totalTime = 0;
	this->lastLapIndex = 0;

	for(int i = 0; i < 96; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = 2;
	}
}

/**
 * Initialize manager
 */
void Profiler::initialize()
{
	Profiler::reset(this);

	this->initialized = true;
/*
	_vipRegisters[__GPLT0] = 0x50;
	_vipRegisters[__GPLT1] = 0x50;
	_vipRegisters[__GPLT2] = 0x54;
	_vipRegisters[__GPLT3] = 0x54;
	_vipRegisters[__JPLT0] = 0x54;
	_vipRegisters[__JPLT1] = 0x54;
	_vipRegisters[__JPLT2] = 0x54;
	_vipRegisters[__JPLT3] = 0x54;

	_vipRegisters[0x30 | __PRINTING_PALETTE] = 0xE4;
	*/
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

	this->skipFrames = __ENABLE_PROFILER_SKIP_FRAMES;

	Printing::clear(Printing::getInstance());
	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);
	Printing::text(Printing::getInstance(), "\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 27, NULL);
	PRINT_TEXT("PROFILER", 1, 0);

	this->printedProcessesNames = true;

	PRINT_TEXT("Total time  ms    %", 25, 1);
	PRINT_FLOAT(this->totalTime, 25 + 11, 2);
	PRINT_FLOAT((this->totalTime * 100) / this->timePerInterruptInMS, 25 + 11 + 5, 2);

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

	__SET_BRIGHT(2, 2, 2);

	VIPManager::setupBrightnessRepeat(VIPManager::getInstance(), (BrightnessRepeatSpec*)&profileBrightnessRepeatSpec);

	for(int i = 0; i < 96; i++)
	{
		profileBrightnessRepeatSpec.brightnessRepeat[i] = 2;
	}
}

void Profiler::lap(const char* processName)
{
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
	float gameFrameTimePercentage = (elapsedTime * 100) / this->timePerInterruptInMS;

	this->totalTime += elapsedTime;

	int columnTableEntries = 96;
	u8 value = 0;

	if(this->currentProfilingProcess % 2)
	{
		value = 1;
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

	int printingColumn = this->lastLapIndex / 2;

	Printing::setOrientation(Printing::getInstance(), kPrintingOrientationVertical);
	Printing::setDirection(Printing::getInstance(), kPrintingDirectionRTL);
	Printing::text(Printing::getInstance(), processName, printingColumn, 26, "Profiler");

	Printing::setOrientation(Printing::getInstance(), kPrintingOrientationVertical);
	Printing::setDirection(Printing::getInstance(), kPrintingDirectionRTL);
	Printing::float(Printing::getInstance(), elapsedTime, printingColumn, 14 + (elapsedTime >= 10), "Profiler");

	Printing::setOrientation(Printing::getInstance(), kPrintingOrientationVertical);
	Printing::setDirection(Printing::getInstance(), kPrintingDirectionRTL);
	Printing::text(Printing::getInstance(), ":;", printingColumn, 11, "Profiler"); // "ms"

	Printing::setOrientation(Printing::getInstance(), kPrintingOrientationVertical);
	Printing::setDirection(Printing::getInstance(), kPrintingDirectionRTL);
	Printing::float(Printing::getInstance(), gameFrameTimePercentage, printingColumn, 8 + (gameFrameTimePercentage >= 10), "Profiler");

	Printing::setOrientation(Printing::getInstance(), kPrintingOrientationVertical);
	Printing::setDirection(Printing::getInstance(), kPrintingDirectionRTL);
	Printing::text(Printing::getInstance(), "/", printingColumn, 5, "Profiler"); // "%"

	this->lastLapIndex += entries;

	this->previousTimerCounter = currentTimerCounter;
	this->currentProfilingProcess++;
}


#endif
