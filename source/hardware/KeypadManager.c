/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <KeypadManager.h>
#include <HardwareManager.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


#define KeypadManager_ATTRIBUTES										\
																		\
	/* super's attributes */											\
	Object_ATTRIBUTES;													\
																		\
	/*  */																\
	u16 currentKey;														\
																		\
	/*  */																\
	u16 previousKey;													\
	

// define the KeypadManager
__CLASS_DEFINITION(KeypadManager);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void KeypadManager_constructor(KeypadManager this);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */




//////////////////////////////////////////////////////////////////////////////////////////////////////////////

__SINGLETON(KeypadManager);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void KeypadManager_constructor(KeypadManager this){
	
	ASSERT(this, "KeypadManager::constructor: null this");

	__CONSTRUCT_BASE(Object);
	
	this->currentKey = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void KeypadManager_destructor(KeypadManager this){

	ASSERT(this, "KeypadManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enable keypad reads
void KeypadManager_enable(KeypadManager this){

	ASSERT(this, "KeypadManager::enable: null this");

	HW_REGS[SCR] = 0;
	HW_REGS[SCR] &= ~(S_HWDIS | S_INTDIS);
	HW_REGS[SCR] |= S_HW;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// disable keypad reads
void KeypadManager_disable(KeypadManager this){

	ASSERT(this, "KeypadManager::disable: null this");

	HW_REGS[SCR] = (S_INTDIS | S_HW);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read keypad
u16 KeypadManager_read(KeypadManager this){

	ASSERT(this, "KeypadManager::read: null this");

	KeypadManager_disable(this);
	
	unsigned int volatile *readingStatus =	(unsigned int *)&HW_REGS[SCR];
	
	//wait for screen to idle	
	while (*readingStatus & S_STAT);
	
	// now read the key
	this->previousKey = this->currentKey;
	this->currentKey = (((HW_REGS[SDHR] << 8)) | HW_REGS[SDLR]) & 0xFFFC;

	KeypadManager_enable(this);
	
	return this->currentKey;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get pressed key
u16 KeypadManager_getPressedKey(KeypadManager this){
	
	return this->currentKey & ~this->previousKey;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get released key
u16 KeypadManager_getReleasedKey(KeypadManager this){
	
	return this->currentKey != this->previousKey? this->previousKey & ~this->currentKey: 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get hold key
u16 KeypadManager_getHoldKey(KeypadManager this){
	
	return this->currentKey & this->previousKey? this->currentKey & this->previousKey: 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get previous key
u16 KeypadManager_getPreviousKey(KeypadManager this){
	
	return this->currentKey & this->previousKey? this->currentKey & this->previousKey: 0;
}
