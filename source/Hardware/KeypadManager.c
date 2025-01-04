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

#include <Printing.h>

#include "KeypadManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 volatile* _readingStatus = NULL;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::interruptHandler()
{
	KeypadManager::disableInterrupt(KeypadManager::getInstance());
	Printing::resetCoordinates(Printing::getInstance());
	Printing::text(Printing::getInstance(), "KYP interrupt", 48 - 13, 26, NULL);
	Printing::hex(Printing::getInstance(), (((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]), 48 - 13, 27, 8, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::reset()
{
	this->userInput.dummyKey = K_NON;

	KeypadManager::flush(this);

	this->reseted = true;
	this->enabled = false;
	this->accumulatedUserInput = 0;
	this->userInputToRegister = (UserInput){0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::enable()
{
	this->enabled = true;
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);
	KeypadManager::flush(this);
	this->reseted = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::disable()
{
	this->enabled = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 KeypadManager::isEnabled()
{
	return this->enabled;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

UserInput KeypadManager::readUserInput(bool waitForStableReading)
{
	if(!waitForStableReading)
	{
		if(*_readingStatus & __S_STAT)
		{
			return (UserInput){K_NON, K_NON, K_NON, K_NON, K_NON, K_NON, K_NON, K_NON};
		}
	}
	else
	{
		// wait for keypad to stabilize
		while(*_readingStatus & __S_STAT);
	}

	// now read the keys
	this->userInput.allKeys = (((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]);

	// enable next reading cycle
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);

	// store keys
	if(0 == this->userInput.powerFlag)
	{
		if(K_PWR == (this->userInput.allKeys & K_PWR))
		{
			KeypadManager::fireEvent(this, kEventKeypadManagerRaisedPowerFlag);

			this->userInput.powerFlag = this->userInput.allKeys & K_PWR;
		}
	}

	this->userInput.allKeys &= K_ANY;

	if(this->reseted)
	{
		this->userInput.pressedKey = 0;
		this->userInput.releasedKey = 0;
	}
	else
	{
		this->userInput.pressedKey = KeypadManager::getPressedKey(this) & this->userInputToRegister.pressedKey;
		this->userInput.releasedKey = KeypadManager::getReleasedKey(this) & this->userInputToRegister.releasedKey;
	}

	this->userInput.holdKey = KeypadManager::getHoldKey(this) & this->userInputToRegister.holdKey;
	this->userInput.previousKey = this->userInput.allKeys;
	this->userInput.holdKeyDuration = (this->userInput.holdKey && this->userInput.holdKey == this->userInput.previousKey)
		? this->userInput.holdKeyDuration + 1
		: 0;

	this->accumulatedUserInput += this->userInput.allKeys;

	this->reseted = false;

	return this->userInput;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::enableDummyKey()
{
	this->userInput.dummyKey = K_ANY;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::disableDummyKey()
{
	this->userInput.dummyKey = K_NON;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::registerInput(uint16 inputToRegister)
{
#ifdef __TOOLS
	inputToRegister = __KEY_PRESSED | __KEY_RELEASED | __KEY_HOLD;
#endif
	this->userInputToRegister.pressedKey = __KEY_PRESSED & inputToRegister? 0xFFFF : 0;
	this->userInputToRegister.releasedKey = __KEY_RELEASED & inputToRegister? 0xFFFF : 0;
	this->userInputToRegister.holdKey = __KEY_HOLD & inputToRegister? 0xFFFF : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

long KeypadManager::getAccumulatedUserInput()
{
	return this->accumulatedUserInput;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void KeypadManager::printUserInput(int32 x, int32 y)
{
	int32 xDisplacement = 13;

	PRINT_TEXT("USER INPUT:", x, y++);

	PRINT_TEXT("allKeys:", x, ++y);
	PRINT_HEX(this->userInput.allKeys, x + xDisplacement, y);

	PRINT_TEXT("pressedKey:", x, ++y);
	PRINT_HEX(this->userInput.pressedKey, x + xDisplacement, y);

	PRINT_TEXT("releasedKey:", x, ++y);
	PRINT_HEX(this->userInput.releasedKey, x + xDisplacement, y);

	PRINT_TEXT("holdKey:", x, ++y);
	PRINT_HEX(this->userInput.holdKey, x + xDisplacement, y);

	PRINT_TEXT("holdKeyDuration:", x, ++y);
	PRINT_HEX(this->userInput.holdKeyDuration, x + xDisplacement, y);

	PRINT_TEXT("powerFlag:", x, ++y);
	PRINT_HEX(this->userInput.powerFlag, x + xDisplacement, y);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	KeypadManager::reset(this);

	_readingStatus = (uint32 *)&_hardwareRegisters[__SCR];
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::destructor()
{
	// allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::enableInterrupt()
{
	KeypadManager::flush(this);

	_hardwareRegisters[__SCR] = 0;
	_hardwareRegisters[__SCR] &= ~(__S_HWDIS | __S_INTDIS);
	_hardwareRegisters[__SCR] |= __S_HW;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::disableInterrupt()
{
	_hardwareRegisters[__SCR] |= __S_INTDIS;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void KeypadManager::flush()
{
	this->userInput = (UserInput){K_NON, K_NON, K_NON, K_NON, K_NON, K_NON, K_NON, this->userInput.dummyKey};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 KeypadManager::getPressedKey()
{
	return this->userInput.allKeys & ~this->userInput.previousKey;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 KeypadManager::getReleasedKey()
{
	return ~this->userInput.allKeys & this->userInput.previousKey;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 KeypadManager::getHoldKey()
{
	return this->userInput.allKeys & this->userInput.previousKey;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16 KeypadManager::getPreviousKey()
{
	return this->userInput.previousKey;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
