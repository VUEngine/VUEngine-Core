/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include "SRAMManager.h"


//=========================================================================================================
// CLASS' ATTRIBUTES
//=========================================================================================================

extern uint32 _sramBssEnd;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define	__SRAM_ACCESS_DELAY				200
#define	__SRAM_DUMMY_READ_CYCLES		8
#define	__SRAM_DUMMY_READ_LENGTH		100


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void SRAMManager::reset()
{
	// Dummy, don't remove
}
//---------------------------------------------------------------------------------------------------------
void SRAMManager::clear(int32 startOffset, int32 endOffset)
{
	int32 i = startOffset;
	for(; i < endOffset; i++)
	{
		this->spaceAddress[i] = 0;
	}
}
//---------------------------------------------------------------------------------------------------------
void SRAMManager::save(const BYTE* const source, int32 memberOffset, int32 dataSize)
{
	int32 i = 0;

	uint16* destination = this->spaceAddress + memberOffset;
	ASSERT(0 == ((int32)destination % 2), "SRAMManager::save: odd destination");

	for(; i < dataSize; i++)
	{
		destination[i] = source[i];
	}
}
//---------------------------------------------------------------------------------------------------------
void SRAMManager::read(BYTE* destination, int32 memberOffset, int32 dataSize)
{
	int32 i = 0;

	uint16* source = this->spaceAddress + memberOffset;
	ASSERT(0 == ((int32)source % 2), "SRAMManager::constructor: odd source");

	for(; i < dataSize; i++)
	{
		destination[i] = source[i] & 0x00FF;
	}
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void SRAMManager::constructor()
{
	Base::constructor();

	this->spaceAddress = (uint16*)&_sramBssEnd;

	SRAMManager::initialize(this);
}
//---------------------------------------------------------------------------------------------------------
void SRAMManager::destructor()
{
	// allow a new construct
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void SRAMManager::initialize()
{
	int32 i = __SRAM_DUMMY_READ_CYCLES;
	for(; i--;)
	{
		uint16 dummyChar[__SRAM_DUMMY_READ_LENGTH];
		SRAMManager::read(this, (BYTE*)&dummyChar, i, sizeof(dummyChar));
	}
}
//---------------------------------------------------------------------------------------------------------

