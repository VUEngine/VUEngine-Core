/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with keypadManager source code.
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
	KeypadManager::disableInterrupt();
	Printing::resetCoordinates();
	Printing::text("KYP interrupt", 48 - 13, 26, NULL);
	Printing::hex((((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]), 48 - 13, 27, 8, NULL);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::registerEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	KeypadManager keypadManager = KeypadManager::getInstance();

	KeypadManager::addEventListener(keypadManager, listener, callback, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::unregisterEventListener(ListenerObject listener, EventListener callback, uint16 eventCode)
{
	KeypadManager keypadManager = KeypadManager::getInstance();

	KeypadManager::removeEventListener(keypadManager, listener, callback, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::reset()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	keypadManager->userInput.dummyKey = K_NON;

	KeypadManager::flush(keypadManager);

	keypadManager->reseted = true;
	keypadManager->enabled = false;
	keypadManager->accumulatedUserInput = 0;
	keypadManager->userInputToRegister = (UserInput){0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
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

static int32 KeypadManager::isEnabled()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	return keypadManager->enabled;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static UserInput KeypadManager::readUserInput(bool waitForStableReading)
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
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
	keypadManager->userInput.allKeys = (((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]);

	// Enable next reading cycle
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);

	// Store keys
	if(0 == keypadManager->userInput.powerFlag)
	{
		if(K_PWR == (keypadManager->userInput.allKeys & K_PWR))
		{
			KeypadManager::fireEvent(keypadManager, kEventKeypadManagerRaisedPowerFlag);

			keypadManager->userInput.powerFlag = keypadManager->userInput.allKeys & K_PWR;
		}
	}

	keypadManager->userInput.allKeys &= K_ANY;

	if(keypadManager->reseted)
	{
		keypadManager->userInput.pressedKey = 0;
		keypadManager->userInput.releasedKey = 0;
	}
	else
	{
		keypadManager->userInput.pressedKey = KeypadManager::getPressedKey(keypadManager) & keypadManager->userInputToRegister.pressedKey;
		keypadManager->userInput.releasedKey = KeypadManager::getReleasedKey(keypadManager) & keypadManager->userInputToRegister.releasedKey;
	}

	keypadManager->userInput.holdKey = KeypadManager::getHoldKey(keypadManager) & keypadManager->userInputToRegister.holdKey;
	keypadManager->userInput.previousKey = keypadManager->userInput.allKeys;
	keypadManager->userInput.holdKeyDuration = (keypadManager->userInput.holdKey && keypadManager->userInput.holdKey == keypadManager->userInput.previousKey)
		? keypadManager->userInput.holdKeyDuration + 1
		: 0;

	keypadManager->accumulatedUserInput += keypadManager->userInput.allKeys;

	keypadManager->reseted = false;

	return keypadManager->userInput;
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
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
static void KeypadManager::enableInterrupt()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	KeypadManager::flush(keypadManager);

	_hardwareRegisters[__SCR] = 0;
	_hardwareRegisters[__SCR] &= ~(__S_HWDIS | __S_INTDIS);
	_hardwareRegisters[__SCR] |= __S_HW;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::disableInterrupt()
{
	_hardwareRegisters[__SCR] |= __S_INTDIS;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void KeypadManager::flush()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	keypadManager->userInput = (UserInput){K_NON, K_NON, K_NON, K_NON, K_NON, K_NON, K_NON, keypadManager->userInput.dummyKey};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 KeypadManager::getPressedKey()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	return keypadManager->userInput.allKeys & ~keypadManager->userInput.previousKey;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 KeypadManager::getReleasedKey()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	return ~keypadManager->userInput.allKeys & keypadManager->userInput.previousKey;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 KeypadManager::getHoldKey()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	return keypadManager->userInput.allKeys & keypadManager->userInput.previousKey;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint16 KeypadManager::getPreviousKey()
{
	KeypadManager keypadManager = KeypadManager::getInstance();
	
	return keypadManager->userInput.previousKey;
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

	KeypadManager::reset();

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

