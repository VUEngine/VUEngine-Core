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

#ifndef TEXTURE_H_
#define TEXTURE_H_

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
#include <Telegram.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define Texture_METHODS							\
		Object_METHODS							\

#define Texture_SET_VTABLE(ClassName)						\
		Object_SET_VTABLE(ClassName)						\
		__VIRTUAL_SET(ClassName, Texture, handleMessage);

#define Texture_ATTRIBUTES										\
																\
	/* super's attributes */									\
	Object_ATTRIBUTES;											\
																\
	/* texture's id */											\
	int id;														\
																\
	/* char group to use int this texture */					\
	CharGroup charGroup;										\
																\
	/* pointer to ROM definition */								\
	TextureROMDef* textureDefinition;							\
																\
	/* color pallet */											\
	int pallet:2;												\

// A texture which has the logic to be allocated in graphic memory
__CLASS(Texture);


//use a Texture when you want to show a static background or a character that must be scaled according
//its deep on the screen so there exists consistency between the deep and the size of the character


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S ROM DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */



/* ---------------------------------------------------------------------------------------------------------*/
// defines a background in ROM memory
typedef struct TextureDefinition{

	// pointer to the char definition
	CharGroupDefinition charGroupDefinition;
	
	// pointer to the bgtexture definition in ROM
	BYTE* bgmapDefinition;

	// x size, 1 column represents 8 pixeles 
	int cols;
	
	// y size, 1 row represents 8 pixeles
	int rows;
	
	// pallet index to use
	int pallet;
	

}TextureDefinition;

typedef const TextureDefinition TextureROMDef;

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's allocator
__CLASS_NEW_DECLARE(Texture, __PARAMETERS(TextureDefinition* textureDefinition, int id));

// class's destructor
void Texture_destructor(Texture this);

// free char memory
void Texture_freeCharMemory(Texture this);

// write into memory the chars and this
void Texture_write(Texture this);

// this reallocate a write the bgmap definition in graphical memory
void Texture_resetMemoryState(Texture this);

// write into memory the chars and this
void Texture_rewrite(Texture this);

// write map in hbias mode
void Texture_writeHBiasMode(Texture this);

// get texture's number of chars
int Texture_getNumberOfChars(Texture this);

// get texture's y offset within bgmap mem
int Texture_getYOffset(Texture this);

// get texture's x offset within bgmap mem
int Texture_getXOffset(Texture this);

// set texture's x offset within bgmap mem
int Texture_setXOffset(Texture this, int xOffset);


// get texture's cols
int Texture_getTotalCols(Texture this);

// get texture's rows
int Texture_getTotalRows(Texture this);

// get texture's bgmap segment
int Texture_getBgmapSegment(Texture this);

// get texture's chargroup
CharGroup Texture_getCharGroup(Texture this);

// get texture's bgmap definition
BYTE* Texture_getBgmapDef(Texture this);

// set the pallet
void Texture_setPallet(Texture this, int pallet);

// retrieve texture's pallet
int Texture_getPallet(Texture this);

// retrieve texture's rows
int Texture_getRows(Texture this);

// retrieve texture's cols
int Texture_getCols(Texture this);

// retrieve texture's id
int Texture_getId(Texture this);

// process a telegram
int Texture_handleMessage(Texture this, Telegram telegram);

#endif
