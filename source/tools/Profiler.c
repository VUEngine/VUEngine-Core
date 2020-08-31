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

#include <debugConfig.h>
#include <Profiler.h>
#include <Game.h>
#include <GameState.h>
#include <UIContainer.h>
#include <SpriteManager.h>
#include <HardwareManager.h>
#include <TimerManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


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
}

/**
 * Initialize manager
 */
void Profiler::initialize()
{
	Profiler::reset(this);

	this->initialized = true;
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

	if(0 < this->currentProfilingProcess)
	{
		this->printedProcessesNames = true;

		PRINT_TEXT("Total time", 1, ++this->currentProfilingProcess + 10 + 1);
		PRINT_FLOAT(this->totalTime, 18, this->currentProfilingProcess + 10 + 1);
		PRINT_FLOAT((this->totalTime * 100) / this->timePerInterruptInMS, 23, this->currentProfilingProcess + 10 + 1);
	}

	this->currentProfilingProcess = 0;
	this->previousTimerCounter = this->timerCounter;
	this->totalTime = 0;

	TimerManager::enable(this->timerManager, false);
	TimerManager::setTimerCounter(this->timerManager);
	TimerManager::enable(this->timerManager, true);

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

	if(!this->printedProcessesNames && NULL != processName)
	{
		PRINT_TEXT("PROFILER", 1, 7);
		PRINT_TEXT("                  ms    %", 1, 9);
		PRINT_TEXT("                    ", 1, this->currentProfilingProcess + 10);
		PRINT_TEXT(processName, 1, this->currentProfilingProcess + 10);
	}

	if(this->previousTimerCounter < currentTimerCounter)
	{
		this->previousTimerCounter += this->timerCounter;
	}

	u32 elapsedTicks = this->previousTimerCounter - currentTimerCounter;
	float elapsedTime = elapsedTicks * this->timeProportion;

	this->totalTime += elapsedTime;

	PRINT_TEXT("          ", 18, this->currentProfilingProcess + 10);
	PRINT_FLOAT(elapsedTime, 18, this->currentProfilingProcess + 10);
	PRINT_FLOAT((elapsedTime * 100) / this->timePerInterruptInMS, 23, this->currentProfilingProcess + 10);

	this->previousTimerCounter = currentTimerCounter;
	this->currentProfilingProcess++;
}


#endif
