/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SoundWrapper.h>

#include <MessageDispatcher.h>
#include <Printing.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <VirtualList.h>
#include <Utilities.h>
#include <VUEngine.h>

#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------


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
void SoundWrapper::constructor(const Sound* sound, VirtualList channels, int8* waves, uint16 pcmTargetPlaybackFrameRate, EventListener soundReleaseListener, ListenerObject scope)
{
	// construct base Container
	Base::constructor();

	this->turnedOn = false;
	this->paused = true;
	this->sound = sound;
	this->hasMIDITracks = false;
	this->hasPCMTracks = false;
	this->speed = __I_TO_FIX7_9_EXT(1);
	this->pcmTargetPlaybackFrameRate = pcmTargetPlaybackFrameRate;
	this->elapsedMicroseconds = 0;
	this->previouslyElapsedMicroseconds = 0;
	this->totalPlaybackMilliseconds = 0;
	this->autoReleaseOnFinish = true;
	this->playbackType = kSoundWrapperPlaybackNormal;
	this->locked = false;

#ifdef __MUTE_ALL_SOUND
	this->unmute = 0x00;
#else
	this->unmute = 0xFF;
#endif
	this->frequencyModifier = 0;
	this->position = NULL;
	this->volumeReduction = 0;

	// Compute target timerCounter factor
	SoundWrapper::computeTimerResolutionFactor(this);

	this->mainChannel = NULL;
	this->channels = channels;
	this->volumeReductionMultiplier = 1;

	SoundWrapper::setupChannels(this, waves);
	SoundWrapper::configureSoundRegistries(this);

	if(NULL != soundReleaseListener && !isDeleted(scope))
	{
		SoundWrapper::addEventListener(this, scope, soundReleaseListener, kEventSoundReleased);
	}
}

/**
 * Class destructor
 */
void SoundWrapper::destructor()
{
	this->sound = NULL;

	MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), ListenerObject::safeCast(this));

	if(!isDeleted(this->channels))
	{
		SoundManager::releaseChannels(SoundManager::getInstance(), this->channels);

		delete this->channels;
		this->channels = NULL;
	}

	if(this->hasPCMTracks)
	{
		CACHE_RESET;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}

bool SoundWrapper::isUsingChannel(Channel* channel)
{
	if(NULL == this->sound)
	{
		return false;
	}

	if(isDeleted(this->channels))
	{
		return false;
	}

	// Prepare channels
	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		if((Channel*)node->data == channel)
		{
			return true;
		}
	}

	return false;
}

void SoundWrapper::computeTimerResolutionFactor()
{
	uint16 timerResolutionUS = TimerManager::getResolutionInUS(TimerManager::getInstance());
	uint16 timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance()) + __TIMER_COUNTER_DELTA;
	uint16 timerUsPerInterrupt = timerCounter * timerResolutionUS;
	uint16 soundTargetUsPerInterrupt = (__TIME_US(this->sound->targetTimerResolutionUS) + __TIMER_COUNTER_DELTA) * __SOUND_TARGET_US_PER_TICK * (timerResolutionUS / 20);

	this->targetTimerResolutionFactor = __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(soundTargetUsPerInterrupt), __I_TO_FIX7_9_EXT(timerUsPerInterrupt));

	// Compensate for the difference in speed between 20US and 100US timer resolution
	fix7_9_ext timerResolutionRatioReduction = __I_TO_FIX7_9_EXT(1) - __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(timerResolutionUS), __I_TO_FIX7_9_EXT(20));

	if(0 != timerResolutionRatioReduction)
	{
		this->targetTimerResolutionFactor = __FIX7_9_EXT_MULT(this->targetTimerResolutionFactor, timerResolutionRatioReduction);
	}
}

void SoundWrapper::setFrequencyModifier(uint16 frequencyModifier)
{
	this->frequencyModifier = frequencyModifier;
}

uint16 SoundWrapper::getFrequencyModifier()
{
	return this->frequencyModifier;
}

