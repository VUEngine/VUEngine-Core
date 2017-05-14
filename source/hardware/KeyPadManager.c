/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define KeypadManager_ATTRIBUTES																		\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/*  */																							\
		UserInput userInput;																			\
		UserInput userInputToRegister;																	\
		/*  */																							\
		bool enabled;																					\

/**
 * @class	KeypadManager
 * @extends Object
 * @ingroup hardware
 */
__CLASS_DEFINITION(KeypadManager, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void KeypadManager_constructor(KeypadManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

static unsigned int volatile* _readingStatus = NULL;

/**
 * Get instance
 *
 * @fn			KeypadManager_getInstance()
 * @memberof	KeypadManager
 * @public
 *
 * @return		KeypadManager instance
 */
__SINGLETON(KeypadManager);

/**
 * Class constructor
 *
 * @memberof	KeypadManager
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) KeypadManager_constructor(KeypadManager this)
{
	ASSERT(this, "KeypadManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	KeypadManager_flush(this);
	this->enabled = false;

	this->userInput = (UserInput){0, 0, 0, 0, 0, 0};
	this->userInputToRegister = (UserInput){0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

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
void KeypadManager_destructor(KeypadManager this)
{
	ASSERT(this, "KeypadManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Enable user input interrupts
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
void KeypadManager_enableInterrupt(KeypadManager this __attribute__ ((unused)))
{
	ASSERT(this, "KeypadManager::enable: null this");

	KeypadManager_flush(this);

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
void KeypadManager_disableInterrupt(KeypadManager this __attribute__ ((unused)))
{
	ASSERT(this, "KeypadManager::disable: null this");

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
void KeypadManager_enable(KeypadManager this)
{
	ASSERT(this, "KeypadManager::enable: null this");

	this->enabled = true;
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);

	KeypadManager_flush(this);
}

/**
 * Disable user input
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
void KeypadManager_disable(KeypadManager this)
{
	ASSERT(this, "KeypadManager::disable: null this");

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
int KeypadManager_isEnabled(KeypadManager this)
{
	ASSERT(this, "KeypadManager::disable: null this");

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
UserInput KeypadManager_read(KeypadManager this)
{
	ASSERT(this, "KeypadManager::read: null this");

	// wait keypad to stabilize
	while(*_readingStatus & __S_STAT);

	// now read the key
	this->userInput.allKeys = (((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]);

	// enable next reading cycle
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);

	this->userInput.powerFlag 	= this->userInput.allKeys & 0x0001;
	this->userInput.allKeys 	&= 0xFFFC;
	this->userInput.pressedKey 	= (this->userInput.allKeys & ~this->userInput.previousKey) & this->userInputToRegister.pressedKey;
	this->userInput.releasedKey = (~this->userInput.allKeys & this->userInput.previousKey) & this->userInputToRegister.releasedKey;
	this->userInput.holdKey 	= (this->userInput.allKeys & this->userInput.previousKey) & this->userInputToRegister.holdKey;
	this->userInput.previousKey = this->userInput.allKeys;

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
UserInput KeypadManager_getUserInput(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getUserInput: null this");

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
void KeypadManager_flush(KeypadManager this)
{
	ASSERT(this, "KeypadManager::flush: null this");

	this->userInput = (UserInput){0, 0, 0, 0, 0, 0};
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
u16 KeypadManager_getPressedKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getPressedKey: null this");

	return this->userInput.pressedKey;
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
u16 KeypadManager_getReleasedKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::read: null this");

	return this->userInput.releasedKey;
}

// get hold key
u16 KeypadManager_getHoldKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getHoldKey: null this");

	return this->userInput.holdKey;
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
u16 KeypadManager_getPreviousKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getPreviousKey: null this");

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
void KeypadManager_registerInput(KeypadManager this, u16 inputToRegister)
{
	ASSERT(this, "KeypadManager::registerInput: null this");

	this->userInputToRegister.pressedKey = __KEY_PRESSED & inputToRegister? 0xFFFF : 0;
	this->userInputToRegister.releasedKey = __KEY_RELEASED & inputToRegister? 0xFFFF : 0;
	this->userInputToRegister.holdKey = __KEY_HOLD & inputToRegister? 0xFFFF : 0;
}

/**
 * Interrupt handler
 *
 * @memberof	KeypadManager
 * @public
 *
 */
void KeypadManager_interruptHandler(void)
{
	Printing_resetWorldCoordinates(Printing_getInstance());
	Printing_text(Printing_getInstance(), "KYP interrupt", 48 - 13, 0, NULL);
}

