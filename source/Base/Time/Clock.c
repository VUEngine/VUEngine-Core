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

#include <ClockManager.h>
#include <Printing.h>
#include <Utilities.h>

#include "Clock.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Clock::printTime(uint32 milliseconds, int32 x, int32 y, const char* font, uint32 precision)
{
	char output[] = "00:00";
	uint32 minutes = (uint32)(milliseconds / (__MILLISECONDS_PER_SECOND * 60));
	uint32 seconds = (uint32)(milliseconds / __MILLISECONDS_PER_SECOND);

	char* minutesString = Utilities::itoa(minutes, 10, 2);

	output[0] = minutesString[0];
	output[1] = minutesString[1];

	char* secondsString = Utilities::itoa((seconds - minutes * 60) % 60, 10, 2);

	output[3] = secondsString[0];
	output[4] = secondsString[1];

	Printing::text(output, x, y, font);

	switch(precision)
	{
		case kTimePrecision1:

			Clock::printDeciseconds(milliseconds, x + 6, y, font);
			break;

		case kTimePrecision2:

			Clock::printCentiseconds(milliseconds, x + 6, y, font);
			break;

		case kTimePrecision3:

			Clock::printMilliseconds(milliseconds, x + 6, y, font);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Clock::printDeciseconds(uint32 milliseconds, int32 col, int32 row, const char* font)
{
	uint32 deciSeconds = ((milliseconds + 50) / 100);
	deciSeconds -= ((deciSeconds / 10) * 10);

	Printing::int32(deciSeconds, col, row, font);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Clock::printCentiseconds(uint32 milliseconds, int32 col, int32 row, const char* font)
{
	uint32 centiSeconds = ((milliseconds + 5) / 10);
	centiSeconds -= ((centiSeconds / 100) * 100);

	if(centiSeconds >= 10)
	{
		Printing::int32(centiSeconds, col, row, font);
	}
	else
	{
		Printing::int32(0, col, row, font);
		Printing::int32(centiSeconds, col + 1, row, font);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Clock::printMilliseconds(uint32 milliseconds, int32 col, int32 row, const char* font)
{
	milliseconds -= ((milliseconds / 1000) * 1000);

	if(milliseconds >= 100)
	{
		Printing::int32(milliseconds, col, row, font);
	}
	else if(milliseconds >= 10)
	{
		Printing::int32(0, col, row, font);
		Printing::int32(milliseconds, col + 1, row, font);
	}
	else
	{
		Printing::int32(0, col, row, font);
		Printing::int32(0, col + 1, row, font);
		Printing::int32(milliseconds, col + 2, row, font);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Clock::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Initialize time
	this->milliseconds = 0;

	// Initialize state
	this->paused = true;

	this->previousSecond = 0;
	this->previousMinute = 0;

	// Register clock
	ClockManager::register(ClockManager::getInstance(), this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Clock::destructor()
{
	// Unregister the clock
	ClockManager::unregister(ClockManager::getInstance(), this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Clock::start()
{
	Clock::reset(this);
	this->paused = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Clock::stop()
{
	Clock::reset(this);
	this->paused = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Clock::pause(bool pause)
{
	this->paused = pause;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Clock::reset()
{
	this->milliseconds = 0;
	this->previousSecond = 0;
	this->previousMinute = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Clock::update(uint32 elapsedMilliseconds)
{
	// Increase count
	if(this->paused)
	{
		return;
	}

	this->milliseconds += elapsedMilliseconds;

	if(NULL != this->events)
	{
		uint32 currentSecond = Clock::getSeconds(this);

		if(currentSecond != this->previousSecond)
		{
			this->previousSecond = currentSecond;

			Clock::fireEvent(this, kEventSecondChanged);
			NM_ASSERT(!isDeleted(this), "Clock::update: deleted this during kEventSecondChanged");

			uint32 currentMinute = Clock::getMinutes(this);

			if(currentMinute != this->previousMinute)
			{
				this->previousMinute = currentMinute;

				Clock::fireEvent(this, kEventMinuteChanged);
				NM_ASSERT(!isDeleted(this), "Clock::update: deleted this during kEventMinuteChanged");
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Clock::isPaused()
{
	return this->paused;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Clock::getMilliseconds()
{
	return this->milliseconds;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Clock::getSeconds()
{
	return (uint32)(this->milliseconds / __MILLISECONDS_PER_SECOND);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Clock::getMinutes()
{
	return (uint32)(this->milliseconds / (__MILLISECONDS_PER_SECOND * 60));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Clock::print(int32 col, int32 row, const char* font)
{
	Clock::printTime(this->milliseconds, col, row, font, kTimePrecision0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
