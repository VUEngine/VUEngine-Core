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
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <HardwareManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// Copy a block of data from one area in memory to another.
void Mem_copy(u8* dest, const u8* src, u32 num)
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
		: "r" (((u32)dest & 0x3) << 2), "r" (((u32)src & 0x3) << 2), "r" (num << 3), "r" ((u32)dest & ~0x3), "r" ((u32)src & ~0x3) // input
		: "r1", "r26", "r27", "r28", "r29", "r30" // trashed
		);
*/

	u32 i;

	for(i = 0; i < num; i++)
	{
		*dest++ = *src++;
	}
}

void Mem_clear(u32* dest, u32 num )
{
	u32 i;
	for(i = 0; i < num; i++) *dest++ = 0;
}

void Mem_add(u8* dest, const u8* src, u32 num, u32 offset)
{
	u16* dest16 = (u16*)dest;
	u16* src16 = (u16*)src;

	u32 i;

	for(i = 0; i < num; i++)
	{
		*dest16++ = *src16++ + offset;
	}
}
