/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <MessageDispatcher.h>
#include <Printing.h>
#include <SoundTrack.h>
#include <TimerManager.h>
#include <VirtualList.h>
#include <Utilities.h>
#include <VUEngine.h>

#include "Sound.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class SoundTrack;
friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Must redefine these because they are defined as strings
#undef __CHAR_DARK_RED_BOX
#define __CHAR_DARK_RED_BOX						'\x0E'
#undef __CHAR_BRIGHT_RED_BOX
#define __CHAR_BRIGHT_RED_BOX					'\x10'

#define __MIDI_CONVERTER_FREQUENCY_US			20
#define __SOUND_TARGET_US_PER_TICK				__MIDI_CONVERTER_FREQUENCY_US

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Mirror _mirror = {false, false, false};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Sound::setMirror(Mirror mirror)
{
	_mirror = mirror;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::constructor(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope)
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->state = kSoundOff;
	this->soundSpec = soundSpec;
	this->speed = __I_TO_FIX7_9_EXT(1);
	this->previouslyElapsedTicks = 0;
	this->totalPlaybackMilliseconds = 0;
	this->autoReleaseOnFinish = false;
	this->playbackType = kSoundPlaybackNone;
	this->locked = false;

	this->targetTimerResolutionFactor = Sound::computeTimerResolutionFactor(this);
	this->tickStep = __FIX7_9_EXT_MULT(this->speed, this->targetTimerResolutionFactor);

#ifdef __MUTE_ALL_SOUND
	this->unmute = 0x00;
#else
	this->unmute = 0xFF;
#endif

	this->soundTracks = NULL;
	this->mainSoundTrack = NULL;
	this->position = NULL;
	this->volumeReduction = 0;
	this->volumeReductionMultiplier = 1;
	this->volumenScalePower = 0;
	this->frequencyDelta = 0;

	Sound::configureTracks(this);

	if(NULL != soundReleaseListener && !isDeleted(scope))
	{
		Sound::addEventListener(this, scope, soundReleaseListener, kEventSoundReleased);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::destructor()
{
	this->soundSpec = NULL;

	MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), ListenerObject::safeCast(this));

	if(!isDeleted(this->soundTracks))
	{
		VirtualList::deleteData(this->soundTracks);
		delete this->soundTracks;
		this->soundTracks = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::play(const Vector3D* position, uint32 playbackType)
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->soundTracks))
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

				for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
				{
					SoundTrack soundTrack = SoundTrack::safeCast(node->data);

					SoundTrack::start(soundTrack, wasPaused);
				}

				if(!wasPaused)
				{
					this->previouslyElapsedTicks = 0;
				}
			}

			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::stop()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->soundTracks))
	{
		return;
	}

	this->state = kSoundOff;

	for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
	{
		SoundTrack soundTrack = SoundTrack::safeCast(node->data);

		SoundTrack::stop(soundTrack);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::pause()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->soundTracks))
	{
		return;
	}

	if(kSoundPlaying == this->state)
	{
		this->state = kSoundPaused;

		for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
		{
			SoundTrack soundTrack = SoundTrack::safeCast(node->data);
			
			SoundTrack::pause(soundTrack);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::unpause()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->soundTracks))
	{
		return;
	}

	if(kSoundPaused == this->state)
	{
		this->state = kSoundPlaying;

		for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
		{
			SoundTrack soundTrack = SoundTrack::safeCast(node->data);

			SoundTrack::unpause(soundTrack);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::suspend()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->soundTracks))
	{
		return;
	}

	if(kSoundPlaying != this->state)
	{
		return;
	}

	for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
	{
		SoundTrack soundTrack = SoundTrack::safeCast(node->data);

		SoundTrack::suspend(soundTrack);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::resume()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->soundTracks))
	{
		return;
	}

	if(kSoundPlaying != this->state)
	{
		return;
	}

	for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
	{
		SoundTrack soundTrack = SoundTrack::safeCast(node->data);
		
		SoundTrack::resume(soundTrack);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::mute()
{
	this->unmute = 0x00;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::unmute()
{
	this->unmute = 0xFF;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::rewind()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	if(isDeleted(this->soundTracks))
	{
		return;
	}

	this->targetTimerResolutionFactor = Sound::computeTimerResolutionFactor(this);
	this->tickStep = __FIX7_9_EXT_MULT(this->speed, this->targetTimerResolutionFactor);

	this->previouslyElapsedTicks = 0;

	for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
	{
		SoundTrack soundTrack = SoundTrack::safeCast(node->data);

		SoundTrack::rewind(soundTrack);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::release()
{
	if(NULL == this->soundSpec)
	{
		return;
	}

	Sound::stop(this);

	this->soundSpec = NULL;

	if(!isDeleted(this->soundTracks))
	{
		VirtualList::deleteData(this->soundTracks);
		delete this->soundTracks;
		this->soundTracks = NULL;
	}

	if(!isDeleted(this->events))
	{
		Sound::fireEvent(this, kEventSoundReleased);
		NM_ASSERT(!isDeleted(this), "Sound::release: deleted this during kEventSoundReleased");
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::lock()
{
	this->locked = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::unlock()
{
	this->locked = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::autoReleaseOnFinish(bool autoReleaseOnFinish)
{
	this->autoReleaseOnFinish = autoReleaseOnFinish;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::setSpeed(fix7_9_ext speed)
{
	// Prevent timer interrupts to unsync tracks
	this->speed = 0 >= speed ? __F_TO_FIX7_9_EXT(0.01f) : speed < __I_TO_FIX7_9_EXT(16) ? speed : __I_TO_FIX7_9_EXT(16);

	this->targetTimerResolutionFactor = Sound::computeTimerResolutionFactor(this);
	this->tickStep = __FIX7_9_EXT_MULT(this->speed, this->targetTimerResolutionFactor);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fix7_9_ext Sound::getSpeed()
{
	return this->speed;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::setVolumenScalePower(uint8 volumenScalePower)
{
	if(4 < volumenScalePower)
	{
		volumenScalePower = 4;
	}

	this->volumenScalePower = volumenScalePower;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::setFrequencyDelta(uint16 frequencyDelta)
{
	this->frequencyDelta = frequencyDelta;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 Sound::getFrequencyDelta()
{
	return this->frequencyDelta;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sound::isPlaying()
{
	return kSoundPlaying == this->state;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sound::isPaused()
{
	return kSoundPaused == this->state;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sound::isFadingIn()
{
	return kSoundPlaybackFadeIn == this->playbackType;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Sound::isFadingOut()
{
	return kSoundPlaybackFadeOut == this->playbackType || kSoundPlaybackFadeOutAndRelease == this->playbackType;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::update(uint32 elapsedMicroseconds, uint32 targetPCMUpdates)
{
	if(kSoundPlaying !=	this->state)
	{
		return;
	}

	bool finished = true;

	fixed_t leftVolumeFactor = -1;
	fixed_t rightVolumeFactor = -1;

	if(NULL != this->position)
	{
#ifndef __LEGACY_COORDINATE_PROJECTION
		Vector3D relativePosition = Vector3D::rotate(Vector3D::getRelativeToCamera(*this->position), *_cameraInvertedRotation);
#else
		Vector3D relativePosition = 
			Vector3D::rotate
			(
				Vector3D::sub
				(
					Vector3D::getRelativeToCamera
					(
						*this->position), (Vector3D){__HALF_SCREEN_WIDTH_METERS, __HALF_SCREEN_HEIGHT_METERS, 0}
					), 
					*_cameraInvertedRotation
			);
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

		leftVolumeFactor  = 
			__1I_FIXED - 
			__FIXED_EXT_DIV(squaredDistanceToLeftEar, __FIXED_SQUARE(__PIXELS_TO_METERS(__SOUND_STEREO_ATTENUATION_DISTANCE)));
		
		rightVolumeFactor = 
			__1I_FIXED - 
			__FIXED_EXT_DIV(squaredDistanceToRightEar, __FIXED_SQUARE(__PIXELS_TO_METERS(__SOUND_STEREO_ATTENUATION_DISTANCE)));

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

	for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
	{
		SoundTrack soundTrack = SoundTrack::safeCast(node->data);

		finished = SoundTrack::update(
										soundTrack, 
										elapsedMicroseconds, 
										targetPCMUpdates, 
										this->tickStep, 
										this->targetTimerResolutionFactor, 
										leftVolumeFactor, 
										rightVolumeFactor, 
										this->volumeReduction, 
										this->volumenScalePower, 
										this->frequencyDelta) && finished;
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

	PRINT_TEXT(kSoundPlaying == this->state ? "\x07\x07" : " \x0B ", x + 15, y++);

	y+=2;

	PRINT_TEXT("TRACK INFO", trackInfoXOffset, y++);

	PRINT_TEXT("Tracks", trackInfoXOffset, ++y);
	PRINT_INT(VirtualList::getCount(this->soundTracks), trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Loop", trackInfoXOffset, ++y);
	PRINT_TEXT
	(
		this->soundSpec->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y++
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::printPlaybackTime(int32 x, int32 y)
{
	static uint32 previousSecond = 0;

	if(NULL == this->mainSoundTrack || 0 == this->mainSoundTrack->ticks)
	{
		return;
	}

	float elapsedTicksPercentaje = SoundTrack::getElapsedTicksPercentaje(this->mainSoundTrack);
	
	uint32 currentSecond = elapsedTicksPercentaje * this->totalPlaybackMilliseconds / __MILLISECONDS_PER_SECOND;

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::printPlaybackProgress(int32 x, int32 y)
{
	if(NULL == this->mainSoundTrack || 0 == this->mainSoundTrack->ticks)
	{
		return;
	}

	float elapsedTicksPercentaje = SoundTrack::getElapsedTicksPercentaje(this->mainSoundTrack);

	if(0 > elapsedTicksPercentaje || 1 < elapsedTicksPercentaje)
	{
		elapsedTicksPercentaje = 1;		
	}

	uint32 position = elapsedTicksPercentaje * 32;

	char boxesArray[33] = 
	{
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, 
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, 
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, 
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX,
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, 
		__CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, __CHAR_DARK_RED_BOX, '\0'
	};

	for(uint16 i = 0; i < position && 32 >= i; i++)
	{
		boxesArray[i] = __CHAR_BRIGHT_RED_BOX;
	}

	PRINT_TEXT(boxesArray, x, y);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fix7_9_ext Sound::computeTimerResolutionFactor()
{
	uint32 timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance()) + __TIMER_COUNTER_DELTA;
	uint32 timerUsPerInterrupt = timerCounter * __SOUND_TARGET_US_PER_TICK;
	uint32 targetTimerResolutionUS = 0 != this->soundSpec->targetTimerResolutionUS ? this->soundSpec->targetTimerResolutionUS : 1000;
	uint32 soundTargetUsPerInterrupt = (__TIME_US(targetTimerResolutionUS) + __TIMER_COUNTER_DELTA) * __SOUND_TARGET_US_PER_TICK;

	NM_ASSERT(0 < soundTargetUsPerInterrupt, "Sound::computeTimerResolutionFactor: zero soundTargetUsPerInterrupt");

	return __FIX7_9_EXT_DIV(__I_TO_FIX7_9_EXT(timerUsPerInterrupt), __I_TO_FIX7_9_EXT(soundTargetUsPerInterrupt));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::setVolumeReduction(int8 volumeReduction)
{
	if(Sound::isFadingIn(this) || Sound::isFadingOut(this))
	{
		return;
	}

	this->volumeReduction = volumeReduction;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::configureTracks()
{
	if(NULL == this->soundSpec || NULL == this->soundSpec->soundTrackSpecs)
	{
		return;
	}

	if(NULL == this->soundTracks)
	{
		this->soundTracks = new VirtualList();
	}

	SoundTrack longestSoundTrack = NULL;

	for(int16 i = 0; NULL != this->soundSpec->soundTrackSpecs[i]; i++)
	{
		SoundTrack soundTrack = new SoundTrack(this->soundSpec->soundTrackSpecs[i]);

		VirtualList::pushBack(this->soundTracks, soundTrack);

		if(NULL == longestSoundTrack || SoundTrack::getTicks(longestSoundTrack) < SoundTrack::getTicks(soundTrack))
		{
			longestSoundTrack = soundTrack;
		}

	}

	this->mainSoundTrack = longestSoundTrack;

#ifdef __SOUND_TEST
	this->totalPlaybackMilliseconds = 
		SoundTrack::getTotalPlaybackMilliseconds(this->mainSoundTrack, this->soundSpec->targetTimerResolutionUS);
#endif

}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

__attribute__((noinline)) 
void Sound::updateVolumeReduction()
{
	uint32 elapsedMilliseconds = 
		this->soundSpec->targetTimerResolutionUS * (this->mainSoundTrack->elapsedTicks 
		- this->previouslyElapsedTicks) / __MICROSECONDS_PER_MILLISECOND;

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

		this->previouslyElapsedTicks = this->mainSoundTrack->elapsedTicks;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

