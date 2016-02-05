/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <KeypadManager.h>
#include <HardwareManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define KeypadManager_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/*  */																		\
	u16 currentKey;																\
																				\
	/*  */																		\
	u16 previousKey;															\
																				\
	/*  */																		\
	u8 enabled;																	\

// define the KeypadManager
__CLASS_DEFINITION(KeypadManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void KeypadManager_constructor(KeypadManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

static unsigned int volatile* readingStatus = NULL;

__SINGLETON(KeypadManager);

// class's constructor
static void KeypadManager_constructor(KeypadManager this)
{
	ASSERT(this, "KeypadManager::constructor: null this");

	__CONSTRUCT_BASE();

	this->currentKey = 0;
	this->previousKey = 0;
	this->enabled = false;

	readingStatus = (unsigned int *)&HW_REGS[SCR];
}

// class's destructor
void KeypadManager_destructor(KeypadManager this)
{
	ASSERT(this, "KeypadManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// enable keypad reads
void KeypadManager_enableInterrupt(KeypadManager this)
{
	ASSERT(this, "KeypadManager::enable: null this");

	HW_REGS[SCR] = 0;
	HW_REGS[SCR] &= ~(S_HWDIS | S_INTDIS);
//	HW_REGS[SCR] |= S_HW;
}

// disable keypad reads
void KeypadManager_disableInterrupt(KeypadManager this)
{
	ASSERT(this, "KeypadManager::disable: null this");

	HW_REGS[SCR] = (S_INTDIS | S_HW);
}

// enable keypad reads
void KeypadManager_enable(KeypadManager this)
{
	ASSERT(this, "KeypadManager::enable: null this");

	this->enabled = true;
}

// disable keypad reads
void KeypadManager_disable(KeypadManager this)
{
	ASSERT(this, "KeypadManager::disable: null this");

	this->enabled = false;
}

// get status
int KeypadManager_isEnabled(KeypadManager this)
{
	ASSERT(this, "KeypadManager::disable: null this");

	return this->enabled;
}

// read keypad
void KeypadManager_read(KeypadManager this)
{
	ASSERT(this, "KeypadManager::read: null this");

	// disable interrupt
	HW_REGS[SCR] = (S_INTDIS | S_HW);

	//wait for screen to idle
	while(*readingStatus & S_STAT);

	// now read the key
	this->currentKey |= (((HW_REGS[SDHR] << 8)) | HW_REGS[SDLR]) & 0xFFFD;
}

// clear previous saved key
void KeypadManager_clear(KeypadManager this)
{
	ASSERT(this, "KeypadManager::clear: null this");

	this->previousKey = this->currentKey;
	this->currentKey = 0;
}

// clear previous saved keys
void KeypadManager_flush(KeypadManager this)
{
	ASSERT(this, "KeypadManager::flush: null this");

	this->currentKey = 0;
	this->previousKey = 0;
}

// get pressed key
u16 KeypadManager_getPressedKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getPressedKey: null this");

	return this->currentKey & ~this->previousKey;
}

// get released key
u16 KeypadManager_getReleasedKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::read: null this");

	return this->previousKey & ~this->currentKey;
}

// get hold key
u16 KeypadManager_getHoldKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getHoldKey: null this");

	return this->currentKey & this->previousKey;
}

// get previous key
u16 KeypadManager_getPreviousKey(KeypadManager this)
{
	ASSERT(this, "KeypadManager::getPreviousKey: null this");

	return this->previousKey;
	//return this->currentKey & this->previousKey ? this->currentKey & this->previousKey : 0;
}

// keypad's interrupt handler
void KeypadManager_interruptHandler(void)
{
	// broadcast keypad event
	Printing_text(Printing_getInstance(), "KYP interrupt", 48 - 13, 0, NULL);
}
