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

#include <Camera.h>
#include <Printing.h>
#include <Profiler.h>
#include <Sound.h>
#include <VirtualList.h>
#include <VSUManager.h>
#include <WaveForms.h>

#include "SoundManager.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Sound;
friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

static SoundManager _soundManager = NULL;
static VSUManager _vsuManager = NULL;


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void SoundManager::playSounds(uint32 elapsedMicroseconds)
{
	VSUManager::update(_vsuManager);

	for(VirtualNode node = _soundManager->sounds->head; NULL != node; node = node->next)
	{
		Sound::update(Sound::safeCast(node->data), elapsedMicroseconds, _soundManager->targetPCMUpdates);
	}
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void SoundManager::reset()
{
	VSUManager::reset(_vsuManager);

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
//---------------------------------------------------------------------------------------------------------
void SoundManager::update()
{
}
//---------------------------------------------------------------------------------------------------------
void SoundManager::setPCMTargetPlaybackRefreshRate(uint16 pcmTargetPlaybackRefreshRate)
{
	if(0 == pcmTargetPlaybackRefreshRate)
	{
		pcmTargetPlaybackRefreshRate = __DEFAULT_PCM_HZ;
	}

	this->targetPCMUpdates = __MICROSECONDS_PER_SECOND / pcmTargetPlaybackRefreshRate;

	SoundTrack::setPCMTargetPlaybackRefreshRate(pcmTargetPlaybackRefreshRate);
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
bool SoundManager::playSound(const SoundSpec* soundSpec, const Vector3D* position, uint32 playbackType, EventListener soundReleaseListener, ListenerObject scope)
{
	if(this->lock || NULL == soundSpec)
	{
		return false;
	}

	Sound sound = SoundManager::doGetSound(this, soundSpec, soundReleaseListener, scope);

	if(!isDeleted(sound))
	{
		Sound::autoReleaseOnFinish(sound, true);
		Sound::play(sound, position, playbackType);

		return true;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------
Sound SoundManager::getSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope)
{
	if(this->lock || NULL == soundReleaseListener || NULL == scope)
	{
		return NULL;
	}

	return SoundManager::doGetSound(this, soundSpec, soundReleaseListener, scope);
}
//---------------------------------------------------------------------------------------------------------
Sound SoundManager::findSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope)
{
	if(NULL == soundReleaseListener || NULL == scope)
	{
		return NULL;
	}

	for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
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
//---------------------------------------------------------------------------------------------------------
void SoundManager::muteAllSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::mute(sound);
	}
}
//---------------------------------------------------------------------------------------------------------
void SoundManager::unmuteAllSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::unmute(sound);
	}
}
//---------------------------------------------------------------------------------------------------------
void SoundManager::rewindAllSounds()
{
	VirtualNode node = this->sounds->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::rewind(sound);
	}
}
//---------------------------------------------------------------------------------------------------------
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

	if(release)
	{
		SoundManager::purgeReleasedSounds(this);
	}

	if(NULL == excludedSounds)
	{
		VSUManager::stopAllSounds(VSUManager::getInstance());
	}
}
//---------------------------------------------------------------------------------------------------------
void SoundManager::lock()
{
	this->lock = true;
}
//---------------------------------------------------------------------------------------------------------
void SoundManager::unlock()
{
	this->lock = false;
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void SoundManager::constructor()
{	
	Base::constructor();

	this->sounds = new VirtualList();

	this->lock = false;
	this->targetPCMUpdates = 0;

	_soundManager = this;
	_vsuManager = VSUManager::getInstance();
}
//---------------------------------------------------------------------------------------------------------
void SoundManager::destructor()
{
	_soundManager = NULL;

	if(!isDeleted(this->sounds))
	{
		VirtualList::deleteData(this->sounds);
		delete this->sounds;
		this->sounds = NULL;
	}

	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
Sound SoundManager::doGetSound(const SoundSpec* soundSpec, EventListener soundReleaseListener, ListenerObject scope)
{
	SoundManager::purgeReleasedSounds(this);

	if(NULL == soundSpec)
	{
		return NULL;
	}

	Sound sound = new Sound(soundSpec, soundReleaseListener, scope);

	VirtualList::pushBack(this->sounds, sound);

	return sound;
}
//---------------------------------------------------------------------------------------------------------
void SoundManager::purgeReleasedSounds()
{
	for(VirtualNode node = this->sounds->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		Sound sound = Sound::safeCast(node->data);

		if(NULL == sound->soundSpec)
		{
			VirtualList::removeNode(this->sounds, node);

			delete sound;
		}
	}
}
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------------------------