/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CLOCK_H_
#define CLOCK_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

enum ClockPrintPrecision
{
	kTimePrecision0 = 0,
	kTimePrecision1,
	kTimePrecision2,
	kTimePrecision3
};

/// @ingroup base
class Clock : ListenerObject
{
	// time elapsed
	uint32 milliSeconds;
	// register
	uint32 previousSecond;
	// register
	uint32 previousMinute;
	// flag to pause the clock
	bool paused;

	static void printTime(uint32 milliseconds, int32 col, int32 row, const char* font, uint32 precision);
	static void printDeciseconds(uint32 milliSeconds, int32 col, int32 row, const char* font);
	static void printCentiseconds(uint32 milliSeconds, int32 col, int32 row, const char* font);
	static void printMilliseconds(uint32 milliSeconds, int32 col, int32 row, const char* font);

	/// @publicsection
	void constructor();
	uint32 getElapsedTime();
	uint32 getMilliSeconds();
	uint32 getMinutes();
	uint32 getSeconds();
	uint32 getTime();
	int32 getTimeInCurrentSecond();
	bool isPaused();
	void pause(bool paused);
	void print(int32 col, int32 row, const char* font);
	void reset();
	void setTime(int32 hours, int32 minutes, int32 seconds);
	void setTimeInMilliSeconds(uint32 milliSeconds);
	void setTimeInSeconds(float totalSeconds);
	void start();
	void stop();
	void update(uint32 millisecondsElapsed);
}


#endif
