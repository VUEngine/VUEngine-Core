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

#ifndef MEM_H_
#define MEM_H_


//---------------------------------------------------------------------------------------------------------
// 											 INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>


//---------------------------------------------------------------------------------------------------------
// 											PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void Mem_copy (u8* dest, const u8* src, u16 num);
void Mem_set (u8* dest, u16 src, u16 num);
void Mem_clearFast(u32* dest,  u16 num );
void Mem_clear (u16* dest,  u16 num );
void Mem_subtract (u8* dest, const u8* src, u16 num, u16 offset,u8 modifier);
void Mem_add (u8* dest, const u8* src, u16 num, u16 offset);


#endif