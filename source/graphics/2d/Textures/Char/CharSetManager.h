/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef CHARSETMEM_H_
#define CHARSETMEM_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */
// declare the virtual methods
#define CharSetManager_METHODS													\
		Object_METHODS															\

// declare the virtual methods which are redefined
#define CharSetManager_SET_VTABLE(ClassName)									\
		Object_SET_VTABLE(ClassName)											\

__CLASS(CharSetManager);


//---------------------------------------------------------------------------------------------------------
//											PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

CharSetManager CharSetManager_getInstance();

void CharSetManager_destructor(CharSetManager this);
void CharSetManager_reset(CharSetManager this);
CharSet CharSetManager_getCharSet(CharSetManager this, CharSetDefinition* charSetDefinition);
void CharSetManager_releaseCharSet(CharSetManager this, CharSet charSet);
void CharSetManager_defragmentProgressively(CharSetManager this);
int CharSetManager_getTotalUsedChars(CharSetManager this, int segment);
int CharSetManager_getTotalFreeChars(CharSetManager this, int segment);
int CharSetManager_getTotalCharSets(CharSetManager this, int segment);
void CharSetManager_print(CharSetManager this, int x, int y);


#endif