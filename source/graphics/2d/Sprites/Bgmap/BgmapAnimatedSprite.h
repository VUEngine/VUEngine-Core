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

#ifndef BGMAP_ANIMATED_SPRITE_H_
#define BGMAP_ANIMATED_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>
#include <AnimationController.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define BgmapAnimatedSprite_METHODS(ClassName)															\
	    BgmapSprite_METHODS(ClassName)																	\

// declare the virtual methods which are redefined
#define BgmapAnimatedSprite_SET_VTABLE(ClassName)														\
        BgmapSprite_SET_VTABLE(ClassName)																\
        __VIRTUAL_SET(ClassName, BgmapAnimatedSprite, writeAnimation);									\

#define BgmapAnimatedSprite_ATTRIBUTES																	\
        /* super's attributes */																		\
        BgmapSprite_ATTRIBUTES																			\
        /* bgmap's source coordinates */																\
        TextureSource originalTextureSource;															\

__CLASS(BgmapAnimatedSprite);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(BgmapAnimatedSprite, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner);

void BgmapAnimatedSprite_destructor(BgmapAnimatedSprite this);
void BgmapAnimatedSprite_writeAnimation(BgmapAnimatedSprite this);


#endif
