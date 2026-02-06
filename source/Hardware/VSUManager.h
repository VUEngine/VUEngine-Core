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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __DEFAULT_PCM_HZ						8000
#define __TOTAL_WAVEFORMS						5
#define __TOTAL_SOUND_SOURCES					6
#define __TOTAL_MODULATION_CHANNELS 			1
#define __TOTAL_NOISE_CHANNELS					1
#define __TOTAL_NORMAL_CHANNELS					(__TOTAL_SOUND_SOURCES - __TOTAL_MODULATION_CHANNELS - __TOTAL_NOISE_CHANNELS)
#define __TOTAL_POTENTIAL_NORMAL_CHANNELS 		(__TOTAL_NORMAL_CHANNELS + __TOTAL_MODULATION_CHANNELS)
#define __MAXIMUM_VOLUME						0xF

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Sound source types
/// @memberof VSUManager
enum VSUSoundSourceTypes
{
	kSoundSourceNormal = (1 << 0),
	kSoundSourceModulation = (1 << 1),
	kSoundSourceNoise = (1 << 2),
};

/// Playback types
/// @memberof VSUManager
enum VSUPlaybackModes
{
	kPlaybackNative = 0,
	kPlaybackPCM
};

/// A struct that holds the Waveform Data
/// @memberof VSUManager
typedef struct WaveformData
{
	/// Waveform's data
	int8 data[32];

	/// Data's CRC
	uint32 crc;

} WaveformData;

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

	/// Pointer to the waveform data
	const int8* data;

	/// Data's CRC
	uint32 crc;

} Waveform;

/// Sound source mapping
/// @memberof VSUManager
typedef struct VSUSoundSource
{
	uint8 SxINT;
	uint8 spacer1[3];
	uint8 SxLRV;
	uint8 spacer2[3];
	uint8 SxFQL;
	uint8 spacer3[3];
	uint8 SxFQH;
	uint8 spacer4[3];
	uint8 SxEV0;
	uint8 spacer5[3];
	uint8 SxEV1;
	uint8 spacer6[3];
	uint8 SxRAM;
	uint8 spacer7[3];
	uint8 SxSWP;
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

	/// VSU waveform to use
	const Waveform* waveform;

	/// Time when the configuration elapses
	fix7_9_ext timeout;

	/// Sound source type
	uint32 type;

	/// SxINT values
	uint8 SxINT;

	/// SxLRV values
	uint8 SxLRV;

	/// SxFQL values
	uint8 SxFQL;

	/// SxFQH values
	uint8 SxFQH;

	/// SxEV0 values
	uint8 SxEV0;

	/// SxEV1 values
	uint8 SxEV1;

	/// SxRAM index
	uint8 SxRAM;

	/// SxSWP values
	uint8 SxSWP;

	/// SxMOD pointer
	const int8* SxMOD;

	/// Skip if no sound source available?
	bool skippable;

} VSUSoundSourceConfiguration;

/// Sound source configuration request
/// @memberof VSUManager
typedef struct VSUSoundSourceConfigurationRequest
{
	/// Requester object
	Object requester;

	/// Time when the configuration elapses
	fix7_9_ext timeout;

	/// Sound source type
	uint32 type;

	/// SxINT values
	uint8 SxINT;

	/// SxLRV values
	uint8 SxLRV;

	/// SxFQL values
	uint8 SxFQL;

	/// SxFQH values
	uint8 SxFQH;

	/// SxEV0 values
	uint8 SxEV0;

	/// SxEV1 values
	uint8 SxEV1;

	/// SxRAM index
	const WaveformData* SxRAM;

	/// SxSWP values
	uint8 SxSWP;

	/// SxMOD pointer
	const int8* SxMOD;

	/// Skip if no sound source available?
	bool skippable;

} VSUSoundSourceConfigurationRequest;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class VSUManager
///
/// Inherits from Object
///
/// Manages the VSU.
singleton class VSUManager : Object
{
	/// @protectedsection

	/// List of queued sound source configurations
	VirtualList queuedVSUSoundSourceConfigurationRequests;

	/// Mapping of VSU sound source configurations
	VSUSoundSourceConfiguration vsuSoundSourceConfigurations[__TOTAL_SOUND_SOURCES];

	/// Mapping of waveworms
	Waveform waveforms[__TOTAL_WAVEFORMS];

	/// Elapsed ticks
	fix7_9_ext ticks;

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

	/// Play the allocated sounds.
	/// @param elapsedMicroseconds: Elapsed time between call
	void playSounds(uint32 elapsedMicroseconds);

	/// Apply a sound source configuration to a VSU sound source with the provided data.
	/// @param vsuSoundSourceConfigurationRequest: VSU sound source configuration
	static void applySoundSourceConfiguration(const VSUSoundSourceConfigurationRequest* vsuSoundSourceConfigurationRequest);

	/// Stop sound output in the sound sources in use by the requester object.
	/// @param requester: Object using a sound source
	static void stopSoundSourcesUsedBy(Object requester);

	/// Retriev the playback mode (stops any playing sound).
	/// @return kPlaybackNative or kPlaybackPCM
	static uint32 getMode();

	/// Print the manager's status.
	static void print(int32 x, int32 y);

	/// Print waveforms.
	static void printWaveFormStatus(int32 x, int32 y);

	/// Print channels' status.
	static void printChannels(int32 x, int32 y);

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

	/// Flush all pending sound requests.
	void flushQueuedSounds();
}

#endif
