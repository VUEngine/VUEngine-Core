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

#ifndef O_ANIMATED_SPRITE_H_
#define O_ANIMATED_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <OSprite.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define OAnimatedSprite_METHODS													\
	OSprite_METHODS																\

// declare the virtual methods which are redefined
#define OAnimatedSprite_SET_VTABLE(ClassName)									\
	OSprite_SET_VTABLE(ClassName)												\
	__VIRTUAL_SET(ClassName, OAnimatedSprite, writeAnimation);					\

#define OAnimatedSprite_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	OSprite_ATTRIBUTES;															\
																				\
	/* bgmap's source coordinates */											\
	TextureSource originalTextureSource;										\

// declare a Sprite, which holds a texture and a drawing specification
__CLASS(OAnimatedSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(OAnimatedSprite, const SpriteDefinition* spriteDefinition, Object owner);

void OAnimatedSprite_destructor(OAnimatedSprite this);
void OAnimatedSprite_writeAnimation(OAnimatedSprite this);


#endif