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

/// Class SoundTest
///
/// Inherits from ListenerObject
///
/// Implements profiler that permits to measure how much time a process takes to complete.
singleton class Profiler : ListenerObject
{
	/// Laps during the current profiling cycle
	VirtualList laps;

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

	/// Initialize the profiler.
	static void initialize();

	/// Reset the profiler's state.
	static void reset();

	/// Start a new cycle of profiling.
	static void start();

	/// End the current profiling cycle.
	static void end();

	/// Register a lap during the current profiling cycle.
	/// @param lapType: Type of lap to record
	/// @param processName: Name of the process during the lap
	static void lap(uint32 lapType, const char* processName);
}

#endif