/**
 * Set playback speed. Changing the speed during playback may cause
 * the tracks to go out of sync because of the channel's current ticks.
 *
 * @speed 	fix7_9_ext PCM playback max speed is 100%
 */
void SoundWrapper::setSpeed(fix7_9_ext speed)
{
	// Prevent timer interrupts to unsync tracks
	if(!this->hasPCMTracks)
	{
		this->speed = 0 >= speed ? __F_TO_FIX7_9_EXT(0.01f) : speed < __I_TO_FIX7_9_EXT(2) ? speed : __I_TO_FIX7_9_EXT(2);
	}
}

/**
 * Return playback speed.
 */
fix7_9_ext SoundWrapper::getSpeed()
{
	return this->speed;
}

/**
 * Set volume reduction
 */
void SoundWrapper::setVolumeReduction(int8 volumeReduction)
{
	this->volumeReduction = volumeReduction;
}

/**
 * Get volume reduction
 */
int8 SoundWrapper::getVolumeReduction()
{
	return this->volumeReduction;
}

/**
 * Get channel
 *
 *
 * @param index uint8
 * @return Channel*
 */
const Channel* SoundWrapper::getChannel(uint8 index)
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
	return SoundWrapper::isTurnedOn(this) && this->paused;
}

/**
 * Is turned on?
 *
 * @return bool
 */
bool SoundWrapper::isTurnedOn()
{
	return this->turnedOn;
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
 *  Is fading in?
 *
 * @return bool
 */
bool SoundWrapper::isFadingIn()
{
	return kSoundWrapperPlaybackFadeIn == this->playbackType;
}

/**
 *  Is fading out?
 *
 * @return bool
 */
bool SoundWrapper::isFadingOut()
{
	return kSoundWrapperPlaybackFadeOut == this->playbackType;
}

/**
 * Play
 *
 */
void SoundWrapper::play(const Vector3D* position, uint32 playbackType)
{
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	this->playbackType = playbackType;

	switch(playbackType)
	{
		case kSoundWrapperPlaybackFadeIn:

			if(this->paused || !this->turnedOn)
			{
				SoundWrapper::setVolumeReduction(this, __MAXIMUM_VOLUME * this->volumeReductionMultiplier);
			}
			else if(!this->paused)
			{
				return;
			}

			break;

			// intentional fall through
		case kSoundWrapperPlaybackNormal:
			
			SoundWrapper::setVolumeReduction(this, 0);
			break;
	}

	switch(playbackType)
	{
		case kSoundWrapperPlaybackFadeIn:
		case kSoundWrapperPlaybackNormal:
			{
				bool wasPaused = this->paused && this->turnedOn;

				this->paused = false;
				this->turnedOn = true;

				this->position = position;

				// Prepare channels
				for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
				{
					Channel* channel = (Channel*)node->data;

					channel->finished = false;

					if(!wasPaused)
					{
						channel->ticks = 0;
						channel->cursor = 0;
					}

					_soundRegistries[channel->number].SxFQH = 0;
					_soundRegistries[channel->number].SxFQL = 0;
					_soundRegistries[channel->number].SxLRV = 0;
					_soundRegistries[channel->number].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;
				}

				if(!wasPaused)
				{
					this->elapsedMicroseconds = 0;
					this->previouslyElapsedMicroseconds = 0;
					
					if(this->hasPCMTracks)
					{
						CACHE_DISABLE;
						CACHE_CLEAR;

						SoundManager::startPCMPlayback(SoundManager::getInstance());
					}
				}	
			}

			break;
	}
}

/**
 * Pause
 *
 */
void SoundWrapper::pause()
{
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	if(!this->paused && this->turnedOn)
	{
		this->paused = true;

		// Silence all channels first
		for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;
			_soundRegistries[channel->number].SxINT = __SOUND_WRAPPER_STOP_SOUND;
		}
	}
}

/**
 * Unpause
 *
 */
