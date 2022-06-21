/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SRAM_MANAGER_H_
#define SRAM_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// forward declare game's custom save data struct
struct SaveData;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class SRAMManager : ListenerObject
{
	// save space start address
	uint16* saveSpaceStartAddress;

	/// @publicsection
	static SRAMManager getInstance();

	void reset();
	void clear(int32 startOffset, int32 endOffset);
	void save(const BYTE* const source, int32 memberOffset, int32 dataSize);
	void read(BYTE* destination, int32 memberOffset, int32 dataSize);
}


#endif
