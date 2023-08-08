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

#include <Clock.h>

#include <ClockManager.h>
#include <MessageDispatcher.h>
#include <Printing.h>
#include <Utilities.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void Clock::constructor()
{
	Base::constructor();

	// initialize time
	this->milliSeconds = 0;

	// initialize state
	this->paused = true;

	this->previousSecond = 0;
	this->previousMinute = 0;

	// register clock
	ClockManager::register(ClockManager::getInstance(), this);
}

/**
 * Class destructor
 */
void Clock::destructor()
{
	// unregister the clock
	ClockManager::unregister(ClockManager::getInstance(), this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Print formatted class' attributes's states
 *
 * @param col
 * @param row
 * @param font
 */
void Clock::print(int32 col, int32 row, const char* font)
{
	Clock::printTime(this->milliSeconds, col, row, font, kTimePrecision0);
}

/**
 * Called on each timer interrupt
 *
 * @param millisecondsElapsed	Time elapsed between calls
 */
void Clock::update(uint32 millisecondsElapsed)
{
	// increase count
	if(this->paused)
	{
		return;
	}

	this->milliSeconds += millisecondsElapsed;

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

/**
 * Reset clock's attributes
 */
void Clock::reset()
{
	this->milliSeconds = 0;
	this->previousSecond = 0;
	this->previousMinute = 0;
}

/**
 * Retrieve clock's milliseconds
 *
 * @return	Current milliseconds
 */
uint32 Clock::getMilliSeconds()
{
	return this->milliSeconds;
}

/**
 * Retrieve clock's seconds
 *
 * @return	Current seconds
 */
uint32 Clock::getSeconds()
{
	return (uint32)(this->milliSeconds / __MILLISECONDS_PER_SECOND);
}

/**
 * Retrieve clock's minutes
 *
 * @return	Current minutes
 */
uint32 Clock::getMinutes()
{
	return (uint32)(this->milliSeconds / (__MILLISECONDS_PER_SECOND * 60));
}

/**
 * Retrieve clock's total elapsed time in milliseconds
 *
 * @return	Current milliseconds
 */
uint32 Clock::getTime()
{
	return this->milliSeconds;
}

/**
 * Retrieve current elapsed milliseconds in the current second
 *
 * @return	Elapsed milliseconds in the current second
 */
int32 Clock::getTimeInCurrentSecond()
{
	return __MILLISECONDS_PER_SECOND * (this->milliSeconds * 0.001f - __F_FLOOR(this->milliSeconds * 0.001f));
}

/**
 * Set clock's total elapsed time from seconds parameters
 *
 * @param totalSeconds
 */
void Clock::setTimeInSeconds(float totalSeconds)
{
	this->milliSeconds = totalSeconds * __MILLISECONDS_PER_SECOND;
}

/**
 * Set clock's total elapsed time
 *
 * @param milliSeconds
 */
void Clock::setTimeInMilliSeconds(uint32 milliSeconds)
{
	this->milliSeconds = milliSeconds;
}

/**
 * Start the clock
 */
void Clock::start()
{
	Clock::reset(this);

	this->paused = false;
}

/**
 * Stop the clock
 */
void Clock::stop()
{
	Clock::reset(this);
	this->paused = true;
}

/**
 * Pause the clock
 *
 * @param paused	Set to paused or unpaused?
 */
void Clock::pause(bool paused)
{
	this->paused = paused;
}

/**
 * Whether the clock is running or not
 *
 * @return	Paused flag
 */
bool Clock::isPaused()
{
	return this->paused;
}

/**
 * Print formatted class' attributes's states
 *
 * @param col
 * @param row
 * @param font
 */
static void Clock::printTime(uint32 milliSeconds, int32 col, int32 row, const char* font, uint32 precision)
{
	char output[] = "00:00";
	uint32 minutes = (uint32)(milliSeconds / (__MILLISECONDS_PER_SECOND * 60));
	uint32 seconds = (uint32)(milliSeconds / __MILLISECONDS_PER_SECOND);

	char* minutesString = Utilities::itoa(minutes, 10, 2);

	output[0] = minutesString[0];
	output[1] = minutesString[1];

	char* secondsString = Utilities::itoa((seconds - minutes * 60) % 60, 10, 2);

	output[3] = secondsString[0];
	output[4] = secondsString[1];

	Printing::text(Printing::getInstance(), output, col, row, font);

	switch(precision)
	{
		case kTimePrecision1:

			Clock::printDeciseconds(milliSeconds, col + 6, row, font);
			break;

		case kTimePrecision2:

			Clock::printCentiseconds(milliSeconds, col + 6, row, font);
			break;

		case kTimePrecision3:

			Clock::printMilliseconds(milliSeconds, col + 6, row, font);
			break;
	}
}

/**
 * Print formatted class' attributes's states
 *
 * @param col
 * @param row
 * @param font
 */
static void Clock::printDeciseconds(uint32 milliSeconds, int32 col, int32 row, const char* font)
{
	uint32 deciSeconds = ((milliSeconds + 50) / 100);
	deciSeconds -= ((deciSeconds / 10) * 10);

	Printing::int32(Printing::getInstance(), deciSeconds, col, row, font);
}

/**
 * Print formatted class' attributes's states
 *
 * @param col
 * @param row
 * @param font
 */
static void Clock::printCentiseconds(uint32 milliSeconds, int32 col, int32 row, const char* font)
{
	uint32 centiSeconds = ((milliSeconds + 5) / 10);
	centiSeconds -= ((centiSeconds / 100) * 100);

	if(centiSeconds >= 10)
	{
		Printing::int32(Printing::getInstance(), centiSeconds, col, row, font);
	}
	else
	{
		Printing::int32(Printing::getInstance(), 0, col, row, font);
		Printing::int32(Printing::getInstance(), centiSeconds, col + 1, row, font);
	}
}

/**
 * Print formatted class' attributes's states
 *
 * @param col
 * @param row
 * @param font
 */
static void Clock::printMilliseconds(uint32 milliSeconds, int32 col, int32 row, const char* font)
{
	milliSeconds -= ((milliSeconds / 1000) * 1000);

	if(milliSeconds >= 100)
	{
		Printing::int32(Printing::getInstance(), milliSeconds, col, row, font);
	}
	else if(milliSeconds >= 10)
	{
		Printing::int32(Printing::getInstance(), 0, col, row, font);
		Printing::int32(Printing::getInstance(), milliSeconds, col + 1, row, font);
	}
	else
	{
		Printing::int32(Printing::getInstance(), 0, col, row, font);
		Printing::int32(Printing::getInstance(), 0, col + 1, row, font);
		Printing::int32(Printing::getInstance(), milliSeconds, col + 2, row, font);
	}
}