void SoundWrapper::unpause()
{
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	if(this->paused && this->turnedOn)
	{
		// Silence all channels first
		for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
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
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	this->turnedOn = false;

	// Silence all channels first
	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		_soundRegistries[channel->number].SxINT = __SOUND_WRAPPER_STOP_SOUND;
	}
}

/**
 * Turn on
 *
 */
void SoundWrapper::turnOn()
{
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	// Silence all channels first
	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
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
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	this->elapsedMicroseconds = 0;
	this->previouslyElapsedMicroseconds = 0;
	this->volumeReduction = 0;
	this->playbackType = kSoundWrapperPlaybackNormal;

	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		channel->finished = false;
		channel->cursor = 0;

		switch(channel->soundChannelConfiguration.trackType)
		{
			case kMIDI:

				SoundWrapper::computeMIDIDummyTicksPerNote(channel);
				break;

			case kPCM:
				break;

#ifndef __RELEASE
			case kUnknownType:

				NM_ASSERT(false, "SoundWrapper::rewind: unknown track type");
				break;
#endif
			default:

				NM_ASSERT(false, "SoundWrapper::rewind: invalid channel");
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
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	this->turnedOn = false;
	this->paused = true;

	// Silence all channels first
	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		channel->cursor = 0;

		// If turned of right away, pops and cracks are perceptible
		_soundRegistries[channel->number].SxINT = __SOUND_WRAPPER_STOP_SOUND;
	}
}

/**
 * Release
 *
 */
void SoundWrapper::release()
{
	if(NULL == this->sound)
	{
		return;
	}

	SoundWrapper::stop(this);

	this->sound = NULL;

	if(!isDeleted(this->channels))
	{
		SoundManager::releaseChannels(SoundManager::getInstance(), this->channels);

		delete this->channels;
		this->channels = NULL;
	}

	if(!isDeleted(this->events))
	{
		SoundWrapper::fireEvent(this, kEventSoundReleased);
		NM_ASSERT(!isDeleted(this), "SoundWrapper::release: deleted this during kEventSoundReleased");
	}

}

/**
 * Release
 *
 */
void SoundWrapper::autoReleaseOnFinish(bool value)
{
	this->autoReleaseOnFinish = value;
}

void SoundWrapper::mute()
{
	this->unmute = 0x00;
}

void SoundWrapper::unmute()
{
	this->unmute = 0xFF;
}

void SoundWrapper::lock()
{
	this->locked = true;
}

void SoundWrapper::unlock()
{
	this->locked = false;
}

void SoundWrapper::setupChannels(int8* waves)
{
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	VirtualNode node = this->channels->head;
	this->mainChannel = (Channel*)node->data;

	uint16 i = 0;

	for(; NULL != node; node = node->next, i++)
	{
		Channel* channel = (Channel*)node->data;

		channel->sound = this->sound;
		channel->finished = false;
		channel->cursor = 0;
		channel->soundChannel = i;
		channel->soundChannelConfiguration = *channel->sound->soundChannels[i]->soundChannelConfiguration;
		channel->soundChannelConfiguration.SxRAM = waves[i];

		switch(channel->soundChannelConfiguration.trackType)
		{
			case kMIDI:

				this->hasMIDITracks = true;
				channel->soundTrack.dataMIDI = (uint16*)this->sound->soundChannels[channel->soundChannel]->soundTrack.dataMIDI;
				channel->length = SoundWrapper::computeMIDITrackLength((uint16*)channel->soundTrack.dataMIDI);
				SoundWrapper::computeMIDIDummyTicksPerNote(channel);
				break;

			case kPCM:

				this->hasPCMTracks = true;
				channel->soundTrack.dataPCM = (uint8*)this->sound->soundChannels[channel->soundChannel]->soundTrack.dataPCM;
				channel->length = this->sound->soundChannels[channel->soundChannel]->length;
				SoundWrapper::computePCMNextTicksPerNote(channel, 0, this->speed, this->targetTimerResolutionFactor);
				break;

#ifndef __RELEASE
			case kUnknownType:

				NM_ASSERT(false, "SoundWrapper::setupChannels: unknown track type");
				break;
#endif
			default:

				NM_ASSERT(false, "SoundWrapper::setupChannels: invalid track type");
				break;
		}

		channel->ticks = 0;
	}

	node = this->channels->head;

	Channel* channelWithLongestTrack = (Channel*)node->data;

	// Find the the channel with the longest track
	for(node = node->next; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		if(channelWithLongestTrack->length < channel->length)
		{
			channelWithLongestTrack = channel;
		}
	}

#ifdef __SOUND_TEST
	this->totalPlaybackMilliseconds = SoundWrapper::getTotalPlaybackMilliseconds(this, channelWithLongestTrack);
#endif

	this->volumeReductionMultiplier = this->hasPCMTracks ? VirtualList::getSize(this->channels) : 1;
}

