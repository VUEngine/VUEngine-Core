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
	u32 i;
	num /= 4;

	u32* dest32 = (u32*)dest;
	u32* src32 = (u32*)src;

	for(i = 0; i < num; i++)
	{
		*dest32++ = *src32++;
	}
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
