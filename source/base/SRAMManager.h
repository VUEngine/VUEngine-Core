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

#ifndef SRAM_MANAGER_H_
#define SRAM_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define SRAMManager_METHODS																				\
		Object_METHODS																					\

// declare the virtual methods which are redefined
#define SRAMManager_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(SRAMManager);

// forward declare game's custom user's data struct
struct UserData;

extern const struct UserData* _userData;

//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

SRAMManager SRAMManager_getInstance();

void SRAMManager_destructor(SRAMManager this);
void SRAMManager_save(SRAMManager this, const BYTE* const source, u16* memberAddress, int dataSize);
void SRAMManager_read(SRAMManager this, BYTE* destination, u16* memberAddress, int dataSize);


#endif