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
#include <HardwareManager.h>
#include <MessageDispatcher.h>
#include <Utilities.h>
#include <debugConfig.h>


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
void SoundWrapper::constructor(Sound* sound, VirtualList channels, s8* waves, u16 pcmTargetPlaybackFrameRate, EventListener soundReleaseListener, Object scope)
{
	// construct base Container
	Base::constructor();

	this->turnedOn = false;
	this->paused = true;
	this->sound = sound;
	this->hasMIDITracks = false;
	this->hasPCMTracks = false;
	this->speed = __I_TO_FIX17_15(1);
	this->pcmTargetPlaybackFrameRate = pcmTargetPlaybackFrameRate;
	this->elapsedMicroseconds = 0;
	this->totalPlaybackMilliseconds = 0;
#ifdef __MUTE_ALL_SOUND
	this->unmute = false;
#else
	this->unmute = true;
#endif
	this->frequencyModifier = 0;
	this->position = NULL;
	this->volumeReduction = 0;

	// Compute target timerCounter factor
	SoundWrapper::computeTimerResolutionFactor(this);

	this->channels = new VirtualList();

	VirtualList::copy(this->channels, channels);
	SoundWrapper::setupChannels(this, waves);
	SoundWrapper::configureSoundRegistries(this);

	SoundWrapper::addEventListener(this, scope, soundReleaseListener, kEventSoundReleased);
}

/**
 * Class destructor
 */
void SoundWrapper::destructor()
{
	SoundWrapper::stop(this);

	if(!isDeleted(this->channels))
	{
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
	u16 timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance()) + __TIMER_COUNTER_DELTA;
	u16 timerUsPerInterrupt = timerCounter * timerResolutionUS;
	u16 soundTargetUsPerInterrupt = (__TIME_US(this->sound->targetTimerResolutionUS) + __TIMER_COUNTER_DELTA) * __SOUND_TARGET_US_PER_TICK * (timerResolutionUS / 20);
	this->targetTimerResolutionFactor = __FIX17_15_DIV(__I_TO_FIX17_15(soundTargetUsPerInterrupt), __I_TO_FIX17_15(timerUsPerInterrupt));

	// Compensate for the difference in speed between 20US and 100US timer resolution
	fix17_15 timerResolutionRatioReduction = __I_TO_FIX17_15(1) - __FIX17_15_DIV(__I_TO_FIX17_15(timerResolutionUS), __I_TO_FIX17_15(100));

	if(0 != timerResolutionRatioReduction)
	{
		this->targetTimerResolutionFactor = __FIX17_15_MULT(this->targetTimerResolutionFactor, timerResolutionRatioReduction);
	}
}

void SoundWrapper::setFrequencyModifier(u16 frequencyModifier)
{
	this->frequencyModifier = frequencyModifier;
}

u16 SoundWrapper::getFrequencyModifier()
{
	return this->frequencyModifier;
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
	if(!this->hasPCMTracks)
	{
		bool paused = this->paused;
		this->paused = true;
		this->speed = 0 >= speed ? __F_TO_FIX17_15(0.01f) : speed <= __F_TO_FIX17_15(2.0f) ? speed : __F_TO_FIX17_15(10.0f);

		this->paused = paused;
	}
}

/**
 * Return playback speed. 
 */
fix17_15 SoundWrapper::getSpeed()
{
	return this->speed;
}

/**
 * Set volume reduction
 */
void SoundWrapper::setVolumeReduction(s8 volumeReduction)
{
	this->volumeReduction = volumeReduction;
}

