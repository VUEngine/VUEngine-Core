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

#ifndef M_BGMAP_SPRITE_H_
#define M_BGMAP_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define MBgmapSprite_METHODS																			\
	BgmapSprite_METHODS																					\

// declare the virtual methods which are redefined
#define MBgmapSprite_SET_VTABLE(ClassName)																\
	BgmapSprite_SET_VTABLE(ClassName)																	\
	__VIRTUAL_SET(ClassName, MBgmapSprite, position);													\
	__VIRTUAL_SET(ClassName, MBgmapSprite, setPosition);												\
	__VIRTUAL_SET(ClassName, MBgmapSprite, render);														\
	__VIRTUAL_SET(ClassName, MBgmapSprite, getPosition);												\
	__VIRTUAL_SET(ClassName, MBgmapSprite, addDisplacement);												\

#define MBgmapSprite_ATTRIBUTES																			\
																										\
	/* super's attributes */																			\
	BgmapSprite_ATTRIBUTES;																				\
																										\
	/* this is our texture */																			\
	VirtualList textures;																				\
																										\
	/* pinter to definition */																			\
	const MBgmapSpriteDefinition* mSpriteDefinition;													\
																										\
	/* total size of the bgmap, used for loop/not loop */												\
	Point size;																							\
																										\
	/* for total size of the bgmap calculation */														\
	Point sizeMultiplier;																				\
																										\
	/* to speed up rendering */																			\
	u16 textureXOffset;																					\
	u16 textureYOffset;																					\

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
void MBgmapSprite_position(MBgmapSprite this, const VBVec3D* position);
void MBgmapSprite_setPosition(MBgmapSprite this, const VBVec2D* position);
void MBgmapSprite_render(MBgmapSprite this);
VBVec2D MBgmapSprite_getPosition(MBgmapSprite this);
void MBgmapSprite_addDisplacement(MBgmapSprite this, const VBVec2D* displacement);


#endif