/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOUND_H_
#define SOUND_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>
#include <MIDI.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class VirtualList;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __SOUND_WRAPPER_STOP_SOUND 											0x21

#define __MAXIMUM_VOLUME													0xF
#define __MIDI_CONVERTER_FREQUENCY_US										20
#define __SOUND_TARGET_US_PER_TICK											__MIDI_CONVERTER_FREQUENCY_US

#define __SOUND_LR															0xFF
#define __SOUND_L															0xF0
#define __SOUND_R															0x0F

#define __MAXIMUM_MIDI_FREQUENCY											D_8

#define __SOUND_WRAPPER_FADE_DELAY											100

#define __SOUND_TARGET_TIMER_US_PER_INTERRUPT_HELPER(TargetHz)				(1000000 / TargetHz - 66)
#define __SOUND_TARGET_TIMER_US_PER_INTERRUPT(TargetHz)						(__SOUND_TARGET_TIMER_US_PER_INTERRUPT_HELPER(TargetHz) + 20 * (int)((__SOUND_TARGET_TIMER_US_PER_INTERRUPT_HELPER(TargetHz) % 20) / 20.0f + 0.5f) - (__SOUND_TARGET_TIMER_US_PER_INTERRUPT_HELPER(TargetHz) % 20))


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// Sound channel types
/// @memberof Sound
enum SoundChannelTypes
{
	kChannelNormal 			= (1 << 0),
	kChannelModulation		= (1 << 1),
	kChannelNoise			= (1 << 2)
};

/// Sound track types
/// @memberof Sound
enum SoundTrackTypes
{
	kUnknownType = 0,
	kMIDI,
	kPCM
};

/// Sound playback types
/// @memberof Sound
enum SoundPlaybackTypes
{
	kSoundPlaybackNone = 0,
	kSoundPlaybackNormal,
	kSoundPlaybackFadeIn,
	kSoundPlaybackFadeOut,
	kSoundPlaybackFadeOutAndRelease
};


/// Sound state
/// @memberof Sound
enum SoundState
{
	kSoundOff = 0,
	kSoundPaused,
	kSoundPlaying,
};

/// Sound channel configuration struct
/// @memberof Sound
typedef struct SoundChannelConfiguration
{
	/// kMIDI, kPCM
	uint32 trackType;

	/// SxINT
	uint8 SxINT;

	/// Volume SxLRV
	uint8 SxLRV;

	/// SxRAM
	uint8 SxRAM;

	/// SxEV0
	uint8 SxEV0;

	/// SxEV1
	uint8 SxEV1;

	/// SxFQH
	uint8 SxFQH;

	/// SxFQL
	uint8 SxFQL;

	/// Ch. 5 only
	uint8 S5SWP;

	/// Waveform data pointer
	const int8* waveFormData;

	/// kChannelNormal, kChannelModulation, kChannelNoise
	uint32 channelType;

	/// Volume
	uint8 volume;

} SoundChannelConfiguration;

/// A SoundChannelConfiguration spec that is stored in ROM
/// @memberof Sound
typedef const SoundChannelConfiguration SoundChannelConfigurationROM;

/// A SoundChannel spec struct
/// @memberof Sound
typedef struct SoundChannel
{
	/// Configuration
	SoundChannelConfiguration* soundChannelConfiguration;

	/// Total number of samples
	uint32 samples;

	/// Sound track
	union SoundTrack
	{
		/// Sound track 8Bit (PCM)
		const uint8* dataPCM;

		/// Sound track 16Bit (MIDI)
		const uint16* dataMIDI;

	} soundTrack;

} SoundChannel;

/// A SoundChannel spec that is stored in ROM
/// @memberof Sound
typedef const SoundChannel SoundChannelROM;

/// A Sound spec
/// @memberof Sound
typedef struct SoundSpec
{
	/// Name
	char* name;

	/// Play in loop
	bool loop;

	/// Target timer resolution in us
	uint16 targetTimerResolutionUS;

	/// Tracks
	SoundChannel** soundChannels;

} SoundSpec;

/// A Sound spec that is stored in ROM
/// @memberof Sound
typedef const SoundSpec SoundROMSpec;


/// A Waveform struct
/// @memberof Sound
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

/// A Channel struct
/// @memberof Sound
typedef struct Channel
{
	/// Channel configuration
	SoundChannelConfiguration soundChannelConfiguration;

	/// Sound definition
	const SoundSpec* soundSpec;

	/// Channel's effective length
	uint32 samples;

	/// Position within the sound track
	uint32 cursor;

	/// Next curst ticks target
	fix7_9_ext nextElapsedTicksTarget;

	/// Total number of ticks
	uint32 ticks;

	/// Ticks before moving the cursor
	fix7_9_ext elapsedTicks;

	/// Tick step per timer interrupt
	fix7_9_ext tickStep;

	/// Sound track
	union ChannelSoundTrack
	{
		/// Sound track 8Bit (PCM)
		const uint8* dataPCM;

		/// Sound track 16Bit (MIDI)
		const uint16* dataMIDI;

	} soundTrack;

	/// Channel's type (normal, modulation or noise)
	uint32 type;

	/// Channel's sound registries index
	uint8 index;

	/// Channel's sound's spec channels index
	uint8 soundChannel;

	/// If true, the channel's track's playback is complete
	bool finished;

} Channel;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Sound
///
/// Inherits from ListenerObject
///
/// Implements sound playback.
/// @ingroup sound
class Sound : ListenerObject
{
	/// @protectedsection

	/// Pointer to the spec that defines how to initialize the sound 
	const SoundSpec* soundSpec;

