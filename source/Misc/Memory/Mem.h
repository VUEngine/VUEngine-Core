/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MEM_H_
#define MEM_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __NUMBER_OF_COPIES_TO_ENABLE_CACHE		10

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Mem
///
/// Inherits from ListenerObject
///
/// Performs some memory copy and related procedures.
static class Mem : Object
{
	/// @publicsection

	/// Copy a block of BYTEs from the source memory address to the destination memory address
	/// @param destination: Starting destination address
	/// @param source: Starting source address
	/// @param numberOfBYTES: Total number of BYTEs to copy
	static inline void copyBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES);

	/// Copy a block of HWORDs from the source memory address to the destination memory address
	/// @param destination: Starting destination address
	/// @param source: Starting source address
	/// @param numberOfHWORDS: Total number of HWORDs to copy
	static inline void copyHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS);

	/// Copy a block of WORDs from the source memory address to the destination memory address
	/// @param destination: Starting destination address
	/// @param source: Starting source address
	/// @param numberOfWORDS: Total number of WORDs to copy
	static inline void copyWORD(WORD* destination, const WORD* source, uint32 numberOfWORDS);

	/// Apply a bitwise OR operation to the specified number of BYTEs from the provided source addresses and place the results 
	/// in the destination address onwards
	/// @param destination: Starting destination address
	/// @param source1: Starting first source address
	/// @param source2: Starting second source address
	/// @param numberOfBYTES: Total number of BYTEs to combine
	static inline void combineBYTEs(BYTE* destination, const BYTE* source1, const BYTE* source2, uint32 numberOfBYTES);

	/// Apply a bitwise OR operation to the specified number of HWORDs from the provided source addresses and place the results 
	/// in the destination address onwards
	/// @param destination: Starting destination address
	/// @param source1: Starting first source address
	/// @param source2: Starting second source address
	/// @param numberOfWORDS: Total number of HWORDs to combine
	static inline void combineHWORDs(HWORD* destination, const HWORD* source1, const HWORD* source2, uint32 numberOfWORDS);

	/// Apply a bitwise OR operation to the specified number of WORDs from the provided source addresses and place the results 
	/// in the destination address onwards
	/// @param destination: Starting destination address
	/// @param source1: Starting first source address
	/// @param source2: Starting second source address
	/// @param numberOfWORDS: Total number of WORDs to combine
	static inline void combineWORDs(WORD* destination, const WORD* source1, const WORD* source2, uint32 numberOfWORDS);

	/// Add the provided offset to the data from the provided memory source and place the results in the destination address onwards
	/// @param destination: Starting destination address
	/// @param source: Starting source address
	/// @param numberOfBYTES: Total number of BYTEs to combine
	/// @param offset: Value to add to the data in the source address
	static inline void addOffsetToBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES, uint32 offset);

	/// Add the provided offset to the data from the provided memory source and place the results in the destination address onwards
	/// @param destination: Starting destination address
	/// @param source: Starting source address
	/// @param numberOfHWORDS: Total number of HWORDs to combine
	/// @param offset: Value to add to the data in the source address
	static inline void addOffsetToHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset);

	/// Add the provided offset to the data from the provided memory source and place the results in the destination address onwards
	/// @param destination: Starting destination address
	/// @param source: Starting source address
	/// @param numberOfWORDS: Total number of WORDs to combine
	/// @param offset: Value to add to the data in the source address
	static inline void addOffsetToWORD(WORD* destination, const WORD* source, uint32 numberOfWORDS, uint32 offset);

	/// Write a determined number of zeros from the specified memory address onwards.
	/// @param destination: Starting address
	/// @param numberOfBYTES: Total number of zeros to write
	static void clear(BYTE* destination, uint32 numberOfBYTES);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Mem::copyBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES)
{
	for(; 0 < numberOfBYTES; numberOfBYTES--)
	{
		*destination++ = *source++;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Mem::copyHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS)
{
	for(; 0 < numberOfHWORDS; numberOfHWORDS--)
	{
		*destination++ = *source++;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Mem::copyWORD(WORD* destination, const WORD* source, uint32 numberOfWORDS)
{
	for(; 0 < numberOfWORDS; numberOfWORDS--)
	{
		*destination++ = *source++;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Mem::combineBYTEs(BYTE* destination, const BYTE* source1, const BYTE* source2, uint32 numberOfBYTES)
{
	for(; 0 < numberOfBYTES; numberOfBYTES--)
	{
		*destination++ = *source1++ | *source2++;
	}
}

static inline void Mem::combineHWORDs(HWORD* destination, const HWORD* source1, const HWORD* source2, uint32 numberOfHWORDS)
{
	for(; 0 < numberOfHWORDS; numberOfHWORDS--)
	{
		*destination++ = *source1++ | *source2++;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Mem::combineWORDs(WORD* destination, const WORD* source1, const WORD* source2, uint32 numberOfWORDS)
{
	for(; 0 < numberOfWORDS; numberOfWORDS--)
	{
		*destination++ = *source1++ | *source2++;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Mem::addOffsetToBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES, uint32 offset)
{
	for(; 0 < numberOfBYTES; numberOfBYTES--)
	{
		*destination++ = *source++ + offset;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Mem::addOffsetToHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset)
{
	for(; 0 < numberOfHWORDS; numberOfHWORDS--)
	{
		*destination++ = *source++ + offset;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline void Mem::addOffsetToWORD(WORD* destination, const WORD* source, uint32 numberOfWORDS, uint32 offset)
{
	for(; 0 < numberOfWORDS; numberOfWORDS--)
	{
		*destination++ = *source++ + offset;
	}
}

#endif
