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


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Clock
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

// class's constructor
static void Clock_constructor(Clock this)
{
	ASSERT(this, "Clock::constructor: null this");

	__CONSTRUCT_BASE();

	// initialize time
	this->milliSeconds = 0;
	this->previousMilliSeconds = 0;

	// initialize state
	this->paused = true;

	this->previousSecond = 0;
	this->previousMinute = 0;

	// register clock
	ClockManager_register(ClockManager_getInstance(), this);
}

// class's destructor
void Clock_destructor(Clock this)
{
	ASSERT(this, "Clock::destructor: null this");

	// unregister the clock
	ClockManager_unregister(ClockManager_getInstance(), this);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// time delay
void Clock_delay(Clock this, int milliSeconds)
{
	ASSERT(this, "Clock::delay: null this");
	u32 time = this->milliSeconds;

	if(this->paused)
	{
		return;
	}
	else
	{
		u32 volatile *clockTime = (u32 *)&this->milliSeconds;

		while ((*clockTime - time) < milliSeconds);
	}
}

// print formated class's attributes's states
void Clock_print(Clock this, int col, int row, const char* font)
{
	ASSERT(this, "Clock::print: null this");

	int minutes = Clock_getMinutes(this);
	int seconds = (Clock_getSeconds(this) - minutes * 60) % 60;

	int minutesPosition = col;
	int secondsPosition = col + 3;

	//print minutes
	if(minutes < 10)
	{
		Printing_text(Printing_getInstance(), "0", minutesPosition, row, font);
		minutesPosition++;
	}

	Printing_int(Printing_getInstance(), minutes, minutesPosition, row, font);

	// print divisor
	Printing_text(Printing_getInstance(), ":", secondsPosition - 1, row, font);

	//print seconds
	if(seconds < 10)
	{
		Printing_text(Printing_getInstance(), "0", secondsPosition, row, font);
		secondsPosition++;
	}

	Printing_int(Printing_getInstance(), seconds, secondsPosition, row, font);
}

// called on each timer interrupt
void Clock_update(Clock this, u32 ticks)
{
	ASSERT(this, "Clock::update: null this");

	// increase count
	if(!this->paused)
	{
		// calculate milliseconds
		this->previousMilliSeconds = this->milliSeconds;
		this->milliSeconds += ticks;

		u8 currentSecond = Clock_getSeconds(this);

		if(currentSecond != this->previousSecond)
		{
			this->previousSecond = currentSecond;

			Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_SECOND_CHANGED);

			u8 currentMinute = Clock_getMinutes(this);

			if(currentMinute != this->previousMinute)
			{
				this->previousMinute = currentMinute;

				Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_MINUTE_CHANGED);
			}
		}
	}
}

// reset clock's attributes
void Clock_reset(Clock this)
{
	ASSERT(this, "Clock::reset: null this");

	this->milliSeconds = 0;
	this->previousMilliSeconds = 0;

	this->previousSecond = 0;
	this->previousMinute = 0;
}

// retrieve clock's milliseconds
u32 Clock_getMilliSeconds(Clock this)
{
	ASSERT(this, "Clock::getMilliSeconds: null this");

	return this->milliSeconds;
}

u32 Clock_getPreviousMilliSeconds(Clock this)
{
	ASSERT(this, "Clock::getPreviousMilliSeconds: null this");

	return this->previousMilliSeconds;
}

//retrieve clock's seconds
u16 Clock_getSeconds(Clock this)
{
	ASSERT(this, "Clock::getSeconds: null this");

	return (u16)(this->milliSeconds / 1000);
}

// retrieve clock's minutes
u8 Clock_getMinutes(Clock this)
{
	ASSERT(this, "Clock::getMinutes: null this");

	return (u8)(this->milliSeconds / (1000 * 60));
}

// retrieve clock's total elapsed time in seconds
u32 Clock_getTime(Clock this)
{
	ASSERT(this, "Clock::getTime: null this");

	return this->milliSeconds;
}

// retrieve clock's elapsed time in last cycle
u32 Clock_getElapsedTime(Clock this)
{
	ASSERT(this, "Clock::getTimeElapse: null this");

	return this->paused? 0: this->milliSeconds - this->previousMilliSeconds;
}

// retrieve current elapsed milliseconds in the current second
int Clock_getTimeInCurrentSecond(Clock this)
{
	ASSERT(this, "Clock::getTimeInCurrentSecond: null this");

	return 1000 * (this->milliSeconds * 0.001f - F_FLOOR(this->milliSeconds * 0.001f));
}

// set clock's total elapsed time from seconds parameters
void Clock_setTimeInSeconds(Clock this, float totalSeconds)
{
	ASSERT(this, "Clock::setTimeInSeconds: null this");

	this->milliSeconds = totalSeconds * 1000;
}

// start the clock
void Clock_start(Clock this)
{
	ASSERT(this, "Clock::start: null this");
	Clock_reset(this);

	this->paused = false;
}

// stop the clock
void Clock_stop(Clock this)
{
	ASSERT(this, "Clock::stop: null this");

	Clock_reset(this);
	this->paused = true;
}

// pause the clock
void Clock_pause(Clock this, bool paused)
{
	ASSERT(this, "Clock::pause: null this");

	this->paused = paused;
}

// whether the clock is running or not
bool Clock_isPaused(Clock this)
{
	ASSERT(this, "Clock::isPaused: null this");

	return this->paused;
}