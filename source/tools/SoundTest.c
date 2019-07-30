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
#include <TimerManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

extern SoundROM* _userSounds[];


//---------------------------------------------------------------------------------------------------------
//												ENUMS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			SoundTest::getInstance()
 * @memberof	SoundTest
 * @public
 * @return		SoundTest instance
 */


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
	// Don't print at full speed to not affect PCM playback
	static u16 delay = __TARGET_FPS / 2;

	if(0 == --delay)
	{
		delay = __TARGET_FPS / 2;

		if(!isDeleted(this->soundWrapper))
		{
			SoundWrapper::printVolume(this->soundWrapper, 4, 11);
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
	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager::showLayer(SpriteManager::getInstance(), 0);

	TimerManager::setResolution(TimerManager::getInstance(), __TIMER_100US);
	TimerManager::setTimePerInterruptUnits(TimerManager::getInstance(), kMS);
	TimerManager::setTimePerInterrupt(TimerManager::getInstance(), 1);

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

	PRINT_TEXT("SOUND TEST", 19, 0);

	int xControls = 1;
	int yControls = 17;

	PRINT_TEXT("CONTROLS", xControls, yControls++);
	PRINT_TEXT("Track  : LU/LD", xControls, yControls++);

	if(SoundWrapper::isPaused(this->soundWrapper))
	{
		PRINT_TEXT("Play   : A", xControls, yControls++);
	}
	else
	{
		PRINT_TEXT("Pause  : A", xControls, yControls++);
	}

	PRINT_TEXT("Rewind : B", xControls, yControls++);

	PRINT_TEXT("Speed  : LL/LR", xControls, yControls++);

	++yControls;
	PRINT_TEXT("TIMER CONTROLS", xControls, yControls++);
	PRINT_TEXT("T Freq : RU", xControls, yControls++);
	PRINT_TEXT("T Scl  : RD", xControls, yControls++);
	PRINT_TEXT("T Res  : RL/RR", xControls, yControls++);

	SoundTest::printTimer(this);
}

void SoundTest::processUserInput(u16 pressedKey)
{
	bool timerChanged = false;

	// Track controls
	if(K_LU & pressedKey)
	{
		SoundTest::loadPreviousSound(this);
	}
	else if(K_LD & pressedKey)
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
		SoundWrapper::printMetadata(this->soundWrapper, 2, 3);
	}
	else if(K_B & pressedKey)
	{
		if(isDeleted(this->soundWrapper))
		{
			SoundTest::loadSound(this);
		}

		SoundWrapper::rewind(this->soundWrapper);
		SoundWrapper::printMetadata(this->soundWrapper, 2, 3);
	}
	else if(K_LL & pressedKey)
	{
		if(isDeleted(this->soundWrapper))
		{
			SoundTest::loadSound(this);
		}

		SoundWrapper::setSpeed(this->soundWrapper, SoundWrapper::getSpeed(this->soundWrapper) - __F_TO_FIX15_17(0.01f));
		SoundWrapper::printMetadata(this->soundWrapper, 2, 3);
	}
	else if(K_LR & pressedKey)
	{
		if(isDeleted(this->soundWrapper))
		{
			SoundTest::loadSound(this);
		}

		SoundWrapper::setSpeed(this->soundWrapper, SoundWrapper::getSpeed(this->soundWrapper) +  __F_TO_FIX15_17(0.01f));
		SoundWrapper::printMetadata(this->soundWrapper, 2, 3);
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
				timePerInterrupt = 1;
				break;

			case kMS:

				timePerInterruptUnits = kUS;
				timePerInterrupt = 100;
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

				timePerInterrupt -= 10;
				break;

			case kMS:

				timePerInterrupt -= 1;
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

				timePerInterrupt += 10;
				break;

			case kMS:

				timePerInterrupt += 1;
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
			SoundWrapper::play(this->soundWrapper, NULL);		
		}
	}
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
	Game::disableKeypad(Game::getInstance());

	SoundTest::releaseSoundWrapper(this);

	this->soundWrapper = SoundManager::getSound(SoundManager::getInstance(), (Sound*)_userSounds[this->selectedSound], true);

	NM_ASSERT(!isDeleted(this->soundWrapper), "SoundTest::loadSound: no sound");

	if(!isDeleted(this->soundWrapper))
	{
		SoundWrapper::printMetadata(this->soundWrapper, 2, 3);
		SoundWrapper::printVolume(this->soundWrapper, 4, 11);

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
	TimerManager::print(TimerManager::getInstance(), 25, 22);
}

void SoundTest::applyTimerSettings()
{
	TimerManager::enable(TimerManager::getInstance(), false);
	TimerManager::initialize(TimerManager::getInstance());
	TimerManager::enable(TimerManager::getInstance(), true);


	SoundTest::printTimer(this);
}