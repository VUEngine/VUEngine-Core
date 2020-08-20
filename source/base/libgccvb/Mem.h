/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

#ifndef MEM_H_
#define MEM_H_


//---------------------------------------------------------------------------------------------------------
//											 INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>


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
	static void clear(BYTE* destination, u32 numberOfBYTES);
	static inline void copyBYTE(BYTE* destination, const BYTE* source, u32 numberOfBYTES);
	static inline void copyHWORD(HWORD* destination, const HWORD* source, u32 numberOfHWORDS);
	static inline void copyWORD(WORD* destination, const WORD* source, u32 numberOfWORDS);
	static inline void addBYTE(BYTE* destination, const BYTE* source, u32 numberOfBYTES, u32 offset);
	static void addHWORD(HWORD* destination, const HWORD* source, u32 numberOfHWORDS, u32 offset);
}

// TODO: input registers should not be modified according to GCC's docs

// Copy a block of data from one area in memory to another.
static inline void Mem::copyBYTE(BYTE* destination, const BYTE* source, u32 numberOfBYTES)
{
	for(; 0 < numberOfBYTES; numberOfBYTES--)
	{
		*destination++ = *source++;
	}
}

static inline void Mem::copyHWORD(HWORD* destination, const HWORD* source, u32 numberOfHWORDS)
{
	for(; 0 < numberOfHWORDS; numberOfHWORDS--)
	{
		*destination++ = *source++;
	}
}

static inline void Mem::copyWORD(WORD* destination, const WORD* source, u32 numberOfWORDS)
{
	for(; 0 < numberOfWORDS; numberOfWORDS--)
	{
		*destination++ = *source++;
	}
}

static inline void Mem::addBYTE(BYTE* destination, const BYTE* source, u32 numberOfBYTES, u32 offset)
{
	for(; 0 < numberOfBYTES; numberOfBYTES--)
	{
		*destination++ = *source++ + offset;
	}
}


#endif
