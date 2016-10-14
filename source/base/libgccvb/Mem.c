/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
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
    asm("          \n\t"      \
        "mov r29,r1\n\t"      \
        "mov %0,r26\n\t"      \
        "mov %1,r27\n\t"      \
        "mov %2,r28\n\t"      \
        "mov %3,r29\n\t"      \
        "mov %4,r30\n\t"      \
        ".hword 0x7C0B\n\t"   \
        "mov r1,r29"
        : /* output */
        : "r" (((u32)dest & 0x3) << 2), "r" (((u32)src & 0x3) << 2), "r" (num << 3), "r" ((u32)dest & ~0x3), "r" ((u32)src & ~0x3) /* input */
        : "r1", "r26", "r27", "r28", "r29", "r30" /* trashed */
        );
     /*
	u32 i;

	for(i = 0; i < num; i++)
	{
		*dest++ = *src++;
	}
	*/
}

void Mem_clear(u32* dest,  u32 num )
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
