/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SoundTest.h>
#include <Game.h>
#include <HardwareManager.h>
#include <KeypadManager.h>
#include <VIPManager.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <Utilities.h>
#include <TimerManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

extern SoundROM* _userSounds[];


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void SoundTest::constructor()
{
	Base::constructor();

	this->soundWrapper = NULL;
	this->selectedSound = 0;
}

/**
 * Class destructor
 */
void SoundTest::destructor()
{
	SoundTest::releaseSoundWrapper(this);

	this->soundWrapper = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Release sound
 */
void SoundTest::releaseSoundWrapper()
{
	if(!isDeleted(this->soundWrapper))
	{
		SoundManager::releaseSoundWrapper(SoundManager::getInstance(), this->soundWrapper);

		this->soundWrapper = NULL;
	}
}

/**
 * Update
 */
void SoundTest::update()
{
}

void SoundTest::update2()
{

	// Don't print at full speed to not affect PCM playback
	static s16 delay = 10000;

	if(0 >= --delay)
	{
		delay = 10000;

		if(!isDeleted(this->soundWrapper))
		{
			SoundWrapper::printProgress(this->soundWrapper, 1, 6);
			SoundWrapper::printVolume(this->soundWrapper, 1, 18);
		}
	}
}

/**
 * Show editor
 *
 * @param gameState Current game state
 */
void SoundTest::show()
{
	SoundManager::reset(SoundManager::getInstance());
	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager::showLayer(SpriteManager::getInstance(), 0);
	Printing::resetCoordinates(Printing::getInstance());

	TimerManager::setResolution(TimerManager::getInstance(), __TIMER_100US);
	TimerManager::setTimePerInterruptUnits(TimerManager::getInstance(), kMS);
	TimerManager::setTimePerInterrupt(TimerManager::getInstance(), 10);

	SoundTest::applyTimerSettings(this);
	SoundTest::loadSound(this);
	SoundTest::printGUI(this, false);
	SoundTest::dimmGame(this);
}

/**
 * Hide editor
 */
void SoundTest::hide()
{
	SoundTest::releaseSoundWrapper(this);
	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager::recoverLayers(SpriteManager::getInstance());
	SoundTest::lightUpGame(this);
}

void SoundTest::printGUI(bool clearScreen)
{
	if(isDeleted(this->soundWrapper))
	{
		SoundTest::loadSound(this);
	}

	if(clearScreen)
	{
		Printing::clear(Printing::getInstance());
	}

	Printing::text(Printing::getInstance(), "\x08 SOUND TEST \x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08", 0, 0, NULL);

	if(NULL == _userSounds[this->selectedSound])
	{
		Printing::text(Printing::getInstance(), "No sounds found", 1, 4, NULL);
		Printing::text(Printing::getInstance(), "Define some in _userSounds global variable", 1, 6, NULL);
		return;
	}

	Printing::text(Printing::getInstance(), __CHAR_SELECTOR_LEFT, 1, 2, NULL);

	u16 totalSounds = SoundTest::getTotalSounds(this);

	int selectedSoundDigits = Utilities::getDigitCount(this->selectedSound);
	int totalSoundsDigits = Utilities::getDigitCount(totalSounds);
	Printing::int(Printing::getInstance(), this->selectedSound + 1, 1 + 1, 2, NULL);
	Printing::text(Printing::getInstance(), "/" , 1 + 1 + selectedSoundDigits, 2, NULL);
	Printing::int(Printing::getInstance(), SoundTest::getTotalSounds(this), 1 + 1 + selectedSoundDigits + 1, 2, NULL);
	Printing::text(Printing::getInstance(), __CHAR_SELECTOR, 1 + 1 + selectedSoundDigits + 1 + totalSoundsDigits, 2, NULL);

	if(isDeleted(this->soundWrapper))
	{
		return;
	}

	int xControls = 37;
	int yControls = 4;

	// Controls
	if(SoundWrapper::isPaused(this->soundWrapper))
	{
		Printing::text(Printing::getInstance(), "Play     \x13", xControls, yControls++, NULL);
	}
	else
	{
		Printing::text(Printing::getInstance(), "Pause    \x13", xControls, yControls++, NULL);
	}
	Printing::text(Printing::getInstance(), "Rewind   \x14", xControls, yControls++, NULL);
	Printing::text(Printing::getInstance(), "Track  \x1E\x1C\x1D", xControls, yControls++, NULL);
	Printing::text(Printing::getInstance(), "Speed  \x1E\x1A\x1B", xControls, yControls++, NULL);
	yControls++;
	Printing::text(Printing::getInstance(), "T.Freq. \x1F\x1A", xControls, yControls++, NULL);
	Printing::text(Printing::getInstance(), "T.Scale \x1F\x1B", xControls, yControls++, NULL);
	Printing::text(Printing::getInstance(), "T.Res. \x1F\x1C\x1D", xControls, yControls++, NULL);

	SoundTest::printTimer(this);
}

void SoundTest::processUserInput(u16 pressedKey)
{
	if(NULL == _userSounds[this->selectedSound])
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
		SoundTest::printGUI(this, false);

		if(isDeleted(this->soundWrapper))
		{
			SoundTest::loadSound(this);
		}

		if(SoundWrapper::isPaused(this->soundWrapper))
		{
			SoundWrapper::play(this->soundWrapper, NULL);
		}
		else
		{
			SoundWrapper::pause(this->soundWrapper);
		}

		SoundTest::printGUI(this, false);
	}
	else if(K_B & pressedKey)
	{
		if(isDeleted(this->soundWrapper))
		{
			SoundTest::loadSound(this);
		}

		SoundWrapper::rewind(this->soundWrapper);
	}
	else if(K_LD & pressedKey)
	{
		if(isDeleted(this->soundWrapper))
		{
			SoundTest::loadSound(this);
		}

		SoundWrapper::setSpeed(this->soundWrapper, SoundWrapper::getSpeed(this->soundWrapper) - __F_TO_FIX17_15(0.01f));
	}
	else if(K_LU & pressedKey)
	{
		if(isDeleted(this->soundWrapper))
		{
			SoundTest::loadSound(this);
		}

		SoundWrapper::setSpeed(this->soundWrapper, SoundWrapper::getSpeed(this->soundWrapper) +  __F_TO_FIX17_15(0.01f));
	}
	// Timer controls
	else if(K_RU & pressedKey)
	{
		u16 timerResolution = TimerManager::getResolution(TimerManager::getInstance());

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

		TimerManager::setResolution(TimerManager::getInstance(), timerResolution);
		timerChanged = true;
	}
	else if(K_RD & pressedKey)
	{
		u16 timePerInterruptUnits = TimerManager::getTimePerInterruptUnits(TimerManager::getInstance());
		u16 timePerInterrupt = TimerManager::getTimePerInterrupt(TimerManager::getInstance());

		switch(timePerInterruptUnits)
		{
			case kUS:

				timePerInterruptUnits = kMS;
				timePerInterrupt = 10;
				break;

			case kMS:

				timePerInterruptUnits = kUS;
				timePerInterrupt = 1000;
				break;

			default:

				ASSERT(false, "SoundTest::processUserInput: wrong timer resolution scale");
				break;
		}

		TimerManager::setTimePerInterruptUnits(TimerManager::getInstance(), timePerInterruptUnits);
		TimerManager::setTimePerInterrupt(TimerManager::getInstance(), timePerInterrupt);
		timerChanged = true;
	}
	else if(K_RL & pressedKey)
	{
		u16 timePerInterrupt = TimerManager::getTimePerInterrupt(TimerManager::getInstance());
		u16 timePerInterruptUnits = TimerManager::getTimePerInterruptUnits(TimerManager::getInstance());

		switch(timePerInterruptUnits)
		{
			case kUS:

				timePerInterrupt -= __MINIMUM_TIME_PER_INTERRUPT_US;
				break;

			case kMS:

				timePerInterrupt -= __MINIMUM_TIME_PER_INTERRUPT_MS;
				break;
		}

		TimerManager::setTimePerInterrupt(TimerManager::getInstance(), timePerInterrupt);
		timerChanged = true;
	}
	else if(K_RR & pressedKey)
	{
		u16 timePerInterrupt = TimerManager::getTimePerInterrupt(TimerManager::getInstance());
		u16 timePerInterruptUnits = TimerManager::getTimePerInterruptUnits(TimerManager::getInstance());

		switch(timePerInterruptUnits)
		{
			case kUS:

				timePerInterrupt += __MINIMUM_TIME_PER_INTERRUPT_US;
				break;

			case kMS:

				timePerInterrupt += __MINIMUM_TIME_PER_INTERRUPT_MS;
				break;
		}

		TimerManager::setTimePerInterrupt(TimerManager::getInstance(), timePerInterrupt);
		timerChanged = true;
	}

	if(timerChanged)
	{
		SoundTest::applyTimerSettings(this);

		SoundTest::printTimer(this);

		if(!isDeleted(this->soundWrapper))
		{
			SoundWrapper::pause(this->soundWrapper);
			SoundWrapper::rewind(this->soundWrapper);
			SoundWrapper::computeTimerResolutionFactor(this->soundWrapper);

			if(!SoundWrapper::isPaused(this->soundWrapper))
			{
				SoundWrapper::play(this->soundWrapper, NULL);
			}
		}
	}

	SoundWrapper::printMetadata(this->soundWrapper, 1, 4);
}

