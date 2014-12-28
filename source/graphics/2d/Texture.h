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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharGroup.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Texture_METHODS															\
		Object_METHODS															\

#define Texture_SET_VTABLE(ClassName)											\
		Object_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, Texture, handleMessage);

#define Texture_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* char group to use int this texture */									\
	CharGroup charGroup;														\
																				\
	/* pointer to ROM definition */												\
	TextureROMDef* textureDefinition;											\
																				\
	/* texture's id */															\
	u16 id;																		\
																				\
	/* color palette */															\
	u8 palette: 2;																\

// A texture which has the logic to be allocated in graphic memory
__CLASS(Texture);

//use a Texture when you want to show a static background or a character that must be scaled according
//its deep on the screen so there exists consistency between the deep and the size of the character


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a background in ROM memory
typedef struct TextureDefinition
{
	// pointer to the char definition
	CharGroupDefinition charGroupDefinition;

	// pointer to the bgtexture definition in ROM
	BYTE* bgmapDefinition;

	// x size, 1 column represents 8 pixeles
	u8 cols;

	// y size, 1 row represents 8 pixeles
	u8 rows;

	// palette index to use
	u8 palette;

}TextureDefinition;

typedef const TextureDefinition TextureROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Texture, __PARAMETERS(TextureDefinition* textureDefinition, u16 id));

void Texture_destructor(Texture this);
void Texture_freeCharMemory(Texture this);
void Texture_write(Texture this);
void Texture_resetMemoryState(Texture this);
void Texture_rewrite(Texture this);
void Texture_writeHBiasMode(Texture this);
int Texture_getNumberOfChars(Texture this);
u8 Texture_getXOffset(Texture this);
u8 Texture_getYOffset(Texture this);
u8 Texture_getTotalCols(Texture this);
u8 Texture_getTotalRows(Texture this);
u8 Texture_getBgmapSegment(Texture this);
CharGroup Texture_getCharGroup(Texture this);
BYTE* Texture_getBgmapDef(Texture this);
void Texture_setPallet(Texture this, u8 palette);
u8 Texture_getPallet(Texture this);
u8 Texture_getRows(Texture this);
u8 Texture_getCols(Texture this);
u16 Texture_getId(Texture this);
bool Texture_handleMessage(Texture this, Telegram telegram);


#endif