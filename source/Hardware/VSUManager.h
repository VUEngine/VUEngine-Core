/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VSU_MANAGER_H_
#define VSU_MANAGER_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __DEFAULT_PCM_HZ					8000
#define __TOTAL_SOUND_SOURCES				6
#define __TOTAL_MODULATION_CHANNELS			1
#define __TOTAL_NOISE_CHANNELS				1
#define __TOTAL_NORMAL_CHANNELS				(__TOTAL_SOUND_SOURCES - __TOTAL_MODULATION_CHANNELS - __TOTAL_NOISE_CHANNELS)
#define __TOTAL_POTENTIAL_NORMAL_CHANNELS	(__TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS)
#define __TOTAL_WAVEFORMS					__TOTAL_POTENTIAL_NORMAL_CHANNELS


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// Sound source types
/// @memberof VSUManager
enum VSUSoundSouceTypes
{
	kSoundSourceNormal = 0,
	kSoundSourceModulation,
	kSoundSourceNoise
};

/// Playback types
/// @memberof VSUManager
enum VSUPlaybackModes
{
	kPlaybackNative = 0,
	kPlaybackPCM
};

/// A Waveform struct
/// @memberof VSUManager
typedef struct Waveform
{
	/// Waveform's index
	uint8 index;

	/// Count of channels using this waveform
	int8 usageCount;

	/// Pointer to the VSU's waveform address
	uint8* wave;

	/// If true, waveform data has to be rewritten
	uint8 overwrite;

	/// Pointer to the waveform's data
	const int8* data;

} Waveform;

/// Sound source mapping
/// @memberof VSUManager
typedef struct VSUSoundSource
{
	// This table is for the most part untested, but looks to be accurate
	//				 	|		D7	   ||		D6	   ||		D5	   ||		D4	   ||		D3	   ||		D2	   ||		D1	   ||		D0	   |
	uint8 SxINT; //		[----Enable----][--XXXXXXXXXX--][-Interval/??--][--------------------------------Interval Data---------------------------------]
	uint8 spacer1[3];
	uint8 SxLRV; //		[---------------------------L Level----------------------------][---------------------------R Level----------------------------]
	uint8 spacer2[3];
	uint8 SxFQL; //		[------------------------------------------------------Frequency Low Byte------------------------------------------------------]
	uint8 spacer3[3];
	uint8 SxFQH; //		[--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Frequency High Byte-------------]
	uint8 spacer4[3];
	uint8 SxEV0; //		[---------------------Initial Envelope Value-------------------][------U/D-----][-----------------Envelope Step----------------]
	uint8 spacer5[3];
			 //Ch. 1-4 	[--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
			 //Ch. 5	[--XXXXXXXXXX--][------E/D-----][----?/Short---][--Mod./Sweep--][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	uint8 SxEV1; //Ch. 6	[--XXXXXXXXXX--][----------------------E/D---------------------][--XXXXXXXXXX--][--XXXXXXXXXX--][------R/S-----][----On/Off----]
	uint8 spacer6[3];
	//Ch. 1-5 only (I believe address is only 3 bits, but may be 4, needs testing)
	uint8 SxRAM; //		[--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--XXXXXXXXXX--][--------------Waveform RAM Address------------]
	uint8 spacer7[3];
	//Ch. 5 only
	uint8 SxSWP; //		[------CLK-----][-------------Sweep/Modulation Time------------][------U/D-----][----------------Number of Shifts--------------]
	uint8 spacer8[35];
} VSUSoundSource;

/// Sound source configuration
/// @memberof VSUManager
typedef struct VSUSoundSourceConfiguration
{
	/// Requester object
	Object requester;
	
	/// VSU sound source to configure
	VSUSoundSource* vsuSoundSource;

	/// Time when the configuration elapses
	fix7_9_ext timeout;

	/// Sound source type
	uint32 type;

	/// SxINT values
	int16 SxINT;

	/// SxLRV values
	int16 SxLRV;

	/// SxFQL values
	int16 SxFQL;

	/// SxFQH values
	int16 SxFQH;

	/// SxEV0 values
	int16 SxEV0;

	/// SxEV1 values
	int16 SxEV1;

	/// SxRAM pointer
	const int8* SxRAM;

	/// SxSWP values
	int16 SxSWP;	

	/// SxMOD pointer
	const int8* SxMOD;

	/// Sweep/Mod?
	bool sweepMod;

	/// Noise?
	bool noise;

	/// Skip if no sound source available?
	bool skippable;
	
} VSUSoundSourceConfiguration;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class VSUManager
///
/// Inherits from Object
///
/// Manages the VSU.
singleton class VSUManager : Object
{
	/// @protectedsection

	/// List of queued sound source configurations
	VirtualList queuedVSUSoundSourceConfigurations;

	/// Mapping of VSU sound source configurations
	VSUSoundSourceConfiguration vsuSoundSourceConfigurations[__TOTAL_SOUND_SOURCES];

	/// Mapping of waveworms
	Waveform waveforms[__TOTAL_WAVEFORMS];

	/// Elapsed ticks
	fix7_9_ext ticks;

	/// Target PCM cycles per game cycle
	uint32 targetPCMUpdates;

	/// Playback mode
	uint32 playbackMode;

	/// If false and if there are no sound sources availables at the time of request, 
	/// the petition is ignored
	bool allowQueueingSoundRequests;
	
	/// Flag to skip sound source releasing if not necessary
	bool haveUsedSoundSources;
	
	/// Flag to skip pending sound source dispatching if not necessary
	bool haveQueuedRequests;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return VSUManager singleton
	static VSUManager getInstance();

	/// Play the allocated sounds.
	/// @param elapsedMicroseconds: Elapsed time between call
	static void playSounds(uint32 elapsedMicroseconds);

	/// Apply a sound source configuration to a VSU sound source with the provided data.
	/// @param vsuSoundSourceConfiguration: VSU sound source configuration
	void applySoundSourceConfiguration(const VSUSoundSourceConfiguration* vsuSoundSourceConfiguration);

	/// Apply a sound source configuration to a VSU sound source with the provided data for PCM playback.
	/// @param sample: PCM sample data
	void applyPCMSampleToSoundSource(int8 sample);

	/// Reset the manager's state.
	void reset();

	/// Set the playback mode (stops any playing sound).
	/// @param playbackMode: kPlaybackNative or kPlaybackPCM
	void setMode(uint32 playbackMode);

	/// Update the manager.
	void update();

	/// Stop all sound sources.
	void stopAllSounds();

	/// Enable queueing petitions to play sounds.
	void enableQueue();

	/// Disable queueing petitions to play sounds (if there are no
	/// sound sources availables at the time of request, the petition
	/// is ignored).
	void disableQueue();

	/// Print the manager's status.
	void print(int32 x, int32 y);

	/// Print waveforms.
	void printWaveFormStatus(int32 x, int32 y);
}


#endif
