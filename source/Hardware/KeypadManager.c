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

uint32 volatile* _readingStatus = NULL;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::interruptHandler()
{
	KeypadManager::disableInterrupt(KeypadManager::getInstance());
#ifndef __RELEASE
	Printing::resetCoordinates();
	Printing::text("KYP interrupt", 48 - 13, 26, NULL);
	Printing::hex((((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]), 48 - 13, 27, 8, NULL);
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::enable()
{
	KeypadManager keypadManager = KeypadManager::getInstance();

	keypadManager->enabled = true;
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);
	KeypadManager::flush(keypadManager);
	keypadManager->reseted = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::disable()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	keypadManager->enabled = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 KeypadManager::isEnabled()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	return keypadManager->enabled;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::enableDummyKey()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	keypadManager->userInput.dummyKey = K_ANY;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::disableDummyKey()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	keypadManager->userInput.dummyKey = K_NON;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::registerInput(uint16 inputToRegister)
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
#ifdef __TOOLS
	inputToRegister = __KEY_PRESSED | __KEY_RELEASED | __KEY_HOLD;
#endif
	keypadManager->userInputToRegister.pressedKey = __KEY_PRESSED & inputToRegister? 0xFFFF : 0;
	keypadManager->userInputToRegister.releasedKey = __KEY_RELEASED & inputToRegister? 0xFFFF : 0;
	keypadManager->userInputToRegister.holdKey = __KEY_HOLD & inputToRegister? 0xFFFF : 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static long KeypadManager::getAccumulatedUserInput()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	return keypadManager->accumulatedUserInput;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void KeypadManager::printUserInput(int32 x, int32 y)
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	int32 xDisplacement = 13;

	PRINT_TEXT("USER INPUT:", x, y++);

	PRINT_TEXT("allKeys:", x, ++y);
	PRINT_HEX(keypadManager->userInput.allKeys, x + xDisplacement, y);

	PRINT_TEXT("pressedKey:", x, ++y);
	PRINT_HEX(keypadManager->userInput.pressedKey, x + xDisplacement, y);

	PRINT_TEXT("releasedKey:", x, ++y);
	PRINT_HEX(keypadManager->userInput.releasedKey, x + xDisplacement, y);

	PRINT_TEXT("holdKey:", x, ++y);
	PRINT_HEX(keypadManager->userInput.holdKey, x + xDisplacement, y);

	PRINT_TEXT("holdKeyDuration:", x, ++y);
	PRINT_HEX(keypadManager->userInput.holdKeyDuration, x + xDisplacement, y);

	PRINT_TEXT("powerFlag:", x, ++y);
	PRINT_HEX(keypadManager->userInput.powerFlag, x + xDisplacement, y);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void KeypadManager::reset()
{	
	this->userInput.dummyKey = K_NON;

	KeypadManager::flush(this);

	this->reseted = true;
	this->enabled = false;
	this->accumulatedUserInput = 0;
	this->userInputToRegister = (UserInput){0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
}


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure UserInput KeypadManager::readUserInput(bool waitForStableReading)
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
		// Wait for keypad to stabilize
		while(*_readingStatus & __S_STAT);
	}

	// Now read the keys
	this->userInput.allKeys = (((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]);

	// Enable next reading cycle
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);

	// Store keys
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
	// Allow a new construct
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
