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
#include <TimerManager.h>
#include <SoundManager.h>
#include <Utilities.h>


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern SoundRegistry* const _soundRegistries;
extern const Optical* _optical;


//---------------------------------------------------------------------------------------------------------
//												 FRIENDS
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param channel	Channel*
 */
void SoundWrapper::constructor(Sound* sound, VirtualList channels, s8* waves, u16 pcmTargetPlaybackFrameRate)
{
	// construct base Container
	Base::constructor();

	this->paused = true;
	this->sound = sound;
	this->hasMIDITracks = false;
	this->hasPCMTracks = false;
	this->speed = __I_TO_FIX17_15(1);
	this->pcmTargetPlaybackFrameRate = pcmTargetPlaybackFrameRate;

	// Compute target timerCounter factor
	SoundWrapper::computeTimerResolutionFactor(this);

	this->channels = new VirtualList();

	VirtualList::copy(this->channels, channels);
	SoundWrapper::setupChannels(this, waves);
	SoundWrapper::configureSoundRegistries(this);
}

/**
 * Class destructor
 */
void SoundWrapper::destructor()
{
	SoundWrapper::stop(this);

	if(!isDeleted(this->channels))
	{
		VirtualNode node = this->channels->head;

		for(; node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;
			channel->sound = NULL;
		}

		delete this->channels;
		this->channels = NULL;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

void SoundWrapper::computeTimerResolutionFactor()
{
	u16 timerResolutionUS = TimerManager::getResolutionInUS(TimerManager::getInstance());
	u16 timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance()) + 1;
	u16 timerUsPerInterrupt = timerCounter * timerResolutionUS;
	u16 soundTargetUsPerInterrupt = (__TIME_US(this->sound->targetTimerResolutionUS) + 1 ) * __SOUND_TARGET_US_PER_TICK;
	this->targetTimerResolutionFactor = __FIX17_15_DIV(__I_TO_FIX17_15(soundTargetUsPerInterrupt), __I_TO_FIX17_15(timerUsPerInterrupt));

	// Compensate for the difference in speed between 20US and 100US timer resolution
	fix17_15 timerResolutionRatioReduction = __I_TO_FIX17_15(1) - __FIX17_15_DIV(__I_TO_FIX17_15(timerResolutionUS), __I_TO_FIX17_15(100));

	if(0 != timerResolutionRatioReduction)
	{
		this->targetTimerResolutionFactor = __FIX17_15_MULT(this->targetTimerResolutionFactor, timerResolutionRatioReduction);
	}
}

fix17_15 SoundWrapper::getSpeed()
{
	return this->speed;
}

/**
 * Set playback speed. Changing the speed during playback may cause
 * the tracks to go out of sync because of the channel's current ticks.
 *
 * @speed 	fix17_15 PCM playback max speed is 100%
 */
void SoundWrapper::setSpeed(fix17_15 speed)
{
	// Prevent timer interrupts to unsync tracks
	bool paused = this->paused;
	this->paused = true;
	this->speed = 0 >= speed ? __F_TO_FIX17_15(0.01f) : speed <= __F_TO_FIX17_15(2.0f) ? speed : __F_TO_FIX17_15(2.0f);

	VirtualNode node = this->channels->head;

	// Prepare channels
	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		SoundWrapper::computeNextTicksPerNote(this, channel, channel->ticks);
	}

	this->paused = paused;
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

	fix17_15 maxOutputLevel = __I_TO_FIX17_15(__MAXIMUM_VOLUME);
	fix17_15 leftDistance = __ABS(__FIX17_15_MULT(relativePosition.x - __PIXELS_TO_METERS(__LEFT_EAR_CENTER), __SOUND_STEREO_ATTENUATION_FACTOR));
	fix17_15 rightDistance = __ABS(__FIX17_15_MULT(relativePosition.x - __PIXELS_TO_METERS(__RIGHT_EAR_CENTER), __SOUND_STEREO_ATTENUATION_FACTOR));

	fix17_15 leftOutput = maxOutputLevel - __FIX17_15_MULT(maxOutputLevel, __FIX17_15_DIV(leftDistance, _optical->horizontalViewPointCenter));
	u32 leftVolume = __FIX17_15_TO_I(leftOutput - __FIX17_15_MULT(leftOutput, relativePosition.z >> _optical->maximumXViewDistancePower));

	fix17_15 rightOutput = maxOutputLevel - __FIX17_15_MULT(maxOutputLevel, __FIX17_15_DIV(rightDistance, _optical->horizontalViewPointCenter));
	u32 rightVolume = __FIX17_15_TO_I(rightOutput - __FIX17_15_MULT(rightOutput, relativePosition.z >> _optical->maximumXViewDistancePower));

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
 * Is paused?
 *
 * @return bool
 */
bool SoundWrapper::isPaused()
{
	return this->paused;
}

/**
 * Play
 *
 */
void SoundWrapper::play(const Vector3D* position)
{
	bool wasPaused = this->paused;
	this->paused = false;

	u8 SxLRV = 0x00;

	if(NULL != position)
	{
		SxLRV = SoundWrapper::getVolumeFromPosition(this, position);
	}

	VirtualNode node = this->channels->head;

	// Prepare channels
	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		_soundRegistries[channel->number].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;

		// Don't allow the sound to come out just yet
		if(position)
		{
			channel->soundChannelConfiguration.SxLRV = SxLRV;
		}
	}

	if(!wasPaused && this->hasPCMTracks)
	{
		SoundManager::startPCMPlayback(SoundManager::getInstance());
	}
}

