/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SRAM_MANAGER_H_
#define SRAM_MANAGER_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Forward declaration of a struct that each game has to define
struct SaveData;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class SRAMManager
///
/// Inherits from Object
///
/// Manages SRAM space since most carts requiere a proxy to address 
/// by not having all the pins routed.
singleton class SRAMManager : Object
{
	/// @protectedsection

	/// SRAM start address
	uint16* spaceAddress;

	/// @publicsection
	/// Method to retrieve the singleton instance
	/// @return SRAMManager singleton
	static SRAMManager getInstance();

	/// Reset the manager's state.
	void reset();

	/// Delete all data in SRAM in the provided range.
	/// @param startOffset: Start offset of range to clear
	/// @param endOffset: End address of range to clear
	void clear(int32 startOffset, int32 endOffset);

	/// Save data to SRAM.
	/// @param source			WRAM address from were data will be copied
	/// @param memberOffset		WRAM address offset
	/// @param dataSize			Number of BYTES to read
	void save(const BYTE* const source, int32 memberOffset, int32 dataSize);

	/// Retrieve data from SRAM.
	/// @param destination		WRAM address were data will be loaded
	/// @param memberOffset		WRAM address offset
	/// @param dataSize			Number of BYTES to read
	void read(BYTE* destination, int32 memberOffset, int32 dataSize);
}


#endif
