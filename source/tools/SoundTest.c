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
}

/**
 * Show editor
 *
 * @param gameState Current game state
 */
void SoundTest::show()
{
	HardwareManager::setupTimer(HardwareManager::getInstance(), __TIMER_20US, __TIME_MS(1));
	VIPManager::clearBgmapSegment(VIPManager::getInstance(), BgmapTextureManager::getPrintingBgmapSegment(BgmapTextureManager::getInstance()), __PRINTABLE_BGMAP_AREA);
	SpriteManager::showLayer(SpriteManager::getInstance(), 0);

	SoundTest::loadSound(this);
	SoundTest::printGUI(this);
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

void SoundTest::printGUI()
{
	if(isDeleted(this->soundWrapper))
	{
		SoundTest::loadSound(this);
	}

	PRINT_TEXT("SOUND TEST", 19, 0);

	PRINT_TEXT("Track  : LU/LD", 1, 20);

	if(SoundWrapper::isPaused(this->soundWrapper))
	{
		PRINT_TEXT("Play   : A", 1, 21);
	}
	else
	{
		PRINT_TEXT("Pause  : A", 1, 21);
	}
	
	PRINT_TEXT("Rewind : B", 1, 22);
	PRINT_TEXT("Ticks  : RU/RD", 1, 23);
}

void SoundTest::processUserInput(u16 pressedKey)
{
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
		SoundTest::printGUI(this);

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
		
		SoundTest::printGUI(this);
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
	else if(K_RU & pressedKey)
	{
		if(isDeleted(this->soundWrapper))
		{
			SoundTest::loadSound(this);
		}

		SoundWrapper::setSpeed(this->soundWrapper, SoundWrapper::getSpeed(this->soundWrapper) + __F_TO_FIX15_17(0.01f));
		SoundWrapper::printMetadata(this->soundWrapper, 2, 3);
	}
	else if(K_RD & pressedKey)
	{
		if(isDeleted(this->soundWrapper))
		{
			SoundTest::loadSound(this);
		}

		SoundWrapper::setSpeed(this->soundWrapper, SoundWrapper::getSpeed(this->soundWrapper) -  __F_TO_FIX15_17(0.01f));
		SoundWrapper::printMetadata(this->soundWrapper, 2, 3);
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