u16 SoundTest::getTotalSounds()
{
	u16 totalSounds = 0;

	for(; _userSounds[totalSounds]; totalSounds++);

	return totalSounds;
}

void SoundTest::loadPreviousSound()
{
	u16 totalSounds = SoundTest::getTotalSounds(this);

	if(0 == this->selectedSound)
	{
		this->selectedSound = totalSounds - 1;
	}
	else
	{
		this->selectedSound--;
	}

	SoundTest::printGUI(this, true);
	SoundTest::loadSound(this);
}
void SoundTest::loadNextSound()
{
	u16 totalSounds = SoundTest::getTotalSounds(this);

	if(totalSounds - 1 == this->selectedSound)
	{
		this->selectedSound = 0;
	}
	else
	{
		this->selectedSound++;
	}

	SoundTest::printGUI(this, true);
	SoundTest::loadSound(this);
}

void SoundTest::loadSound()
{
	if(NULL == _userSounds[this->selectedSound])
	{
		return;
	}

	Game::disableKeypad(Game::getInstance());

	SoundTest::releaseSoundWrapper(this);

	this->soundWrapper = SoundManager::getSound(SoundManager::getInstance(), (Sound*)_userSounds[this->selectedSound], true);

	NM_ASSERT(!isDeleted(this->soundWrapper), "SoundTest::loadSound: no sound");

	if(!isDeleted(this->soundWrapper))
	{
		SoundWrapper::printMetadata(this->soundWrapper, 1, 4);
		SoundWrapper::printVolume(this->soundWrapper, 1, 18);

		SoundWrapper::addEventListener(this->soundWrapper, Object::safeCast(this), (EventListener)SoundTest::onSoundFinish, kSoundFinished);
		SoundWrapper::addEventListener(this->soundWrapper, Object::safeCast(this), (EventListener)SoundTest::onSoundReleased, kSoundReleased);
	}

	Game::enableKeypad(Game::getInstance());
}

void SoundTest::onSoundFinish(Object eventFirer __attribute__((unused)))
{
}

void SoundTest::onSoundReleased(Object eventFirer __attribute__((unused)))
{
	this->soundWrapper = NULL;
}

void SoundTest::printTimer()
{
	if(NULL == _userSounds[this->selectedSound])
	{
		return;
	}

	TimerManager::print(TimerManager::getInstance(), 1, 11);
}

void SoundTest::applyTimerSettings()
{
	TimerManager::enable(TimerManager::getInstance(), false);
	TimerManager::initialize(TimerManager::getInstance());
	TimerManager::enable(TimerManager::getInstance(), true);


	SoundTest::printTimer(this);
}