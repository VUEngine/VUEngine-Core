/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <MessageDispatcher.h>
#include <Printing.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <VirtualList.h>
#include <Utilities.h>
#include <VUEngine.h>

#include "Sound.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;

extern SoundRegistry* const _soundRegistries;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

// Must redefine these because they are defined as strings
#undef __CHAR_DARK_RED_BOX
#define __CHAR_DARK_RED_BOX			'\x0E'
#undef __CHAR_BRIGHT_RED_BOX
#define __CHAR_BRIGHT_RED_BOX		'\x10'


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

static Mirror _mirror = {false, false, false};
static uint16 _pcmTargetPlaybackRefreshRate = 4000;


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void Sound::setMirror(Mirror mirror)
{
	_mirror = mirror;
}
//---------------------------------------------------------------------------------------------------------
static void Sound::setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate)
{
	_pcmTargetPlaybackRefreshRate = pcmTargetPlaybackRefreshRate;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Sound::constructor(const SoundSpec* soundSpec, VirtualList channels, int8* waves, EventListener soundReleaseListener, ListenerObject scope)
{
	// construct base Container
	Base::constructor();

	this->state = kSoundOff;
	this->soundSpec = soundSpec;
	this->MIDITracks = 0;
	this->PCMTracks = 0;
	this->speed = __I_TO_FIX7_9_EXT(1);
	this->previouslyElapsedTicks = 0;
	this->totalPlaybackMilliseconds = 0;
	this->autoReleaseOnFinish = true;
	this->playbackType = kSoundPlaybackNone;
	this->locked = false;

#ifdef __MUTE_ALL_SOUND
	this->unmute = 0x00;
#else
	this->unmute = 0xFF;
#endif

	this->mainChannel = NULL;
	this->channels = channels;
	this->position = NULL;
	this->volumeReduction = 0;
	this->volumeReductionMultiplier = 1;
	this->volumenScalePower = 0;
	this->frequencyDelta = 0;

	Sound::setupChannels(this, waves);
	Sound::configureSoundRegistries(this);

	if(NULL != soundReleaseListener && !isDeleted(scope))
	{
		Sound::addEventListener(this, scope, soundReleaseListener, kEventSoundReleased);
	}
}
//---------------------------------------------------------------------------------------------------------
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

	if(0 < this->PCMTracks)
	{
		CACHE_RESET;
	}

	// destroy the super Container
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
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

	switch(playbackType)
	{
		case kSoundPlaybackFadeIn:

			if(kSoundPlaying != this->state)
			{
				Sound::setVolumeReduction(this, __MAXIMUM_VOLUME * this->volumeReductionMultiplier);
			}
			else
			{
				return;
			}

			break;

			// intentional fall through
		case kSoundPlaybackNormal:
			
			Sound::setVolumeReduction(this, 0);
			break;
	}

	this->playbackType = playbackType;

	switch(playbackType)
	{
		case kSoundPlaybackFadeIn:
		case kSoundPlaybackNormal:
			{
				bool wasPaused = kSoundPaused == this->state;

				this->state = kSoundPlaying;

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

					_soundRegistries[channel->index].SxFQH = 0;
					_soundRegistries[channel->index].SxFQL = 0;
					_soundRegistries[channel->index].SxLRV = 0;
					_soundRegistries[channel->index].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;
				}

				if(!wasPaused)
				{
					this->previouslyElapsedTicks = 0;
					
					if(0 < this->PCMTracks)
					{
						CACHE_DISABLE;
						CACHE_CLEAR;
					}
				}	
			}

			break;
	}
}
//---------------------------------------------------------------------------------------------------------
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

	this->state = kSoundOff;

	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;

		channel->cursor = 0;
		channel->elapsedTicks = 0;
		channel->nextElapsedTicksTarget = 0;

		// If turned of right away, pops and cracks are perceptible
		_soundRegistries[channel->index].SxINT |= __SOUND_WRAPPER_STOP_SOUND;
	}
}
//---------------------------------------------------------------------------------------------------------
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

	if(kSoundPlaying == this->state)
	{
		this->state = kSoundPaused;

		for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;
			_soundRegistries[channel->index].SxINT |= __SOUND_WRAPPER_STOP_SOUND;
		}
	}
}
//---------------------------------------------------------------------------------------------------------
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

	if(kSoundPaused == this->state)
	{
		this->state = kSoundPlaying;

		for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
		{
			Channel* channel = (Channel*)node->data;
			_soundRegistries[channel->index].SxLRV = 0x00;
			_soundRegistries[channel->index].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;
		}
	}
}
//---------------------------------------------------------------------------------------------------------
void Sound::suspend()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	if(kSoundPlaying != this->state)
	{
		return;
	}

	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		_soundRegistries[channel->index].SxINT |= __SOUND_WRAPPER_STOP_SOUND;
	}
}
//---------------------------------------------------------------------------------------------------------
void Sound::resume()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->channels))
	{
		return;
	}

	if(kSoundPlaying != this->state)
	{
		return;
	}

	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		_soundRegistries[channel->index].SxLRV = 0x00;
		_soundRegistries[channel->index].SxINT = channel->soundChannelConfiguration.SxINT | 0x80;
	}
}
//---------------------------------------------------------------------------------------------------------
void Sound::mute()
{
	this->unmute = 0x00;
}
//---------------------------------------------------------------------------------------------------------
void Sound::unmute()
{
	this->unmute = 0xFF;
}
//---------------------------------------------------------------------------------------------------------
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

	fix7_9_ext targetTimerResolutionFactor = Sound::computeTimerResolutionFactor(this);

	this->previouslyElapsedTicks = 0;

	for(VirtualNode node = this->channels->head; NULL != node; node = node->next)
	{
		Channel* channel = (Channel*)node->data;
		channel->finished = false;
		channel->cursor = 0;
		channel->elapsedTicks = 0;
		channel->nextElapsedTicksTarget = 0;
		channel->tickStep = __FIX7_9_EXT_MULT(this->speed, targetTimerResolutionFactor);
	}
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void Sound::lock()
{
	this->locked = true;
}
//---------------------------------------------------------------------------------------------------------
void Sound::unlock()
{
	this->locked = false;
}
//---------------------------------------------------------------------------------------------------------
void Sound::autoReleaseOnFinish(bool autoReleaseOnFinish)
{
	this->autoReleaseOnFinish = autoReleaseOnFinish;
}
//---------------------------------------------------------------------------------------------------------
void Sound::setSpeed(fix7_9_ext speed)
{
	// Prevent timer interrupts to unsync tracks
	if(0 == this->PCMTracks)
	{
		this->speed = 0 >= speed ? __F_TO_FIX7_9_EXT(0.01f) : speed < __I_TO_FIX7_9_EXT(16) ? speed : __I_TO_FIX7_9_EXT(16);
	}
}
//---------------------------------------------------------------------------------------------------------
fix7_9_ext Sound::getSpeed()
{
	return this->speed;
}
//---------------------------------------------------------------------------------------------------------
void Sound::setVolumenScalePower(uint8 volumenScalePower)
{
	if(4 < volumenScalePower)
	{
		volumenScalePower = 4;
	}

	this->volumenScalePower = volumenScalePower;
}
//---------------------------------------------------------------------------------------------------------
void Sound::setFrequencyDelta(uint16 frequencyDelta)
{
	this->frequencyDelta = frequencyDelta;
}
//---------------------------------------------------------------------------------------------------------
uint16 Sound::getFrequencyDelta()
{
	return this->frequencyDelta;
}
//---------------------------------------------------------------------------------------------------------
bool Sound::hasMIDITracks()
{
	return 0 < this->MIDITracks;
}
//---------------------------------------------------------------------------------------------------------
bool Sound::hasPCMTracks()
{
	return 0 < this->PCMTracks;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
bool Sound::isPlaying()
{
	return kSoundPlaying == this->state;
}
//---------------------------------------------------------------------------------------------------------
bool Sound::isPaused()
{
	return kSoundPaused == this->state;
}
//---------------------------------------------------------------------------------------------------------
bool Sound::isFadingIn()
{
	return kSoundPlaybackFadeIn == this->playbackType;
}
//---------------------------------------------------------------------------------------------------------
bool Sound::isFadingOut()
{
	return kSoundPlaybackFadeOut == this->playbackType || kSoundPlaybackFadeOutAndRelease == this->playbackType;
}
//---------------------------------------------------------------------------------------------------------
void Sound::updateMIDIPlayback(uint32 elapsedMicroseconds __attribute__((unused)))
{
	if(kSoundPlaying !=	this->state)
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
				if(_mirror.x)
				{
					relativePosition.x = -relativePosition.x;
				}

				if(_mirror.y)
				{
					relativePosition.y = -relativePosition.y;
				}

				if(_mirror.z)
				{
					relativePosition.z = -relativePosition.z;
				}

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
//---------------------------------------------------------------------------------------------------------
void Sound::updatePCMPlayback(uint32 elapsedMicroseconds, uint32 targetPCMUpdates)
{
	CACHE_ENABLE;

	if(kSoundPlaying !=	this->state)
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
				_soundRegistries[channel->index].SxLRV = 0xFF;
				volume -= __MAXIMUM_VOLUME;
			}
			else
			{
				_soundRegistries[channel->index].SxLRV = ((volume << 4) | volume);
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
//---------------------------------------------------------------------------------------------------------
void Sound::print(int32 x, int32 y)
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

	PRINT_TEXT(kSoundPaused == this->state ? " \x0B " : "\x07\x07", x + 15, y++);

	y+=2;

	PRINT_TEXT("TRACK INFO", trackInfoXOffset, y++);

	PRINT_TEXT("MIDI", trackInfoXOffset, ++y);
	PRINT_TEXT(0 < this->MIDITracks ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("PCM", trackInfoXOffset, ++y);
	PRINT_TEXT(0 < this->PCMTracks ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Channels", trackInfoXOffset, ++y);
	PRINT_INT(VirtualList::getCount(this->channels), trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Loop", trackInfoXOffset, ++y);
	PRINT_TEXT(this->soundSpec->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y++);

	Sound::printVolume(this, 1, y, true);
}
//---------------------------------------------------------------------------------------------------------
void Sound::printVolume(int32 x, int32 y, bool printHeader)
{
	if(0 < this->PCMTracks)
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
			'C', '0' + channel->index,
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
//---------------------------------------------------------------------------------------------------------
void Sound::printPlaybackTime(int32 x, int32 y)
{
	static uint32 previousSecond = 0;

	if(NULL == this->mainChannel || 0 == this->mainChannel->ticks)
	{
		return;
	}

	float elapsedTicksProportion = 0;
	
	if(0 < this->PCMTracks)
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
//---------------------------------------------------------------------------------------------------------
void Sound::printPlaybackProgress(int32 x, int32 y)
{
	if(NULL == this->mainChannel || 0 == this->mainChannel->ticks)
	{
		return;
	}

	float elapsedTicksProportion = 0;
	
	if(0 < this->PCMTracks)
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
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
fix7_9_ext Sound::computeTimerResolutionFactor()
{
	uint16 timerResolutionUS = TimerManager::getResolutionInUS(TimerManager::getInstance());
	uint16 timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance()) + __TIMER_COUNTER_DELTA;
	uint16 timerUsPerInterrupt = timerCounter * __SOUND_TARGET_US_PER_TICK;
	uint16 targetTimerResolutionUS = 0 != this->soundSpec->targetTimerResolutionUS ? this->soundSpec->targetTimerResolutionUS : 1000;
	uint16 soundTargetUsPerInterrupt = (__TIME_US(targetTimerResolutionUS) + __TIMER_COUNTER_DELTA) * __SOUND_TARGET_US_PER_TICK;

	NM_ASSERT(0 < timerResolutionUS, "Sound::computeTimerResolutionFactor: zero timerResolutionUS");
	NM_ASSERT(0 < soundTargetUsPerInterrupt, "Sound::computeTimerResolutionFactor: zero soundTargetUsPerInterrupt");

	fix7_9_ext targetTimerResolutionFactor = __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(timerUsPerInterrupt), __I_TO_FIX7_9_EXT(soundTargetUsPerInterrupt));

	// Compensate for the difference in speed between 20US and 100US timer resolution
	fix7_9_ext timerResolutionRatioReduction = __I_TO_FIX7_9_EXT(1) - __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(__SOUND_TARGET_US_PER_TICK), __I_TO_FIX7_9_EXT(timerResolutionUS));

	if(0 != timerResolutionRatioReduction)
	{
		timerResolutionRatioReduction = __I_TO_FIX7_9_EXT(1) - __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(__SOUND_TARGET_US_PER_TICK), __I_TO_FIX7_9_EXT(timerResolutionUS - 0*(timerResolutionUS >> 3)));

		targetTimerResolutionFactor = __FIX7_9_EXT_MULT(targetTimerResolutionFactor, timerResolutionRatioReduction);
	}

	return targetTimerResolutionFactor;
}
//---------------------------------------------------------------------------------------------------------
void Sound::setVolumeReduction(int8 volumeReduction)
{
	if(Sound::isFadingIn(this) || Sound::isFadingOut(this))
	{
		return;
	}

	this->volumeReduction = volumeReduction;
}
//---------------------------------------------------------------------------------------------------------
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

	fix7_9_ext targetTimerResolutionFactor = Sound::computeTimerResolutionFactor(this);

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
		channel->tickStep = __FIX7_9_EXT_MULT(this->speed, targetTimerResolutionFactor);

		switch(channel->soundChannelConfiguration.trackType)
		{
			case kMIDI:

				this->MIDITracks++;
				channel->soundTrack.dataMIDI = (uint16*)this->soundSpec->soundChannels[channel->soundChannel]->soundTrack.dataMIDI;
				Sound::computeMIDITrackSamples(channel);
				break;

			case kPCM:

				this->PCMTracks++;
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

	this->volumeReductionMultiplier = 0 < this->PCMTracks ? VirtualList::getCount(this->channels) : 1;
}
//---------------------------------------------------------------------------------------------------------
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

		_soundRegistries[channel->index].SxINT = 0x00;
		_soundRegistries[channel->index].SxLRV = 0x00;
		_soundRegistries[channel->index].SxEV0 = channel->soundChannelConfiguration.SxEV0;
		_soundRegistries[channel->index].SxEV1 = channel->soundChannelConfiguration.SxEV1;
		_soundRegistries[channel->index].SxFQH = channel->soundChannelConfiguration.SxFQH;
		_soundRegistries[channel->index].SxFQL = channel->soundChannelConfiguration.SxFQL;
		_soundRegistries[channel->index].SxRAM = channel->soundChannelConfiguration.SxRAM;

		if(kChannelModulation == channel->type)
		{
			_soundRegistries[channel->index].S5SWP = 0;
		}
	}
}
//---------------------------------------------------------------------------------------------------------
static void Sound::computeMIDITrackSamples(Channel* channel)
{
	uint16* soundTrackData = (uint16*)channel->soundTrack.dataMIDI;

	NM_ASSERT(soundTrackData, "Sound::computeMIDITrackSamples: null soundTrack");

	for(channel->samples = 0; ENDSOUND != soundTrackData[channel->samples] && LOOPSOUND != soundTrackData[channel->samples]; channel->samples++);

	for(uint16 sample = 0; sample < channel->samples; sample++, channel->ticks += soundTrackData[channel->samples + sample]);

}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
static inline bool Sound::checkIfPlaybackFinishedOnChannel(Channel* channel)
{
	return channel->cursor >= channel->samples;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
void Sound::playMIDINote(Channel* channel, fixed_t leftVolumeFactor, fixed_t rightVolumeFactor)
{
	int16 note = channel->soundTrack.dataMIDI[channel->cursor];
	uint8 volume = Sound::clampMIDIOutputValue(channel->soundTrack.dataMIDI[(channel->samples << 1) + 1 + channel->cursor] - this->volumeReduction) & this->unmute;

	int16 leftVolume = volume;
	int16 rightVolume = volume;

	if(0 != volume)
	{
		fixed_t volumeHelper = __I_TO_FIXED(volume);

		if(0 <= leftVolumeFactor)
		{
			leftVolume = __FIXED_TO_I(__FIXED_MULT(volumeHelper, leftVolumeFactor));
		}

		if(0 <= rightVolumeFactor)
		{
			rightVolume = __FIXED_TO_I(__FIXED_MULT(volumeHelper, rightVolumeFactor));
		}
	}

	leftVolume >>= this->volumenScalePower;
	rightVolume >>= this->volumenScalePower;

	uint8 SxLRV = ((leftVolume << 4) | rightVolume) & channel->soundChannelConfiguration.volume;

	switch(note)
	{
		case PAU:

			_soundRegistries[channel->index].SxEV1 = channel->soundChannelConfiguration.SxEV1 | 0x1;
			break;

		case HOLD:

#ifdef __SOUND_TEST
			_soundRegistries[channel->index].SxLRV = channel->soundChannelConfiguration.SxLRV = SxLRV;
#else
#ifdef __SHOW_SOUND_STATUS
			_soundRegistries[channel->index].SxLRV = channel->soundChannelConfiguration.SxLRV = SxLRV;
#else
			_soundRegistries[channel->index].SxLRV = SxLRV;
#endif
#endif
			break;

		default:

			note += this->frequencyDelta;

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

			_soundRegistries[channel->index].SxLRV = SxLRV;
			_soundRegistries[channel->index].SxFQH = (note >> 8);
			_soundRegistries[channel->index].SxFQL = (note & 0xFF);
			_soundRegistries[channel->index].SxEV0 = channel->soundChannelConfiguration.SxEV0;
			_soundRegistries[channel->index].SxEV1 = channel->soundChannelConfiguration.SxEV1;

			if(kChannelNoise == channel->soundChannelConfiguration.channelType)
			{
				uint8 tapLocation = channel->soundTrack.dataMIDI[(channel->samples * 3) + 1 + channel->cursor];
				_soundRegistries[channel->index].SxEV1 = (tapLocation << 4) | (0x0F & channel->soundChannelConfiguration.SxEV1);
			}
			
			break;

	}
}
//---------------------------------------------------------------------------------------------------------
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
					this->playbackType = kSoundPlaybackNone;
					Sound::pause(this);
				}

				break;

			case kSoundPlaybackFadeOutAndRelease:

				this->volumeReduction += (this->volumeReductionMultiplier >> 1) + 1;

				if(__MAXIMUM_VOLUME * this->volumeReductionMultiplier <= this->volumeReduction)
				{
					this->volumeReduction = __MAXIMUM_VOLUME * this->volumeReductionMultiplier;
					this->playbackType = kSoundPlaybackNone;
					Sound::release(this);
				}

				break;
		}

		this->previouslyElapsedTicks = this->mainChannel->elapsedTicks;
	}
}
//---------------------------------------------------------------------------------------------------------
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

			return (channel->samples * __MICROSECONDS_PER_MILLISECOND) / _pcmTargetPlaybackRefreshRate;
			break;
	}

	return 0;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
