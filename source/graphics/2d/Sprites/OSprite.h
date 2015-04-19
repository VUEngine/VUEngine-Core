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

#ifndef OSPRITE_H_
#define OSPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <MiscStructs.h>
#include <Texture.h>


//---------------------------------------------------------------------------------------------------------
// 											 MACROS
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define OSprite_METHODS															\
	Sprite_METHODS																\

// declare the virtual methods which are redefined
#define OSprite_SET_VTABLE(ClassName)											\
	Sprite_SET_VTABLE(ClassName)												\

#define OSprite_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Sprite_ATTRIBUTES;															\

// declare a OSprite, which holds a texture and a drawing specification
__CLASS(OSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct OSpriteDefinition
{
	// the normal sprite definition
	SpriteDefinition spriteDefinition;
	
	// texture to use with the sprite
	TextureDefinition** textureDefinitions;

} OSpriteDefinition;

typedef const OSpriteDefinition OSpriteROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(OSprite, const OSpriteDefinition* oSpriteDefinition);

void OSprite_constructor(OSprite this, const OSpriteDefinition* oSpriteDefinition);
void OSprite_destructor(OSprite this);


#endif