bool SoundWrapper::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kSoundWrapperFadeIn:

			if(0 < SoundWrapper::getVolumeReduction(this))
			{
				SoundWrapper::setVolumeReduction(this, SoundWrapper::getVolumeReduction(this) - 1);
				MessageDispatcher::dispatchMessage(100 + Utilities::random(Utilities::randomSeed(), 50), Object::safeCast(this), Object::safeCast(this), kSoundWrapperFadeIn, NULL);
			}
			break;

		case kSoundWrapperFadeOut:

			if(__MAXIMUM_VOLUME > SoundWrapper::getVolumeReduction(this))
			{
				SoundWrapper::setVolumeReduction(this, SoundWrapper::getVolumeReduction(this) + 1);
				MessageDispatcher::dispatchMessage(100 + Utilities::random(Utilities::randomSeed(), 50), Object::safeCast(this), Object::safeCast(this), kSoundWrapperFadeOut, NULL);
			}
			else
			{
				SoundWrapper::release(this);
			}
			
			break;
	}

	return Base::handleMessage(this, telegram);
}

/**
 * Get volume reduction
 */
s8 SoundWrapper::getVolumeReduction()
{
	return this->volumeReduction;
}

/**
 * Get channel
 *
 *
 * @param index u8
 * @return Channel*
 */
const Channel* SoundWrapper::getChannel(u8 index)
{
	return VirtualList::getObjectAtPosition(this->channels, index);
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
 *  Has PCM tracks?
 *
 * @return bool
 */
bool SoundWrapper::hasPCMTracks()
{
	return this->hasPCMTracks;
}

/**
 * Play
 *
 */
void SoundWrapper::play(const Vector3D* position, u32 playbackType)
{
	bool wasPaused = this->paused;
	this->paused = false;
	this->turnedOn = true;

	this->position = position;

	if(wasPaused)
	{
		VirtualNode node = this->channels->head;

		// Prepare channels
		for(; node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;
			_soundRegistries[channel->number].SxFQH = 0;
			_soundRegistries[channel->number].SxFQL = 0;
			_soundRegistries[channel->number].SxLRV = 0;
			_soundRegistries[channel->number].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;
		}
	}

	if(!wasPaused)
	{
		this->elapsedMicroseconds = 0;

		if(this->hasPCMTracks)
		{
			SoundManager::startPCMPlayback(SoundManager::getInstance());
		}
	}	

	switch(playbackType)
	{
		case kSoundWrapperPlaybackFadeIn:

			SoundWrapper::setVolumeReduction(this, __MAXIMUM_VOLUME);
			MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), Object::safeCast(this));
			MessageDispatcher::dispatchMessage(100 + Utilities::random(Utilities::randomSeed(), 50), Object::safeCast(this), Object::safeCast(this), kSoundWrapperFadeIn, NULL);
			break;

		case kSoundWrapperPlaybackFadeOut:

			MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), Object::safeCast(this));
			MessageDispatcher::dispatchMessage(100 + Utilities::random(Utilities::randomSeed(), 50), Object::safeCast(this), Object::safeCast(this), kSoundWrapperFadeOut, NULL);
			break;

		case kSoundWrapperPlaybackNormal:
		default:
			break;
	}

	if(wasPaused)
	{
		SoundWrapper::updateMIDIPlayback(this, 0);
	}
}

/**
 * Pause
 *
 */
void SoundWrapper::pause()
{
	if(this->turnedOn)
	{
		this->paused = true;

		VirtualNode node = this->channels->head;

		// Silence all channels first
		for(; node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;
			_soundRegistries[channel->number].SxLRV = 0x00;
			_soundRegistries[channel->number].SxINT = 0x00;
		}
	}
}

/**
 * Unpause
 *
 */
void SoundWrapper::unpause()
{
	if(this->turnedOn)
	{
		VirtualNode node = this->channels->head;

		// Silence all channels first
		for(; node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;
			_soundRegistries[channel->number].SxLRV = 0x00;
			_soundRegistries[channel->number].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;
		}

		this->paused = false;
	}
}

/**
 * Turn off
 *
 */
void SoundWrapper::turnOff()
{
	this->turnedOn = false;

	VirtualNode node = this->channels->head;

	// Silence all channels first
	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		_soundRegistries[channel->number].SxLRV = 0x00;
		_soundRegistries[channel->number].SxINT = 0x00;
	}
}

