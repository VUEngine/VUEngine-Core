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

#ifndef BGMAP_TEXTURE_MANAGER_H_
#define BGMAP_TEXTURE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <BgmapTexture.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */
// declare the virtual methods
#define BgmapTextureManager_METHODS												\
		Object_METHODS															\


// declare the virtual methods which are redefined
#define BgmapTextureManager_SET_VTABLE(ClassName)								\
		Object_SET_VTABLE(ClassName)											\


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
s8 BgmapTextureManager_getXOffset(BgmapTextureManager this, int id);
s8 BgmapTextureManager_getYOffset(BgmapTextureManager this, int id);
u8 BgmapTextureManager_getBgmapSegment(BgmapTextureManager this, int id);
u8 BgmapTextureManager_getAvailableBgmapSegmentForParamTable(BgmapTextureManager this);
u8 BgmapTextureManager_getAvailableBgmapSegments(BgmapTextureManager this);
void BgmapTextureManager_setAvailableBgmapSegments(BgmapTextureManager this, u8 availableBgmapSegments);
void BgmapTextureManager_calculateAvailableBgmapSegments(BgmapTextureManager this);
void BgmapTextureManager_resetAvailableBgmapSegments(BgmapTextureManager this);
u8 BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager this);

void BgmapTextureManager_print(BgmapTextureManager this, int x, int y);


#endif