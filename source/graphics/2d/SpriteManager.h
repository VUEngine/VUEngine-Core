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

#ifndef SPRITE_MANAGER_H_
#define SPRITE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Sprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define SpriteManager_METHODS													\
		Object_METHODS															\
		__VIRTUAL_DEC(render);

// declare the virtual methods which are redefined
#define SpriteManager_SET_VTABLE(ClassName)										\
		Object_SET_VTABLE(ClassName)											\
		__VIRTUAL_SET(ClassName, SpriteManager, render);						\

// declare a SpriteManager
__CLASS(SpriteManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

SpriteManager SpriteManager_getInstance();

void SpriteManager_destructor(SpriteManager this);
void SpriteManager_reset(SpriteManager this);
void SpriteManager_sortLayers(SpriteManager this, int progressively);
void SpriteManager_spriteChangedPosition(SpriteManager this);
void SpriteManager_removeSprite(SpriteManager this, Sprite sprite);
void SpriteManager_addSprite(SpriteManager this, Sprite sprite);
void SpriteManager_render(SpriteManager this);
int SpriteManager_getFreeLayer(SpriteManager this);
void SpriteManager_showLayer(SpriteManager this, u8 layer);
void SpriteManager_recoverLayers(SpriteManager this);
void SpriteManager_print(SpriteManager this, int x, int y);


#endif