/**
 * Turn on
 *
 */
void SoundWrapper::turnOn()
{
	VirtualNode node = this->channels->head;

	// Silence all channels first
	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		_soundRegistries[channel->number].SxLRV = 0x00;
		_soundRegistries[channel->number].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;
	}

	this->turnedOn = true;
}

/**
 * Rewind
 *
 */
void SoundWrapper::rewind()
{
	VirtualNode node = this->channels->head;

	this->elapsedMicroseconds = 0;

	// Silence all channels first
	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		channel->finished = false;
		channel->cursor = 0;
		channel->ticks = 0;

		switch(channel->soundChannelConfiguration.trackType)
		{
			case kMIDI:

				SoundWrapper::computeMIDINextTicksPerNote(channel, channel->ticks, this->speed, this->targetTimerResolutionFactor);
				break;

			case kPCM:

				SoundWrapper::computePCMNextTicksPerNote(channel, channel->ticks, this->speed, this->targetTimerResolutionFactor);
				break;
		}
	}
}

/**
 * Stop
 *
 */
void SoundWrapper::stop()
{
	this->turnedOn = false;
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
	this->sound = NULL;

	SoundWrapper::stop(this);

	SoundManager::releaseSoundWrapper(SoundManager::getInstance(), this);

	MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), Object::safeCast(this));

	if(this->events)
	{
		SoundWrapper::fireEvent(this, kEventSoundReleased);
	}
}

void SoundWrapper::mute()
{
	this->unmute = false;
}

void SoundWrapper::unmute()
{
	this->unmute = true;
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
		channel->soundChannelConfiguration.SxRAM = waves[i];
		channel->volumeReduction = 0;

		switch(channel->soundChannelConfiguration.trackType)
		{
			case kMIDI:

				this->hasMIDITracks = true;
				channel->soundTrack.dataMIDI = (u16*)this->sound->soundChannels[channel->soundChannel]->soundTrack.dataMIDI;
				channel->length = SoundWrapper::computeMIDITrackLength((u16*)channel->soundTrack.dataMIDI);
				SoundWrapper::computeMIDINextTicksPerNote(channel, 0, this->speed, this->targetTimerResolutionFactor);
				break;

			case kPCM:

				this->hasPCMTracks = true;
				channel->soundTrack.dataPCM = (u8*)this->sound->soundChannels[channel->soundChannel]->soundTrack.dataPCM;
				channel->length = this->sound->soundChannels[channel->soundChannel]->length;
				SoundWrapper::computePCMNextTicksPerNote(channel, 0, this->speed, this->targetTimerResolutionFactor);
				channel->volumeReduction = SoundWrapper::computePCMVolumeReduction((u8*)channel->soundTrack.dataPCM, channel->length);
				break;

			default:

				NM_ASSERT(false, "SoundWrapper::setupChannels: unknown track type");
				break;
		}

		channel->ticks = 0;
	}

	node = this->channels->head;

	Channel* channelWithLongestTrack = (Channel*)node->data;

	// Find the the channel with the longest track
	for(node = node->next; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		if(channelWithLongestTrack->length < channel->length)
		{
			channelWithLongestTrack = channel;
		}
	}

	this->totalPlaybackMilliseconds = SoundWrapper::getTotalPlaybackMilliseconds(this, channelWithLongestTrack);
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

		_soundRegistries[channel->number].SxINT = 0x00;
		_soundRegistries[channel->number].SxLRV = 0x00;
		_soundRegistries[channel->number].SxEV0 = channel->soundChannelConfiguration.SxEV0;
		_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1;
		_soundRegistries[channel->number].SxFQH = channel->soundChannelConfiguration.SxFQH;
		_soundRegistries[channel->number].SxFQL = channel->soundChannelConfiguration.SxFQL;
		_soundRegistries[channel->number].SxRAM = channel->soundChannelConfiguration.SxRAM;
	}
}