/**
 * Pause
 *
 */
void SoundWrapper::pause()
{
	this->paused = true;

	VirtualNode node = this->channels->head;

	// Silence all channels first
	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		_soundRegistries[channel->number].SxLRV = 0x00;
	}
}

/**
 * Rewind
 *
 */
void SoundWrapper::rewind()
{
	VirtualNode node = this->channels->head;

	// Silence all channels first
	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		channel->cursor = 0;

		SoundWrapper::computeNextTicksPerNote(this, channel, 0);
	}
}

/**
 * Stop
 *
 */
void SoundWrapper::stop()
{
	this->paused = true;

	VirtualNode node = this->channels->head;

	// Silence all channels first
	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		channel->cursor = 0;
		_soundRegistries[channel->number].SxINT = 0x00;
		_soundRegistries[channel->number].SxLRV = 0x00;
	}
}

/**
 * Release
 *
 */
void SoundWrapper::release()
{
	SoundManager::releaseSoundWrapper(SoundManager::getInstance(), this);
}


void SoundWrapper::setupChannels(s8* waves)
{
	if(isDeleted(this->channels))
	{
		return;
	}

	VirtualNode node = this->channels->head;

	u16 i = 0;

	for(; node; node = node->next, i++)
	{
		Channel* channel = (Channel*)node->data;

		channel->sound = this->sound;
		channel->finished = false;
		channel->cursor = 0;
		channel->soundChannel = i;
		channel->soundChannelConfiguration = *channel->sound->soundChannels[i]->soundChannelConfiguration;
		channel->ticks = 0;
		channel->soundChannelConfiguration.SxRAM = waves[i];

		switch(channel->soundChannelConfiguration.type)
		{
			case kMIDI:

				this->hasMIDITracks = true;
				channel->length = SoundWrapper::computeMIDITrackLength((u16*)this->sound->soundChannels[channel->soundChannel]->soundTrack.dataMIDI);
				break;

			case kPCM:

				this->hasPCMTracks = true;
				channel->length = this->sound->soundChannels[channel->soundChannel]->length;
				break;

			default:

				NM_ASSERT(false, "SoundWrapper::setupChannels: unknown track type");
				break;
		}

		SoundWrapper::computeNextTicksPerNote(this, channel, 0);
	}

	if(this->sound->synchronizedPlayback)
	{
		// Put the channel with the longest track first
		VirtualNode node = this->channels->head;

		for(; node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;

			VirtualNode auxNode = this->channels->tail;

			for(; auxNode && auxNode != node; auxNode = auxNode->previous)
			{
				Channel* auxChannel = (Channel*)auxNode->data;

				if(channel->length < auxChannel->length)
				{
					VirtualNode::swapData(node, auxNode);
				}
			}
		}
	}
}

