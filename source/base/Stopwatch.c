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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Stopwatch.h>
#include <StopwatchManager.h>
#include <HardwareManager.h>
#include <TimerManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void Stopwatch::constructor()
{
	Base::constructor();

	Stopwatch::reset(this);

	// register clock
	StopwatchManager::register(StopwatchManager::getInstance(), this);
}

/**
 * Class destructor
 */
void Stopwatch::destructor()
{
	// unregister the clock
	StopwatchManager::unregister(StopwatchManager::getInstance(), this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Stopwatch::reset()
{
	this->interrupts = 0;
	this->milliSeconds = 0;
	this->previousTimerCounter = 0;
	this->timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance());
	this->timeProportion = TimerManager::getTimePerInterruptInMS(TimerManager::getInstance()) / (float)this->timerCounter;
	this->previousTimerCounter = (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];
}

void Stopwatch::update()
{
	this->interrupts++;
}

u32 Stopwatch::lap()
{
	extern u8* const _hardwareRegisters;

	TimerManager::enable(TimerManager::getInstance(), false);
	u16 currentTimerCounter = (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];
	TimerManager::enable(TimerManager::getInstance(), true);

	if(this->previousTimerCounter < currentTimerCounter)
	{
		if(0 < this->interrupts)
		{
			this->interrupts--;
		}

		this->previousTimerCounter += this->timerCounter;
	}
	
	float elapsedTime = ((this->previousTimerCounter + this->interrupts * this->timerCounter) - currentTimerCounter) * this->timeProportion;

	this->interrupts = 0;

	this->previousTimerCounter = currentTimerCounter;

	this->milliSeconds += (u32)elapsedTime;

	return (u32)elapsedTime;
}