static u16 SoundWrapper::computeMIDITrackLength(u16* soundTrackData)
{
	u16 i = 0;

	NM_ASSERT(soundTrackData, "SoundWrapper::computeMIDITrackLength: null soundTrack");

	for(; soundTrackData[i] != ENDSOUND && soundTrackData[i] != LOOPSOUND; i++);

	return i;
}

static u16 SoundWrapper::computePCMVolumeReduction(u8* soundTrackData, u32 length)
{
	u32 i = 0;
	NM_ASSERT(soundTrackData, "SoundWrapper::computePCMVolumeReduction: null soundTrack");

	u8 maximumVolume = 0;

	CACHE_DISABLE;
	CACHE_CLEAR;
	CACHE_ENABLE;

	for(; i < length; i++)
	{
		if(soundTrackData[i] > maximumVolume)
		{
			maximumVolume = soundTrackData[i];
		}
	}

	CACHE_DISABLE;
	CACHE_CLEAR;
	CACHE_ENABLE;

	u8 multiple = maximumVolume / __MAXIMUM_VOLUME;

	return 0 == multiple ? 0 : (multiple - 1) * __MAXIMUM_VOLUME;
}

static void SoundWrapper::computeMIDINextTicksPerNote(Channel* channel, fix17_15 residue, fix17_15 speed, fix17_15 targetTimerResolutionFactor)
{
	channel->ticks = residue;
	channel->ticksPerNote = __I_TO_FIX17_15(channel->soundTrack.dataMIDI[channel->length + 1 + channel->cursor]);
	channel->ticksPerNote = __FIX17_15_DIV(channel->ticksPerNote, speed);

	fix17_15 effectiveTicksPerNote = __FIX17_15_DIV(channel->ticksPerNote, targetTimerResolutionFactor);
	channel->tickStep = __FIX17_15_DIV(effectiveTicksPerNote, channel->ticksPerNote + 1);
}

static void SoundWrapper::computePCMNextTicksPerNote(Channel* channel, fix17_15 residue __attribute__((unused)), fix17_15 speed __attribute__((unused)), fix17_15 targetTimerResolutionFactor __attribute__((unused)))
{
	channel->ticksPerNote = 0;
	channel->tickStep = __I_TO_FIX17_15(1);
	channel->ticks = 0;
}

static inline u8 SoundWrapper::clampMIDIOutputValue(s8 value)
{
	if(value < 0)
	{
		return 0;
	}
	else if(value > __MAXIMUM_VOLUME)
	{
		return __MAXIMUM_VOLUME;
	}

	return (u8)value;
}

static inline u8 SoundWrapper::clampPCMOutputValue(s8 value)
{
    value &= -(value >= 0);
    return (u8)(value | ((__MAXIMUM_VOLUME - value) >> 7));
}

bool SoundWrapper::checkIfPlaybackFinishedOnChannel(Channel* channel)
{
	if(channel->cursor >= channel->length)
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

	return channel->finished;
}

void SoundWrapper::completedPlayback()
{
	if(this->events)
	{
		SoundWrapper::fireEvent(this, kEventSoundFinished);
	}

	if(!this->sound->loop)
	{
		SoundWrapper::release(this);
	}
	else
	{
		SoundWrapper::rewind(this);
	}
}