void SoundWrapper::configureSoundRegistries()
{
	if(NULL == this->sound)
	{
		return;
	}

	VirtualNode node = this->channels->head;

	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		_soundRegistries[channel->number].SxEV0 = channel->soundChannelConfiguration.SxEV0;
		_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1;
		_soundRegistries[channel->number].SxFQH = channel->soundChannelConfiguration.SxFQH;
		_soundRegistries[channel->number].SxFQL = channel->soundChannelConfiguration.SxFQL;
		_soundRegistries[channel->number].SxRAM = channel->soundChannelConfiguration.SxRAM;
		_soundRegistries[channel->number].SxINT = 0x00;

		// Don't raise the volume just yet
		_soundRegistries[channel->number].SxLRV = 0x00;
	}
}

static u16 SoundWrapper::computeMIDITrackLength(u16* soundTrackData)
{
	u16 i = 0;

	NM_ASSERT(soundTrackData, "SoundWrapper::computeMIDITrackLength: null soundTrack");

	for(; soundTrackData[i] != ENDSOUND && soundTrackData[i] != LOOPSOUND; i++);

	return i;
}

static void SoundWrapper::updateMIDIPlayback(Channel* channel, bool mute)
{
	u16 note = channel->sound->soundChannels[channel->soundChannel]->soundTrack.dataMIDI[channel->cursor];

	// Is it a special note?
	switch(note)
	{
		case PAU:

			_soundRegistries[channel->number].SxLRV = 0;
			break;

		case HOLD:
			// Continue playing the previous note.
			break;

		case ENDSOUND:

			// I handle end sound
			break;

		case LOOPSOUND:
			break;

		default:
			{
				u8 volume = channel->sound->soundChannels[channel->soundChannel]->soundTrack.dataMIDI[channel->length * 2 + 1 + channel->cursor];
				channel->soundChannelConfiguration.SxLRV = (volume << 4) | volume;

				_soundRegistries[channel->number].SxFQL = channel->soundChannelConfiguration.SxFQL = (note & 0xFF);
				_soundRegistries[channel->number].SxFQH = channel->soundChannelConfiguration.SxFQH = (note >> 8);
				_soundRegistries[channel->number].SxLRV = mute ? 0 : channel->soundChannelConfiguration.SxLRV;
			}
			break;
	}
}

static void SoundWrapper::updatePCMPlayback(Channel* channel, bool mute)
{
	u8 volume = channel->sound->soundChannels[channel->soundChannel]->soundTrack.dataPCM[channel->cursor];

	s8 finalVolume = (s8)volume - __MAXIMUM_VOLUME * (channel->soundChannel);

	if(finalVolume  < 0)
	{
		finalVolume  = 0;
	}
	else if (finalVolume  > __MAXIMUM_VOLUME)
	{
		finalVolume  = __MAXIMUM_VOLUME;
	}

	if(mute)
	{
		_soundRegistries[channel->number].SxLRV = 0;
	}
	else
	{
#ifdef __TOOLS
		_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV = (((u8)finalVolume  << 4) & 0xF0) | (((u8)finalVolume ) & 0x0F);
#else
#ifndef __RELEASE
		_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV = (((u8)finalVolume  << 4) & 0xF0) | (((u8)finalVolume ) & 0x0F);
#else
		_soundRegistries[channel->number].SxLRV = (((u8)finalVolume  << 4) & 0xF0) | (((u8)finalVolume ) & 0x0F);
#endif
#endif
	}

//	_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1 = 0x40;
}

