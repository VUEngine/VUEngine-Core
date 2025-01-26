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

#include <Printer.h>
#include <Singleton.h>
#include <Sound.h>
#include <VirtualList.h>
#include <VSUManager.h>
#include <WaveForms.h>

#include "SoundManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Sound;
friend class SoundTrack;
friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool SoundManager::playSound
(
	const SoundSpec* soundSpec, const Vector3D* position, uint32 playbackType, ListenerObject scope
)
{
	SoundManager soundManager = SoundManager::getInstance();

	if(soundManager->lock || NULL == soundSpec)
	{
		return false;
	}

	Sound sound = SoundManager::doGetSound(soundManager, soundSpec, scope);

	if(!isDeleted(sound))
	{
		Sound::autoReleaseOnFinish(sound, true);
		Sound::play(sound, position, playbackType);

		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Sound SoundManager::getSound(const SoundSpec* soundSpec, ListenerObject scope)
{
	SoundManager soundManager = SoundManager::getInstance();

	if(soundManager->lock || NULL == scope)
	{
		return NULL;
	}

	return SoundManager::doGetSound(soundManager, soundSpec, scope);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Sound SoundManager::findSound(const SoundSpec* soundSpec, ListenerObject scope)
{
	SoundManager soundManager = SoundManager::getInstance();

	if(NULL == scope)
	{
		return NULL;
	}

	for(VirtualNode node = soundManager->sounds->head; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound))
		{
			if(soundSpec == sound->soundSpec)
			{
				Sound::addEventListener(sound, scope, kEventSoundReleased);
				return sound;
			}
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void SoundManager::updatePCM (Sound sound, uint32 elapsedMicroseconds, uint32 targetPCMUpdates)
{
	if(kSoundPlaying !=	sound->state)
	{
		return;
	}

	SoundTrack soundTrack = SoundTrack::safeCast(sound->soundTracks->head->data);

	CACHE_ENABLE;

	// Elapsed time during PCM playback is based on the cursor, track's ticks and target Hz
	soundTrack->elapsedTicks += elapsedMicroseconds;

	soundTrack->cursor = soundTrack->elapsedTicks / targetPCMUpdates;

	VSUManager::applyPCMSampleToSoundSource(soundTrack->soundTrackSpec->SxLRV[soundTrack->cursor]);

	CACHE_DISABLE;

	if(soundTrack->cursor >= soundTrack->samples)
	{
		Sound::finishPlayback(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void SoundManager::playSounds(uint32 elapsedMicroseconds)
{
#ifdef __RELEASE
	// This is an aggressive optimization that bypasses the SoundTrack's methods altogether
	// to keep the PCM playback viable on hardware
	if(kPlaybackPCM == VSUManager::getMode() && NULL != this->sounds->head)
	{
		SoundManager::updatePCM(Sound::safeCast(this->sounds->head->data), elapsedMicroseconds, this->targetPCMUpdates);
	}
	else
	{
#endif
		VSUManager::update(VSUManager::getInstance());

		for(VirtualNode node = this->sounds->head, nextNode = NULL; NULL != node; node = nextNode)
		{
			nextNode = node->next;

			nextNode = node->next;

			Sound sound = Sound::safeCast(node->data);

			if(NULL == sound->soundSpec)
			{
				VirtualList::removeNode(this->sounds, node);

				delete sound;
				continue;
			}

			Sound::update(Sound::safeCast(node->data), elapsedMicroseconds, this->targetPCMUpdates);
		}
#ifdef __RELEASE
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void SoundManager::reset()
{
	VSUManager::reset(VSUManager::getInstance());

	for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		NM_ASSERT(!isDeleted(sound), "SoundManager::reset: deleted soundSpec wrapper");

		delete sound;
	}

	VirtualList::clear(this->sounds);

	SoundManager::setPCMTargetPlaybackRefreshRate(this, __DEFAULT_PCM_HZ);
	SoundManager::stopAllSounds(this, false, NULL);
	SoundManager::unlock(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate)
{
	if(0 == pcmTargetPlaybackRefreshRate)
	{
		pcmTargetPlaybackRefreshRate = __DEFAULT_PCM_HZ;
	}

	this->targetPCMUpdates = __MICROSECONDS_PER_SECOND / pcmTargetPlaybackRefreshRate;

	SoundTrack::setPCMTargetPlaybackRefreshRate(pcmTargetPlaybackRefreshRate);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundManager::isPlayingSound(const SoundSpec* soundSpec)
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(soundSpec == sound->soundSpec)
		{
			return true;
		}
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::muteAllSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::mute(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::unmuteAllSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::unmute(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::rewindAllSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::rewind(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::stopAllSounds(bool release, SoundSpec** excludedSounds)
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound))
		{
			bool excludeSoundWrapper = false;

			if(NULL != excludedSounds)
			{
				for(int16 i = 0; NULL != excludedSounds[i]; i++)
				{
					if(excludedSounds[i] == sound->soundSpec)
					{
						if(release)
						{
							Sound::removeAllEventListeners(sound);
						}

						excludeSoundWrapper = true;

						break;
					}
				}
			}

			if(!excludeSoundWrapper)
			{
				if(release)
				{
					Sound::release(sound);
				}
				else
				{
					Sound::stop(sound);
				}
			}
		}
	}

	if(NULL == excludedSounds)
	{
		VSUManager::stopAllSounds(VSUManager::getInstance());
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::lock()
{
	this->lock = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::unlock()
{
	this->lock = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __SOUND_TEST
void SoundManager::printPlaybackTime(int32 x, int32 y)
{
	VirtualNode node = this->sounds->head;

	if(!isDeleted(node))
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound))
		{
			Sound::printPlaybackTime(sound, x, y);
		}
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->sounds = new VirtualList();

	this->lock = false;
	this->targetPCMUpdates = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::destructor()
{
	if(!isDeleted(this->sounds))
	{
		VirtualList::deleteData(this->sounds);
		delete this->sounds;
		this->sounds = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Sound SoundManager::doGetSound(const SoundSpec* soundSpec, ListenerObject scope)
{
#ifdef __RELEASE
	// This is an aggressive optimization that bypasses the SoundTrack's methods altogether
	// to keep the PCM playback viable on hardware
	if(kPlaybackPCM == VSUManager::getMode() && NULL != this->sounds->head)
	{
#ifndef __SHIPPING
		Printer::setDebugMode();
		Printer::clear();
		Error::triggerException("SoundManager::doGetSound: a PCM sound is loaded, unload it first", NULL);		
#endif

		return NULL;
	}
#endif

	if(NULL == soundSpec)
	{
		return NULL;
	}

	Sound sound = new Sound(soundSpec, scope);

	VirtualList::pushBack(this->sounds, sound);

	return sound;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::suspendPlayingSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound) && !Sound::isPaused(sound))
		{
			Sound::suspend(sound);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::resumePlayingSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(!isDeleted(sound) && !Sound::isPaused(sound))
		{
			Sound::resume(sound);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
