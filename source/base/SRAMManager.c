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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <SRAMManager.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define SAVE_RAM_ADDRESS	(u16*)0x06000000

const struct UserData* _userData = (void*)SAVE_RAM_ADDRESS;

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define SRAMManager_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\

// define the manager
__CLASS_DEFINITION(SRAMManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// extern
void SoundManager_playSounds(SoundManager this);

//class's constructor
static void SRAMManager_constructor(SRAMManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(SRAMManager);

// class's constructor
static void SRAMManager_constructor(SRAMManager this)
{
	__CONSTRUCT_BASE();
}


// class's destructor
void SRAMManager_destructor(SRAMManager this)
{
	ASSERT(this, "SRAMManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

void SRAMManager_save(SRAMManager this, const BYTE* const source, u16* memberAddress, int dataSize)
{
	int i = 0;
	
	u16* destination = (u16*)((int)_userData + ((int)memberAddress - (int)_userData) * 2);

	for(; i < dataSize; i++)
	{
		destination[i] = source[i];
	}
}

void SRAMManager_read(SRAMManager this, BYTE* destination, u16* memberAddress, int dataSize)
{
	int i = 0;

	u16* source = (u16*)((int)_userData + ((int)memberAddress - (int)_userData) * 2);
		
	for(; i < dataSize; i++)
	{
		destination[i] = source[i] & 0xFF;
	}
}