void SoundWrapper::computeNextTicksPerNote(Channel* channel, fix17_15 residue)
{
	switch (channel->soundChannelConfiguration.type)
	{
		case kMIDI:
			{
				channel->ticks = residue;
				channel->ticksPerNote = __I_TO_FIX17_15(channel->sound->soundChannels[channel->soundChannel]->soundTrack.dataMIDI[channel->length + 1 + channel->cursor]);

				channel->ticksPerNote = __FIX17_15_DIV(channel->ticksPerNote, this->speed);

				fix17_15 effectiveTicksPerNote = __FIX17_15_DIV(channel->ticksPerNote, this->targetTimerResolutionFactor);
				channel->tickStep = __FIX17_15_DIV(effectiveTicksPerNote, channel->ticksPerNote);
			}
			break;

		case kPCM:

			channel->ticksPerNote = __FIX17_15_DIV(__I_TO_FIX17_15(1), this->speed) >> 1;
			channel->tickStep = __I_TO_FIX17_15(1);
			channel->ticks = 0;
			break;

		default:

			NM_ASSERT(false, "SoundWrapper::computeNextTicksPerNote: unknown track type");
			break;
	}
}

/**
 * Update sound playback
 */
void SoundWrapper::updatePlayback(u32 type, bool mute)
{
	if(NULL == this->sound || this->paused)
	{
		return;
	}

	bool updatePlayback = false;
	bool finished = true;

	VirtualNode node = this->channels->head;
	Channel* firstChannel = this->sound->synchronizedPlayback ? (Channel*)node->data : NULL;

	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		// TODO: optimize playback of types
		if(type != channel->soundChannelConfiguration.type)
		{
			finished &= channel->finished;
			continue;
		}

		if(NULL != firstChannel && channel != firstChannel)
		{
			if(firstChannel->cursor >= channel->length)
			{
				_soundRegistries[channel->number].SxLRV = 0;
				channel->cursor = channel->length - 1;
				channel->finished = true;
			}
			else
			{
				updatePlayback = channel->cursor != firstChannel->cursor;
				channel->cursor = firstChannel->cursor;
				channel->finished = false;
			}
		}
		else
		{
			channel->ticks += channel->tickStep;

			if(channel->ticks > channel->ticksPerNote)
			{
				updatePlayback = true;

				if(++channel->cursor >= channel->length)
				{
					channel->finished = true;

					if(this->sound->loop)
					{
						channel->cursor = 0;
					}
					else
					{
						channel->finished = true;
						channel->cursor = channel->length;
					}
				}

				SoundWrapper::computeNextTicksPerNote(this, channel, channel->ticks - channel->ticksPerNote);
			}
		}

		finished &= channel->finished;

		if(updatePlayback)
		{
			switch(channel->soundChannelConfiguration.type)
			{
				case kMIDI:

					SoundWrapper::updateMIDIPlayback(channel, mute);
					break;

				case kPCM:

					SoundWrapper::updatePCMPlayback(channel, mute);
					break;
			}
		}
	}

	if(finished)
	{
		SoundWrapper::fireEvent(this, kSoundFinished);

		if(!this->sound->loop)
		{
			SoundWrapper::fireEvent(this, kSoundReleased);

			SoundWrapper::release(this);
		}
	}
}

