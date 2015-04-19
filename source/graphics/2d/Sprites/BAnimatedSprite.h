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

#ifndef B_ANIMATED_SPRITE_H_
#define B_ANIMATED_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BSprite.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define BAnimatedSprite_METHODS													\
	BSprite_METHODS																\

// declare the virtual methods which are redefined
#define BAnimatedSprite_SET_VTABLE(ClassName)									\
	BSprite_SET_VTABLE(ClassName)												\
	__VIRTUAL_SET(ClassName, BAnimatedSprite, writeAnimation);					\

#define BAnimatedSprite_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	BSprite_ATTRIBUTES;															\
																				\
	/* bgmap's source coordinates */											\
	TextureSource originalTextureSource;										\

// declare a Sprite, which holds a texture and a drawing specification
__CLASS(BAnimatedSprite);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(BAnimatedSprite, const SpriteDefinition* spriteDefinition, Object owner);

void BAnimatedSprite_destructor(BAnimatedSprite this);
void BAnimatedSprite_writeAnimation(BAnimatedSprite this);


#endif