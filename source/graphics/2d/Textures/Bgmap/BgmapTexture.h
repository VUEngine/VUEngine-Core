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

#ifndef BGMAP_TEXTURE_H_
#define BGMAP_TEXTURE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Texture.h>
#include <CharSet.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __EVENT_TEXTURE_REWRITTEN				"textureRewritten"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define BgmapTexture_METHODS																			\
	Texture_METHODS																						\

#define BgmapTexture_SET_VTABLE(ClassName)																\
	Texture_SET_VTABLE(ClassName)																		\
	__VIRTUAL_SET(ClassName, BgmapTexture, write);														\

#define BgmapTexture_ATTRIBUTES																			\
																										\
	/* super's attributes */																			\
	Texture_ATTRIBUTES;																					\
																										\
	/* how many textures are using me */																\
	u8 usageCount;																						\
																										\
	/* remaining rows to be written */																	\
	u8 remainingRowsToBeWritten;																		\

// A texture which has the logic to be allocated in graphic memory
__CLASS(BgmapTexture);

//use a BgmapTexture when you want to show a static background or a character that must be scaled according
//its depth on the screen so there exists consistency between the depth and the size of the character


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef const TextureDefinition BgmapTextureDefinition;
typedef const BgmapTextureDefinition BgmapTextureROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(BgmapTexture, BgmapTextureDefinition* bgmapTextureDefinition, u16 id);

void BgmapTexture_destructor(BgmapTexture this);
void BgmapTexture_write(BgmapTexture this);
u8 BgmapTexture_getXOffset(BgmapTexture this);
u8 BgmapTexture_getYOffset(BgmapTexture this);
u8 BgmapTexture_getBgmapSegment(BgmapTexture this);
void BgmapTexture_increaseUsageCount(BgmapTexture this);
bool BgmapTexture_decreaseUsageCount(BgmapTexture this);


#endif