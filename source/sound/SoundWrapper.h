/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#define __MAXIMUM_VOLUME					0xF
#define __MIDI_CONVERTER_FREQUENCY_US		20
#define __SOUND_TARGET_US_PER_TICK			__MIDI_CONVERTER_FREQUENCY_US

#define __SOUND_LR							0xFF
#define __SOUND_L							0xF0
#define __SOUND_R							0x0F

#define __MAXIMUM_MIDI_FREQUENCY			D_8


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

enum SoundChannelTypes
{
	kChannelNormal 			= (1 << 0),
	kChannelModulation		= (1 << 1),
	kChannelNoise			= (1 << 2)
};

typedef struct SoundChannelConfiguration
{
	/// kMIDI, kPCM
	u32 trackType;

	/// SxINT
	u8 SxINT;

	/// Volume SxLRV
	u8 SxLRV;

	/// SxRAM
	u8 SxRAM;

	/// SxEV0
	u8 SxEV0;

	/// SxEV1
	u8 SxEV1;

	/// SxFQH
	u8 SxFQH;

	/// SxFQL
	u8 SxFQL;

	/// Ch. 5 only
	u8 S5SWP;

	/// Waveform data pointer
	const s8* waveFormData;

	/// kChannelNormal, kChannelModulation, kChannelNoise
	u32 channelType;

	/// Volume
	u8 volume;

} SoundChannelConfiguration;

typedef const SoundChannelConfiguration SoundChannelConfigurationROM;

typedef struct SoundChannel
{
	/// Configuration
	SoundChannelConfiguration* soundChannelConfiguration;

	/// Length
	u32 length;

	/// Sound track
	union SoundTrack
	{
		/// Sound track 8Bit (PCM)
		const u8* dataPCM;

		/// Sound track 16Bit (MIDI)
		const u16* dataMIDI;

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
	u16 targetTimerResolutionUS;

	/// Tracks
	SoundChannel** soundChannels;

} Sound;

typedef const Sound SoundROM;

typedef struct Waveform
{
	u8 number;
	s8 usageCount;
	u8* wave;
	const s8* data;

} Waveform;

typedef struct Channel
{
	// Channel configuration
	SoundChannelConfiguration soundChannelConfiguration;

	/// Sound definition
	Sound* sound;

	/// Channel's effective length
	u32 length;

	/// Position within the sound track
	u32 cursor;

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
		const u8* dataPCM;

		/// Sound track 16Bit (MIDI)
		const u16* dataMIDI;

	} soundTrack;

	u32 type;

	u8 number;
	u8 soundChannel;
	u8 volumeReduction;
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
	Sound* sound;
	const Vector3D* position;
	VirtualList channels;
	fix17_15 speed;
	fix17_15 targetTimerResolutionFactor;
	u32 elapsedMicroseconds;
	u32 totalPlaybackMilliseconds;
	u16 pcmTargetPlaybackFrameRate;
	u16 frequencyModifier;
	s8 volumeReduction;
	bool paused;
	bool hasMIDITracks;
	bool hasPCMTracks;
	bool unmute;

	/// @publicsection
	void constructor(Sound* sound, VirtualList channels, s8* waves, u16 pcmTargetPlaybackFrameRate, EventListener soundReleaseListener, Object scope);

	const Channel* getChannel(u8 index);
	bool isPaused();
	bool hasPCMTracks();
	void play(const Vector3D* position, u32 playbackType);
	void pause();
	void unpause();
	void rewind();
	void stop();
	void release();
	void mute();
	void unmute();
	void updateMIDIPlayback(u32 elapsedMicroseconds);
	void updatePCMPlayback(u32 elapsedMicroseconds);
	void setSpeed(fix17_15 speed);
	void setVolumeReduction(s8 volumeReduction);
	s8 getVolumeReduction();
	fix17_15 getSpeed();
	void computeTimerResolutionFactor();
	void setFrequencyModifier(u16 frequencyModifier);
	u16 getFrequencyModifier();
	void print(int x, int y);
	void printMetadata(int x, int y);
	void printVolume(int x, int y, bool printHeader);
	void printPlaybackTime(int x, int y);
	void printPlaybackProgress(int x, int y);

	override bool handleMessage(Telegram telegram);
}


#endif
