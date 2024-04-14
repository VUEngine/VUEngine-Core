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

#include <MessageDispatcher.h>
#include <Printing.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <VirtualList.h>
#include <Utilities.h>
#include <VUEngine.h>

#include "Sound.h"


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------


// Must redefine these because they are defined as strings
#undef __CHAR_DARK_RED_BOX
#define __CHAR_DARK_RED_BOX			'\x0E'
#undef __CHAR_BRIGHT_RED_BOX
#define __CHAR_BRIGHT_RED_BOX		'\x10'



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
void Sound::constructor(const SoundSpec* soundSpec, VirtualList channels, int8* waves, uint16 pcmTargetPlaybackFrameRate, EventListener soundReleaseListener, ListenerObject scope)
{
	// construct base Container
	Base::constructor();

	this->turnedOn = false;
	this->paused = true;
	this->soundSpec = soundSpec;
	this->hasMIDITracks = false;
	this->hasPCMTracks = false;
	this->speed = __I_TO_FIX7_9_EXT(1);
	this->pcmTargetPlaybackFrameRate = pcmTargetPlaybackFrameRate;
	this->previouslyElapsedTicks = 0;
	this->totalPlaybackMilliseconds = 0;
	this->autoReleaseOnFinish = true;
	this->playbackType = kSoundPlaybackNormal;
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
	Sound::computeTimerResolutionFactor(this);

	this->mainChannel = NULL;
	this->channels = channels;
	this->volumeReductionMultiplier = 1;

	Sound::setupChannels(this, waves);
	Sound::configureSoundRegistries(this);

	if(NULL != soundReleaseListener && !isDeleted(scope))
	{
		Sound::addEventListener(this, scope, soundReleaseListener, kEventSoundReleased);
	}
}

/**
 * Class destructor
 */
void Sound::destructor()
{
	this->soundSpec = NULL;

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

bool Sound::isUsingChannel(Channel* channel)
{
	if(NULL == this->soundSpec)
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

void Sound::computeTimerResolutionFactor()
{
	uint16 timerResolutionUS = TimerManager::getResolutionInUS(TimerManager::getInstance());
	uint16 timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance()) + __TIMER_COUNTER_DELTA;
	uint16 timerUsPerInterrupt = timerCounter * __SOUND_TARGET_US_PER_TICK;
	uint16 targetTimerResolutionUS = 0 != this->soundSpec->targetTimerResolutionUS ? this->soundSpec->targetTimerResolutionUS : 1000;
	uint16 soundTargetUsPerInterrupt = (__TIME_US(targetTimerResolutionUS) + __TIMER_COUNTER_DELTA) * __SOUND_TARGET_US_PER_TICK;

	NM_ASSERT(0 < timerResolutionUS, "Sound::computeTimerResolutionFactor: zero timerResolutionUS");
	NM_ASSERT(0 < soundTargetUsPerInterrupt, "Sound::computeTimerResolutionFactor: zero soundTargetUsPerInterrupt");

	this->targetTimerResolutionFactor = __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(timerUsPerInterrupt), __I_TO_FIX7_9_EXT(soundTargetUsPerInterrupt));

	// Compensate for the difference in speed between 20US and 100US timer resolution
	fix7_9_ext timerResolutionRatioReduction = __I_TO_FIX7_9_EXT(1) - __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(__SOUND_TARGET_US_PER_TICK), __I_TO_FIX7_9_EXT(timerResolutionUS));

	if(0 != timerResolutionRatioReduction)
	{
		timerResolutionRatioReduction = __I_TO_FIX7_9_EXT(1) - __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(__SOUND_TARGET_US_PER_TICK), __I_TO_FIX7_9_EXT(timerResolutionUS - 0*(timerResolutionUS >> 3)));

		this->targetTimerResolutionFactor = __FIX7_9_EXT_MULT(this->targetTimerResolutionFactor, timerResolutionRatioReduction);
	}
}