void SoundWrapper::configureSoundRegistries()
{
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		_soundRegistries[channel->number].SxINT = 0x00;
		_soundRegistries[channel->number].SxLRV = 0x00;
		_soundRegistries[channel->number].SxEV0 = channel->soundChannelConfiguration.SxEV0;
		_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1;
		_soundRegistries[channel->number].SxFQH = channel->soundChannelConfiguration.SxFQH;
		_soundRegistries[channel->number].SxFQL = channel->soundChannelConfiguration.SxFQL;
		_soundRegistries[channel->number].SxRAM = channel->soundChannelConfiguration.SxRAM;

		if(kChannelModulation == channel->type)
		{
			_soundRegistries[channel->number].S5SWP = 0;
		}
	}
}

static uint16 SoundWrapper::computeMIDITrackLength(uint16* soundTrackData)
{
	uint16 i = 0;

	NM_ASSERT(soundTrackData, "SoundWrapper::computeMIDITrackLength: null soundTrack");

	for(; soundTrackData[i] != ENDSOUND && soundTrackData[i] != LOOPSOUND; i++);

	return i;
}

static void SoundWrapper::computeMIDIDummyTicksPerNote(Channel* channel)
{
	channel->ticks = 0;
	channel->ticksPerNote = 0;
	channel->tickStep = __I_TO_FIX7_9_EXT(1);
}

static void SoundWrapper::computeMIDINextTicksPerNote(Channel* channel, fix7_9_ext residue, fix7_9_ext speed, fix7_9_ext targetTimerResolutionFactor)
{
	channel->ticks = residue;
	channel->ticksPerNote = __I_TO_FIX7_9_EXT(channel->soundTrack.dataMIDI[channel->length + 1 + channel->cursor]);
	channel->ticksPerNote = __FIX7_9_EXT_DIV(channel->ticksPerNote, speed);

	if(0 < channel->ticksPerNote)
	{
		channel->tickStep = __FIX7_9_EXT_DIV(channel->ticksPerNote, __FIX7_9_EXT_MULT(targetTimerResolutionFactor, channel->ticksPerNote));

		if(0 > channel->tickStep)
		{
			channel->tickStep = __I_TO_FIX7_9_EXT(1);
		}
	}
	else
	{
		channel->tickStep = __I_TO_FIX7_9_EXT(1);
	}
}

static void SoundWrapper::computePCMNextTicksPerNote(Channel* channel, fix7_9_ext residue __attribute__((unused)), fix7_9_ext speed __attribute__((unused)), fix7_9_ext targetTimerResolutionFactor __attribute__((unused)))
{
	channel->ticksPerNote = 0;
	channel->tickStep = __I_TO_FIX7_9_EXT(1);
	channel->ticks = 0;
}

static inline uint8 SoundWrapper::clampMIDIOutputValue(int8 value)
{
	if(value < 0)
	{
		return 0;
	}
	else if(value > __MAXIMUM_VOLUME)
	{
		return __MAXIMUM_VOLUME;
	}

	return (uint8)value;
}

