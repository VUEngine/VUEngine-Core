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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SoundWrapper.h>
#include <SoundManager.h>
#include <MIDINotes.h>


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern SoundRegistry* const _soundRegistries;
extern const Optical* _optical;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param channel	Channel*
 */
void SoundWrapper::constructor(u16 channelNumber)
{
	// construct base Container
	Base::constructor();

	this->channelNumber = channelNumber;
	this->leadedSoundWrappers = NULL;
	this->paused = true;
	SoundWrapper::reset(this);
}

/**
 * Class destructor
 */
void SoundWrapper::destructor()
{
	this->channel.sound = NULL;

	if(!isDeleted(this->leadedSoundWrappers))
	{
		delete this->leadedSoundWrappers;
		this->leadedSoundWrappers = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Calculate sound volume according to its spatial position
 *
 * @private
 */
u8 SoundWrapper::getVolumeFromPosition(const Vector3D* position)
{
	// set position inside camera coordinates
	Vector3D relativePosition = Vector3D::getRelativeToCamera(*position);

	fix10_6 maxOutputLevel = __I_TO_FIX10_6(__MAXIMUM_VOLUMEN);
	fix10_6 leftDistance = __ABS(__FIX10_6_MULT(relativePosition.x - __PIXELS_TO_METERS(__LEFT_EAR_CENTER), __SOUND_STEREO_ATTENUATION_FACTOR));
	fix10_6 rightDistance = __ABS(__FIX10_6_MULT(relativePosition.x - __PIXELS_TO_METERS(__RIGHT_EAR_CENTER), __SOUND_STEREO_ATTENUATION_FACTOR));

	fix10_6 leftOutput = maxOutputLevel - __FIX10_6_MULT(maxOutputLevel, __FIX10_6_DIV(leftDistance, _optical->horizontalViewPointCenter));
	u32 leftVolume = __FIX10_6_TO_I(leftOutput - __FIX10_6_MULT(leftOutput, relativePosition.z >> _optical->maximumXViewDistancePower));

	fix10_6 rightOutput = maxOutputLevel - __FIX10_6_MULT(maxOutputLevel, __FIX10_6_DIV(rightDistance, _optical->horizontalViewPointCenter));
	u32 rightVolume = __FIX10_6_TO_I(rightOutput - __FIX10_6_MULT(rightOutput, relativePosition.z >> _optical->maximumXViewDistancePower));

	u8 volume = 0x00;

	/* The maximum sound level for each side is 0xF
	 * In the center position the output level is the one
	 * defined in the sound's spec */
	if(0 < leftVolume)
	{
		volume |= (leftVolume << 4);
	}

	if(0 < rightVolume)
	{
		volume |= rightVolume;
	}
	else
	{
		volume &= 0xF0;
	}

	return volume;
}

/**
 * Play
 *
 */
bool SoundWrapper::play(const Vector3D* position)
{
	if(SoundWrapper::isFree(this))
	{
		return false;
	}

	this->paused = false;
	_soundRegistries[this->channel.number].SxINT = this->channel.soundChannelConfiguration.SxINT | 0x80;	

	if(!isDeleted(this->leadedSoundWrappers))
	{
		VirtualNode node = VirtualList::begin(this->leadedSoundWrappers);

		for(; node; node = VirtualNode::getNext(node))
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(VirtualNode::getData(node));

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::play(soundWrapper, position);
			}
		}
	}

	if(NULL != position)
	{
		this->channel.soundChannelConfiguration.SxLRV = SoundWrapper::getVolumeFromPosition(this, position);
	}

	return true;
}

/**
 * Pause
 *
 */
bool SoundWrapper::pause()
{
	if(SoundWrapper::isFree(this))
	{
		return false;
	}

	this->paused = true;

	if(!isDeleted(this->leadedSoundWrappers))
	{
		VirtualNode node = VirtualList::begin(this->leadedSoundWrappers);

		for(; node; node = VirtualNode::getNext(node))
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(VirtualNode::getData(node));

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::pause(soundWrapper);
			}
		}
	}

	return true;
}

/**
 * Rewind
 *
 */
bool SoundWrapper::rewind()
{
	if(SoundWrapper::isFree(this))
	{
		return false;
	}

	this->channel.cursor = 0;

	if(!isDeleted(this->leadedSoundWrappers))
	{
		VirtualNode node = VirtualList::begin(this->leadedSoundWrappers);

		for(; node; node = VirtualNode::getNext(node))
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(VirtualNode::getData(node));

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::rewind(soundWrapper);
			}
		}
	}

	return true;
}