void Sound::setFrequencyModifier(uint16 frequencyModifier)
{
	this->frequencyModifier = frequencyModifier;
}

uint16 Sound::getFrequencyModifier()
{
	return this->frequencyModifier;
}

/**
 * Set playback speed. Changing the speed during playback may cause
 * the tracks to go out of sync because of the channel's current ticks.
 *
 * @speed 	fix7_9_ext PCM playback max speed is 100%
 */
void Sound::setSpeed(fix7_9_ext speed)
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
fix7_9_ext Sound::getSpeed()
{
	return this->speed;
}

/**
 * Set volume reduction
 */
void Sound::setVolumeReduction(int8 volumeReduction)
{
	this->volumeReduction = volumeReduction;
}

/**
 * Get volume reduction
 */
int8 Sound::getVolumeReduction()
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
const Channel* Sound::getChannel(uint8 index)
{
	return VirtualList::getObjectAtPosition(this->channels, index);
}

/**
 * Is playing?
 *
 * @return bool
 */
bool Sound::isPlaying()
{
	return Sound::isTurnedOn(this) && !this->paused;
}

/**
 * Is paused?
 *
 * @return bool
 */
bool Sound::isPaused()
{
	return Sound::isTurnedOn(this) && this->paused;
}

/**
 * Is turned on?
 *
 * @return bool
 */
bool Sound::isTurnedOn()
{
	return this->turnedOn;
}

/**
 *  Has PCM tracks?
 *
 * @return bool
 */
bool Sound::hasPCMTracks()
{
	return this->hasPCMTracks;
}

/**
 *  Is fading in?
 *
 * @return bool
 */
bool Sound::isFadingIn()
{
	return kSoundPlaybackFadeIn == this->playbackType;
}

/**
 *  Is fading out?
 *
 * @return bool
 */
bool Sound::isFadingOut()
{
	return kSoundPlaybackFadeOut == this->playbackType;
}

/**
 * Play
 *
 */
