/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Clock
 * @extends Object
 * @ingroup base
 */



//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 */
void Clock::constructor(Clock this)
{
	ASSERT(this, "Clock::constructor: null this");

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
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 */
void Clock::destructor(Clock this)
{
	ASSERT(this, "Clock::destructor: null this");

	// unregister the clock
	ClockManager::unregister(ClockManager::getInstance(), this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Print formatted class' attributes's states
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 * @param col
 * @param row
 * @param font
 */
void Clock::print(Clock this, int col, int row, const char* font)
{
	ASSERT(this, "Clock::print: null this");

	char output[] = "00:00";
	char* minutes = Utilities::itoa(Clock::getMinutes(this), 10, 2);

	output[0] = minutes[0];
	output[1] = minutes[1];

	char* seconds = Utilities::itoa((Clock::getSeconds(this) - Clock::getMinutes(this) * 60) % 60, 10, 2);

	output[3] = seconds[0];
	output[4] = seconds[1];

	Printing::text(Printing::getInstance(), output, col, row, font);
}

/**
 * Called on each timer interrupt
 *
 * @memberof						Clock
 * @public
 *
 * @param this						Function scope
 * @param millisecondsElapsed		Time elapsed between calls
 */
void Clock::update(Clock this, u32 millisecondsElapsed)
{
	ASSERT(this, "Clock::update: null this");

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

		Object::fireEvent(__SAFE_CAST(Object, this), kEventSecondChanged);

		u32 currentMinute = Clock::getMinutes(this);

		if(currentMinute != this->previousMinute)
		{
			this->previousMinute = currentMinute;

			Object::fireEvent(__SAFE_CAST(Object, this), kEventMinuteChanged);
		}
	}
}

/**
 * Reset clock's attributes
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 */
void Clock::reset(Clock this)
{
	ASSERT(this, "Clock::reset: null this");

	this->milliSeconds = 0;
	this->previousSecond = 0;
	this->previousMinute = 0;
}

/**
 * Retrieve clock's milliseconds
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 *
 * @return		Current milliseconds
 */
u32 Clock::getMilliSeconds(Clock this)
{
	ASSERT(this, "Clock::getMilliSeconds: null this");

	return this->milliSeconds;
}

/**
 * Retrieve clock's seconds
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 *
 * @return		Current seconds
 */
u32 Clock::getSeconds(Clock this)
{
	ASSERT(this, "Clock::getSeconds: null this");

	return (u32)(this->milliSeconds / __MILLISECONDS_IN_SECOND);
}

/**
 * Retrieve clock's minutes
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 *
 * @return		Current minutes
 */
u32 Clock::getMinutes(Clock this)
{
	ASSERT(this, "Clock::getMinutes: null this");

	return (u32)(this->milliSeconds / (__MILLISECONDS_IN_SECOND * 60));
}

/**
 * Retrieve clock's total elapsed time in milliseconds
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 *
 * @return		Current milliseconds
 */
u32 Clock::getTime(Clock this)
{
	ASSERT(this, "Clock::getTime: null this");

	return this->milliSeconds;
}

/**
 * Retrieve current elapsed milliseconds in the current second
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 *
 * @return		Elapsed milliseconds in the current second
 */
int Clock::getTimeInCurrentSecond(Clock this)
{
	ASSERT(this, "Clock::getTimeInCurrentSecond: null this");

	return __MILLISECONDS_IN_SECOND * (this->milliSeconds * 0.001f - __F_FLOOR(this->milliSeconds * 0.001f));
}

/**
 * Set clock's total elapsed time from seconds parameters
 *
 * @memberof			Clock
 * @public
 *
 * @param this			Function scope
 * @param totalSeconds
 */
void Clock::setTimeInSeconds(Clock this, float totalSeconds)
{
	ASSERT(this, "Clock::setTimeInSeconds: null this");

	this->milliSeconds = totalSeconds * __MILLISECONDS_IN_SECOND;
}

/**
 * Set clock's total elapsed time
 *
 * @memberof			Clock
 * @public
 *
 * @param this			Function scope
 * @param milliSeconds
 */
void Clock::setTimeInMilliSeconds(Clock this, u32 milliSeconds)
{
	ASSERT(this, "Clock::setTimeInSeconds: null this");

	this->milliSeconds = milliSeconds;
}

/**
 * Start the clock
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 */
void Clock::start(Clock this)
{
	ASSERT(this, "Clock::start: null this");
	Clock::reset(this);

	this->paused = false;
}

/**
 * Stop the clock
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 */
void Clock::stop(Clock this)
{
	ASSERT(this, "Clock::stop: null this");

	Clock::reset(this);
	this->paused = true;
}

/**
 * Pause the clock
 *
 * @memberof		Clock
 * @public
 *
 * @param this		Function scope
 * @param paused	Set to paused or unpaused?
 */
void Clock::pause(Clock this, bool paused)
{
	ASSERT(this, "Clock::pause: null this");

	this->paused = paused;
}

/**
 * Whether the clock is running or not
 *
 * @memberof	Clock
 * @public
 *
 * @param this	Function scope
 *
 * @return		Paused flag
 */
bool Clock::isPaused(Clock this)
{
	ASSERT(this, "Clock::isPaused: null this");

	return this->paused;
}
