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

#include <SRAMManager.h>
#include <Game.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define	__SRAM_ACCESS_DELAY				200
#define	__SRAM_DUMMY_READ_CYCLES		8
#define	__SRAM_DUMMY_READ_LENGTH		100

extern u32 _sram_bss_end;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	SRAMManager
 * @extends Object
 * @ingroup hardware
 */



//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void SRAMManager::constructor(SRAMManager this);
static void SRAMManager::initialize(SRAMManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			SRAMManager::getInstance()
 * @memberof	SRAMManager
 * @public
 *
 * @return		SRAMManager instance
 */
__SINGLETON(SRAMManager);

/**
 * Class constructor
 *
 * @memberof	SRAMManager
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) SRAMManager::constructor(SRAMManager this)
{
	ASSERT(this, "SRAMManager::constructor: null this");

	Base::constructor();

	this->saveSpaceStartAddress = (u16*)&_sram_bss_end;

	SRAMManager::initialize(this);
}

/**
 * Class destructor
 *
 * @memberof	SRAMManager
 * @public
 *
 * @param this	Function scope
 */
void SRAMManager::destructor(SRAMManager this)
{
	ASSERT(this, "SRAMManager::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Initialize SRAM
 *
 * @memberof	SRAMManager
 * @private
 *
 * @param this	Function scope
 */
static void SRAMManager::initialize(SRAMManager this)
{
	ASSERT(this, "SRAMManager::initialize: null this");

	int i = __SRAM_DUMMY_READ_CYCLES;
	for(; i--;)
	{
		u16 dummyChar[__SRAM_DUMMY_READ_LENGTH];
		SRAMManager::read(this, (BYTE*)&dummyChar, i, sizeof(dummyChar));
	}
}

/**
 * Delete all data in SRAM range
 *
 * @memberof				SRAMManager
 * @public
 *
 * @param this				Function scope
 * @param startOffset		Start address of range to clear
 * @param endOffset			End address of range to clear
 */
void SRAMManager::clear(SRAMManager this, int startOffset, int endOffset)
{
	ASSERT(this, "SRAMManager::clear: null this");

	int i = startOffset;
	for(; i < endOffset; i++)
	{
		this->saveSpaceStartAddress[i] = 0;
	}
}

/**
 * Save data from SRAM
 *
 * @memberof				SRAMManager
 * @public
 *
 * @param this				Function scope
 * @param source			WRAM address from were data will be copied
 * @param memberOffset		WRAM address offset
 * @param dataSize			Number of BYTES to read
 */
void SRAMManager::save(SRAMManager this, const BYTE* const source, int memberOffset, int dataSize)
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

/**
 * Retrieve data from SRAM
 *
 * @memberof				SRAMManager
 * @public
 *
 * @param this				Function scope
 * @param destination		WRAM address were data will be loaded
 * @param memberOffset		WRAM address offset
 * @param dataSize			Number of BYTES to read
 */
void SRAMManager::read(SRAMManager this, BYTE* destination, int memberOffset, int dataSize)
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
