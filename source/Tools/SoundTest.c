/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifdef __SOUND_TEST

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapTextureManager.h>
#include <HardwareManager.h>
#include <KeypadManager.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <TimerManager.h>
#include <Utilities.h>
#include <VIPManager.h>
#include <VUEngine.h>

#include "SoundTest.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

extern SoundROMSpec* _userSounds[];

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::update()
{
	if(!isDeleted(this->sound))
	{
		static uint16 delay = 0;

		if(delay++ >= __TARGET_FPS)
		{
			Sound::printPlaybackProgress(this->sound, 1, 6);
			Sound::printPlaybackTime(this->sound, 24, 8);
			delay = 0;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::show()
{
	this->soundIndex = 0;

	SoundManager::reset(SoundManager::getInstance());

	Printing::clear();
	SpriteManager::hideAllSprites(SpriteManager::safeCast(ComponentManager::getManager(kSpriteComponent)), NULL, false);
	Printing::resetCoordinates();
	Printing::show(Printing::getInstance());

	TimerManager::setResolution(__TIMER_100US);
	TimerManager::setTargetTimePerInterruptUnits(kMS);
	TimerManager::setTargetTimePerInterrupt(10);

	SoundTest::applyTimerSettings(this);
	SoundTest::loadSound(this);
	SoundTest::dimmGame(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::hide()
{
	SoundTest::releaseSound(this);
	Printing::clear();
	SpriteManager::showAllSprites(SpriteManager::safeCast(ComponentManager::getManager(kSpriteComponent)), NULL, true);
	SoundTest::lightUpGame(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::processUserInput(uint16 pressedKey)
{
	if(NULL == _userSounds[this->soundIndex])
	{
		return;
	}

	bool timerChanged = false;

	// Track controls
	if(K_LL & pressedKey)
	{
		SoundTest::loadPreviousSound(this);
	}
	else if(K_LR & pressedKey)
	{
		SoundTest::loadNextSound(this);
	}
	else if(K_A & pressedKey)
	{
		if(isDeleted(this->sound))
		{
			SoundTest::loadSound(this);
		}

		if(!isDeleted(this->sound))
		{
			if(!Sound::isPlaying(this->sound))
			{
				Sound::play(this->sound, NULL, kSoundPlaybackNormal);
			}
			else
			{
				Sound::pause(this->sound);
			}
		}
	}
	else if(K_B & pressedKey)
	{
		if(isDeleted(this->sound))
		{
			SoundTest::loadSound(this);
		}

		Sound::rewind(this->sound);
	}
	else if(K_LD & pressedKey)
	{
		if(isDeleted(this->sound))
		{
			SoundTest::loadSound(this);
		}

		Sound::setSpeed(this->sound, Sound::getSpeed(this->sound) - __F_TO_FIX7_9(0.01f));
	}
	else if(K_LU & pressedKey)
	{
		if(isDeleted(this->sound))
		{
			SoundTest::loadSound(this);
		}

		Sound::setSpeed(this->sound, Sound::getSpeed(this->sound) +  __F_TO_FIX7_9(0.01f));
	}
	// Timer controls
	else if(K_RU & pressedKey)
	{
		uint16 timerResolution = TimerManager::getResolution(TimerManager::getInstance());

		switch(timerResolution)
		{
			case __TIMER_20US:

				timerResolution = __TIMER_100US;
				break;

			case __TIMER_100US:

				timerResolution = __TIMER_20US;
				break;

			default:

				ASSERT(false, "SoundTest::processUserInput: wrong timer frequency");
				break;
		}

		TimerManager::setResolution(timerResolution);
		timerChanged = true;
	}
	else if(K_RD & pressedKey)
	{
		uint16 targetTimePerInterrupttUnits = TimerManager::getTargetTimePerInterruptUnits(TimerManager::getInstance());
		uint16 targetTimePerInterrupt = TimerManager::getTargetTimePerInterrupt(TimerManager::getInstance());

		switch(targetTimePerInterrupttUnits)
		{
			case kUS:

				targetTimePerInterrupttUnits = kMS;
				targetTimePerInterrupt = 10;
				break;

			case kMS:

				targetTimePerInterrupttUnits = kUS;
				targetTimePerInterrupt = 1000;
				break;

			default:

				ASSERT(false, "SoundTest::processUserInput: wrong timer resolution scale");
				break;
		}

		TimerManager::setTargetTimePerInterruptUnits(targetTimePerInterrupttUnits);
		TimerManager::setTargetTimePerInterrupt(targetTimePerInterrupt);
		timerChanged = true;
	}
	else if(K_RL & pressedKey)
	{
		uint16 targetTimePerInterrupt = TimerManager::getTargetTimePerInterrupt(TimerManager::getInstance());

		targetTimePerInterrupt -= TimerManager::getMinimumTimePerInterruptStep(TimerManager::getInstance());

		TimerManager::setTargetTimePerInterrupt(targetTimePerInterrupt);
		timerChanged = true;
	}
	else if(K_RR & pressedKey)
	{
		uint16 targetTimePerInterrupt = TimerManager::getTargetTimePerInterrupt(TimerManager::getInstance());

		targetTimePerInterrupt += TimerManager::getMinimumTimePerInterruptStep(TimerManager::getInstance());

		TimerManager::setTargetTimePerInterrupt(targetTimePerInterrupt);
		timerChanged = true;
	}

	if(timerChanged)
	{
		SoundTest::applyTimerSettings(this);

		SoundTest::printTimer(this);

		if(!isDeleted(this->sound))
		{
			Sound::pause(this->sound);
			Sound::rewind(this->sound);

			if(!Sound::isPaused(this->sound))
			{
				Sound::play(this->sound, NULL, kSoundPlaybackFadeIn);
			}
		}
	}

	if(!isDeleted(this->sound))
	{
		Sound::print(this->sound, 1, 4);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->sound = NULL;
	this->soundIndex = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::destructor()
{
	SoundTest::releaseSound(this);

	this->sound = NULL;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool SoundTest::onEvent(ListenerObject eventFirer __attribute__((unused)), uint32 eventCode)
{
	switch(eventCode)
	{
		case kEventSoundFinished:
		{
			if(!isDeleted(this->sound))
			{
				Sound::printPlaybackTime(this->sound, 24, 8);
				Sound::printPlaybackProgress(this->sound, 1, 6);
			}

			return true;
		}

		case kEventSoundReleased:
		{
			if(Sound::safeCast(eventFirer) == this->sound)
			{
				this->sound = NULL;		
			}

			return false;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::releaseSound()
{
	if(!isDeleted(this->sound))
	{
		Sound::release(this->sound);

		this->sound = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 SoundTest::getTotalSounds()
{
	uint16 totalSounds = 0;

	for(; _userSounds[totalSounds]; totalSounds++);

	return totalSounds;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::loadPreviousSound()
{
	uint16 totalSounds = SoundTest::getTotalSounds(this);

	if(0 == this->soundIndex)
	{
		this->soundIndex = totalSounds - 1;
	}
	else
	{
		this->soundIndex--;
	}

	SoundTest::loadSound(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::loadNextSound()
{
	uint16 totalSounds = SoundTest::getTotalSounds(this);

	if(totalSounds - 1 == this->soundIndex)
	{
		this->soundIndex = 0;
	}
	else
	{
		this->soundIndex++;
	}

	SoundTest::loadSound(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::loadSound()
{
	if(NULL == _userSounds[this->soundIndex])
	{
		SoundTest::printGUI(this, false);
		return;
	}

	KeypadManager::disable();

#ifdef __SOUND_TEST
	Printing::clear();
	PRINT_TEXT("Loading...", 1, 4);
#endif

	SoundTest::releaseSound(this);

	TimerManager::reset(TimerManager::getInstance());
	TimerManager::setResolution(__TIMER_20US);
	TimerManager::setTargetTimePerInterruptUnits(kUS);
	TimerManager::setTargetTimePerInterrupt(_userSounds[this->soundIndex]->targetTimerResolutionUS);

	this->sound = SoundManager::getSound((SoundSpec*)_userSounds[this->soundIndex], ListenerObject::safeCast(this));

	NM_ASSERT(!isDeleted(this->sound), "SoundTest::loadSound: no sound");

	if(!isDeleted(this->sound))
	{
		Sound::addEventListener(this->sound, ListenerObject::safeCast(this), kEventSoundFinished);
		SoundTest::applyTimerSettings(this);

#ifdef __SOUND_TEST
	PRINT_TEXT("          ", 1, 4);
#endif
	}

	KeypadManager::enable();

	SoundTest::printGUI(this, false);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::applyTimerSettings()
{
	TimerManager::applySettings(true);

	SoundTest::printTimer(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::printTimer()
{
	if(NULL == _userSounds[this->soundIndex])
	{
		return;
	}

	TimerManager::print(1, 11);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void SoundTest::printGUI(bool clearScreen)
{
	if(clearScreen)
	{
		Printing::clear();
	}

	Printing::text
	(
		"\x08 SOUND TEST \x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08"
		"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL
	);

	if(NULL == _userSounds[this->soundIndex])
	{
		Printing::text("No sounds found", 1, 4, NULL);
		Printing::text("Define some in _userSounds global variable", 1, 6, NULL);
		return;
	}

	Printing::text(__CHAR_SELECTOR_LEFT, 1, 2, NULL);

	uint16 totalSounds = SoundTest::getTotalSounds(this);

	int32 selectedSoundDigits = Math::getDigitsCount(this->soundIndex + 1);
	int32 totalSoundsDigits = Math::getDigitsCount(totalSounds);
	Printing::int32(this->soundIndex + 1, 1 + 1, 2, NULL);
	Printing::text("/" , 1 + 1 + selectedSoundDigits, 2, NULL);
	Printing::int32(SoundTest::getTotalSounds(this), 1 + 1 + selectedSoundDigits + 1, 2, NULL);
	Printing::text(__CHAR_SELECTOR, 1 + 1 + selectedSoundDigits + 1 + totalSoundsDigits, 2, NULL);

	if(isDeleted(this->sound))
	{
		return;
	}

	int32 xControls = 37;
	int32 yControls = 4;

	// Controls
	if(!Sound::isPlaying(this->sound))
	{
		Printing::text("Play     \x13", xControls, yControls++, NULL);
	}
	else
	{
		Printing::text("Pause    \x13", xControls, yControls++, NULL);
	}
	Printing::text("Rewind   \x14", xControls, yControls++, NULL);
	yControls++;
	Printing::text("T.Freq. \x1F\x1A", xControls, yControls++, NULL);
	Printing::text("T.Scale \x1F\x1B", xControls, yControls++, NULL);
	Printing::text("T.Res. \x1F\x1C\x1D", xControls, yControls++, NULL);

	SoundTest::printTimer(this);
	Sound::print(this->sound, 1, 4);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
