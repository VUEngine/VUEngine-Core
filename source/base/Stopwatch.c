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
	this->previousTimerCounter = 0;
	this->timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance());
	this->timeProportion = TimerManager::getTimePerInterruptInMS(TimerManager::getInstance()) / (float)this->timerCounter;
	this->previousTimerCounter = (_hardwareRegisters[__THR] << 8 ) | _hardwareRegisters[__TLR];
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

	this->milliSeconds += (uint32)elapsedTime;

	return elapsedTime;
}
