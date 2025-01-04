/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PROFILER_H_
#define PROFILER_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class TimerManager;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

enum ProfilerLapTypes
{
	kProfilerLapTypeNormalProcess 								= 0x00000001 << 0,
	kProfilerLapTypeStartInterrupt								= 0x00000001 << 1,
	kProfilerLapTypeVIPInterruptFRAMESTARTProcess				= 0x00000001 << 2,
	kProfilerLapTypeVIPInterruptGAMESTARTProcess				= 0x00000001 << 3,
	kProfilerLapTypeVIPInterruptXPENDProcess					= 0x00000001 << 4,
	kProfilerLapTypeTimerInterruptProcess						= 0x00000001 << 5,
	kProfilerLapTypeCommunicationsInterruptProcess				= 0x00000001 << 6,
};


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class SoundTest
///
/// Inherits from ListenerObject
///
/// Implements profiler that permits to measure how much time a process takes to complete.
singleton class Profiler : ListenerObject
{
	/// Laps during the current profiling cycle
	VirtualList laps;

	/// 
	TimerManager timerManager;
	float timeProportion;
	float lastCycleTotalTime;
	float totalTime;
	uint32 interruptFlags;
	uint32 timePerGameFrameInMS;
	uint32 cycles;
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
	/// Method to retrieve the singleton instance
	/// @return Profiler singleton
	static Profiler getInstance();

	/// Initialize the profiler.
	void initialize();

	/// Reset the profiler's state.
	void reset();

	/// Start a new cycle of profiling.
	void start();

	/// End the current profiling cycle.
	void end();

	/// Register a lap during the current profiling cycle.
	/// @param lapType: Type of lap to record
	/// @param processName: Name of the process during the lap
	void lap(uint32 lapType, const char* processName);
}

#endif