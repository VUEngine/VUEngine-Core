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
void Mem_copy (u8* dest, const u8* src, u16 num)
{
	u16 i;

	for(i = 0; i < num; i++)
	{
		*dest++ = *src++;
	}
}

// Set each byte in a block of data to a given value.
void Mem_set (u8* dest, u16 src, u16 num)
{
	u16 i;
	for(i = 0; i < num; i++,dest++)
	{
		*dest += src;
		dest++;
		*dest = src >> 8;
	}
}

void Mem_clearFast(u32* dest,  u16 num )
{
	u16 i;
	num >>= 1;

	//memset(dest, 0, num);
	for(i = 0; i < num; i += 16)
	{
		//*dest++ = 0x00000000;
		dest[i]    = 0x00000000;
		dest[i+1]  = 0x00000000;
		dest[i+2]  = 0x00000000;
		dest[i+3]  = 0x00000000;
		dest[i+4]  = 0x00000000;
		dest[i+5]  = 0x00000000;
		dest[i+6]  = 0x00000000;
		dest[i+7]  = 0x00000000;
		dest[i+8]  = 0x00000000;
		dest[i+9]  = 0x00000000;
		dest[i+10] = 0x00000000;
		dest[i+11] = 0x00000000;
		dest[i+12] = 0x00000000;
		dest[i+13] = 0x00000000;
		dest[i+14] = 0x00000000;
		dest[i+15] = 0x00000000;
	}
}

void Mem_clear (u16* dest,  u16 num )
{
	u16 i;
	for(i = 0; i < num; i++) *dest++ = 0x0000;
}

void Mem_subtract (u8* dest, const u8* src, u16 num, u16 offset,u8 modifier)
{
}

void Mem_add (u8* dest, const u8* src, u16 num, u16 offset)
{
	u16 i;
	int carry;
	for(i = 0; i < num; i++)
	{
		*dest++ = carry = *src++ + offset;
		*dest++ =(*src++ + (offset >> 8)) | (carry >> 8);
	}
}
