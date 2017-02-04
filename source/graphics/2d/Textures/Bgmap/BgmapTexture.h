/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef BGMAP_TEXTURE_H_
#define BGMAP_TEXTURE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Texture.h>
#include <CharSet.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define BgmapTexture_METHODS(ClassName)																	\
		Texture_METHODS(ClassName)																		\

#define BgmapTexture_SET_VTABLE(ClassName)																\
		Texture_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, BgmapTexture, write);													\
		__VIRTUAL_SET(ClassName, BgmapTexture, rewrite);												\

#define BgmapTexture_ATTRIBUTES																			\
		/* super's attributes */																		\
		Texture_ATTRIBUTES																				\
		/* how many textures are using me */															\
		s8 usageCount;																					\
		/* remaining rows to be written */																\
		s8 remainingRowsToBeWritten;																	\

// A texture which has the logic to be allocated in graphic memory
__CLASS(BgmapTexture);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef const TextureDefinition BgmapTextureDefinition;
typedef const BgmapTextureDefinition BgmapTextureROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(BgmapTexture, BgmapTextureDefinition* bgmapTextureDefinition, u16 id);

void BgmapTexture_destructor(BgmapTexture this);
void BgmapTexture_write(BgmapTexture this);
void BgmapTexture_rewrite(BgmapTexture this);
s8 BgmapTexture_getRemainingRowsToBeWritten(BgmapTexture this);
s16 BgmapTexture_getXOffset(BgmapTexture this);
s16 BgmapTexture_getYOffset(BgmapTexture this);
u16 BgmapTexture_getBgmapSegment(BgmapTexture this);
void BgmapTexture_increaseUsageCount(BgmapTexture this);
bool BgmapTexture_decreaseUsageCount(BgmapTexture this);


#endif
