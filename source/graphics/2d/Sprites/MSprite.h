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

#ifndef MSPRITE_H_
#define MSPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											 MACROS
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define MSprite_METHODS															\
	BSprite_METHODS																\

// declare the virtual methods which are redefined
#define MSprite_SET_VTABLE(ClassName)											\
	BSprite_SET_VTABLE(ClassName)												\
	__VIRTUAL_SET(ClassName, MSprite, synchronizePosition);						\

#define MSprite_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	BSprite_ATTRIBUTES;															\
																				\
	/* this is our texture */													\
	VirtualList textures;														\
																				\
	/* pinter to definition */													\
	const MSpriteDefinition* mSpriteDefinition;									\
																				\
	/* total size of the bgmap, used for loop/not loop */						\
	Point size;																	\
																				\
	/* fot total size of the bgmap calculation */								\
	Point sizeMultiplier;																	\

// declare a MSprite, which holds a texture and a drawing specification
__CLASS(MSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct MSpriteDefinition
{
	// the normal sprite definition
	BSpriteDefinition bSpriteDefinition;
	
	// texture to use with the sprite
	TextureDefinition** textureDefinitions;

	// SCX/SCY value
	u16 scValue;

	// flag to loop the x axis
	u8 xLoop;

	// flag to loop the y axis
	u8 yLoop;

} MSpriteDefinition;

typedef const MSpriteDefinition MSpriteROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(MSprite, const MSpriteDefinition* mSpriteDefinition);

void MSprite_constructor(MSprite this, const MSpriteDefinition* mSpriteDefinition);
void MSprite_destructor(MSprite this);
void MSprite_synchronizePosition(MSprite this, VBVec3D position3D);


#endif