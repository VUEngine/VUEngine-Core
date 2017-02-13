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

#include <KeypadManager.h>
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
		u32 currentKey;																					\
		/*  */																							\
		u32 previousKey;																				\
		/*  */																							\
		u32 enabled;																					\

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

	this->currentKey = 0;
	this->previousKey = 0;
	this->enabled = false;

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
	this->currentKey = this->previousKey = 0;
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);
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
void KeypadManager_read(KeypadManager this)
{
	ASSERT(this, "KeypadManager::read: null this");

	// wait for screen to idle
	while(*_readingStatus & __S_STAT);

	// now read the key
	this->currentKey |= (((_hardwareRegisters[__SDHR] << 8)) | _hardwareRegisters[__SDLR]) & 0x0000FFFD;

	// enable next reading cycle
	_hardwareRegisters[__SCR] = (__S_INTDIS | __S_HW);
}

/**
 * Clear the current user input
 *
 * @memberof	KeypadManager
 * @public
 *
 * @param this	Function scope
 */
void KeypadManager_clear(KeypadManager this)
{
	ASSERT(this, "KeypadManager::clear: null this");

	this->previousKey = this->currentKey;
	this->currentKey = 0;
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

	this->currentKey = 0;
	this->previousKey = 0;
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
u32 KeypadManager_getPressedKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getPressedKey: null this");

	return this->currentKey & ~this->previousKey;
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
u32 KeypadManager_getReleasedKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::read: null this");

	return this->previousKey & ~this->currentKey;
}

// get hold key
u32 KeypadManager_getHoldKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getHoldKey: null this");

	return this->currentKey & this->previousKey;
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
u32 KeypadManager_getPreviousKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getPreviousKey: null this");

	return this->previousKey;
	//return this->currentKey & this->previousKey ? this->currentKey & this->previousKey : 0;
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
	// broadcast keypad event
	Printing_text(Printing_getInstance(), "KYP interrupt", 48 - 13, 0, NULL);
}
