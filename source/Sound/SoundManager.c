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
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundManager::onEvent(ListenerObject eventFirer, uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventTimerManagerInterrupt:
		{
			if(SoundManager::playSounds(this))
			{
				return true;
			}

			return false;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void SoundManager::updateSounds()
{
	if(NULL == this->sounds->head)
	{
		return;		
	}

	for(VirtualNode node = this->sounds->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		Sound sound = Sound::safeCast(node->data);

		if(!Sound::updatePlaybackState(sound))
		{
			VirtualList::removeNode(this->sounds, node);
			
			delete sound;
			continue;
		}
	}

	if(NULL == this->sounds->head)
	{
		TimerManager::removeEventListener(TimerManager::getInstance(), ListenerObject::safeCast(this), kEventTimerManagerInterrupt);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundManager::playSounds()
{
	VSUManager::update(VSUManager::getInstance());

	for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::update(sound);
	}

	return NULL != this->sounds->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void SoundManager::reset()
{
	TimerManager::removeEventListener(TimerManager::getInstance(), ListenerObject::safeCast(this), kEventTimerManagerInterrupt);

	VSUManager::reset(VSUManager::getInstance());

	for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		NM_ASSERT(!isDeleted(sound), "SoundManager::reset: deleted soundSpec wrapper");

		delete sound;
	}

	VirtualList::clear(this->sounds);

	SoundManager::stopAllSounds(this, false, NULL);
	SoundManager::unlock(this);
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
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::destructor()
{
	TimerManager::removeEventListener(TimerManager::getInstance(), ListenerObject::safeCast(this), kEventTimerManagerInterrupt);

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
	if(NULL == soundSpec)
	{
		return NULL;
	}

	Sound sound = new Sound(soundSpec, scope);

	if(NULL == this->sounds->head)
	{
		TimerManager::addEventListener(TimerManager::getInstance(), ListenerObject::safeCast(this), kEventTimerManagerInterrupt);	
	}

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
