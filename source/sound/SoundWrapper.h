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

#include <SpatialObject.h>
#include <Sprite.h>
#include <Body.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __MAXIMUM_VOLUMEN			0xF


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------


typedef struct SoundChannelConfiguration
{
	/// kMIDI, kPCM
	u32 type;

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
	const u8* waveFormData;

	/// Is modulation
	bool isModulation;

} SoundChannelConfiguration;

typedef const SoundChannelConfiguration SoundChannelConfigurationROM;

typedef struct SoundChannel
{
	/// Configuration
	SoundChannelConfiguration* soundChannelConfiguration;

	/// Length
	u32 length;

	/// Delay before moving the cursor
	u16 delay;

	/// Sound track
	const u8* soundTrack;

} SoundChannel;

typedef const SoundChannel SoundChannelROM;

typedef struct Sound
{
	/// Play in loop
	bool loop;

	/// Combine all channels into a single sound
	bool combineChannels;

	/// Tracks
	SoundChannel** soundChannels;

} Sound;

typedef const Sound SoundROM;

typedef struct Waveform
{
	u8 number;
	u8* wave;
	const u8* data;

} Waveform;

typedef struct Channel
{
	// Channel configuration
	SoundChannelConfiguration soundChannelConfiguration;

	/// Sound definition
	Sound* sound;

	/// Position within the sound track
	u32 cursor;

	/// Delay before moving the cursor
	u16 delay;

	/// Leader channel to sync PCM playback on combined channels
	struct Channel* leaderChannel;

	u8 number;
	u8 soundChannel;
	u8 partners;
	
} Channel;

enum SoundTrackTypes
{
	kUnknownType = 0,
	kMIDI,
	kPCM
};

//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities-particles
class SoundWrapper : Object
{
	Channel channel;
	VirtualList leadedSoundWrappers;
	u16 channelNumber;
	bool paused;

	/// @publicsection
	void constructor(u16 channelNumber);

	bool play(const Vector3D* position);
	bool pause();
	bool rewind();
	bool stop();
	bool release();
	void setup(Sound* sound, SoundWrapper leaderSound, u8 soundChannel, u8 wave, u8 soundChannelsCount);
	void addLeadedSound(SoundWrapper leadedSound);
	void updatePlayback();
	bool isFree();
	u32 getType();
	void print(int x, int y);
}


#endif
