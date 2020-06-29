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
/*
	asm("          \n\t"      \
		"mov r29,r1\n\t"      \
		"mov %0,r26\n\t"      \
		"mov %1,r27\n\t"      \
		"mov %2,r28\n\t"      \
		"mov %3,r29\n\t"      \
		"mov %4,r30\n\t"      \
		".hword 0x7C0B\n\t"   \
		"mov r1,r29"
		: // output
		: "r" (((u32)destination & 0x3) << 2), "r" (((u32)source & 0x3) << 2), "r" (numberOfBYTES << 3), "r" ((u32)destination & ~0x3), "r" ((u32)source & ~0x3) // input
		: "r1", "r26", "r27", "r28", "r29", "r30" // trashed
		);
*/

	const BYTE* finalSource = source + numberOfBYTES;

	asm("				\n\t"      \
		"jr end%=		\n\t"      \
		"loop%=:		\n\t"      \
		"ld.b 0[%1],r10	\n\t"      \
		"st.b r10,0[%0] \n\t"      \
		"add 1,%0		\n\t"      \
		"add 1,%1		\n\t"      \
		"end%=:			\n\t"      \
		"cmp %1,%2		\n\t"      \
		"bgt loop%=		\n\t"
		: // No Output
		: "r" (destination), "r" (source), "r" (finalSource)
		: "r10", "sp", "lp" // regs used
	);
}

static inline void Mem::copyHWORD(HWORD* destination, const HWORD* source, u32 numberOfHWORDS)
{
	const HWORD* finalSource = source + numberOfHWORDS;

	asm("				\n\t"      \
		"jr end%=		\n\t"      \
		"loop%=:		\n\t"      \
		"ld.h 0[%1],r10	\n\t"      \
		"st.h r10,0[%0] \n\t"      \
		"add 2,%0		\n\t"      \
		"add 2,%1		\n\t"      \
		"end%=:			\n\t"      \
		"cmp %1,%2		\n\t"      \
		"bgt loop%=		\n\t"
		: // No Output
		: "r" (destination), "r" (source), "r" (finalSource)
		: "r10", "sp", "lp" // regs used
	);
}

static inline void Mem::copyWORD(WORD* destination, const WORD* source, u32 numberOfWORDS)
{
	const WORD* finalSource = source + numberOfWORDS;

	asm("				\n\t"      \
		"jr end%=		\n\t"      \
		"loop%=:		\n\t"      \
		"ld.w 0[%1],r10	\n\t"      \
		"st.w r10,0[%0] \n\t"      \
		"add 4,%0		\n\t"      \
		"add 4,%1		\n\t"      \
		"end%=:			\n\t"      \
		"cmp %1,%2		\n\t"      \
		"bgt loop%=		\n\t"
		: // No Output
		: "r" (destination), "r" (source), "r" (finalSource)
		: "r10", "sp", "lp" // regs used
	);
}

static inline void Mem::addBYTE(BYTE* destination, const BYTE* source, u32 numberOfBYTES, u32 offset)
{
	const BYTE* finalSource = source + numberOfBYTES;

    asm("					\n\t"      \
		"jr end%=			\n\t"      \
		"loop%=:			\n\t"      \
		"ld.b 0[%1],r10		\n\t"      \
		"add %3,r10			\n\t"      \
		"st.b r10,0[%0]		\n\t"      \
		"add 1,%0			\n\t"      \
		"add 1,%1			\n\t"      \
		"end%=:				\n\t"      \
		"cmp %1,%2			\n\t"      \
		"bgt loop%=			\n\t"      \
		: // No Output
		: "r" (destination), "r" (source), "r" (finalSource), "r" (offset)
		: "r10", "sp", "lp" // regs used
    );
}


#endif
