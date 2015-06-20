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

#ifndef M_BGMAP_SPRITE_H_
#define M_BGMAP_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											 MACROS
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define MBgmapSprite_METHODS													\
	BgmapSprite_METHODS															\

// declare the virtual methods which are redefined
#define MBgmapSprite_SET_VTABLE(ClassName)										\
	BgmapSprite_SET_VTABLE(ClassName)											\
	__VIRTUAL_SET(ClassName, MBgmapSprite, positione);							\

#define MBgmapSprite_ATTRIBUTES													\
																				\
	/* super's attributes */													\
	BgmapSprite_ATTRIBUTES;														\
																				\
	/* this is our texture */													\
	VirtualList textures;														\
																				\
	/* pinter to definition */													\
	const MBgmapSpriteDefinition* mSpriteDefinition;							\
																				\
	/* total size of the bgmap, used for loop/not loop */						\
	Point size;																	\
																				\
	/* fot total size of the bgmap calculation */								\
	Point sizeMultiplier;																	\

// declare a MBgmapSprite, which holds a texture and a drawing specification
__CLASS(MBgmapSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct MBgmapSpriteDefinition
{
	// the normal sprite definition
	BgmapSpriteDefinition bSpriteDefinition;
	
	// texture to use with the sprite
	TextureDefinition** textureDefinitions;

	// SCX/SCY value
	u16 scValue;

	// flag to loop the x axis
	u8 xLoop;

	// flag to loop the y axis
	u8 yLoop;

} MBgmapSpriteDefinition;

typedef const MBgmapSpriteDefinition MBgmapSpriteROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(MBgmapSprite, const MBgmapSpriteDefinition* mSpriteDefinition, Object );

void MBgmapSprite_constructor(MBgmapSprite this, const MBgmapSpriteDefinition* mSpriteDefinition, Object owner);
void MBgmapSprite_destructor(MBgmapSprite this);
void MBgmapSprite_positione(MBgmapSprite this, VBVec3D position3D);


#endif