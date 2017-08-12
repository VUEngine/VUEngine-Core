/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#include <HardwareManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// Copy a block of data from one area in memory to another.
void Mem_copy(BYTE* destination, const BYTE* source, u32 numberOfBytes)
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
		: "r" (((u32)destination & 0x3) << 2), "r" (((u32)source & 0x3) << 2), "r" (numberOfBytes << 3), "r" ((u32)destination & ~0x3), "r" ((u32)source & ~0x3) // input
		: "r1", "r26", "r27", "r28", "r29", "r30" // trashed
		);
*/
	u32 numberOfWORDS = numberOfBytes >> 2;

	WORD* destinationWORD = (WORD*) destination;
	WORD* sourceWORD = (WORD*) source;
	WORD* finalSourceWORD = sourceWORD + numberOfWORDS;

	CACHE_DISABLE;
	CACHE_CLEAR;
	CACHE_ENABLE;

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
		: "r" (destinationWORD), "r" (sourceWORD), "r" (finalSourceWORD)
		: "r10" // regs used
	);
}

void Mem_clear(BYTE* destination, u32 numberOfBytes)
{
	u32 i;

	for(i = 0; i < numberOfBytes; i++)
	{
		*destination++ = 0;
	}
}

void Mem_add(BYTE* destination, const BYTE* source, u32 numberOfBytes, u32 offset)
{
	u32 numberOfHWORDS = numberOfBytes >> 1;

	HWORD* destinationHWORD = (HWORD*) destination;
	HWORD* sourceHWORD = (HWORD*) source;
	HWORD* finalSourceHWORD = sourceHWORD + numberOfHWORDS;

    asm("					\n\t"      \
		"jr end%=			\n\t"      \
		"loop%=:			\n\t"      \
		"ld.h 0[%1],r10		\n\t"      \
		"add %3,r10			\n\t"      \
		"st.h r10,0[%0]		\n\t"      \
		"add 2,%0			\n\t"      \
		"add 2,%1			\n\t"      \
		"end%=:				\n\t"      \
		"cmp %1,%2			\n\t"      \
		"bgt loop%=			\n\t"      \
    : // No Output
    : "r" (destinationHWORD), "r" (sourceHWORD), "r" (finalSourceHWORD), "r" (offset)
	: "r10" // regs used
    );
}
