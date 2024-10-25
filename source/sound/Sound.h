/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SOUND_H_
#define SOUND_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <MIDI.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

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


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class VirtualList;

enum SoundChannelTypes
{
	kChannelNormal 			= (1 << 0),
	kChannelModulation		= (1 << 1),
	kChannelNoise			= (1 << 2)
};

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

typedef const SoundChannelConfiguration SoundChannelConfigurationROM;

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

typedef const SoundChannel SoundChannelROM;

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

typedef const SoundSpec SoundROMSpec;

typedef struct Waveform
{
	uint8 number;
	int8 usageCount;
	uint8* wave;
	uint8 overwrite;
	const int8* data;

} Waveform;

typedef struct Channel
{
	// Channel configuration
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

	uint32 type;

	uint8 number;
	uint8 soundChannel;
	bool finished;

} Channel;

enum SoundTrackTypes
{
	kUnknownType = 0,
	kMIDI,
	kPCM
};

enum SoundPlaybackTypes
{
	kSoundPlaybackNone = 0,
	kSoundPlaybackNormal,
	kSoundPlaybackFadeIn,
	kSoundPlaybackFadeOut,
	kSoundPlaybackFadeOutAndRelease
};

enum SoundMessages
{
	kSoundFadeIn = 23,
	kSoundFadeOut,
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities-particles
class Sound : ListenerObject
{
	const SoundSpec* soundSpec;
	const Vector3D* position;
	VirtualList channels;
	Channel* mainChannel;
	fix7_9_ext speed;
	fix7_9_ext targetTimerResolutionFactor;
	uint32 previouslyElapsedTicks;
	uint32 totalPlaybackMilliseconds;
	uint16 pcmTargetPlaybackFrameRate;
	uint16 frequencyModifier;
	uint16 volumeReductionMultiplier;
	int8 volumeReduction;
	uint8 playbackType;
	bool turnedOn;
	bool paused;
	bool hasMIDITracks;
	bool hasPCMTracks;
	bool unmute;
	bool autoReleaseOnFinish;
	bool locked;

	/// @publicsection
	static void setMirror(Mirror mirror);
	void constructor(const SoundSpec* soundSpec, VirtualList channels, int8* waves, uint16 pcmTargetPlaybackFrameRate, EventListener soundReleaseListener, ListenerObject scope);

	const Channel* getChannel(uint8 index);
	bool isUsingChannel(Channel* channel);
	bool isPlaying();
	bool isPaused();
	bool isTurnedOn();
	bool hasPCMTracks();
	bool isFadingIn();
	bool isFadingOut();
	void play(const Vector3D* position, uint32 playbackType);
	void pause();
	void unpause();
	void turnOff();
	void turnOn();
	void rewind(bool playbackType);
	void stop();
	void release();
	void mute();
	void unmute();
	void lock();
	void unlock();
	void autoReleaseOnFinish(bool value);
	void updateMIDIPlayback(uint32 elapsedMicroseconds);
	void updatePCMPlayback(uint32 elapsedMicroseconds, uint32 targetPCMUpdates);
	void setSpeed(fix7_9_ext speed);
	fix7_9_ext getSpeed();
	void setVolumeReduction(int8 volumeReduction);
	int8 getVolumeReduction();
	void computeTimerResolutionFactor();
	void setFrequencyModifier(uint16 frequencyModifier);
	uint16 getFrequencyModifier();
	void print(int32 x, int32 y);
	void printMetadata(int32 x, int32 y, bool printDetails);
	void printVolume(int32 x, int32 y, bool printHeader);
	void printPlaybackTime(int32 x, int32 y);
	void printPlaybackProgress(int32 x, int32 y);
}


#endif
