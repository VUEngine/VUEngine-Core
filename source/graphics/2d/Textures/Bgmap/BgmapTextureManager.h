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

#ifndef BGMAP_TEXTURE_MANAGER_H_
#define BGMAP_TEXTURE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <BgmapTexture.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __BGMAP_SEGMENT_SIZE	8192


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// Defines as a pointer to a structure that's not defined here and so is not accessible to the outside world

// declare the virtual methods
#define BgmapTextureManager_METHODS(ClassName)															\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define BgmapTextureManager_SET_VTABLE(ClassName)														\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(BgmapTextureManager);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

BgmapTextureManager BgmapTextureManager_getInstance();

void BgmapTextureManager_destructor(BgmapTextureManager this);
void BgmapTextureManager_reset(BgmapTextureManager this);
void BgmapTextureManager_releaseTexture(BgmapTextureManager this, BgmapTexture bgmapTexture);
void BgmapTextureManager_allocateText(BgmapTextureManager this, BgmapTexture bgmapTexture);
BgmapTexture BgmapTextureManager_getTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition);
s16 BgmapTextureManager_getXOffset(BgmapTextureManager this, int id);
s16 BgmapTextureManager_getYOffset(BgmapTextureManager this, int id);
void BgmapTextureManager_calculateAvailableBgmapSegments(BgmapTextureManager this);
s16 BgmapTextureManager_getAvailableBgmapSegmentsForTextures(BgmapTextureManager this);
s16 BgmapTextureManager_getPrintingBgmapSegment(BgmapTextureManager this);
void BgmapTextureManager_setSpareBgmapSegments(BgmapTextureManager this, u8 paramTableSegments);
void BgmapTextureManager_print(BgmapTextureManager this, int x, int y);


#endif
