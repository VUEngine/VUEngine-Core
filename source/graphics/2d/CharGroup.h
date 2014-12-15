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

#ifndef CHARGROUP_H_
#define CHARGROUP_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

//definition of a chargroup of an animated character or background
#define __ANIMATED			0x01

//definition of a chargroup of an unanimated character or background
#define __NO_ANIMATED		0x02

//definition of a chargroup of an animated character which it's all frames are written
//and shared
#define __ANIMATED_SHARED	0x03

// future expansion
#define __ANIMATED_SHARED_2	0x04


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */

#define CharGroup_METHODS														\
		Object_METHODS															\

#define CharGroup_SET_VTABLE(ClassName)											\
		Object_SET_VTABLE(ClassName)											\

#define CharGroup_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* memory displacement */													\
	u16 offset;																	\
																				\
	/* memory segment */														\
	u8 charset: 2;																\
																				\
	/* allocation type */														\
	u8 allocationType: 3;														\
																				\
	/* number of chars */														\
	u16 numberOfChars: 10;														\
																				\
	/* array definition of the charSet */										\
	BYTE* charDefinition;														\
																				\
	/* array definition of the charSet */										\
	u16 charDefinitionDisplacement;												\
																				\
	/* owner */																	\
	Object owner;																\

__CLASS(CharGroup);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct CharGroupDefinition
{
	// number of chars
	u16 numberOfChars;

	// the way its chars and bgtexture will be allocated in graphic memory
	u16 allocationType;

	// pointer to the char definition in ROM
	BYTE* charDefinition;

} CharGroupDefinition;

typedef const CharGroupDefinition CharGroupROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(CharGroup, __PARAMETERS(CharGroupDefinition* charGroupDefinition, Object owner));

void CharGroup_destructor(CharGroup this);
int CharGroup_getAllocationType(CharGroup this);
u16 CharGroup_getOffset(CharGroup this);
void CharGroup_setOffset(CharGroup this, u16 offset);
BYTE* CharGroup_getCharDefinition(CharGroup this);
void CharGroup_setCharDefinition(CharGroup this, void *charDefinition);
void CharGroup_setNumberOfChars(CharGroup this, int numberOfChars);
int CharGroup_getNumberOfChars(CharGroup this);
int CharGroup_getCharSet(CharGroup this);
void CharGroup_setCharSet(CharGroup this, int charSet);
void CharGroup_copy(CharGroup this, CharGroup source);
void CharGroup_write(CharGroup this);
void CharGroup_rewrite(CharGroup this);
void CharGroup_setCharDefinitionDisplacement(CharGroup this, u16 charDefinitionDisplacement);


#endif