/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
	this->timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance());
	this->timeProportion = TimerManager::getTimePerInterruptInMS(TimerManager::getInstance()) / (float)this->timerCounter;
	this->previousTimerCounter = this->timerCounter;
}

void Stopwatch::update()
{
	this->interrupts++;
}

float Stopwatch::lap()
{
	extern uint8* const _hardwareRegisters;

	TimerManager::enable(TimerManager::getInstance(), false);
	uint16 currentTimerCounter = (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];

	uint16 timerCounter = 0;

	if(0 == this->interrupts)
	{
		if(currentTimerCounter > this->previousTimerCounter)
		{
			timerCounter = __GAME_FRAME_DURATION / this->timeProportion;
		}
		else
		{
			timerCounter = this->previousTimerCounter - currentTimerCounter;
		}
	}
	else if(1 == this->interrupts)
	{
		timerCounter = this->previousTimerCounter + (this->timerCounter - currentTimerCounter);
	}
	else
	{
		timerCounter = this->previousTimerCounter + (this->timerCounter - currentTimerCounter);
		timerCounter += (this->interrupts - 1) * this->timerCounter;
	}

	float elapsedTime = timerCounter * this->timeProportion;

	this->interrupts = 0;

	this->previousTimerCounter = currentTimerCounter;

	this->milliSeconds += elapsedTime;

	TimerManager::enable(TimerManager::getInstance(), true);

	return elapsedTime;
}
