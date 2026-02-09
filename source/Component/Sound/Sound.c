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

#include <ComponentManager.h>
#include <TimerManager.h>
#include <VirtualList.h>
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

#define __MIDI_CONVERTER_FREQUENCY_US			20
#define __SOUND_TARGET_US_PER_TICK				__MIDI_CONVERTER_FREQUENCY_US

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Mirror _mirror = {false, false, false};
static uint8 _soundsoundGroups[kSoundGroupOther + 1] = { [0 ... kSoundGroupOther] = __MAXIMUM_VOLUME };

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Sound Sound::get(const SoundSpec* soundSpec, Entity owner, ListenerObject scope)
{
	if(NULL == soundSpec)
	{
		return NULL;
	}

	Sound sound = NULL; 
	sound = Sound::safeCast(ComponentManager::createComponent(owner, (ComponentSpec*)soundSpec));

	if(!isDeleted(sound) && !isDeleted(scope))
	{
		Sound::addEventListener(sound, scope, kEventSoundReleased);
	}

	return sound;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool Sound::playSound(const SoundSpec* soundSpec, Entity owner, uint32 playbackType, ListenerObject scope)
{
	Sound sound = Sound::get(soundSpec, owner, scope);

	if(!isDeleted(sound))
	{
		Sound::autoReleaseOnFinish(sound, true);
		Sound::play(sound, playbackType);

		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Sound::setVolume(uint32 soundGroup, uint8 volume)
{
	if(kSoundGroupNone > (int32)soundGroup || kSoundGroupOther < (int32)soundGroup)
	{
		return;
	}

	if(__MAXIMUM_VOLUME < volume)
	{
		volume = __MAXIMUM_VOLUME;
	}

	_soundsoundGroups[soundGroup] = volume;
}

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

void Sound::constructor(Entity owner, const SoundSpec* soundSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, (const ComponentSpec*)&soundSpec->componentSpec);

	this->state = kSoundOff;
	this->speed = __I_TO_FIX7_9_EXT(1);
	this->previouslyElapsedTicks = 0;
    this->totalElapsedTicks = 0;
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
	this->volumeReduction = 0;
	this->volumeReductionMultiplier = 1;
	this->volumenScalePower = 0;
	this->frequencyDelta = 0;

	Sound::configureTracks(this);

	if(!isDeleted(this->owner))
	{
		Sound::addEventListener(this, ListenerObject::safeCast(this->owner), kEventSoundReleased);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::destructor()
{
	if(!isDeleted(this->soundTracks))
	{
		VirtualList::deleteData(this->soundTracks);
		delete this->soundTracks;
		this->soundTracks = NULL;
	}

	Sound::fireEvent(this, kEventSoundReleased);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const SoundSpec* Sound::getSpec()
{
	return ((SoundSpec*)this->componentSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::fastForward(uint32 elapsedTicks)
{
	if(kSoundRelease == this->state)
	{
		return;
	}

	if(isDeleted(this->soundTracks))
	{
		return;
	}

	Sound::rewind(this);

	this->targetTimerResolutionFactor = Sound::computeTimerResolutionFactor(this);
	this->tickStep = __FIX7_9_EXT_MULT(this->speed, this->targetTimerResolutionFactor);

	this->previouslyElapsedTicks = 0;
    this->totalElapsedTicks = 0;

	while(this->totalElapsedTicks < __I_TO_FIX7_9_EXT(elapsedTicks))
	{
		for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
		{
			SoundTrack soundTrack = SoundTrack::safeCast(node->data);

			SoundTrack::update
			(
				soundTrack, 
				this->tickStep, 
				this->targetTimerResolutionFactor, 
				0, 
				0,
				_soundsoundGroups[((SoundSpec*)this->componentSpec)->soundGroup],
				__MAXIMUM_VOLUME,
				__MAXIMUM_VOLUME,
				0
			);
		}

		this->totalElapsedTicks += this->tickStep;		
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::play(uint32 playbackType)
{
	if(kSoundRelease == this->state)
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
		{
			if(kSoundPlaying != this->state)
			{
				Sound::setVolumeReduction(this, __MAXIMUM_VOLUME * this->volumeReductionMultiplier);
			}
			else
			{
				return;
			}

			break;
		}

		case kSoundPlaybackNormal:
		{	
			Sound::setVolumeReduction(this, 0);
			break;
		}
	}

	this->playbackType = playbackType;

	switch(playbackType)
	{
		case kSoundPlaybackFadeIn:
		case kSoundPlaybackNormal:
		{
			bool wasPaused = kSoundPaused == this->state;

			this->state = kSoundPlaying;

			for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
			{
				SoundTrack soundTrack = SoundTrack::safeCast(node->data);

				SoundTrack::start(soundTrack, wasPaused);
			}

			if(!wasPaused)
			{
				this->previouslyElapsedTicks = 0;
			}

			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::stop()
{
	if(kSoundRelease == this->state)
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
	if(kSoundRelease == this->state)
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
	if(kSoundRelease == this->state)
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
	if(kSoundRelease == this->state)
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
	if(kSoundRelease == this->state)
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
	if(kSoundRelease == this->state)
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
    this->totalElapsedTicks = 0;

	for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
	{
		SoundTrack soundTrack = SoundTrack::safeCast(node->data);

		SoundTrack::rewind(soundTrack);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::release()
{
	if(kSoundRelease == this->state)
	{
		return;
	}

	Sound::stop(this);

	this->state = kSoundRelease;
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

uint32 Sound::getTotalElapsedTicks()
{
	return __FIX7_9_EXT_TO_I(this->totalElapsedTicks);
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

bool Sound::updatePlaybackState()
{
	if(kSoundRelease == this->state)
	{
		return false;
	}

	if(kSoundFinished != this->state)
	{
		return true;
	}

	Sound::fireEvent(this, kEventSoundFinished);

	if(!((SoundSpec*)this->componentSpec)->loop)
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

	return kSoundRelease != this->state;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::update()
{
	if(kSoundPlaying !=	this->state)
	{
		return;
	}

	bool finished = true;
	fixed_t leftVolumeFactor = -1;
	fixed_t rightVolumeFactor = -1;

	if(NULL != this->transformation && __NON_TRANSFORMED != this->transformation->invalid)
	{
#ifndef __LEGACY_COORDINATE_PROJECTION
		Vector3D relativePosition = Vector3D::rotate(Vector3D::getRelativeToCamera(this->transformation->position), *_cameraInvertedRotation);
#else
		Vector3D relativePosition = 
			Vector3D::rotate
			(
				Vector3D::sub
				(
					Vector3D::getRelativeToCamera
					(
						this->transformation->position), (Vector3D){__HALF_SCREEN_WIDTH_METERS, __HALF_SCREEN_HEIGHT_METERS, 0}
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

		finished = 
			SoundTrack::update
			(
				soundTrack, 
				this->tickStep, 
				this->targetTimerResolutionFactor,
				_soundsoundGroups[((SoundSpec*)this->componentSpec)->soundGroup],
				leftVolumeFactor, 
				rightVolumeFactor, 
				this->volumeReduction, 
				this->volumenScalePower, 
				this->frequencyDelta
			) && finished;
	}

    this->totalElapsedTicks += this->tickStep;

	if(finished)
	{
		if(!((SoundSpec*)this->componentSpec)->loop)
		{
			this->state = kSoundFinished;
		}
		else
		{
			Sound::loop(this);
		}
	}
	else if(kSoundPlaybackNormal != this->playbackType)
	{
		Sound::updateVolumeReduction(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void Sound::print(int32 x, int32 y)
{
	PRINT_TEXT("                                  ", x, y);
	PRINT_TEXT(((SoundSpec*)this->componentSpec)->name, x, y++);
	y++;

 	Sound::printPlaybackProgress(this, x, y++, 32);
 
	uint8 trackInfoXOffset = x + 22;
	uint8 trackInfoValuesXOffset = 9;
	uint16 speed = __FIX7_9_EXT_TO_I(__FIX7_9_EXT_MULT(this->speed, __I_TO_FIX7_9_EXT(100)));

	y++;

	Sound::printPlaybackTime(this, x + 23, y);
	PRINT_TEXT("/", x + 27, y);
	Sound::printTiming(this, this->totalPlaybackMilliseconds / __MILLISECONDS_PER_SECOND, x + 28, y);

	PRINT_TEXT("Speed", x, y);
	PRINT_TEXT("    ", x + 6, y);
	PRINT_INT(speed, x + 6, y);
	PRINT_TEXT("%", x + 6 + ((speed < 10) ? 1 : (speed < 100) ? 2 : 3), y);

	PRINT_TEXT(kSoundPlaying == this->state ? "  " : "\x07\x07", x + 15, y++);

	y += 2;

	PRINT_TEXT("TRACK INFO", trackInfoXOffset, y++);

	PRINT_TEXT("Tracks", trackInfoXOffset, ++y);
	PRINT_INT(VirtualList::getCount(this->soundTracks), trackInfoXOffset + trackInfoValuesXOffset, y);

	PRINT_TEXT("Loop", trackInfoXOffset, ++y);
	PRINT_TEXT
	(
		((SoundSpec*)this->componentSpec)->loop ? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, trackInfoXOffset + trackInfoValuesXOffset, y++
	);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void Sound::printPlaybackTime(int32 x, int32 y)
{
	static uint32 previousSecond = 0;

	if(NULL == this->mainSoundTrack || 0 == this->mainSoundTrack->ticks)
	{
		return;
	}

	float elapsedTicksPercentage = SoundTrack::getElapsedTicksPercentage(this->mainSoundTrack);
	
	uint32 currentSecond = elapsedTicksPercentage * this->totalPlaybackMilliseconds / __MILLISECONDS_PER_SECOND;

	if(previousSecond > currentSecond)
	{
		previousSecond = currentSecond;
	}

	if(currentSecond > previousSecond)
	{
		previousSecond = currentSecond;

		Sound::printTiming(this, currentSecond, x, y);
	}
	else if(0 == elapsedTicksPercentage)
	{
		Sound::printTiming(this, 0, x, y);
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void Sound::printPlaybackProgress(int32 x, int32 y, int32 width)
{
	if(NULL == this->mainSoundTrack || 0 == this->mainSoundTrack->ticks)
	{
		return;
	}

	float elapsedTicksPercentage = SoundTrack::getElapsedTicksPercentage(this->mainSoundTrack);

	if(0 > elapsedTicksPercentage || 1 < elapsedTicksPercentage)
	{
		elapsedTicksPercentage = 1;		
	}

	uint32 position = elapsedTicksPercentage * width;

	for(uint16 i = 0; i < width; i++)
	{
		if (i < position)
		{
			Printer::text(__CHAR_BRIGHT_RED_BOX, x + i, y, NULL);
		}
		else
		{
			Printer::text(__CHAR_DARK_RED_BOX, x + i, y, NULL);
		}
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

fix7_9_ext Sound::computeTimerResolutionFactor()
{
	uint32 timerCounter = TimerManager::getTimerCounter(TimerManager::getInstance()) + __TIMER_COUNTER_DELTA;
	uint32 timerUsPerInterrupt = timerCounter * __SOUND_TARGET_US_PER_TICK;
	uint32 targetTimerResolutionUS = 
		0 != ((SoundSpec*)this->componentSpec)->targetTimerResolutionUS 
		? 
		((SoundSpec*)this->componentSpec)->targetTimerResolutionUS : 1000;

	targetTimerResolutionUS = 0 == targetTimerResolutionUS? 1000: targetTimerResolutionUS;
	
	uint32 soundTargetUsPerInterrupt = 
		(__TIME_US(targetTimerResolutionUS) + __TIMER_COUNTER_DELTA) * __SOUND_TARGET_US_PER_TICK;

	NM_ASSERT(0 < soundTargetUsPerInterrupt, "Sound::computeTimerResolutionFactor: zero soundTargetUsPerInterrupt");

	return __FIX19_13_TO_FIX7_9_EXT(__FIX19_13_DIV(__I_TO_FIX19_13(timerUsPerInterrupt), __I_TO_FIX19_13(soundTargetUsPerInterrupt)));
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
	if(kSoundRelease == this->state || NULL == ((SoundSpec*)this->componentSpec)->soundTrackSpecs)
	{
		return;
	}

	if(NULL == this->soundTracks)
	{
		this->soundTracks = new VirtualList();
	}

	SoundTrack longestSoundTrack = NULL;

	for(int16 i = 0; NULL != ((SoundSpec*)this->componentSpec)->soundTrackSpecs[i]; i++)
	{
		SoundTrack soundTrack = new SoundTrack(((SoundSpec*)this->componentSpec)->soundTrackSpecs[i]);

		VirtualList::pushBack(this->soundTracks, soundTrack);

		if(NULL == longestSoundTrack || SoundTrack::getTicks(longestSoundTrack) < SoundTrack::getTicks(soundTrack))
		{
			longestSoundTrack = soundTrack;
		}

	}

	this->mainSoundTrack = longestSoundTrack;

	if(!isDeleted(this->mainSoundTrack))
	{
		this->totalPlaybackMilliseconds = 
			SoundTrack::getTotalPlaybackMilliseconds(this->mainSoundTrack, ((SoundSpec*)this->componentSpec)->targetTimerResolutionUS);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

__attribute__((noinline)) 
void Sound::updateVolumeReduction()
{
	uint32 elapsedMilliseconds = 
		((SoundSpec*)this->componentSpec)->targetTimerResolutionUS * (this->mainSoundTrack->elapsedTicks 
		- this->previouslyElapsedTicks) / __MICROSECONDS_PER_MILLISECOND;

	if(VUEngine::getGameFrameDuration() <= elapsedMilliseconds)
	{
		switch(this->playbackType)
		{
			case kSoundPlaybackFadeIn:
			{
				this->volumeReduction -= (this->volumeReductionMultiplier >> 1) + 1;

				if(0 >= this->volumeReduction)
				{
					this->volumeReduction = 0;
					this->playbackType = kSoundPlaybackNormal;
				}

				break;
			}

			case kSoundPlaybackFadeOut:
			{
				this->volumeReduction += (this->volumeReductionMultiplier >> 1) + 1;

				if(__MAXIMUM_VOLUME * this->volumeReductionMultiplier <= this->volumeReduction)
				{
					this->volumeReduction = __MAXIMUM_VOLUME * this->volumeReductionMultiplier;
					this->playbackType = kSoundPlaybackNone;
					Sound::pause(this);
				}

				break;
			}

			case kSoundPlaybackFadeOutAndRelease:
			{
				this->volumeReduction += (this->volumeReductionMultiplier >> 1) + 1;

				if(__MAXIMUM_VOLUME * this->volumeReductionMultiplier <= this->volumeReduction)
				{
					this->volumeReduction = __MAXIMUM_VOLUME * this->volumeReductionMultiplier;
					this->playbackType = kSoundPlaybackNone;
					this->state = kSoundFinished;
					this->autoReleaseOnFinish = true;
				}

				break;
			}
		}

		this->previouslyElapsedTicks = this->mainSoundTrack->elapsedTicks;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Sound::loop()
{
	if(kSoundRelease == this->state)
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
    this->totalElapsedTicks = 0;

	for(VirtualNode node = this->soundTracks->head; NULL != node; node = node->next)
	{
		SoundTrack soundTrack = SoundTrack::safeCast(node->data);
		fix7_9_ext soundTrackElapsedTicks = SoundTrack::loop(soundTrack);
		
		if(0 == this->totalElapsedTicks || this->totalElapsedTicks > soundTrackElapsedTicks)
		{
			this->totalElapsedTicks = soundTrackElapsedTicks;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
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
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
