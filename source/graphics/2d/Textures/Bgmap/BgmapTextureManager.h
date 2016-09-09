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

#ifndef BGMAP_TEXTURE_MANAGER_H_
#define BGMAP_TEXTURE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <BgmapTexture.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __BGMAP_SEGMENT_SIZE        8192


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// Defines as a pointer to a structure that's not defined here and so is not accessible to the outside world

// declare the virtual methods
#define BgmapTextureManager_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																					\

// declare the virtual methods which are redefined
#define BgmapTextureManager_SET_VTABLE(ClassName)														\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(BgmapTextureManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

BgmapTextureManager BgmapTextureManager_getInstance();

void BgmapTextureManager_destructor(BgmapTextureManager this);
void BgmapTextureManager_reset(BgmapTextureManager this);
void BgmapTextureManager_releaseTexture(BgmapTextureManager this, BgmapTexture bgmapTexture);
void BgmapTextureManager_allocateText(BgmapTextureManager this, BgmapTexture bgmapTexture);
BgmapTexture BgmapTextureManager_getTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition);
s32 BgmapTextureManager_getXOffset(BgmapTextureManager this, int id);
s32 BgmapTextureManager_getYOffset(BgmapTextureManager this, int id);
u32 BgmapTextureManager_getBgmapSegment(BgmapTextureManager this, int id);
void BgmapTextureManager_calculateAvailableBgmapSegments(BgmapTextureManager this);
s32 BgmapTextureManager_getAvailableBgmapSegments(BgmapTextureManager this);
s32 BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager this);
void BgmapTextureManager_setSpareBgmapSegments(BgmapTextureManager this, u8 paramTableSegments);
void BgmapTextureManager_print(BgmapTextureManager this, int x, int y);


#endif
