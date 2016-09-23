/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Clock.h>
#include <ClockManager.h>
#include <MessageDispatcher.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class       Clock
 * @extends     Object
 *
 * @var u32     milliSeconds
 * @brief       time elapsed
 * @memberof    Clock
 *
 * @var u32     previousSecond
 * @brief       register
 * @memberof    Clock
 *
 * @var u32     previousMinute
 * @brief       register
 * @memberof    Clock
 *
 * @var bool    paused
 * @brief       flag to pause the clock
 * @memberof    Clock
 */

__CLASS_DEFINITION(Clock, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Clock_constructor(Clock this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Clock)
__CLASS_NEW_END(Clock);

/**
 * Class constructor
 *
 * @memberof    Clock
 * @private
 *
 * @param this  Function scope
 */
static void Clock_constructor(Clock this)
{
	ASSERT(this, "Clock::constructor: null this");

	__CONSTRUCT_BASE(Object);

	// initialize time
	this->milliSeconds = 0;

	// initialize state
	this->paused = true;

	this->previousSecond = 0;
	this->previousMinute = 0;

	// register clock
	ClockManager_register(ClockManager_getInstance(), this);
}

/**
 * Class destructor
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 */
void Clock_destructor(Clock this)
{
	ASSERT(this, "Clock::destructor: null this");

	// unregister the clock
	ClockManager_unregister(ClockManager_getInstance(), this);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Print formatted class' attributes's states
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 * @param col
 * @param row
 * @param font
 */
void Clock_print(Clock this, int col, int row, const char* font)
{
	ASSERT(this, "Clock::print: null this");

	int minutes = Clock_getMinutes(this);
	int seconds = (Clock_getSeconds(this) - minutes * 60) % 60;

	int minutesPosition = col;
	int secondsPosition = col + 3;

	// print minutes
	if(minutes < 10)
	{
		Printing_text(Printing_getInstance(), "0", minutesPosition, row, font);
		minutesPosition++;
	}

	Printing_int(Printing_getInstance(), minutes, minutesPosition, row, font);

	// print divisor
	Printing_text(Printing_getInstance(), ":", secondsPosition - 1, row, font);

	// print seconds
	if(seconds < 10)
	{
		Printing_text(Printing_getInstance(), "0", secondsPosition, row, font);
		secondsPosition++;
	}

	Printing_int(Printing_getInstance(), seconds, secondsPosition, row, font);
}

/**
 * Called on each timer interrupt
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 * @param ticks
 */
void Clock_update(Clock this, u32 ticks)
{
	ASSERT(this, "Clock::update: null this");

	// increase count
	if(!this->paused)
	{
		this->milliSeconds += ticks;

		u32 currentSecond = Clock_getSeconds(this);

		if(currentSecond != this->previousSecond)
		{
			this->previousSecond = currentSecond;

			Object_fireEvent(__SAFE_CAST(Object, this), kEventSecondChanged);

			u32 currentMinute = Clock_getMinutes(this);

			if(currentMinute != this->previousMinute)
			{
				this->previousMinute = currentMinute;

				Object_fireEvent(__SAFE_CAST(Object, this), kEventMinuteChanged);
			}
		}
	}
}

/**
 * Reset clock's attributes
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 */
void Clock_reset(Clock this)
{
	ASSERT(this, "Clock::reset: null this");

	this->milliSeconds = 0;
	this->previousSecond = 0;
	this->previousMinute = 0;
}

/**
 * Retrieve clock's milliseconds
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 *
 * @return      Current milliseconds
 */
u32 Clock_getMilliSeconds(Clock this)
{
	ASSERT(this, "Clock::getMilliSeconds: null this");

	return this->milliSeconds;
}

/**
 * Retrieve clock's seconds
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 *
 * @return      Current seconds
 */
u32 Clock_getSeconds(Clock this)
{
	ASSERT(this, "Clock::getSeconds: null this");

	return (u32)(this->milliSeconds / __MILLISECONDS_IN_SECOND);
}

/**
 * Retrieve clock's minutes
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 *
 * @return      Current minutes
 */
u32 Clock_getMinutes(Clock this)
{
	ASSERT(this, "Clock::getMinutes: null this");

	return (u32)(this->milliSeconds / (__MILLISECONDS_IN_SECOND * 60));
}

/**
 * Retrieve clock's total elapsed time in milliseconds
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 *
 * @return      Current milliseconds
 */
u32 Clock_getTime(Clock this)
{
	ASSERT(this, "Clock::getTime: null this");

	return this->milliSeconds;
}

/**
 * Retrieve current elapsed milliseconds in the current second
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 *
 * @return      Elapsed milliseconds in the current second
 */
int Clock_getTimeInCurrentSecond(Clock this)
{
	ASSERT(this, "Clock::getTimeInCurrentSecond: null this");

	return __MILLISECONDS_IN_SECOND * (this->milliSeconds * 0.001f - F_FLOOR(this->milliSeconds * 0.001f));
}

/**
 * Set clock's total elapsed time from seconds parameters
 *
 * @memberof            Clock
 * @public
 *
 * @param this          Function scope
 * @param totalSeconds
 */
void Clock_setTimeInSeconds(Clock this, float totalSeconds)
{
	ASSERT(this, "Clock::setTimeInSeconds: null this");

	this->milliSeconds = totalSeconds * __MILLISECONDS_IN_SECOND;
}

/**
 * Start the clock
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 */
void Clock_start(Clock this)
{
	ASSERT(this, "Clock::start: null this");
	Clock_reset(this);

	this->paused = false;
}

/**
 * Stop the clock
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 */
void Clock_stop(Clock this)
{
	ASSERT(this, "Clock::stop: null this");

	Clock_reset(this);
	this->paused = true;
}

/**
 * Pause the clock
 *
 * @memberof        Clock
 * @public
 *
 * @param this      Function scope
 * @param paused    Set to paused or unpaused?
 */
void Clock_pause(Clock this, bool paused)
{
	ASSERT(this, "Clock::pause: null this");

	this->paused = paused;
}

/**
 * Whether the clock is running or not
 *
 * @memberof    Clock
 * @public
 *
 * @param this  Function scope
 *
 * @return      Paused flag
 */
bool Clock_isPaused(Clock this)
{
	ASSERT(this, "Clock::isPaused: null this");

	return this->paused;
}