inline bool SoundWrapper::checkIfPlaybackFinishedOnChannel(Channel* channel)
{
	if(channel->cursor >= channel->length)
	{
		channel->finished = true;
		return true;
	}

	return false;
}

void SoundWrapper::completedPlayback()
{
	if(!isDeleted(this->events))
	{
		SoundWrapper::fireEvent(this, kEventSoundFinished);
		NM_ASSERT(!isDeleted(this), "SoundWrapper::completedPlayback: deleted this during kEventSoundFinished");
	}

	if(!this->sound->loop)
	{
		if(this->autoReleaseOnFinish)
		{
			SoundWrapper::release(this);
		}
		else
		{
			SoundWrapper::stop(this);
		}
	}
	else
	{
		SoundWrapper::rewind(this);
	}
}

void SoundWrapper::playMIDINote(Channel* channel, fixed_t leftVolumeFactor, fixed_t rightVolumeFactor)
{
	int16 note = channel->soundTrack.dataMIDI[channel->cursor];
	uint8 volume = (SoundWrapper::clampMIDIOutputValue(channel->soundTrack.dataMIDI[(channel->length << 1) + 1 + channel->cursor] - this->volumeReduction)) & this->unmute;

	int16 leftVolume = volume;
	int16 rightVolume = volume;

	if(0 != volume && 0 <= leftVolumeFactor + rightVolumeFactor)
	{
		fixed_t volumeHelper = __I_TO_FIXED(volume);

		leftVolume = __FIXED_TO_I(__FIXED_MULT(volumeHelper, leftVolumeFactor));
		rightVolume = __FIXED_TO_I(__FIXED_MULT(volumeHelper, rightVolumeFactor));
	}

	uint8 SxLRV = ((leftVolume << 4) | rightVolume) & channel->soundChannelConfiguration.volume;

	// Is it a special note?
	switch(note)
	{
		case PAU:

			_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1 | 0x1;
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
			channel->soundChannelConfiguration.SxLRV = SxLRV;
			channel->soundChannelConfiguration.SxFQL = (note & 0xFF);
			channel->soundChannelConfiguration.SxFQH = (note >> 8);
#else
#ifdef __SHOW_SOUND_STATUS
			channel->soundChannelConfiguration.SxLRV = SxLRV;
			channel->soundChannelConfiguration.SxFQL = (note & 0xFF);
			channel->soundChannelConfiguration.SxFQH = (note >> 8);
#endif
#endif

			_soundRegistries[channel->number].SxLRV = SxLRV;
			_soundRegistries[channel->number].SxFQH = (note >> 8);
			_soundRegistries[channel->number].SxFQL = (note & 0xFF);
			_soundRegistries[channel->number].SxEV0 = channel->soundChannelConfiguration.SxEV0;
			_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1;

			if(kChannelNoise == channel->soundChannelConfiguration.channelType)
			{
				uint8 tapLocation = channel->soundTrack.dataMIDI[(channel->length * 3) + 1 + channel->cursor];
				_soundRegistries[channel->number].SxEV1 = (tapLocation << 4) | (0x0F & channel->soundChannelConfiguration.SxEV1);
			}
			else
			{
				_soundRegistries[channel->number].SxEV1 = channel->soundChannelConfiguration.SxEV1;
			}
			
			break;

	}
}

