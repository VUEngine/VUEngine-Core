/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <KeypadManager.h>
#include <HardwareManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

static uint32 volatile* _readingStatus = NULL;

/**
 * Get instance
 *
 * @fn			KeypadManager::getInstance()
 * @memberof	KeypadManager
 * @public
 * @return		KeypadManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void KeypadManager::constructor()
{
	Base::constructor();

	KeypadManager::reset(this);

	_readingStatus = (uint32 *)&_hardwareRegisters[__SCR];
}

/**
 * Class destructor
 */
void KeypadManager::destructor()
{
	// allow a new construct
	Base::destructor();
}

/**
 * Enable user input interrupts
 */
void KeypadManager::enableInterrupt()
{
	KeypadManager::flush(this);

	_hardwareRegisters[__SCR] = 0;
	_hardwareRegisters[__SCR] &= ~(__S_HWDIS | __S_INTDIS);
	_hardwareRegisters[__SCR] |= __S_HW;
}

/**
 * Disable user input interrupts
 */
void KeypadManager::disableInterrupt()
{
	_hardwareRegisters[__SCR] |= __S_INTDIS;
}

/**
 * Enable user input
 */
void KeypadManager::enable()
{
	this->enabled = true;
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);
	KeypadManager::flush(this);
}

/**
 * Disable user input
 */
void KeypadManager::disable()
{
	this->enabled = false;
}

/**
 * Check if user input is enabled
 *
 * @return			True if user input is enabled
 */
int32 KeypadManager::isEnabled()
{
	return this->enabled;
}

/**
 * Read user input
 */
UserInput KeypadManager::captureUserInput()
{
	// wait for keypad to stabilize
	while(*_readingStatus & __S_STAT);

	// now read the keys
	this->userInput.allKeys = (((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]);

	// enable next reading cycle
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);

	// store keys
	if(0 == this->userInput.powerFlag)
	{
		if(this->userInput.allKeys & K_PWR)
		{
			KeypadManager::fireEvent(this, kEventKeypadManagerRaisedPowerFlag);

			this->userInput.powerFlag 	= this->userInput.allKeys & K_PWR;
		}
	}

	this->userInput.allKeys 	&= K_ANY;

	if(this->reseted)
	{
		this->userInput.pressedKey 	= 0;
		this->userInput.releasedKey = 0;
	}
	else
	{
		this->userInput.pressedKey 	= KeypadManager::getPressedKey(this) & this->userInputToRegister.pressedKey;
		this->userInput.releasedKey = KeypadManager::getReleasedKey(this) & this->userInputToRegister.releasedKey;
	}

	this->userInput.holdKey 	= KeypadManager::getHoldKey(this) & this->userInputToRegister.holdKey;
	this->userInput.previousKey = this->userInput.allKeys;
	this->userInput.holdKeyDuration = (this->userInput.holdKey && this->userInput.holdKey == this->userInput.previousKey)
		? this->userInput.holdKeyDuration + 1
		: 0;

	this->accumulatedUserInput += this->userInput.allKeys;

	this->reseted = false;

	return this->userInput;
}

/**
 * Retrieve user input
 *
 * @return		User input
 */
UserInput KeypadManager::getUserInput()
{
	return this->userInput;
}

/**
 * Clear any user input previously registered
 */
void KeypadManager::flush()
{
	this->userInput = (UserInput){0, 0, 0, 0, 0, 0, 0};
}

/**
 * Retrieve the current pressed keys
 *
 * @return 		Currently pressed keys
 */
uint16 KeypadManager::getPressedKey()
{
	return this->userInput.allKeys & ~this->userInput.previousKey;
}

/**
 * Retrieve the current released keys
 *
 * @return 		Currently released keys
 */
uint16 KeypadManager::getReleasedKey()
{
	return ~this->userInput.allKeys & this->userInput.previousKey;
}

/**
 * Retrieves the currently held key(s)
 *
 * @return 		Currently held keys
 */
uint16 KeypadManager::getHoldKey()
{
	return this->userInput.allKeys & this->userInput.previousKey;
}

/**
 * Retrieves the duration (in game frames) for which the current key(s) have been held.
 *
 * @return 		Duration of currently held keys
 */
uint32 KeypadManager::getHoldKeyDuration()
{
	return this->userInput.holdKeyDuration;
}

/**
 * Retrieve the previously pressed keys
 *
 * @return 		Previously pressed keys
 */
uint16 KeypadManager::getPreviousKey()
{
	return this->userInput.previousKey;
}

/**
 * Reset
 *
 */
void KeypadManager::reset()
{
	KeypadManager::flush(this);

	this->reseted = true;
	this->enabled = false;
	this->accumulatedUserInput = 0;
	this->userInputToRegister = (UserInput){0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
}

/**
 * Set the user input to register
 *
 * @param inputToRegister	Flag
 */
void KeypadManager::registerInput(uint16 inputToRegister)
{
	this->userInputToRegister.pressedKey = __KEY_PRESSED & inputToRegister? 0xFFFF : 0;
	this->userInputToRegister.releasedKey = __KEY_RELEASED & inputToRegister? 0xFFFF : 0;
	this->userInputToRegister.holdKey = __KEY_HOLD & inputToRegister? 0xFFFF : 0;
}

/**
 * Retrieve the total user input so far
 *
 * @return Accumulated user input
 */
long KeypadManager::getAccumulatedUserInput()
{
	return this->accumulatedUserInput;
}


/**
 * Interrupt handler
 */
static void KeypadManager::interruptHandler()
{
	KeypadManager::disableInterrupt(KeypadManager::getInstance());
	Printing::resetCoordinates(Printing::getInstance());
	Printing::text(Printing::getInstance(), "KYP interrupt", 48 - 13, 26, NULL);
	Printing::hex(Printing::getInstance(), (((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]), 48 - 13, 27, 8, NULL);
}

static void KeypadManager::printUserInput(const UserInput* userInput, int32 x, int32 y)
{
	if(!userInput)
	{
		return;
	}

	int32 xDisplacement = 13;

	PRINT_TEXT("USER INPUT:", x, y++);

	PRINT_TEXT("allKeys:", x, ++y);
	PRINT_HEX(userInput->allKeys, x + xDisplacement, y);

	PRINT_TEXT("pressedKey:", x, ++y);
	PRINT_HEX(userInput->pressedKey, x + xDisplacement, y);

	PRINT_TEXT("releasedKey:", x, ++y);
	PRINT_HEX(userInput->releasedKey, x + xDisplacement, y);

	PRINT_TEXT("holdKey:", x, ++y);
	PRINT_HEX(userInput->holdKey, x + xDisplacement, y);

	PRINT_TEXT("holdKeyDuration:", x, ++y);
	PRINT_HEX(userInput->holdKeyDuration, x + xDisplacement, y);

	PRINT_TEXT("powerFlag:", x, ++y);
	PRINT_HEX(userInput->powerFlag, x + xDisplacement, y);
}

