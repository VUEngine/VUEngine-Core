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

#include <Sound.h>
#include <TimerManager.h>
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
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->lock = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::destructor()
{	
	TimerManager::removeEventListener(TimerManager::getInstance(), ListenerObject::safeCast(this), kEventTimerManagerInterrupt);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

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

			return NULL == this->components->head;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 SoundManager::getType()
{
	return kSoundComponent;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::enable()
{
	Base::enable(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::disable()
{
	Base::disable(this);

	VSUManager::flushQueuedSounds(VSUManager::getInstance());

	SoundManager::destroyAllComponents(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Sound SoundManager::create(Entity owner, const SoundSpec* soundSpec)
{
	if(NULL == soundSpec || this->lock)
	{
		return NULL;
	}

	TimerManager::addEventListener(TimerManager::getInstance(), ListenerObject::safeCast(this), kEventTimerManagerInterrupt);

	return ((Sound (*)(Entity, const SoundSpec*)) ((ComponentSpec*)soundSpec)->allocator)(owner, soundSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::purgeComponents()
{
	TimerManager::removeEventListener(TimerManager::getInstance(), ListenerObject::safeCast(this), kEventTimerManagerInterrupt);

	VSUManager::flushQueuedSounds(VSUManager::getInstance());

	Base::purgeComponents(this);	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::update()
{
	if(NULL == this->components->head)
	{
		return;		
	}

	for(VirtualNode node = this->components->head, nextNode = NULL; NULL != node; node = nextNode)
	{
		nextNode = node->next;

		Sound sound = Sound::safeCast(node->data);

		if(!Sound::updatePlaybackState(sound))
		{
			VirtualList::removeNode(this->components, node);
			delete sound;
			continue;
		}
	}

	if(NULL == this->components->head)
	{
		TimerManager::removeEventListener(TimerManager::getInstance(), ListenerObject::safeCast(this), kEventTimerManagerInterrupt);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundManager::playSounds()
{
	VSUManager::update(VSUManager::getInstance());

	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::update(sound);
	}

	return NULL != this->components->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundManager::isPlayingSound(const SoundSpec* soundSpec)
{
	VirtualNode node = this->components->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		if(soundSpec == (SoundSpec*)sound->componentSpec)
		{
			return true;
		}
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::muteAllSounds()
{
	VirtualNode node = this->components->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::mute(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::unmuteAllSounds()
{
	VirtualNode node = this->components->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::unmute(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::rewindAllSounds()
{
	VirtualNode node = this->components->head;

	for(; NULL != node; node = node->next)
	{
		Sound sound = Sound::safeCast(node->data);

		Sound::rewind(sound);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::pauseSounds()
{
	if(!isDeleted(this->components))
	{
		for(VirtualNode node = this->components->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::pause(sound);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::unpauseSounds()
{
	if(!isDeleted(this->components))
	{
		for(VirtualNode node = this->components->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::unpause(sound);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundManager::stopAllSounds(bool release, SoundSpec** excludedSounds)
{
	VirtualNode node = this->components->head;

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
					if(excludedSounds[i] == (SoundSpec*)sound->componentSpec)
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

void SoundManager::fadeSounds(uint32 playbackType)
{
	if(!isDeleted(this->components))
	{
		for(VirtualNode node = this->components->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::play(sound, playbackType);
			}
		}
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
	VirtualNode node = this->components->head;

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

void SoundManager::suspendPlayingSounds()
{
	VirtualNode node = this->components->head;

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
	VirtualNode node = this->components->head;

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
