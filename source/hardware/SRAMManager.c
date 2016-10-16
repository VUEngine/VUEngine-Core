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
#define	__SRAM_DUMMY_READ_LENGTH		100

extern u32 _sram_bss_end;


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define SRAMManager_ATTRIBUTES																			\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* save space start address */																	\
        u16* saveSpaceStartAddress;																	    \

// define the manager
__CLASS_DEFINITION(SRAMManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void SRAMManager_constructor(SRAMManager this);
static void SRAMManager_initialize(SRAMManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(SRAMManager);

// class's constructor
static void __attribute__ ((noinline)) SRAMManager_constructor(SRAMManager this)
{
	ASSERT(this, "SRAMManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

    this->saveSpaceStartAddress = (u16*)&_sram_bss_end;

	SRAMManager_initialize(this);
}

// class's destructor
void SRAMManager_destructor(SRAMManager this)
{
	ASSERT(this, "SRAMManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

static void SRAMManager_initialize(SRAMManager this)
{
	ASSERT(this, "SRAMManager::initialize: null this");

	int i = __SRAM_DUMMY_READ_CYCLES;
	for(; i--;)
	{
		u16 dummyChar[__SRAM_DUMMY_READ_LENGTH];
		SRAMManager_read(this, (BYTE*)&dummyChar, i, sizeof(dummyChar));
	}
}

void SRAMManager_clear(SRAMManager this)
{
	ASSERT(this, "SRAMManager::clear: null this");

	int i = 0;

	// TODO: max value should not be hardcoded
	for(; i < 8192; i++)
	{
		this->saveSpaceStartAddress[i] = 0;
	}
}

void SRAMManager_save(SRAMManager this, const BYTE* const source, int memberOffset, int dataSize)
{
	ASSERT(this, "SRAMManager::save: null this");

	int i = 0;

	u16* destination = this->saveSpaceStartAddress + memberOffset;
	ASSERT(0 == ((int)destination % 2), "SRAMManager::save: odd destination");

	for(; i < dataSize; i++)
	{
		destination[i] = source[i];
	}
}

void SRAMManager_read(SRAMManager this, BYTE* destination, int memberOffset, int dataSize)
{
	ASSERT(this, "SRAMManager::read: null this");

	int i = 0;

	u16* source = this->saveSpaceStartAddress + memberOffset;
	ASSERT(0 == ((int)source % 2), "SRAMManager::constructor: odd source");

	for(; i < dataSize; i++)
	{
		destination[i] = source[i] & 0x00FF;
	}
}