/**
 * Stop
 *
 */
bool SoundWrapper::stop()
{
	if(SoundWrapper::isFree(this))
	{
		return false;
	}

	this->paused = true;
	this->channel.cursor = 0;
	_soundRegistries[this->channel.number].SxINT = this->channel.soundChannelConfiguration.SxINT = 0;	

	if(!isDeleted(this->leadedSoundWrappers))
	{
		VirtualNode node = VirtualList::begin(this->leadedSoundWrappers);

		for(; node; node = VirtualNode::getNext(node))
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(VirtualNode::getData(node));

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::stop(soundWrapper);
			}
		}
	}

	return true;
}


/**
 * Release
 *
 */
bool SoundWrapper::release()
{
	if(SoundWrapper::isFree(this))
	{
		return false;
	}

	_soundRegistries[this->channel.number].SxINT = this->channel.soundChannelConfiguration.SxINT = 0;	

	if(!isDeleted(this->leadedSoundWrappers))
	{
		VirtualNode node = VirtualList::begin(this->leadedSoundWrappers);

		for(; node; node = VirtualNode::getNext(node))
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(VirtualNode::getData(node));

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::release(soundWrapper);
			}
		}
	}

	SoundWrapper::reset(this);

	return true;
}

/**
 * Reset
 *
 */
void SoundWrapper::reset()
{
	this->paused = true;

	this->channel.number = this->channelNumber;
	this->channel.sound = NULL;
	this->channel.cursor = 0;
	this->channel.delay = 0;
	this->channel.soundChannel = 0;
	this->channel.partners = 0;
	this->channel.leaderChannel = NULL;

	this->channel.soundChannelConfiguration.type = kUnknownType;
	this->channel.soundChannelConfiguration.SxLRV = 0;
	this->channel.soundChannelConfiguration.SxRAM = 0;
	this->channel.soundChannelConfiguration.SxEV0 = 0;
	this->channel.soundChannelConfiguration.SxEV1 = 0;
	this->channel.soundChannelConfiguration.SxFQH = 0;
	this->channel.soundChannelConfiguration.SxFQL = 0;
	this->channel.soundChannelConfiguration.waveFormData = NULL;
	this->channel.soundChannelConfiguration.isModulation = false;

	if(!isDeleted(this->leadedSoundWrappers))
	{
		VirtualNode node = VirtualList::begin(this->leadedSoundWrappers);

		for(; node; node = VirtualNode::getNext(node))
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(VirtualNode::getData(node));

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::reset(soundWrapper);
			}
		}
	}

	if(!isDeleted(this->leadedSoundWrappers))
	{
		delete this->leadedSoundWrappers;
	}

	this->leadedSoundWrappers = NULL;
}

void SoundWrapper::setup(Sound* sound, SoundWrapper leaderSound, u8 soundChannel, u8 wave, u8 soundChannelsCount)
{
	this->channel.sound = sound;
	this->channel.cursor = 0;
	this->channel.delay = 0;
	this->channel.soundChannel = soundChannel;
	this->channel.soundChannelConfiguration = *sound->soundChannels[soundChannel]->soundChannelConfiguration;
	this->channel.soundChannelConfiguration.SxRAM = wave;
	this->channel.partners = sound->combineChannels ? soundChannelsCount - 1: 0;
	this->channel.leaderChannel = &leaderSound->channel;

	this->delay = this->channel.sound->soundChannels[this->channel.soundChannel]->delay;
	this->length = this->channel.sound->soundChannels[this->channel.soundChannel]->length;

	if(kMIDI == this->channel.soundChannelConfiguration.type)
	{
		this->length = SoundWrapper::computeMIDITrackLength(this);
	}

	SoundWrapper::configureSoundRegistries(this);
}

u16 SoundWrapper::computeMIDITrackLength()
{
	u16 i = 0;

	u16* soundTrackData = (u16*)this->channel.sound->soundChannels[this->channel.soundChannel]->soundTrack.dataMIDI;

	NM_ASSERT(soundTrackData, "SoundWrapper::computeMIDITrackLength: null soundTrack");

	for(; soundTrackData[i] != ENDSOUND && soundTrackData[i] != LOOPSOUND; i++);

	return i;
}