void SoundWrapper::print(int x, int y)
{
	int xDisplacement = 9;

	VirtualNode node = this->channels->head;

	// Prepare channels
	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		PRINT_TEXT("CHANNEL: ", x, y);
		PRINT_INT(channel->number, x + xDisplacement, y);

		PRINT_TEXT("Type:         ", x, ++y);

		char* soundType = "?";
		switch(channel->soundChannelConfiguration.type)
		{
			case kMIDI:

				soundType = "MIDI";
				break;

			case kPCM:

				soundType = "PCM";
				break;
		}

		PRINT_TEXT(soundType, x + xDisplacement, y);

		PRINT_TEXT("Cursor:        ", x, ++y);
		PRINT_INT(channel->cursor, x + xDisplacement, y);

		PRINT_TEXT("Snd Chnl: ", x, ++y);
		PRINT_INT(channel->soundChannel, x + xDisplacement, y);

		PRINT_TEXT("SxINT: ", x, ++y);
		PRINT_HEX_EXT(channel->soundChannelConfiguration.SxINT | (NULL == channel->sound ? 0 : 0x80), x + xDisplacement, y, 2);

		PRINT_TEXT("SxLRV: ", x, ++y);
		PRINT_HEX_EXT(channel->soundChannelConfiguration.SxLRV, x + xDisplacement, y, 2);

		PRINT_TEXT("SxRAM: ", x, ++y);
		PRINT_HEX_EXT(channel->soundChannelConfiguration.SxRAM, x + xDisplacement, y, 2);

		PRINT_TEXT("SxEV0: ", x, ++y);
		PRINT_HEX_EXT(channel->soundChannelConfiguration.SxEV0, x + xDisplacement, y, 2);

		PRINT_TEXT("SxEV1: ", x, ++y);
		PRINT_HEX_EXT(channel->soundChannelConfiguration.SxEV1, x + xDisplacement, y, 2);

		PRINT_TEXT("SxFQH: ", x, ++y);
		PRINT_HEX_EXT(channel->soundChannelConfiguration.SxFQH, x + xDisplacement, y, 2);

		PRINT_TEXT("SxFQH: ", x, ++y);
		PRINT_HEX_EXT(channel->soundChannelConfiguration.SxFQL, x + xDisplacement, y, 2);

		PRINT_TEXT("Loop: ", x, ++y);
		PRINT_TEXT(channel->sound->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + xDisplacement, y);

		PRINT_TEXT("Length: ", x, ++y);
		PRINT_INT(channel->length, x + xDisplacement, y);

		PRINT_TEXT("Note: ", x, ++y);
		switch(channel->soundChannelConfiguration.type)
		{
			case kMIDI:

				PRINT_HEX_EXT(channel->sound->soundChannels[channel->soundChannel]->soundTrack.dataMIDI[channel->cursor], x + xDisplacement, y, 2);
				break;

			case kPCM:

				PRINT_HEX_EXT(channel->sound->soundChannels[channel->soundChannel]->soundTrack.dataPCM[channel->cursor], x + xDisplacement, y, 2);
				break;
		}
	}
}

u32 SoundWrapper::getTotalPlaybackTime()
{
	Channel* firstChannel = (Channel*)this->channels->head->data;

	switch(firstChannel->soundChannelConfiguration.type)
	{
		case kMIDI:
			{
				u32 totalNotesTiming = 0;

				u16* soundTrackData = (u16*)this->sound->soundChannels[firstChannel->soundChannel]->soundTrack.dataMIDI;

				for(u32 i = 0; i < firstChannel->length; i++, totalNotesTiming += soundTrackData[firstChannel->length + i]);

				return (u32)((long)totalNotesTiming * this->sound->targetTimerResolutionUS / __MICROSECONDS_IN_SECOND);
			}
			break;

		case kPCM:

			return firstChannel->length / this->pcmTargetPlaybackFrameRate;
			break;
	}

	return 0;
}

u32 SoundWrapper::getCurrentPlaybackTime()
{
	Channel* firstChannel = (Channel*)this->channels->head->data;

	switch(firstChannel->soundChannelConfiguration.type)
	{
		case kMIDI:
			{
				u32 totalNotesTiming = 0;

				u16* soundTrackData = (u16*)this->sound->soundChannels[firstChannel->soundChannel]->soundTrack.dataMIDI;

				for(u32 i = 0; i < firstChannel->cursor; i++, totalNotesTiming += soundTrackData[firstChannel->length + i]);

				return (u32)((long)totalNotesTiming * this->sound->targetTimerResolutionUS / __MICROSECONDS_IN_SECOND);
			}
			break;

		case kPCM:

			return firstChannel->cursor / this->pcmTargetPlaybackFrameRate;
			break;
	}

	return 0;
}

void SoundWrapper::printProgress(int x, int y)
{
	Channel* firstChannel = (Channel*)this->channels->head->data;
	u32 position = (firstChannel->cursor << 5) / firstChannel->length;

	for(u8 i = 0; i < 32; i++)
	{
		PRINT_TEXT((position > i) ? __CHAR_BRIGHT_RED_BOX : __CHAR_DARK_RED_BOX, x + i, y);
	}

	SoundWrapper::printTiming(this, SoundWrapper::getCurrentPlaybackTime(this), x + 23, y + 2);
}

