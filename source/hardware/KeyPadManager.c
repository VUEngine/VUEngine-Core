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

#include <KeyPadManager.h>
#include <HardwareManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	KeypadManager
 * @extends Object
 * @ingroup hardware
 */


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

static unsigned int volatile* _readingStatus = NULL;

/**
 * Get instance
 *
 * @fn			KeypadManager::getInstance()
 * @memberof	KeypadManager
 * @public
 *
 * @return		KeypadManager instance
 */


/**
 * Class constructor
 *
 * @memberof	KeypadManager
 * @private
 *
 * @param this	Function scope
 */
void KeypadManager::constructor()
{
	Base::constructor();

	KeypadManager::flush(this);
	this->enabled = false;

	this->userInput = (UserInput){0, 0, 0, 0, 0, 0, 0};
	this->userInputToRegister = (UserInput){0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

	_readingStatus = (unsigned int *)&_hardwareRegisters[__SCR];
}

/**
 * Class destructor
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
void KeypadManager::destructor()
{
	// allow a new construct
	Base::destructor();
}

/**
 * Enable user input interrupts
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
void KeypadManager::enableInterrupt()
{
	KeypadManager::flush(this);

	_hardwareRegisters[__SCR] = 0;
	_hardwareRegisters[__SCR] &= ~(__S_HWDIS | __S_INTDIS);
//	_hardwareRegisters[__SCR] |= __S_HW;
}

/**
 * Disable user input interrupts
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
void KeypadManager::disableInterrupt()
{
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);
}

/**
 * Enable user input
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
void KeypadManager::enable()
{
	this->enabled = true;
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);

	KeypadManager::flush(this);
}

/**
 * Disable user input
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
void KeypadManager::disable()
{
	this->enabled = false;
}

/**
 * Check if user input is enabled
 *
 * @memberof		KeypadManager
 * @public
 *
 * @param this		Function scope
 *
 * @return			True if user input is enabled
 */
int KeypadManager::isEnabled()
{
	return this->enabled;
}

/**
 * Read user input
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
UserInput KeypadManager::read()
{
	// wait for keypad to stabilize
	while(*_readingStatus & __S_STAT);

	// now read the keys
	this->userInput.allKeys = (((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]);

	// enable next reading cycle
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);

	// store keys
	this->userInput.powerFlag 	= this->userInput.allKeys & 0x0001;
	this->userInput.allKeys 	&= 0xFFFC;
	this->userInput.pressedKey 	= KeypadManager::getPressedKey(this) & this->userInputToRegister.pressedKey;
	this->userInput.releasedKey = KeypadManager::getReleasedKey(this) & this->userInputToRegister.releasedKey;
	this->userInput.holdKey 	= KeypadManager::getHoldKey(this) & this->userInputToRegister.holdKey;
	this->userInput.previousKey = this->userInput.allKeys;
	this->userInput.holdKeyDuration = (this->userInput.holdKey == this->userInput.previousKey)
		? this->userInput.holdKeyDuration + 1
		: 0;

	return this->userInput;
}

/**
 * Retrieve user input
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 *
 * @return		User input
 */
UserInput KeypadManager::getUserInput()
{
	return this->userInput;
}

/**
 * Clear any user input previously registered
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
void KeypadManager::flush()
{
	this->userInput = (UserInput){0, 0, 0, 0, 0, 0, 0};
}

/**
 * Retrieve the current pressed keys
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Currently pressed keys
 */
u16 KeypadManager::getPressedKey()
{
	return this->userInput.allKeys & ~this->userInput.previousKey;
}

/**
 * Retrieve the current released keys
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Currently released keys
 */
u16 KeypadManager::getReleasedKey()
{
	return ~this->userInput.allKeys & this->userInput.previousKey;
}

/**
 * Retrieves the currently held key(s)
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Currently held keys
 */
u16 KeypadManager::getHoldKey()
{
	return this->userInput.allKeys & this->userInput.previousKey;
}

/**
 * Retrieves the duration (in game frames) for which the current key(s) have been held.
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Duration of currently held keys
 */
u32 KeypadManager::getHoldKeyDuration()
{
	return this->userInput.holdKeyDuration;
}

/**
 * Retrieve the previously pressed keys
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 *
 * @return 		Previously pressed keys
 */
u16 KeypadManager::getPreviousKey()
{
	return this->userInput.previousKey;
}

/**
 * Set the user input to register
 *
 * @memberof				KeypadManager
 * @public
 *
 * @param this				Function scope
 * @param inputToRegister	Flag
 */
void KeypadManager::registerInput(u16 inputToRegister)
{
	this->userInputToRegister.pressedKey = __KEY_PRESSED & inputToRegister? 0xFFFF : 0;
	this->userInputToRegister.releasedKey = __KEY_RELEASED & inputToRegister? 0xFFFF : 0;
	this->userInputToRegister.holdKey = __KEY_HOLD & inputToRegister? 0xFFFF : 0;
}

/**
 * Interrupt handler
 *
 * @memberof	KeypadManager
 * @public
 */
static void KeypadManager::interruptHandler()
{
	Printing::resetWorldCoordinates(Printing::getInstance());
	Printing::text(Printing::getInstance(), "KYP interrupt", 48 - 13, 0, NULL);
}