void Sound::play(const Vector3D* position, uint32 playbackType)
{
	if(NULL == this->soundSpec)
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
		case kSoundPlaybackFadeIn:

			if(this->paused || !this->turnedOn)
			{
				Sound::setVolumeReduction(this, __MAXIMUM_VOLUME * this->volumeReductionMultiplier);
			}
			else if(!this->paused)
			{
				return;
			}

			break;

			// intentional fall through
		case kSoundPlaybackNormal:
			
			Sound::setVolumeReduction(this, 0);
			break;
	}

	switch(playbackType)
	{
		case kSoundPlaybackFadeIn:
		case kSoundPlaybackNormal:
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
						channel->elapsedTicks = 0;
						channel->cursor = 0;
					}

					_soundRegistries[channel->number].SxFQH = 0;
					_soundRegistries[channel->number].SxFQL = 0;
					_soundRegistries[channel->number].SxLRV = 0;
					_soundRegistries[channel->number].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;
				}

				if(!wasPaused)
				{
					this->previouslyElapsedTicks = 0;
					
					if(this->hasPCMTracks)
					{
						CACHE_DISABLE;
						CACHE_CLEAR;
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
void Sound::pause()
{
	if(NULL == this->soundSpec)
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
void Sound::unpause()
{
	if(NULL == this->soundSpec)
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
void Sound::turnOff()
{
	if(NULL == this->soundSpec)
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
void Sound::turnOn()
{
	if(NULL == this->soundSpec)
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
void Sound::rewind()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	this->previouslyElapsedTicks = 0;
	this->volumeReduction = 0;
	this->playbackType = kSoundPlaybackNormal;

	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		channel->finished = false;
		channel->cursor = 0;
		channel->elapsedTicks = 0;
		channel->nextElapsedTicksTarget = 0;
		channel->tickStep = __FIX7_9_EXT_MULT(this->speed, this->targetTimerResolutionFactor);
	}
}

/**
 * Stop
 *
 */
void Sound::stop()
{
	if(NULL == this->soundSpec)
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
		channel->elapsedTicks = 0;
		channel->nextElapsedTicksTarget = 0;

		// If turned of right away, pops and cracks are perceptible
		_soundRegistries[channel->number].SxINT = __SOUND_WRAPPER_STOP_SOUND;
	}
}

/**
 * Release
 *
 */
void Sound::release()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	Sound::stop(this);

	this->soundSpec = NULL;

	if(!isDeleted(this->channels))
	{
		SoundManager::releaseChannels(SoundManager::getInstance(), this->channels);

		delete this->channels;
		this->channels = NULL;
	}

	if(!isDeleted(this->events))
	{
		Sound::fireEvent(this, kEventSoundReleased);
		NM_ASSERT(!isDeleted(this), "Sound::release: deleted this during kEventSoundReleased");
	}

}

/**
 * Release
 *
 */
void Sound::autoReleaseOnFinish(bool value)
{
	this->autoReleaseOnFinish = value;
}

void Sound::mute()
{
	this->unmute = 0x00;
}

void Sound::unmute()
{
	this->unmute = 0xFF;
}

void Sound::lock()
{
	this->locked = true;
}

void Sound::unlock()
{
	this->locked = false;
}

void Sound::setupChannels(int8* waves)
{
	if(NULL == this->soundSpec)
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

		channel->soundSpec = this->soundSpec;
		channel->finished = false;
		channel->cursor = 0;
		channel->soundChannel = i;
		channel->soundChannelConfiguration = *channel->soundSpec->soundChannels[i]->soundChannelConfiguration;
		channel->soundChannelConfiguration.SxRAM = waves[i];
		channel->ticks = 0;
		channel->nextElapsedTicksTarget = 0;
		channel->tickStep = __FIX7_9_EXT_MULT(this->speed, this->targetTimerResolutionFactor);

		switch(channel->soundChannelConfiguration.trackType)
		{
			case kMIDI:

				this->hasMIDITracks = true;
				channel->soundTrack.dataMIDI = (uint16*)this->soundSpec->soundChannels[channel->soundChannel]->soundTrack.dataMIDI;
				Sound::computeMIDITrackSamples(channel);
				break;

			case kPCM:

				this->hasPCMTracks = true;
				channel->soundTrack.dataPCM = (uint8*)this->soundSpec->soundChannels[channel->soundChannel]->soundTrack.dataPCM;
				channel->ticks = channel->samples = this->soundSpec->soundChannels[channel->soundChannel]->samples;
				break;

#ifndef __RELEASE
			case kUnknownType:

				NM_ASSERT(false, "Sound::setupChannels: unknown track type");
				break;
#endif
			default:

				NM_ASSERT(false, "Sound::setupChannels: invalid track type");
				break;
		}

		channel->elapsedTicks = 0;
	}

	node = this->channels->head;

	Channel* channelWithLongestTrack = (Channel*)node->data;

	// Find the the channel with the longest track
	for(node = node->next; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		if(channelWithLongestTrack->ticks < channel->ticks)
		{
			channelWithLongestTrack = channel;
		}
	}

	this->mainChannel = channelWithLongestTrack;

#ifdef __SOUND_TEST
	this->totalPlaybackMilliseconds = Sound::getTotalPlaybackMilliseconds(this, channelWithLongestTrack);
#endif

	this->volumeReductionMultiplier = this->hasPCMTracks ? VirtualList::getSize(this->channels) : 1;
}

void Sound::configureSoundRegistries()
{
	if(NULL == this->soundSpec)
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

static void Sound::computeMIDITrackSamples(Channel* channel)
{
	uint16* soundTrackData = (uint16*)channel->soundTrack.dataMIDI;

	NM_ASSERT(soundTrackData, "Sound::computeMIDITrackSamples: null soundTrack");

	for(channel->samples = 0; ENDSOUND != soundTrackData[channel->samples] && LOOPSOUND != soundTrackData[channel->samples]; channel->samples++);

	for(uint16 sample = 0; sample < channel->samples; sample++, channel->ticks += soundTrackData[channel->samples + sample]);

}

static inline uint8 Sound::clampMIDIOutputValue(int8 value)
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

static inline bool Sound::checkIfPlaybackFinishedOnChannel(Channel* channel)
{
	return channel->cursor >= channel->samples;
}

void Sound::completedPlayback()
{
	if(!this->soundSpec->loop)
	{
		if(this->autoReleaseOnFinish)
		{
			Sound::release(this);
		}
		else
		{
			Sound::stop(this);
		}
	}
	else
	{
		Sound::rewind(this);
	}

	if(!isDeleted(this->events))
	{
		Sound::fireEvent(this, kEventSoundFinished);
		NM_ASSERT(!isDeleted(this), "Sound::completedPlayback: deleted this during kEventSoundFinished");
	}
}

void Sound::playMIDINote(Channel* channel, fixed_t leftVolumeFactor, fixed_t rightVolumeFactor)
{
	int16 note = channel->soundTrack.dataMIDI[channel->cursor];
	uint8 volume = (Sound::clampMIDIOutputValue(channel->soundTrack.dataMIDI[(channel->samples << 1) + 1 + channel->cursor] - this->volumeReduction)) & this->unmute;

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
				uint8 tapLocation = channel->soundTrack.dataMIDI[(channel->samples * 3) + 1 + channel->cursor];
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
void Sound::updateVolumeReduction()
{
	uint32 elapsedMilliseconds = this->soundSpec->targetTimerResolutionUS * (this->mainChannel->elapsedTicks - this->previouslyElapsedTicks) / __MICROSECONDS_PER_MILLISECOND;

	if(VUEngine::getGameFrameDuration(_vuEngine) <= elapsedMilliseconds)
	{
		switch(this->playbackType)
		{
			case kSoundPlaybackFadeIn:

				this->volumeReduction -= (this->volumeReductionMultiplier >> 1) + 1;

				if(0 >= this->volumeReduction)
				{
					this->volumeReduction = 0;
					this->playbackType = kSoundPlaybackNormal;
				}

				break;

			case kSoundPlaybackFadeOut:

				this->volumeReduction += (this->volumeReductionMultiplier >> 1) + 1;

				if(__MAXIMUM_VOLUME * this->volumeReductionMultiplier <= this->volumeReduction)
				{
					this->volumeReduction = __MAXIMUM_VOLUME * this->volumeReductionMultiplier;
					this->playbackType = kSoundPlaybackNormal;
					Sound::pause(this);
				}

				break;

			case kSoundPlaybackFadeOutAndRelease:

				this->volumeReduction += (this->volumeReductionMultiplier >> 1) + 1;

				if(__MAXIMUM_VOLUME * this->volumeReductionMultiplier <= this->volumeReduction)
				{
					this->volumeReduction = __MAXIMUM_VOLUME * this->volumeReductionMultiplier;
					this->playbackType = kSoundPlaybackNormal;
					Sound::release(this);
				}

				break;
		}

		this->previouslyElapsedTicks = this->mainChannel->elapsedTicks;
	}
}

void Sound::updateMIDIPlayback(uint32 elapsedMicroseconds __attribute__((unused)))
{
	// Optimization, if no soundSpec or paused, the sum will be different than 0
	if(this->paused + (!this->turnedOn))
	{
		return;
	}

	NM_ASSERT(NULL != this->channels, "Sound::updateMIDIPlayback: invalid channels list");

	bool finished = true;

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

		finished = false;

		channel->elapsedTicks += channel->tickStep;

		if(channel->elapsedTicks >= channel->nextElapsedTicksTarget)
		{
			channel->finished = Sound::checkIfPlaybackFinishedOnChannel(channel);

			if(channel->finished)
			{
				continue;
			}
			
			channel->nextElapsedTicksTarget += __I_TO_FIX7_9_EXT(channel->soundTrack.dataMIDI[channel->samples + 1 + channel->cursor]);

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

			Sound::playMIDINote(this, channel, leftVolumeFactor, rightVolumeFactor);

			channel->cursor++;
		}
	}

	if(finished)
	{
		Sound::completedPlayback(this);
	}

	if(kSoundPlaybackNormal != this->playbackType)
	{
		Sound::updateVolumeReduction(this);
	}
}

void Sound::updatePCMPlayback(uint32 elapsedMicroseconds, uint32 targetPCMUpdates)
{
	CACHE_ENABLE;

	// Optimization, if no soundSpec or paused, the sum will be different than 0
	if(this->paused + (!this->turnedOn))
	{
		return;
	}

	// Elapsed time during PCM playback is based on the cursor, track's ticks and target Hz
	this->mainChannel->elapsedTicks += elapsedMicroseconds;

	this->mainChannel->cursor = this->mainChannel->elapsedTicks / targetPCMUpdates;

	if(this->mainChannel->cursor >= this->mainChannel->samples)
	{
		Sound::completedPlayback(this);
	}
	else
	{
		// PCM playback must be totally in sync on all channels, so, check if completed only
		// in the first one
		int8 volume = (this->mainChannel->soundTrack.dataPCM[this->mainChannel->cursor] - this->volumeReduction) & this->unmute;

		for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;

			if(__MAXIMUM_VOLUME <= volume)
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

		if(kSoundPlaybackNormal != this->playbackType)
		{
			Sound::updateVolumeReduction(this);
		}
	}

	CACHE_DISABLE;
}

#ifndef __SHIPPING
void Sound::print(int32 x, int32 y)
{
	if(NULL == this->soundSpec)
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
		PRINT_HEX_EXT(channel->soundChannelConfiguration.SxINT | (NULL == channel->soundSpec ? 0 : 0x80), x + xDisplacement, y, 2);

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
		PRINT_TEXT(channel->soundSpec->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + xDisplacement, y);

		PRINT_TEXT("Length: ", x, ++y);
		PRINT_INT(channel->samples, x + xDisplacement, y);

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
#endif

uint32 Sound::getTotalPlaybackMilliseconds(Channel* channel)
{
	switch(channel->soundChannelConfiguration.trackType)
	{
		case kMIDI:
			{
				uint32 totalTicks = 0;

				for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
				{
					Channel* channel = (Channel*)node->data;

					if(totalTicks < channel->ticks)
					{
						totalTicks = channel->ticks;
					}
				}

				return (uint32)((long)totalTicks * this->soundSpec->targetTimerResolutionUS / __MICROSECONDS_PER_MILLISECOND);
			}
			break;

		case kPCM:

			return (channel->samples * __MICROSECONDS_PER_MILLISECOND) / this->pcmTargetPlaybackFrameRate;
			break;
	}

	return 0;
}

void Sound::printPlaybackProgress(int32 x, int32 y)
{
	if(NULL == this->mainChannel || 0 == this->mainChannel->ticks)
	{
		return;
	}

	float elapsedTicksProportion = 0;
	
	if(this->hasPCMTracks)
	{
		elapsedTicksProportion = (float)this->mainChannel->cursor / this->mainChannel->samples;
	}
	else
	{
		elapsedTicksProportion = __FIX7_9_EXT_TO_F(this->mainChannel->elapsedTicks) / this->mainChannel->ticks;
	}

	if(0 > elapsedTicksProportion || 1 < elapsedTicksProportion)
	{
		elapsedTicksProportion = 1;		
	}

	uint32 position = elapsedTicksProportion * 32;

	char boxesArray[33] = 
	{
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, '\0'
	};

	for(uint16 i = 0; i < position && 32 >= i; i++)
	{
		boxesArray[i] = __CHAR_BRIGHT_RED_BOX;
	}

	PRINT_TEXT(boxesArray, x, y);
}

void Sound::printTiming(uint32 seconds, int32 x, int32 y)
{
	uint32 minutes = seconds / 60;
	seconds = seconds - minutes * 60;

	int32 minutesDigits = Math::getDigitsCount(minutes);

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

void Sound::printPlaybackTime(int32 x, int32 y)
{
	static uint32 previousSecond = 0;

	if(NULL == this->mainChannel || 0 == this->mainChannel->ticks)
	{
		return;
	}

	float elapsedTicksProportion = 0;
	
	if(this->hasPCMTracks)
	{
		elapsedTicksProportion = (float)this->mainChannel->cursor / this->mainChannel->samples;
	}
	else
	{
		elapsedTicksProportion = __FIX7_9_EXT_TO_F(this->mainChannel->elapsedTicks) / this->mainChannel->ticks;
	}

	if(0 > elapsedTicksProportion || 1 < elapsedTicksProportion)
	{
		elapsedTicksProportion = 1;
	}

	uint32 currentSecond = elapsedTicksProportion * this->totalPlaybackMilliseconds / __MILLISECONDS_PER_SECOND;

	if(previousSecond > currentSecond)
	{
		previousSecond = currentSecond;
	}

	if(currentSecond > previousSecond)
	{
		previousSecond = currentSecond;

		Sound::printTiming(this, currentSecond, x, y);
	}
}

void Sound::printMetadata(int32 x, int32 y, bool printDetails)
{
	PRINT_TEXT("                                  ", x, y);
	PRINT_TEXT(this->soundSpec->name, x, y++);
	y++;

	Sound::printPlaybackProgress(this, x, y++);

	uint8 trackInfoXOffset = x + 22;
	uint8 trackInfoValuesXOffset = 9;
	uint16 speed = __FIX7_9_EXT_TO_I(__FIX7_9_EXT_MULT(this->speed, __I_TO_FIX7_9_EXT(100)));

	y++;

	Sound::printTiming(this, 0, x + 23, y);
	PRINT_TEXT("/", x + 27, y);
	Sound::printTiming(this, this->totalPlaybackMilliseconds / __MILLISECONDS_PER_SECOND, x + 28, y);

	PRINT_TEXT("Speed", x, y);
	PRINT_TEXT("    ", x + 6, y);
	PRINT_INT(speed, x + 6, y);
	PRINT_TEXT("%", x + 6 + ((speed < 10) ? 1 : (speed < 100) ? 2 : 3), y);

	PRINT_TEXT(this->paused ? " \x0B " : "\x07\x07", x + 15, y++);

	if(!printDetails)
	{
		return;
	}

	y+=2;

	PRINT_TEXT("TRACK INFO", trackInfoXOffset, y++);

	PRINT_TEXT("MIDI", trackInfoXOffset, ++y);
	PRINT_TEXT(this->hasMIDITracks ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("PCM", trackInfoXOffset, ++y);
	PRINT_TEXT(this->hasPCMTracks ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Channels", trackInfoXOffset, ++y);
	PRINT_INT(VirtualList::getSize(this->channels), trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Loop", trackInfoXOffset, ++y);
	PRINT_TEXT(this->soundSpec->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y++);

	Sound::printVolume(this, 1, y, true);
}

void Sound::printVolume(int32 x, int32 y, bool printHeader)
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
	}
	else
	{
		++y;
		++y;
		++y;
	}

	++x;

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

				NM_ASSERT(false, "Sound::printVolume: unknown track type");
				break;
#endif
			default:

				NM_ASSERT(false, "Sound::printVolume: invalid channel");
				break;
		}

		char boxesArray[] = 
		{
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			'C', '0' + channel->number,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
			'\0'
		};

		for(i = 0; i < leftValue && 15 > i; i++)
		{
			boxesArray[15 - i - 1] = __CHAR_BRIGHT_RED_BOX;
		}

		for(i = 0; i < rightValue && 15 > i; i++)
		{
			boxesArray[15 + 2 + i] = __CHAR_BRIGHT_RED_BOX;
		}

		PRINT_TEXT(boxesArray, x, y);

		y++;
	}
}
