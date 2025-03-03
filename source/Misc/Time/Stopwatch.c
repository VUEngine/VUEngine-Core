/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <StopwatchManager.h>
#include <TimerManager.h>
#include <VUEngine.h>

#include "Stopwatch.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stopwatch::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	Stopwatch::reset(this);

	// Register clock
	StopwatchManager::register(StopwatchManager::getInstance(), this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stopwatch::destructor()
{
	// Unregister the clock
	StopwatchManager::unregister(StopwatchManager::getInstance(), this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stopwatch::reset()
{
	this->interrupts = 0;
	this->milliSeconds = 0;
	this->timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance());
	this->timeProportion = TimerManager::getTargetTimePerInterruptInMS(TimerManager::getInstance()) / (float)this->timerCounter;
	this->previousTimerCounter = this->timerCounter;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stopwatch::update()
{
	this->interrupts++;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

float Stopwatch::lap()
{
	TimerManager::disable(TimerManager::getInstance());

	uint16 currentTimerCounter = TimerManager::getCurrentTimerCounter();

	uint16 timerCounter = 0;

	if(0 == this->interrupts)
	{
		if(currentTimerCounter > this->previousTimerCounter)
		{
			timerCounter = VUEngine::getGameFrameDuration() / this->timeProportion;
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

	TimerManager::enable(TimerManager::getInstance());

	return elapsedTime;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
