/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef MEM_H_
#define MEM_H_


//---------------------------------------------------------------------------------------------------------
//											 INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __NUMBER_OF_COPIES_TO_ENABLE_CACHE		10


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Mem : Object
{
	/// @publicsection
	static void clear(BYTE* destination, uint32 numberOfBYTES);
	static inline void copyBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES);
	static inline void copyHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS);
	static inline void copyWORD(WORD* destination, const WORD* source, uint32 numberOfWORDS);
	static inline void combineBYTEs(BYTE* destination, const BYTE* source1, const BYTE* source2, uint32 numberOfBYTES);
	static inline void combineHWORDs(HWORD* destination, const HWORD* source1, const HWORD* source2, uint32 numberOfWORDS);
	static inline void combineWORDs(WORD* destination, const WORD* source1, const WORD* source2, uint32 numberOfWORDS);
	static inline void addOffsetToBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES, uint32 offset);
	static inline void addOffsetToHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset);
	static inline void addOffsetToWORD(WORD* destination, const WORD* source, uint32 numberOfWORDS, uint32 offset);
}

// TODO: input registers should not be modified according to GCC's docs

// Copy a block of data from one area in memory to another.
static inline void Mem::copyBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES)
{
	for(; 0 < numberOfBYTES; numberOfBYTES--)
	{
		*destination++ = *source++;
	}
}

static inline void Mem::copyHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS)
{
	for(; 0 < numberOfHWORDS; numberOfHWORDS--)
	{
		*destination++ = *source++;
	}
}

static inline void Mem::copyWORD(WORD* destination, const WORD* source, uint32 numberOfWORDS)
{
	for(; 0 < numberOfWORDS; numberOfWORDS--)
	{
		*destination++ = *source++;
	}
}

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

static inline void Mem::combineWORDs(WORD* destination, const WORD* source1, const WORD* source2, uint32 numberOfWORDS)
{
	for(; 0 < numberOfWORDS; numberOfWORDS--)
	{
		*destination++ = *source1++ | *source2++;
	}
}

static inline void Mem::addOffsetToBYTE(BYTE* destination, const BYTE* source, uint32 numberOfBYTES, uint32 offset)
{
	for(; 0 < numberOfBYTES; numberOfBYTES--)
	{
		*destination++ = *source++ + offset;
	}
}

static inline void Mem::addOffsetToHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset)
{
	for(; 0 < numberOfHWORDS; numberOfHWORDS--)
	{
		*destination++ = *source++ + offset;
	}
}

static inline void Mem::addOffsetToWORD(WORD* destination, const WORD* source, uint32 numberOfWORDS, uint32 offset)
{
	for(; 0 < numberOfWORDS; numberOfWORDS--)
	{
		*destination++ = *source++ + offset;
	}
}

#endif
