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

#ifndef B_TEXTURE_MANAGER_H_
#define B_TEXTURE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <BTexture.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */
// declare the virtual methods
#define BTextureManager_METHODS									\
		Object_METHODS											\


// declare the virtual methods which are redefined
#define BTextureManager_SET_VTABLE(ClassName)					\
		Object_SET_VTABLE(ClassName)							\


__CLASS(BTextureManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

BTextureManager BTextureManager_getInstance();

void BTextureManager_destructor(BTextureManager this);
void BTextureManager_reset(BTextureManager this);
void BTextureManager_free(BTextureManager this, BTexture bTexture);
void BTextureManager_allocateText(BTextureManager this, BTexture bTexture);
BTexture BTextureManager_get(BTextureManager this, BTextureDefinition* bTextureDefinition);
s8 BTextureManager_getXOffset(BTextureManager this, int id);
s8 BTextureManager_getYOffset(BTextureManager this, int id);
u8 BTextureManager_getBgmapSegment(BTextureManager this, int id);
u8 BTextureManager_getAvailableBgmapSegments(BTextureManager this);
void BTextureManager_setAvailableBgmapSegments(BTextureManager this, u8 availableBgmapSegments);
void BTextureManager_calculateAvailableBgmapSegments(BTextureManager this);
void BTextureManager_resetAvailableBgmapSegments(BTextureManager this);
u8 BTextureManager_getPrintingBgmapSegment(BTextureManager this);

void BTextureManager_print(BTextureManager this, int x, int y);


#endif