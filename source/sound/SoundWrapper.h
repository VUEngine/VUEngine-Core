/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SOUND_WRAPPER_H_
#define SOUND_WRAPPER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <MIDI.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __SOUND_WRAPPER_STOP_SOUND 			0xA1

#define __MAXIMUM_VOLUME					0xF
#define __MIDI_CONVERTER_FREQUENCY_US		20
#define __SOUND_TARGET_US_PER_TICK			__MIDI_CONVERTER_FREQUENCY_US

#define __SOUND_LR							0xFF
#define __SOUND_L							0xF0
#define __SOUND_R							0x0F

#define __MAXIMUM_MIDI_FREQUENCY			D_8

#define __SOUND_WRAPPER_FADE_DELAY			100

//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

enum SoundChannelTypes
{
	kChannelNormal 			= (1 << 0),
	kChannelModulation		= (1 << 1),
	kChannelNoise			= (1 << 2),
	kChannelNormalExtended	= (1 << 3)
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

	/// Length
	uint32 length;

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

typedef struct Sound
{
	/// Name
	char* name;

	/// Play in loop
	bool loop;

	/// Target timer resolution in us
	uint16 targetTimerResolutionUS;

	/// Tracks
	SoundChannel** soundChannels;

} Sound;

typedef const Sound SoundROM;

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
	Sound* sound;

	/// Channel's effective length
	uint32 length;

	/// Position within the sound track
	uint32 cursor;

	/// Ticks before moving the cursor
	fix17_15 ticksPerNote;

	/// Ticks before moving the cursor
	fix17_15 ticks;

	/// Tick step per timer interrupt
	fix17_15 tickStep;

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
	uint8 volumeReduction;
	bool finished;

} Channel;

enum SoundTrackTypes
{
	kUnknownType = 0,
	kMIDI,
	kPCM
};

enum SoundWrapperPlaybackTypes
{
	kSoundWrapperPlaybackNormal = 0,
	kSoundWrapperPlaybackFadeIn,
	kSoundWrapperPlaybackFadeOut,
};

enum SoundWrapperMessages
{
	kSoundWrapperFadeIn = 0,
	kSoundWrapperFadeOut,
};
//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities-particles
class SoundWrapper : Object
{
	const Sound* sound;
	const Vector3D* position;
	VirtualList channels;
	fix17_15 speed;
	fix17_15 targetTimerResolutionFactor;
	uint32 elapsedMicroseconds;
	uint32 totalPlaybackMilliseconds;
	uint16 pcmTargetPlaybackFrameRate;
	uint16 frequencyModifier;
	int8 volumeReduction;
	uint8 playbackType;
	bool turnedOn;
	bool paused;
	bool hasMIDITracks;
	bool hasPCMTracks;
	bool unmute;
	bool autoReleaseOnFinish;

	/// @publicsection
	void constructor(const Sound* sound, VirtualList channels, int8* waves, uint16 pcmTargetPlaybackFrameRate, EventListener soundReleaseListener, Object scope);

	const Channel* getChannel(uint8 index);
	bool isPaused();
	bool hasPCMTracks();
	bool isFadingIn();
	bool isFadingOut();
	void play(const Vector3D* position, uint32 playbackType);
	void pause();
	void unpause();
	void turnOff();
	void turnOn();
	void rewind();
	void stop();
	void release();
	void mute();
	void unmute();
	void autoReleaseOnFinish(bool value);
	void updateMIDIPlayback(uint32 elapsedMicroseconds);
	void updatePCMPlayback(uint32 elapsedMicroseconds);
	void setSpeed(fix17_15 speed);
	void setVolumeReduction(int8 volumeReduction);
	int8 getVolumeReduction();
	fix17_15 getSpeed();
	void computeTimerResolutionFactor();
	void setFrequencyModifier(uint16 frequencyModifier);
	uint16 getFrequencyModifier();
	void print(int32 x, int32 y);
	void printMetadata(int32 x, int32 y);
	void printVolume(int32 x, int32 y, bool printHeader);
	void printPlaybackTime(int32 x, int32 y);
	void printPlaybackProgress(int32 x, int32 y);

	override bool handleMessage(Telegram telegram);
}


#endif
