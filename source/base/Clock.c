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

#include <Clock.h>
#include <ClockManager.h>
#include <MessageDispatcher.h>
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
void Clock::print(int col, int row, const char* font)
{
	Clock::printTime(this->milliSeconds, col, row, font);
}

/**
 * Called on each timer interrupt
 *
 * @param millisecondsElapsed	Time elapsed between calls
 */
void Clock::update(u32 millisecondsElapsed)
{
	// increase count
	if(this->paused)
	{
		return;
	}

	this->milliSeconds += millisecondsElapsed;

	u32 currentSecond = Clock::getSeconds(this);

	if(currentSecond != this->previousSecond)
	{
		this->previousSecond = currentSecond;

		Clock::fireEvent(this, kEventSecondChanged);
		NM_ASSERT(!isDeleted(this), "Clock::update: deleted this during kEventSecondChanged");

		u32 currentMinute = Clock::getMinutes(this);

		if(currentMinute != this->previousMinute)
		{
			this->previousMinute = currentMinute;

			Clock::fireEvent(this, kEventMinuteChanged);
			NM_ASSERT(!isDeleted(this), "Clock::update: deleted this during kEventMinuteChanged");
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
u32 Clock::getMilliSeconds()
{
	return this->milliSeconds;
}

/**
 * Retrieve clock's seconds
 *
 * @return	Current seconds
 */
u32 Clock::getSeconds()
{
	return (u32)(this->milliSeconds / __MILLISECONDS_PER_SECOND);
}

/**
 * Retrieve clock's minutes
 *
 * @return	Current minutes
 */
u32 Clock::getMinutes()
{
	return (u32)(this->milliSeconds / (__MILLISECONDS_PER_SECOND * 60));
}

/**
 * Retrieve clock's total elapsed time in milliseconds
 *
 * @return	Current milliseconds
 */
u32 Clock::getTime()
{
	return this->milliSeconds;
}

/**
 * Retrieve current elapsed milliseconds in the current second
 *
 * @return	Elapsed milliseconds in the current second
 */
int Clock::getTimeInCurrentSecond()
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
void Clock::setTimeInMilliSeconds(u32 milliSeconds)
{
	this->milliSeconds = milliSeconds;
}

/**
 * Start the clock
 */
void Clock::start()
{	Clock::reset(this);

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
static void Clock::printTime(u32 milliSeconds, int col, int row, const char* font)
{
	char output[] = "00:00";
	u32 minutes = (u32)(milliSeconds / (__MILLISECONDS_PER_SECOND * 60));
	u32 seconds = (u32)(milliSeconds / __MILLISECONDS_PER_SECOND);

	char* minutesString = Utilities::itoa(minutes, 10, 2);

	output[0] = minutesString[0];
	output[1] = minutesString[1];

	char* secondsString = Utilities::itoa((seconds - minutes * 60) % 60, 10, 2);

	output[3] = secondsString[0];
	output[4] = secondsString[1];

	Printing::text(Printing::getInstance(), output, col, row, font);
}

/**
 * Print formatted class' attributes's states
 *
 * @param col
 * @param row
 * @param font
 */
static void Clock::printFullTime(u32 milliSeconds, int col, int row, const char* font)
{
	Clock::printTime(milliSeconds, col, row, font);

	u32 currentDeciseconds = ((milliSeconds + 50) / 100);
	currentDeciseconds -= ((currentDeciseconds / 10) * 10);

	Printing::int(Printing::getInstance(), currentDeciseconds, col + 6, row, font);
}