__attribute__((noinline)) 
void SoundWrapper::updateVolumeReduction()
{
	uint32 elapsedMilliseconds = (this->elapsedMicroseconds - this->previouslyElapsedMicroseconds) / __MICROSECONDS_PER_MILLISECOND;

	if(VUEngine::getGameFrameDuration(_vuEngine) <= elapsedMilliseconds)
	{
		switch(this->playbackType)
		{
			case kSoundWrapperPlaybackFadeIn:

				this->volumeReduction -= (this->volumeReductionMultiplier >> 1) + 1;

				if(0 >= this->volumeReduction)
				{
					this->volumeReduction = 0;
					this->playbackType = kSoundWrapperPlaybackNormal;
				}

				break;

			case kSoundWrapperPlaybackFadeOut:

				this->volumeReduction += (this->volumeReductionMultiplier >> 1) + 1;

				if(__MAXIMUM_VOLUME * this->volumeReductionMultiplier <= this->volumeReduction)
				{
					this->volumeReduction = __MAXIMUM_VOLUME * this->volumeReductionMultiplier;
					this->playbackType = kSoundWrapperPlaybackNormal;
					SoundWrapper::pause(this);
				}

				break;

			case kSoundWrapperPlaybackFadeOutAndRelease:

				this->volumeReduction += (this->volumeReductionMultiplier >> 1) + 1;

				if(__MAXIMUM_VOLUME * this->volumeReductionMultiplier <= this->volumeReduction)
				{
					this->volumeReduction = __MAXIMUM_VOLUME * this->volumeReductionMultiplier;
					this->playbackType = kSoundWrapperPlaybackNormal;
					SoundWrapper::release(this);
				}

				break;
		}

		this->previouslyElapsedMicroseconds = this->elapsedMicroseconds;
	}
}

void SoundWrapper::updateMIDIPlayback(uint32 elapsedMicroseconds)
{
	// Optimization, if no sound or paused, the sum will be different than 0
	if((NULL == this->sound) + this->paused + (!this->turnedOn))
	{
		return;
	}

	NM_ASSERT(NULL != this->channels, "SoundWrapper::updateMIDIPlayback: invalid channels list");

	bool finished = true;

#ifdef __SOUND_TEST
	this->elapsedMicroseconds += __FIX7_9_EXT_TO_F(this->speed) * elapsedMicroseconds;
#endif

	fixed_t leftVolumeFactor = -1;
	fixed_t rightVolumeFactor = -1;

	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
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
		if(channel->finished)
		{
			continue;
		}

		channel->ticks += channel->tickStep;

		if(channel->ticks >= channel->ticksPerNote || 0 == elapsedMicroseconds)
		{
			if(elapsedMicroseconds)
			{
				finished &= SoundWrapper::checkIfPlaybackFinishedOnChannel(this, channel);

				SoundWrapper::computeMIDINextTicksPerNote(channel, channel->ticks - channel->ticksPerNote, this->speed, this->targetTimerResolutionFactor);
			}

			if(NULL != this->position && 0 > leftVolumeFactor + rightVolumeFactor)
			{
#ifndef __LEGACY_COORDINATE_PROJECTION
				Vector3D relativePosition = Vector3D::rotate(Vector3D::getRelativeToCamera(*this->position), *_cameraInvertedRotation);
#else
				Vector3D relativePosition = Vector3D::rotate(Vector3D::sub(Vector3D::getRelativeToCamera(*this->position), (Vector3D){__HALF_SCREEN_WIDTH_METERS, __HALF_SCREEN_HEIGHT_METERS, 0}), *_cameraInvertedRotation);
#endif
				Vector3D leftEar = (Vector3D){__PIXELS_TO_METERS(-__EAR_DISPLACEMENT), 0, 0};
				Vector3D rightEar = (Vector3D){__PIXELS_TO_METERS(__EAR_DISPLACEMENT), 0, 0};

				fixed_ext_t squaredDistanceToLeftEar = Vector3D::squareLength(Vector3D::get(leftEar, relativePosition));
				fixed_ext_t squaredDistanceToRightEar = Vector3D::squareLength(Vector3D::get(rightEar, relativePosition));

				leftVolumeFactor  = __1I_FIXED - __FIXED_EXT_DIV(squaredDistanceToLeftEar, __FIXED_SQUARE(__PIXELS_TO_METERS(__SOUND_STEREO_ATTENUATION_DISTANCE)));
				rightVolumeFactor = __1I_FIXED - __FIXED_EXT_DIV(squaredDistanceToRightEar, __FIXED_SQUARE(__PIXELS_TO_METERS(__SOUND_STEREO_ATTENUATION_DISTANCE)));

				if(leftVolumeFactor > rightVolumeFactor)
				{
					rightVolumeFactor -= (leftVolumeFactor - rightVolumeFactor);
				}
				else
				{
					leftVolumeFactor -= (rightVolumeFactor - leftVolumeFactor);
				}

				leftVolumeFactor = 0 > leftVolumeFactor ? 0 : leftVolumeFactor;
				rightVolumeFactor = 0 > rightVolumeFactor ? 0 : rightVolumeFactor;
			}

			SoundWrapper::playMIDINote(this, channel, leftVolumeFactor, rightVolumeFactor);

			channel->cursor++;
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

	if(kSoundWrapperPlaybackNormal != this->playbackType)
	{
		SoundWrapper::updateVolumeReduction(this);
	}
}

void SoundWrapper::updatePCMPlayback(uint32 elapsedMicroseconds, uint32 targetPCMUpdates)
{
	CACHE_ENABLE;

	// Optimization, if no sound or paused, the sum will be different than 0
	if((NULL == this->sound) + this->paused + (!this->turnedOn))
	{
		return;
	}

	// Elapsed time during PCM playback is based on the cursor, track's length and target Hz
	this->elapsedMicroseconds += elapsedMicroseconds;

	this->mainChannel->cursor = this->elapsedMicroseconds / targetPCMUpdates;

	if(this->mainChannel->cursor >= this->mainChannel->length)
	{
		SoundWrapper::completedPlayback(this);
	}
	else
	{
		// PCM playback must be totally in sync on all channels, so, check if completed only
		// in the first one
		int8 volume = (this->mainChannel->soundTrack.dataPCM[this->mainChannel->cursor] - this->volumeReduction) & this->unmute;

		for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;

			if(0 >= volume)
			{
				_soundRegistries[channel->number].SxLRV = 0;	
			}
			else if(__MAXIMUM_VOLUME <= volume)
			{
				_soundRegistries[channel->number].SxLRV = 0xFF;
				volume -= __MAXIMUM_VOLUME;
			}
			else
			{
				_soundRegistries[channel->number].SxLRV = ((volume << 4) | volume);
				volume = 0;
			}
		}
	
		if(kSoundWrapperPlaybackNormal != this->playbackType)
		{
			SoundWrapper::updateVolumeReduction(this);
		}
	}

	CACHE_DISABLE;
}

