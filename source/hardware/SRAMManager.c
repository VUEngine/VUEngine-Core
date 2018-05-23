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
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			SRAMManager::getInstance()
 * @memberof	SRAMManager
 * @public
 * @return		SRAMManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void SRAMManager::constructor()
{
	Base::constructor();

	this->saveSpaceStartAddress = (u16*)&_sram_bss_end;

	SRAMManager::initialize(this);
}

/**
 * Class destructor
 */
void SRAMManager::destructor()
{
	// allow a new construct
	Base::destructor();
}

/**
 * Initialize SRAM
 *
 * @private
 */
void SRAMManager::initialize()
{
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
 * @param startOffset		Start address of range to clear
 * @param endOffset			End address of range to clear
 */
void SRAMManager::clear(int startOffset, int endOffset)
{
	int i = startOffset;
	for(; i < endOffset; i++)
	{
		this->saveSpaceStartAddress[i] = 0;
	}
}

/**
 * Save data from SRAM
 *
 * @param source			WRAM address from were data will be copied
 * @param memberOffset		WRAM address offset
 * @param dataSize			Number of BYTES to read
 */
void SRAMManager::save(const BYTE* const source, int memberOffset, int dataSize)
{
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
 * @param destination		WRAM address were data will be loaded
 * @param memberOffset		WRAM address offset
 * @param dataSize			Number of BYTES to read
 */
void SRAMManager::read(BYTE* destination, int memberOffset, int dataSize)
{
	int i = 0;

	u16* source = this->saveSpaceStartAddress + memberOffset;
	ASSERT(0 == ((int)source % 2), "SRAMManager::constructor: odd source");

	for(; i < dataSize; i++)
	{
		destination[i] = source[i] & 0x00FF;
	}
}
