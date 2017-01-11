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

void SRAMManager_clear(SRAMManager this, int startOffset, int endOffset)
{
	ASSERT(this, "SRAMManager::clear: null this");

	int i = startOffset;
	for(; i < endOffset; i++)
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
