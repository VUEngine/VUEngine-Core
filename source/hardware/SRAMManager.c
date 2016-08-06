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

#include <SRAMManager.h>
#include <Game.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define	__SRAM_ACCESS_DELAY				200
#define	__SRAM_DUMMY_READ_CYCLES		8
#define	__SRAM_DUMMY_READ_LENGHT		100

extern u32 _sram_bss_end;
const struct UserData* _userData = (void*)&_sram_bss_end;


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define SRAMManager_ATTRIBUTES																			\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\

// define the manager
__CLASS_DEFINITION(SRAMManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void SRAMManager_constructor(SRAMManager this);
void static SRAMManager_initialize(SRAMManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(SRAMManager);

// class's constructor
static void __attribute__ ((noinline)) SRAMManager_constructor(SRAMManager this)
{
	ASSERT(this, "SRAMManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	SRAMManager_initialize(this);
}

// class's destructor
void SRAMManager_destructor(SRAMManager this)
{
	ASSERT(this, "SRAMManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

void static SRAMManager_initialize(SRAMManager this)
{
	ASSERT(this, "SRAMManager::initialize: null this");

	int i = __SRAM_DUMMY_READ_CYCLES;
	for(; i--;)
	{
		u32 dummyChar[__SRAM_DUMMY_READ_LENGHT];
		SRAMManager_read(this, (BYTE*)&dummyChar, NULL, sizeof(dummyChar));
	}
}

void SRAMManager_save(SRAMManager this, const BYTE* const source, u16* memberDisplacement, int dataSize)
{
	ASSERT(this, "SRAMManager::save: null this");

	int i = 0;

	u16* destination = (u16*)((int)_userData + ((int)memberDisplacement << 1));
	ASSERT(0 == ((int)destination % 2), "SRAMManager::save: odd destination");
//	ASSERT(__SAVE_RAM_ADDRESS + 8192 > ((int)destination[dataSize - 1]), "SRAMManager::save: destination out of bounds");

	for(; i < dataSize; i++)
	{
		destination[i] = source[i];
	}
}

void SRAMManager_read(SRAMManager this, BYTE* destination, u16* memberDisplacement, int dataSize)
{
	ASSERT(this, "SRAMManager::read: null this");

	int i = 0;

	u16* source = (u16*)((int)_userData + ((int)memberDisplacement << 1));
	ASSERT(0 == ((int)source % 2), "SRAMManager::constructor: odd source");
//	ASSERT(__SAVE_RAM_ADDRESS + 8192 > ((int)source[dataSize - 1]), "SRAMManager::save: source out of bounds");

	for(; i < dataSize; i++)
	{
		destination[i] = source[i] & 0xFF;
	}
}
