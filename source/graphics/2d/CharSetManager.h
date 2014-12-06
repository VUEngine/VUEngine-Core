/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef CHARSETMEM_H_
#define CHARSETMEM_H_

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <CharGroup.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

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

 /* ---------------------------------------------------------------------------------------------------------
  * ---------------------------------------------------------------------------------------------------------
  * ---------------------------------------------------------------------------------------------------------
  * 										PUBLIC INTERFACE
  * ---------------------------------------------------------------------------------------------------------
  * ---------------------------------------------------------------------------------------------------------
  * ---------------------------------------------------------------------------------------------------------
  */

// it is a singleton!
CharSetManager CharSetManager_getInstance();

// class's destructor
void CharSetManager_destructor(CharSetManager this);
 
// reset
void CharSetManager_reset(CharSetManager this);

// release char graphic memory
void CharSetManager_free(CharSetManager this, CharGroup charGroup);
 
// print class's attributes's states
void CharSetManager_print(CharSetManager this, int x, int y);
 
// if char if part of a background or oder object whose frame doesn't change
int CharSetManager_allocateShared(CharSetManager this, CharGroup charGroup);
 
// allocate a char defintion within char graphic memory
void CharSetManager_allocate(CharSetManager this, CharGroup charGroup);
 
// set number of chars used in a given segment
void CharSetManager_setChars(CharSetManager  this, int charSet, int numberOfChars);

// defrag char memory
void CharSetManager_defragmentProgressively(CharSetManager this);

#endif /*CHARSETMEM_H_*/