void SoundWrapper::playMIDINote(Channel* channel, s16 leftVolumeFactor, s16 rightVolumeFactor)
{
	s16 note = channel->soundTrack.dataMIDI[channel->cursor];
	u8 volume = SoundWrapper::clampMIDIOutputValue(channel->soundTrack.dataMIDI[(channel->length << 1) + 1 + channel->cursor] - this->volumeReduction);

	s16 leftVolume = volume;
	s16 rightVolume = volume;

	if(volume && 0 < leftVolumeFactor + rightVolumeFactor)
	{
		leftVolume -= (leftVolume * leftVolumeFactor) / __METERS_TO_PIXELS(_optical->horizontalViewPointCenter);
		//leftVolume -= leftVolume * (relativePosition.z >> _optical->maximumXViewDistancePower);

		rightVolume -= (rightVolume * rightVolumeFactor) / __METERS_TO_PIXELS(_optical->horizontalViewPointCenter);
		//rightVolume -= rightVolume * (relativePosition.z >> _optical->maximumXViewDistancePower);

		/* The maximum sound level for each side is 0xF
		* In the center position the output level is the one
		* defined in the sound's spec */
		if(0 >= leftVolume)
		{
			leftVolume = 0 < volume ? 1 : 0;
		}

		if(0 >= rightVolume)
		{
			rightVolume = 0 < volume ? 1 : 0;
		}
	}

	u8 SxLRV = (((leftVolume << 4) | rightVolume) & channel->soundChannelConfiguration.volume) * this->unmute;

	// Is it a special note?
	switch(note)
	{
		case PAU:

			_soundRegistries[channel->number].SxLRV = 0;
			break;

		case HOLD:
			// Continue playing the previous note, just modify the voluem
#ifdef __SOUND_TEST
			_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV = SxLRV;
#else
#ifdef __SHOW_SOUND_STATUS
			_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV = SxLRV;
#else
			_soundRegistries[channel->number].SxLRV = SxLRV;
#endif
#endif
			break;

		default:				

			note += this->frequencyModifier;

			if(0 > note)
			{
				note = 0;
			}

#ifdef __SOUND_TEST
			_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV = SxLRV;
			_soundRegistries[channel->number].SxFQL = channel->soundChannelConfiguration.SxFQL = (note & 0xFF);
			_soundRegistries[channel->number].SxFQH = channel->soundChannelConfiguration.SxFQH = (note >> 8);
#else
#ifdef __SHOW_SOUND_STATUS
			_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV = SxLRV;
			_soundRegistries[channel->number].SxFQL = channel->soundChannelConfiguration.SxFQL = (note & 0xFF);
			_soundRegistries[channel->number].SxFQH = channel->soundChannelConfiguration.SxFQH = (note >> 8);
#else
			_soundRegistries[channel->number].SxLRV = SxLRV;
			_soundRegistries[channel->number].SxFQH = (note >> 8);
			_soundRegistries[channel->number].SxFQL = (note & 0xFF);
#endif
#endif
			break;

	}

	if(kChannelNoise == channel->soundChannelConfiguration.channelType)
	{
		u8 tapLocation = channel->soundTrack.dataMIDI[(channel->length * 3) + 1 + channel->cursor];

		channel->soundChannelConfiguration.SxEV1 = (tapLocation << 4) | (0x0F & channel->soundChannelConfiguration.SxEV1);

		_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1;
	}
}