	/// Pointer to vector for spatial positioning of the sound
	const Vector3D* position;

	/// List of VSU channels used by the sound
	VirtualList channels;

	/// Channel used for PCM playback
	Channel* mainChannel;

	/// Playback speed
	fix7_9_ext speed;

	/// Sound's state
	uint32 state;

	/// Elapsed ticks in the previous update
	uint32 previouslyElapsedTicks;

	/// Total playback time
	uint32 totalPlaybackMilliseconds;

	/// Target refresh rate for PCM playback
	uint16 pcmTargetPlaybackRefreshRate;

	/// Delta added to the frequency registers
	uint16 frequencyDelta;

	/// Multiplier used for fade effects
	uint16 volumeReductionMultiplier;

	/// Volume reduction used for fade effects
	int8 volumeReduction;

	/// 2's power to divide to the final volume value
	uint8 volumenScalePower;

	/// Type of playback to perform (SoundPlaybackTypes)
	uint8 playbackType;

	/// MIDI tracks count
	bool MIDITracks;

	/// PCM tracks count
	bool PCMTracks;

	/// If true, sound is not muted
	bool unmute;

	/// If true, the sound is released when playback is complete
	bool autoReleaseOnFinish;

	/// If locked, it cannot be released by external calls
	bool locked;

	/// @publicsection

	/// Mirror the spatial positioning of the sound.
	/// @param mirror: Struct with a flag for each axis to mirror
	static void setMirror(Mirror mirror);

	/// Set the target refresh rate for PCM playback.
	/// @param pcmTargetPlaybackRefreshRate: Target refresh rate for PCM playback
	static void setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate);

	/// Class' constructor
	/// @param soundSpec: Specification that determines how to configure the sound
	/// @param channels: Linked list of VSU channels to use
	/// @param waves: Array of indexes of waveforms to use
	/// @param soundReleaseListener: Callback for when the sound is released
	/// @param scope: Object that listens for the releasing event
	void constructor(const SoundSpec* soundSpec, VirtualList channels, int8* waves, EventListener soundReleaseListener, ListenerObject scope);

	/// Play the sound.
	/// @param position: Pointer to the spatial position of the sound
	/// @param playbackType: Specifies how the playback should start
	void play(const Vector3D* position, uint32 playbackType);

	/// Stop the playback.
	void stop();

	/// Pause the playback.
	void pause();

	/// Unpause the playback.
	void unpause();

	/// Suspend the output of sound.
	void suspend();

	/// Resume the output of sound.
	void resume();

	/// Mute the sound.
	void mute();

	/// Unmute the sound.
	void unmute();

	/// Rewind the playack
	void rewind();

	/// Release this sound.
	void release();

	/// Prevent other requests to get a sound to steal this sound's channels
	void lock();

	/// Allow other requests to get a sound to steal this sound's channels
	void unlock();

	/// Set the flag that allows the sound to auto release itself when playback is complete.
	/// @param autoReleaseOnFinish: If true, the sound is released when playaback is complete
	void autoReleaseOnFinish(bool autoReleaseOnFinish);

	/// Set the playback's speed.
	/// @param speed: Target playback speed
	void setSpeed(fix7_9_ext speed);

	/// Retrieve the playback's speed.
	/// @return Target playback speed
	fix7_9_ext getSpeed();

	/// Set the factor (2's power) by which the final volume is reduced.
	/// @param volumenScalePower: Factor by which the final volume is reduced
	void setVolumenScalePower(uint8 volumenScalePower);

	/// Set the frequency delta to be added to the VSU's frequency registers.
	/// @param frequencyDelta: Delta to be added to the frequency
	void setFrequencyDelta(uint16 frequencyDelta);

	/// Retrieve the frequency delta added to the VSU's frequency registers.
	/// @param frequencyDelta: Delta added to the frequency
	uint16 getFrequencyDelta();

	/// Check if the sound has MIDI tracks.
	/// @return True if the sound has at least one MIDI track; false otherwise
	bool hasMIDITracks();

	/// Check if the sound is using the provided VSU channel.
	/// @return True if the sound has at least one PCM track; false otherwise
	bool hasPCMTracks();

	/// Check if the sound is using the provided VSU channel.
	/// @return True if the sound uses the provided channel
	bool isUsingChannel(Channel* channel);

	/// Check if the sound is playing.
	/// @return True if playback is going on
	bool isPlaying();

	/// Check if the sound is paused.
	/// @return True if playback is paused
	bool isPaused();

	/// Check if the sound is fading in.
	/// @return True if playback fading in
	bool isFadingIn();

	/// Check if the sound is fading out.
	/// @return True if playback fading out
	bool isFadingOut();

	/// Advance the playback on the sound's MIDI tracks.
	/// @param elapsedMicroseconds: Elapsed time since the last call
	void updateMIDIPlayback(uint32 elapsedMicroseconds);

	/// Advance the playback on the sound's PCM tracks.
	/// @param elapsedMicroseconds: Elapsed time since the last call
	/// @param targetPCMUpdates: Ideal Elapsed time since the last call
	void updatePCMPlayback(uint32 elapsedMicroseconds, uint32 targetPCMUpdates);

	/// Print the sounds's properties.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);

	/// Print the sounds's volume.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	/// @param printHeader: If true it print's the header's info
	void printVolume(int32 x, int32 y, bool printHeader);

	/// Print the sounds's playback time.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void printPlaybackTime(int32 x, int32 y);

	/// Print the sounds's playback progress.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void printPlaybackProgress(int32 x, int32 y);
}


#endif
