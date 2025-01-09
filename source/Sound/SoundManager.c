/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with soundManager source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <Printing.h>
#include <Profiler.h>
#include <Sound.h>
#include <VirtualList.h>
#include <VSUManager.h>
#include <WaveForms.h>

#include "SoundManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Sound;
friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static VSUManager _vsuManager = NULL;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::playSounds(uint32 elapsedMicroseconds)
{
	SoundManager soundManager = SoundManager::getInstance();

	VSUManager::update(_vsuManager);

	for(VirtualNode node = soundManager->sounds->head; NULL != node; node = node->next)
	{
		Sound::update(Sound::safeCast(node->data), elapsedMicroseconds, soundManager->targetPCMUpdates);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::reset()
{
	SoundManager soundManager = SoundManager::getInstance();

	VSUManager::reset(_vsuManager);

	for(VirtualNode node = soundManager->sounds->head; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		NM_ASSERT(!isDeleted(sound), "SoundManager::reset: deleted soundSpec wrapper");

		delete sound;
	}

	VirtualList::clear(soundManager->sounds);

	SoundManager::setPCMTargetPlaybackRefreshRate(__DEFAULT_PCM_HZ);
	SoundManager::stopAllSounds(false, NULL);
	SoundManager::unlock();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate)
{
	SoundManager soundManager = SoundManager::getInstance();

	if(0 == pcmTargetPlaybackRefreshRate)
	{
		pcmTargetPlaybackRefreshRate = __DEFAULT_PCM_HZ;
	}

	soundManager->targetPCMUpdates = __MICROSECONDS_PER_SECOND / pcmTargetPlaybackRefreshRate;

	SoundTrack::setPCMTargetPlaybackRefreshRate(pcmTargetPlaybackRefreshRate);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool SoundManager::isPlayingSound(const SoundSpec* soundSpec)
{
	SoundManager soundManager = SoundManager::getInstance();

	VirtualNode node = soundManager->sounds->head;

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

static bool SoundManager::playSound
(
	const SoundSpec* soundSpec, const Vector3D* position, uint32 playbackType, EventListener soundReleaseListener, ListenerObject scope
)
{
	SoundManager soundManager = SoundManager::getInstance();

	if(soundManager->lock || NULL == soundSpec)
	{
		return false;
	}

	Sound sound = SoundManager::doGetSound(soundSpec, soundReleaseListener, scope);

	if(!isDeleted(sound))
	{
		Sound::autoReleaseOnFinish(sound, true);
		Sound::play(sound, position, playbackType);

		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Sound SoundManager::getSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope)
{
	SoundManager soundManager = SoundManager::getInstance();

	if(soundManager->lock || NULL == soundReleaseListener || NULL == scope)
	{
		return NULL;
	}

	return SoundManager::doGetSound(soundSpec, soundReleaseListener, scope);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Sound SoundManager::findSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope)
{
	SoundManager soundManager = SoundManager::getInstance();

	if(NULL == soundReleaseListener || NULL == scope)
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
				Sound::addEventListener(sound, scope, soundReleaseListener, kEventSoundReleased);
				return sound;
			}
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::muteAllSounds()
{
	SoundManager soundManager = SoundManager::getInstance();

	VirtualNode node = soundManager->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::mute(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::unmuteAllSounds()
{
	SoundManager soundManager = SoundManager::getInstance();

	VirtualNode node = soundManager->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::unmute(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::rewindAllSounds()
{
	SoundManager soundManager = SoundManager::getInstance();

	VirtualNode node = soundManager->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::rewind(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::stopAllSounds(bool release, SoundSpec** excludedSounds)
{
	SoundManager soundManager = SoundManager::getInstance();

	VirtualNode node = soundManager->sounds->head;

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

	if(release)
	{
		SoundManager::purgeReleasedSounds();
	}

	if(NULL == excludedSounds)
	{
		VSUManager::stopAllSounds();
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::lock()
{
	SoundManager soundManager = SoundManager::getInstance();

	soundManager->lock = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::unlock()
{
	SoundManager soundManager = SoundManager::getInstance();

	soundManager->lock = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifdef __SOUND_TEST
static void SoundManager::printPlaybackTime(int32 x, int32 y)
{
	SoundManager soundManager = SoundManager::getInstance();

	VirtualNode node = soundManager->sounds->head;

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
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Sound SoundManager::doGetSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope)
{
	SoundManager soundManager = SoundManager::getInstance();

	SoundManager::purgeReleasedSounds();

	if(NULL == soundSpec)
	{
		return NULL;
	}

	Sound sound = new Sound(soundSpec, soundReleaseListener, scope);

	VirtualList::pushBack(soundManager->sounds, sound);

	return sound;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::purgeReleasedSounds()
{
	SoundManager soundManager = SoundManager::getInstance();

	for(VirtualNode node = soundManager->sounds->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		Sound sound = Sound::safeCast(node->data);

		if(NULL == sound->soundSpec)
		{
			VirtualList::removeNode(soundManager->sounds, node);

			delete sound;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void SoundManager::suspendPlayingSounds()
{
	SoundManager soundManager = SoundManager::getInstance();

	VirtualNode node = soundManager->sounds->head;

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

static void SoundManager::resumePlayingSounds()
{
	SoundManager soundManager = SoundManager::getInstance();

	VirtualNode node = soundManager->sounds->head;

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