void SoundWrapper::updateMIDIPlayback(u32 elapsedMicroseconds)
{
	// Skip if sound is NULL since this should be purged
	if((!this->sound) | (this->paused) | (!this->turnedOn))
	{
		return;
	}

	bool finished = elapsedMicroseconds ? true : false;

	VirtualNode node = this->channels->head;

	this->elapsedMicroseconds += __FIX17_15_TO_I(__FIX17_15_MULT(this->speed, __I_TO_FIX17_15(elapsedMicroseconds)));

	s16 leftVolumeFactor = -1;
	s16 rightVolumeFactor = -1;

	for(; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
/*
		// Since this is commented out, there is no support for sounds
		// with mixed types of tracks
		// TODO: optimize playback of types
		if(kMIDI != channel->soundChannelConfiguration.trackType)
		{
			finished &= channel->finished;
			continue;
		}
*/
		channel->ticks += channel->tickStep;

		if(channel->ticks >= channel->ticksPerNote || 0 == elapsedMicroseconds)
		{
			if(elapsedMicroseconds)
			{
				channel->cursor++;

				finished &= SoundWrapper::checkIfPlaybackFinishedOnChannel(this, channel);
		
				SoundWrapper::computeMIDINextTicksPerNote(channel, channel->ticks - channel->ticksPerNote, this->speed, this->targetTimerResolutionFactor);
			}

			if(NULL != this->position && 0 > leftVolumeFactor + rightVolumeFactor)
			{
				PixelVector relativePosition = PixelVector::getRelativeToCamera(PixelVector::getFromVector3D(*this->position, 0));

				s16 verticalDistance = (__ABS(relativePosition.y - __HALF_SCREEN_HEIGHT) * __SOUND_STEREO_VERTICAL_ATTENUATION_FACTOR) / 100;
				s16 leftDistance = (__ABS(relativePosition.x - __LEFT_EAR_CENTER) * __SOUND_STEREO_HORIZONTAL_ATTENUATION_FACTOR) / 100;
				s16 rightDistance = (__ABS(relativePosition.x - __RIGHT_EAR_CENTER) * __SOUND_STEREO_HORIZONTAL_ATTENUATION_FACTOR) / 100;
				
				leftVolumeFactor = (leftDistance + verticalDistance);
				rightVolumeFactor = (rightDistance + verticalDistance);
			}

			SoundWrapper::playMIDINote(this, channel, leftVolumeFactor, rightVolumeFactor);
		}
		else
		{
			finished &= SoundWrapper::checkIfPlaybackFinishedOnChannel(this, channel);
		}
	}

	if(finished)
	{
		SoundWrapper::completedPlayback(this);
	}
}

void SoundWrapper::updatePCMPlayback(u32 elapsedMicroseconds __attribute__((unused)))
{
	// Skip if sound is NULL since this should be purged
	if((!this->sound) | (this->paused))
	{
		return;
	}

	VirtualNode node = this->channels->tail;

	// Elapsed time during PCM playback is based on the cursor, track's length and target Hz
	//this->elapsedMicroseconds += __I_TO_FIX17_15(1);
	Channel* channel = NULL;

	for(; node; node = node->previous)
	{
		channel = (Channel*)node->data;
/*
		// Since this is commented out, there is no support for sounds
		// with mixed types of tracks
		// TODO: optimize playback of types
		if(kPCM != channel->soundChannelConfiguration.trackType)
		{
			finished &= channel->finished;
			continue;
		}
*/
		channel->cursor++;

		u8 volume = this->unmute * SoundWrapper::clampPCMOutputValue(channel->soundTrack.dataPCM[channel->cursor] - channel->volumeReduction - this->volumeReduction);

#ifdef __SOUND_TEST
		_soundRegistries[channel->number].SxLRV = ((volume << 4) | (volume)) & channel->soundChannelConfiguration.volume;
		// No volume printing because it is too heavy on hardware
//		_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV = ((volume << 4) | (volume)) & channel->soundChannelConfiguration.volume;
#else
#ifndef __RELEASE
		_soundRegistries[channel->number].SxLRV = channel->soundChannelConfiguration.SxLRV = ((volume << 4) | (volume)) & channel->soundChannelConfiguration.volume;
#else
		_soundRegistries[channel->number].SxLRV = ((volume << 4) | (volume)) & channel->soundChannelConfiguration.volume;
#endif
#endif
	}

	// PCM playback must be totally sync on all channels, so, check if completed only
	// in the first one
	if(SoundWrapper::checkIfPlaybackFinishedOnChannel(this, channel))
	{
		SoundWrapper::completedPlayback(this);
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
		switch(channel->soundChannelConfiguration.trackType)
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
		switch(channel->soundChannelConfiguration.trackType)
		{
			case kMIDI:

				PRINT_HEX_EXT(channel->soundTrack.dataMIDI[channel->cursor], x + xDisplacement, y, 2);
				break;

			case kPCM:

				PRINT_HEX_EXT(channel->soundTrack.dataPCM[channel->cursor], x + xDisplacement, y, 2);
				break;
		}
	}
}