void SoundWrapper::configureSoundRegistries()
{
	if(NULL == this->channel.sound)
	{
		return;
	}

	_soundRegistries[this->channel.number].SxEV0 = this->channel.soundChannelConfiguration.SxEV0;
	_soundRegistries[this->channel.number].SxEV1 = this->channel.soundChannelConfiguration.SxEV1;
	_soundRegistries[this->channel.number].SxFQH = this->channel.soundChannelConfiguration.SxFQH;
	_soundRegistries[this->channel.number].SxFQL = this->channel.soundChannelConfiguration.SxFQL;
	_soundRegistries[this->channel.number].SxRAM = this->channel.soundChannelConfiguration.SxRAM;
	_soundRegistries[this->channel.number].SxINT = this->channel.soundChannelConfiguration.SxINT;

	// Don't raise the volume just yet
	_soundRegistries[this->channel.number].SxLRV = 0x00;
}

void SoundWrapper::addLeadedSound(SoundWrapper leadedSound)
{
	if(this == leadedSound)
	{
		return;
	}

	if(NULL == this->leadedSoundWrappers)
	{
		this->leadedSoundWrappers = new VirtualList();
	}

	VirtualList::pushBack(this->leadedSoundWrappers, leadedSound);
}

bool SoundWrapper::isSoundLeader(SoundWrapper soundWrapper)
{
	return !isDeleted(soundWrapper) ? this->channel.leaderChannel == &soundWrapper->channel : false;
}

bool SoundWrapper::isFree()
{
	return NULL == this->channel.sound;
}


u32 SoundWrapper::getType()
{
	return this->channel.soundChannelConfiguration.type;
}

void SoundWrapper::updateMIDIPlayback(bool mute)
{
	u16 note = this->channel.sound->soundChannels[this->channel.soundChannel]->soundTrack.dataMIDI[this->channel.cursor];

	// Is it a special note?
	switch(note)
	{
		case PAU:

			_soundRegistries[this->channel.number].SxLRV = 0;
			break;
	
		case HOLD:
			// Continue playing the previous note.
			break;

		case ENDSOUND:

			SoundWrapper::release(this);
			break;

		case LOOPSOUND:
			break;

		default:

			_soundRegistries[this->channel.number].SxFQL = this->channel.soundChannelConfiguration.SxFQL = (note & 0xFF);
			_soundRegistries[this->channel.number].SxFQH = this->channel.soundChannelConfiguration.SxFQH = (note >> 8);

			_soundRegistries[this->channel.number].SxLRV = mute ? 0 : this->channel.soundChannelConfiguration.SxLRV;
			break;
	}
 }

void SoundWrapper::updatePCMPlayback(bool mute)
{
	u8 volume = this->channel.sound->soundChannels[this->channel.soundChannel]->soundTrack.dataPCM[this->channel.cursor];

	s8 finalVolume = (s8)volume - __MAXIMUM_VOLUMEN * (this->channel.soundChannel);

	if(finalVolume  < 0)
	{
		finalVolume  = 0;
	}
	else if (finalVolume  > __MAXIMUM_VOLUMEN)
	{
		finalVolume  = __MAXIMUM_VOLUMEN;
	}			

	if(mute)
	{
		_soundRegistries[this->channel.number].SxLRV = 0;
	}
	else
	{
#ifdef __RELEASE
		_soundRegistries[this->channel.number].SxLRV = (((u8)finalVolume  << 4) & 0xF0) | (((u8)finalVolume ) & 0x0F);
#else
		_soundRegistries[this->channel.number].SxLRV = this->channel.soundChannelConfiguration.SxLRV = (((u8)finalVolume  << 4) & 0xF0) | (((u8)finalVolume ) & 0x0F);
#endif
	}

//	_soundRegistries[this->channel.number].SxEV1 = this->channel.soundChannelConfiguration.SxEV1 = 0x40;
}

/**
 * Update sound playback
 */