void SoundWrapper::printTiming(u32 seconds, int x, int y)
{
	u32 minutes = seconds / 60;
	seconds = seconds - minutes * 60;

	int minutesDigits = Utilities::getDigitCount(minutes);

	PRINT_INT(minutes, x, y);
	PRINT_TEXT(":  ", x + minutesDigits, y);
	if(seconds < 10)
	{
		PRINT_TEXT(":0 ", x + minutesDigits, y);
		PRINT_INT(seconds, x + minutesDigits + 2, y);
	}
	else
	{
		PRINT_INT(seconds, x + minutesDigits + 1, y);
	}

}

void SoundWrapper::printMetadata(int x, int y)
{
	PRINT_TEXT(this->sound->name, x, y++);
	y++;

	SoundWrapper::printProgress(this, x, y++);

	Channel* firstChannel = (Channel*)this->channels->head->data;
	u32 position = ((firstChannel->cursor / 2) << 5) / firstChannel->length;

	u8 trackInfoXOffset = x + 22;
	u8 trackInfoValuesXOffset = 9;
	u16 speed = __FIX17_15_TO_I(__FIX17_15_MULT(this->speed, __I_TO_FIX17_15(100)));

	y++;

	SoundWrapper::printTiming(this, 0, x + 23, y);
	PRINT_TEXT("/", x + 27, y);
	SoundWrapper::printTiming(this, SoundWrapper::getTotalPlaybackTime(this), x + 28, y);

	PRINT_TEXT("Speed", x, y);
	PRINT_TEXT("    ", x + 6, y);
	PRINT_INT(speed, x + 6, y);
	PRINT_TEXT("%", x + 6 + ((speed < 10) ? 1 : (speed < 100) ? 2 : 3), y);
	PRINT_TEXT(!this->paused ? "  " : "\x07\x07", x + 15, y);
	y+=2;

	PRINT_TEXT("TRACK INFO", trackInfoXOffset, y++);

	PRINT_TEXT("MIDI", trackInfoXOffset, ++y);
	PRINT_TEXT(this->hasMIDITracks ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("PCM", trackInfoXOffset, ++y);
	PRINT_TEXT(this->hasPCMTracks ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Sync", trackInfoXOffset, ++y);
	PRINT_TEXT(this->sound->synchronizedPlayback ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Channels", trackInfoXOffset, ++y);
	PRINT_INT(VirtualList::getSize(this->channels), trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Loop", trackInfoXOffset, ++y);
	PRINT_TEXT(this->sound->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);
}

void SoundWrapper::printVolume(int x, int y)
{
	VirtualNode node = this->channels->head;

	PRINT_TEXT("VOLUME", x, ++y);
	++y;
	++y;

	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		PRINT_TEXT("C", x + 15, y);
		PRINT_INT(channel->number, x + 16, y);

		u8 leftVolume = (channel->soundChannelConfiguration.SxLRV & 0xF0) >> 4;
		u8 rightVolume = (channel->soundChannelConfiguration.SxLRV & 0x0F);
		u8 i;

		u8 frequency = channel->soundChannelConfiguration.SxFQH | channel->soundChannelConfiguration.SxFQL;

		switch(channel->soundChannelConfiguration.type)
		{
			case kMIDI:

				for(i=0; i<15; i++) {
					PRINT_TEXT(((frequency * leftVolume / __MAXIMUM_VOLUME) >> 4) > i ? __CHAR_BRIGHT_RED_BOX : __CHAR_DARK_RED_BOX, x + 14 - i, y);
					PRINT_TEXT(((frequency * rightVolume / __MAXIMUM_VOLUME) >> 4) > i ? __CHAR_BRIGHT_RED_BOX : __CHAR_DARK_RED_BOX, x + 17 + i, y);
				}
				break;

			case kPCM:

				for(i=0; i<15; i++) {
					PRINT_TEXT(leftVolume > i ? __CHAR_BRIGHT_RED_BOX : __CHAR_DARK_RED_BOX, x + 14 - i, y);
					PRINT_TEXT(rightVolume > i ? __CHAR_BRIGHT_RED_BOX : __CHAR_DARK_RED_BOX, x + 17 + i, y);
				}
				break;

			default:

				NM_ASSERT(false, "SoundWrapper::printMetadata: unknown track type");
				break;
		}

		y++;
	}
}