u32 SoundWrapper::getTotalPlaybackMilliseconds(Channel* channel)
{
	switch(channel->soundChannelConfiguration.trackType)
	{
		case kMIDI:
			{
				u32 totalNotesTiming = 0;

				u16* soundTrackData = (u16*)channel->soundTrack.dataMIDI;

				for(u32 i = 0; i < channel->length; i++, totalNotesTiming += soundTrackData[channel->length + i]);

				return (u32)((long)totalNotesTiming * this->sound->targetTimerResolutionUS / __MILLISECONDS_PER_SECOND);
			}
			break;

		case kPCM:

			return (channel->length * __MILLISECONDS_PER_SECOND) / this->pcmTargetPlaybackFrameRate;
			break;
	}

	return 0;
}

u32 SoundWrapper::getElapsedMilliseconds()
{
	Channel* firstChannel = (Channel*)this->channels->head->data;

	switch(firstChannel->soundChannelConfiguration.trackType)
	{
		case kMIDI:

			return this->elapsedMicroseconds / __MILLISECONDS_PER_SECOND;
			break;

		case kPCM:

			return (firstChannel->cursor * __MILLISECONDS_PER_SECOND) / this->pcmTargetPlaybackFrameRate;
			break;
	}

	return 0;
}

void SoundWrapper::printPlaybackProgress(int x, int y)
{
	u32 elapsedMilliseconds = SoundWrapper::getElapsedMilliseconds(this);

	static u16 previousPosition = 0;

	u16 position = (elapsedMilliseconds << 5) / this->totalPlaybackMilliseconds;

	if(0 == position)
	{
		previousPosition = 0;

		for(u8 i = 0; i < 32; i++)
		{
			PRINT_TEXT(__CHAR_DARK_RED_BOX, x + i, y);
		}
	}
	else if(previousPosition < position)
	{
		for(u8 i = previousPosition; i < position; i++)
		{
			PRINT_TEXT(__CHAR_BRIGHT_RED_BOX, x + i, y);
		}

		previousPosition = position;
	}
}

void SoundWrapper::printTiming(u32 seconds, int x, int y)
{
	u32 minutes = seconds / 60;
	seconds = seconds - minutes * 60;

	int minutesDigits = 1;//Utilities::getDigitCount(minutes);

	PRINT_INT(minutes, x, y);
	PRINT_TEXT(":", x + minutesDigits, y);

	if(0 == seconds)
	{
		PRINT_TEXT("00", x + minutesDigits + 1, y);
	}
	else if(seconds < 10)
	{
		PRINT_TEXT("0", x + minutesDigits + 1, y);
		PRINT_INT(seconds, x + minutesDigits + 2, y);
	}
	else
	{
		PRINT_INT(seconds, x + minutesDigits + 1, y);
	}
}

void SoundWrapper::printPlaybackTime(int x, int y)
{
	static u32 previousSecond = 0;
	u32 currentSecond = SoundWrapper::getElapsedMilliseconds(this) / __MILLISECONDS_PER_SECOND;

	if(previousSecond > currentSecond)
	{
		previousSecond = currentSecond;
	}

	if(currentSecond > previousSecond)
	{
		previousSecond = currentSecond;

		SoundWrapper::printTiming(this, currentSecond, x, y);
	}
}