void SoundWrapper::updatePlayback(bool mute)
{
	if(NULL == this->channel.sound || this->paused)
	{
		return;
	}

	bool updatePlayback = false;
	bool release = false;

	if(this->channel.partners && this->channel.leaderChannel != &this->channel)
	{
		updatePlayback = this->channel.cursor != this->channel.leaderChannel->cursor;
		this->channel.cursor = this->channel.leaderChannel->cursor;
	}
	else if(++this->channel.delay > this->delay)
	{
		updatePlayback = true;

		this->channel.delay = 0;

		if(++this->channel.cursor >= this->length)
		{
			this->channel.cursor = 0;
			SoundWrapper::fireEvent(this, kSoundFinished);

			if(!this->channel.sound->loop)
			{
				release = true;
			}
		}
	}

	if(updatePlayback)
	{
		switch(this->channel.soundChannelConfiguration.type)
		{
			case kMIDI:

				SoundWrapper::updateMIDIPlayback(this, mute);
				break;

			case kPCM:

				SoundWrapper::updatePCMPlayback(this, mute);
				break;
		}
	}

	if(release)
	{
		SoundWrapper::release(this);
	}
}

void SoundWrapper::printSound(int x, int y)
{
	if(NULL == this->channel.sound)
	{
		return;
	}

	int xDisplacement = 9;

//	PRINT_TEXT("Sound: ", x, ++y);
//	PRINT_HEX((u32)sound, x + xDisplacement, y);

	PRINT_TEXT("Loop: ", x, ++y);
	PRINT_TEXT(this->channel.sound->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + xDisplacement, y);

	PRINT_TEXT("Length: ", x, ++y);
	PRINT_INT(this->length, x + xDisplacement, y);
	
	PRINT_TEXT("Note: ", x, ++y);
	switch(this->channel.soundChannelConfiguration.type)
	{
		case kMIDI:

			PRINT_HEX_EXT(this->channel.sound->soundChannels[this->channel.soundChannel]->soundTrack.dataMIDI[this->channel.cursor], x + xDisplacement, y, 2);
			break;

		case kPCM:

			PRINT_HEX_EXT(this->channel.sound->soundChannels[this->channel.soundChannel]->soundTrack.dataPCM[this->channel.cursor], x + xDisplacement, y, 2);
			break;
	}

}

void SoundWrapper::print(int x, int y)
{
	int xDisplacement = 9;

	PRINT_TEXT("CHANNEL: ", x, y);
	PRINT_INT(this->channel.number, x + xDisplacement, y);

	PRINT_TEXT("Type: ", x, ++y);

	char* soundType = "?";
	switch(this->channel.soundChannelConfiguration.type)
	{
		case kMIDI:

			soundType = "MIDI";
			break;

		case kPCM:

			soundType = "PCM";
			break;
	}

	PRINT_TEXT(soundType, x + xDisplacement, y);

	PRINT_TEXT("Cursor: ", x, ++y);
	PRINT_INT(this->channel.cursor, x + xDisplacement, y);

	PRINT_TEXT("Snd Chnl: ", x, ++y);
	PRINT_INT(this->channel.soundChannel, x + xDisplacement, y);

	PRINT_TEXT("SxINT: ", x, ++y);
	PRINT_HEX_EXT(this->channel.soundChannelConfiguration.SxINT | (NULL == this->channel.sound ? 0 : 0x80), x + xDisplacement, y, 2);

	PRINT_TEXT("SxLRV: ", x, ++y);
	PRINT_HEX_EXT(this->channel.soundChannelConfiguration.SxLRV, x + xDisplacement, y, 2);

	PRINT_TEXT("SxRAM: ", x, ++y);
	PRINT_HEX_EXT(this->channel.soundChannelConfiguration.SxRAM, x + xDisplacement, y, 2);

	PRINT_TEXT("SxEV0: ", x, ++y);
	PRINT_HEX_EXT(this->channel.soundChannelConfiguration.SxEV0, x + xDisplacement, y, 2);

	PRINT_TEXT("SxEV1: ", x, ++y);
	PRINT_HEX_EXT(this->channel.soundChannelConfiguration.SxEV1, x + xDisplacement, y, 2);

	PRINT_TEXT("SxFQH: ", x, ++y);
	PRINT_HEX_EXT(this->channel.soundChannelConfiguration.SxFQH, x + xDisplacement, y, 2);

	PRINT_TEXT("SxFQH: ", x, ++y);
	PRINT_HEX_EXT(this->channel.soundChannelConfiguration.SxFQL, x + xDisplacement, y, 2);
 
	if(NULL == this->channel.sound)
	{
		PRINT_TEXT("Sound:", x, ++y);
		PRINT_TEXT("None", x + xDisplacement, y);
	}
	else
	{
		SoundWrapper::printSound(this, x, y);
	}
}
