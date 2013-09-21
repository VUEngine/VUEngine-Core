/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>


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
 
#define CharGroup_METHODS					\
		Object_METHODS						\


#define CharGroup_SET_VTABLE(ClassName)				\
		Object_SET_VTABLE(ClassName)				\


#define CharGroup_ATTRIBUTES										\
																	\
	/* super's attributes */										\
	Object_ATTRIBUTES;												\
																	\
	/* memory displacement */										\
	int offset;														\
																	\
	/* memory segment */											\
	int charset: 2;													\
																	\
	/* allocation type */											\
	int allocationType: 3;											\
																	\
	/* number of chars */											\
	int numberOfChars: 10;											\
																	\
	/* array definition of the charSet */							\
	BYTE* charDefinition;

__CLASS(CharGroup);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

typedef struct CharGroupDefinition{
	
	// pointer to the char definition in ROM
	BYTE* charDefinition;
	
	// number of chars
	int numberOfChars;
	
	// the way its chars and bgtexture will be allocated in graphic memory
	int allocationType;
	
}CharGroupDefinition;

typedef const CharGroupDefinition CharGroupROMDef;

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// class's allocator
__CLASS_NEW_DECLARE(CharGroup, __PARAMETERS(CharGroupDefinition* charGroupDefinition));

// class's destructor
void CharGroup_destructor(CharGroup this);

// retrieve chargroup's allocation type
int CharGroup_getAllocationType(CharGroup this);

// retrieve chargroup's offset within char segment
int CharGroup_getOffset(CharGroup this);

// set chargroup's offset within the char segment
void CharGroup_setOffset(CharGroup this, int offset);

// get chargroup's char definition	
BYTE* CharGroup_getCharDefinition(CharGroup this);

// set chargroup's char definition
void CharGroup_setCharDefinition(CharGroup this, void *charDefinition);

// set chargroup's number of chars
void CharGroup_setNumberOfChars(CharGroup this, int numberOfChars);

// retrieve chargrop's number of chars
int CharGroup_getNumberOfChars(CharGroup this);

// get chargroup's segment 
int CharGroup_getCharSet(CharGroup this);

// set chargroup's char segment
void CharGroup_setCharSet(CharGroup this, int charSet);

// copy a chargroup
void CharGroup_copy(CharGroup this, CharGroup source);

// write char on memory	
void CharGroup_write(CharGroup this);


#endif







