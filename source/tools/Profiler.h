/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PROFILER_H_
#define PROFILER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <TimerManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

enum ProfilerLapTypes
{
	kProfilerLapTypeNormalProcess 								= 0x00000001 << 0,
	kProfilerLapTypeVIPInterruptFRAMESTARTProcess				= 0x00000001 << 2,
	kProfilerLapTypeVIPInterruptGAMESTARTProcess				= 0x00000001 << 3,
	kProfilerLapTypeVIPInterruptXPENDProcess					= 0x00000001 << 4,
	kProfilerLapTypeTimerInterruptProcess						= 0x00000001 << 5,
	kProfilerLapTypeCommunicationsInterruptProcess				= 0x00000001 << 6,
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class Profiler : ListenerObject
{
	VirtualList laps;
	TimerManager timerManager;
	float timeProportion;
	float totalTime;
	uint32 interruptFlags;
	uint32 timePerGameFrameInMS;
	uint16 timerCounter;
	uint16 previousTimerCounter;
	uint8 currentProfilingProcess;
	bool started;
	bool initialized;
	bool printedProcessesNames;
	uint8 skipFrames;
	uint8 lastLapIndex;
	bool xpend;
	bool playedMIDISounds;
	bool processedCommunications;

	/// @publicsection
	static Profiler getInstance();
	void initialize();
	void reset();
	void start();
	void startInterrupt();
	void end();
	void lap(uint32 lapType, const char* processName);
	void interruptLap(uint32 lapType, const char* processName);
}

#endif