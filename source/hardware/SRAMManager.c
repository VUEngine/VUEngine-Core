/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

extern uint32 _sramBssEnd;


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

	this->saveSpaceStartAddress = (uint16*)&_sramBssEnd;

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
 * Reset
 */
void SRAMManager::reset()
{
	// dummy, don't remove
}

/**
 * Initialize SRAM
 *
 * @private
 */
void SRAMManager::initialize()
{
	int32 i = __SRAM_DUMMY_READ_CYCLES;
	for(; i--;)
	{
		uint16 dummyChar[__SRAM_DUMMY_READ_LENGTH];
		SRAMManager::read(this, (BYTE*)&dummyChar, i, sizeof(dummyChar));
	}
}

/**
 * Delete all data in SRAM range
 *
 * @param startOffset		Start address of range to clear
 * @param endOffset			End address of range to clear
 */
void SRAMManager::clear(int32 startOffset, int32 endOffset)
{
	int32 i = startOffset;
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
void SRAMManager::save(const BYTE* const source, int32 memberOffset, int32 dataSize)
{
	int32 i = 0;

	uint16* destination = this->saveSpaceStartAddress + memberOffset;
	ASSERT(0 == ((int32)destination % 2), "SRAMManager::save: odd destination");

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
void SRAMManager::read(BYTE* destination, int32 memberOffset, int32 dataSize)
{
	int32 i = 0;

	uint16* source = this->saveSpaceStartAddress + memberOffset;
	ASSERT(0 == ((int32)source % 2), "SRAMManager::constructor: odd source");

	for(; i < dataSize; i++)
	{
		destination[i] = source[i] & 0x00FF;
	}
}
