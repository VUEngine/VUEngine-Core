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

#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define MemoryPool_METHODS(ClassName)																	\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define MemoryPool_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(MemoryPool);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

MemoryPool MemoryPool_getInstance();

void MemoryPool_destructor(MemoryPool this);
void MemoryPool_cleanUp(MemoryPool this);
BYTE* MemoryPool_allocate(MemoryPool this, int numBytes);
void MemoryPool_free(MemoryPool this, BYTE* object);
void MemoryPool_printDetailedUsage(MemoryPool this, int x, int y);
void MemoryPool_printResumedUsage(MemoryPool this, int x, int y);


#endif