void SoundWrapper::printMetadata(int x, int y)
{
	PRINT_TEXT(this->sound->name, x, y++);
	y++;

	SoundWrapper::printPlaybackProgress(this, x, y++);

	u8 trackInfoXOffset = x + 22;
	u8 trackInfoValuesXOffset = 9;
	u16 speed = __FIX17_15_TO_I(__FIX17_15_MULT(this->speed, __I_TO_FIX17_15(100)));

	y++;

	SoundWrapper::printTiming(this, SoundWrapper::getElapsedMilliseconds(this) / __MILLISECONDS_PER_SECOND, x + 23, y);
	PRINT_TEXT("/", x + 27, y);
	SoundWrapper::printTiming(this, this->totalPlaybackMilliseconds / __MILLISECONDS_PER_SECOND, x + 28, y);

	if(!this->hasPCMTracks)
	{
		PRINT_TEXT("Speed", x, y);
		PRINT_TEXT("    ", x + 6, y);
		PRINT_INT(speed, x + 6, y);
		PRINT_TEXT("%", x + 6 + ((speed < 10) ? 1 : (speed < 100) ? 2 : 3), y);
	}

	PRINT_TEXT(!this->paused ? "  " : "\x07\x07", x + 15, y++);
	y+=2;

	PRINT_TEXT("TRACK INFO", trackInfoXOffset, y++);

	PRINT_TEXT("MIDI", trackInfoXOffset, ++y);
	PRINT_TEXT(this->hasMIDITracks ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("PCM", trackInfoXOffset, ++y);
	PRINT_TEXT(this->hasPCMTracks ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Channels", trackInfoXOffset, ++y);
	PRINT_INT(VirtualList::getSize(this->channels), trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Loop", trackInfoXOffset, ++y);
	PRINT_TEXT(this->sound->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y++);

	SoundWrapper::printVolume(this, 1, y, true);
}

void SoundWrapper::printVolume(int x, int y, bool printHeader)
{
	if(this->hasPCMTracks)
	{
		return;
	}

	VirtualNode node = this->channels->head;

	if(printHeader)
	{
		PRINT_TEXT("OUTPUT", x, ++y);

		++y;
		++y;

		int yDisplacement = 0;

		for(node = this->channels->head; node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;

			PRINT_TEXT("C", x + 15 - 0, y + yDisplacement);
			PRINT_INT(channel->number, x + 16 - 0, y + yDisplacement);

			for(int i = 0; i < 15; i++)
			{
				PRINT_TEXT(__CHAR_DARK_RED_BOX, x + 14 - i - 0, y + yDisplacement);
				PRINT_TEXT(__CHAR_DARK_RED_BOX, x + 17 + i - 0, y + yDisplacement);
			}

			yDisplacement++;
		}
	}
	else
	{
		++y;
		++y;
		++y;
	}

	for(node = this->channels->head; node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		u8 volume = channel->soundChannelConfiguration.SxLRV;

		u8 leftVolume = (volume) >> 4;
		u8 rightVolume = (volume & 0x0F);
		u8 i;

		u8 frequency = (channel->soundChannelConfiguration.SxFQH << 4) | channel->soundChannelConfiguration.SxFQL;

		u8 leftValue = 0;
		u8 rightValue = 0;

		switch(channel->soundChannelConfiguration.trackType)
		{
			case kMIDI:

				leftValue = ((frequency * leftVolume) / __MAXIMUM_VOLUME) >> 4;
				rightValue = ((frequency * rightVolume) / __MAXIMUM_VOLUME) >> 4;
				break;

			case kPCM:

				leftValue = leftVolume;
				rightValue = rightVolume;
				break;

			default:

				NM_ASSERT(false, "SoundWrapper::printMetadata: unknown track type");
				break;
		}

		for(i = 0; i < leftValue; i++)
		{
			PRINT_TEXT(__CHAR_BRIGHT_RED_BOX, x + 14 - i - 0, y);
		}

		for(; i < 15; i++)
		{
			PRINT_TEXT(__CHAR_DARK_RED_BOX, x + 14 - i - 0, y);
		}

		for(i = 0; i < rightValue; i++)
		{
			PRINT_TEXT(__CHAR_BRIGHT_RED_BOX, x + 17 + i - 0, y);
		}

		for(; i < 15; i++)
		{
			PRINT_TEXT(__CHAR_DARK_RED_BOX, x + 17 + i - 0, y);
		}

		y++;
	}
}