void SoundWrapper::print(int32 x, int32 y)
{
	if(NULL == this->sound)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	int32 xDisplacement = 9;

	// Prepare channels
	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
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

uint32 SoundWrapper::getTotalPlaybackMilliseconds(Channel* channel)
{
	switch(channel->soundChannelConfiguration.trackType)
	{
		case kMIDI:
			{
				uint32 totalSoundTicks = 0;

				for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
				{
					uint32 totalChannelTicks = 0;
					Channel* channel = (Channel*)node->data;

					uint16* soundTrackData = (uint16*)channel->soundTrack.dataMIDI;

					for(uint32 i = 0; i < channel->length; i++, totalChannelTicks += soundTrackData[channel->length + i]);


					if(totalSoundTicks < totalChannelTicks)
					{
						totalSoundTicks = totalChannelTicks;
					}
				}

				return (uint32)((long)totalSoundTicks * this->sound->targetTimerResolutionUS / __MICROSECONDS_PER_MILLISECOND);
			}
			break;

		case kPCM:

			return (channel->length * __MICROSECONDS_PER_MILLISECOND) / this->pcmTargetPlaybackFrameRate;
			break;
	}

	return 0;
}

uint32 SoundWrapper::getElapsedMilliseconds()
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

void SoundWrapper::printPlaybackProgress(int32 x, int32 y)
{
	uint32 elapsedMilliseconds = SoundWrapper::getElapsedMilliseconds(this);

	if(elapsedMilliseconds > this->totalPlaybackMilliseconds)
	{
		elapsedMilliseconds = this->totalPlaybackMilliseconds;
	}

	static uint16 previousPosition = 0;

	uint16 position = (elapsedMilliseconds * 32) / this->totalPlaybackMilliseconds;

	if(32 < previousPosition)
	{
		previousPosition = 32;
	}

	if(0 == position)
	{
		previousPosition = 0;

		for(uint8 i = 0; i < 32; i++)
		{
			PRINT_TEXT(__CHAR_DARK_RED_BOX, x + i, y);
		}
	}
	else if(previousPosition < position)
	{
		for(uint16 i = previousPosition; i < position; i++)
		{
			PRINT_TEXT(__CHAR_BRIGHT_RED_BOX, x + i, y);
		}

		previousPosition = position;
	}
}

void SoundWrapper::printTiming(uint32 seconds, int32 x, int32 y)
{
	uint32 minutes = seconds / 60;
	seconds = seconds - minutes * 60;

	int32 minutesDigits = Utilities::getDigitsCount(minutes);

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

void SoundWrapper::printPlaybackTime(int32 x, int32 y)
{
	static uint32 previousSecond = 0;

	uint32 elapsedMilliseconds = SoundWrapper::getElapsedMilliseconds(this);

	if(elapsedMilliseconds > this->totalPlaybackMilliseconds)
	{
		elapsedMilliseconds = this->totalPlaybackMilliseconds;
	}

	uint32 currentSecond = elapsedMilliseconds/ __MILLISECONDS_PER_SECOND;

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

void SoundWrapper::printMetadata(int32 x, int32 y, bool printDetails)
{
	PRINT_TEXT("                                  ", x, y);
	PRINT_TEXT(this->sound->name, x, y++);
	y++;

	SoundWrapper::printPlaybackProgress(this, x, y++);

	uint8 trackInfoXOffset = x + 22;
	uint8 trackInfoValuesXOffset = 9;
	uint16 speed = __FIX7_9_EXT_TO_I(__FIX7_9_EXT_MULT(this->speed, __I_TO_FIX7_9_EXT(100)));

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

	if(!printDetails)
	{
		return;
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

void SoundWrapper::printVolume(int32 x, int32 y, bool printHeader)
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

		int32 yDisplacement = 0;

		for(node = this->channels->head; NULL != node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;

			PRINT_TEXT("C", x + 15 - 0, y + yDisplacement);
			PRINT_INT(channel->number, x + 16 - 0, y + yDisplacement);

			for(int32 i = 0; i < 15; i++)
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

	uint16 totalVolume = 0;

	for(node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		uint16 volume = channel->soundChannelConfiguration.SxLRV;

		totalVolume += volume;

		uint16 leftVolume = (volume) >> 4;
		uint16 rightVolume = (volume & 0x0F);
		uint16 i;

		uint16 frequency = (channel->soundChannelConfiguration.SxFQH << 4) | channel->soundChannelConfiguration.SxFQL;

		uint16 leftValue = 0;
		uint16 rightValue = 0;

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

#ifndef __RELEASE
			case kUnknownType:

				NM_ASSERT(false, "SoundWrapper::printVolume: unknown track type");
				break;
#endif
			default:

				NM_ASSERT(false, "SoundWrapper::printVolume: invalid channel");
				break;
		}

// Must redefine these because they are defined as strings
#undef __CHAR_DARK_RED_BOX
#define __CHAR_DARK_RED_BOX			'\x0E'
#undef __CHAR_BRIGHT_RED_BOX
#define __CHAR_BRIGHT_RED_BOX		'\x10'

		char boxesArray[] = 
		{
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, 
			'C', '0' + channel->number,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, '\0'
		};

		for(i = 0; i < leftValue && 15 >= i; i++)
		{
			boxesArray[15 - i - 2] = __CHAR_BRIGHT_RED_BOX;
		}

		for(i = 0; i < rightValue && 15 > i; i++)
		{
			boxesArray[15 + i + 2] = __CHAR_BRIGHT_RED_BOX;
		}

		PRINT_TEXT(boxesArray, x, y);

		y++;
	}
}
