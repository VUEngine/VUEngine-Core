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

#ifndef SPRITE_MANAGER_H_
#define SPRITE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Sprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define SpriteManager_METHODS(ClassName)																\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define SpriteManager_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

// declare a SpriteManager
__CLASS(SpriteManager);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

SpriteManager SpriteManager_getInstance();

void SpriteManager_destructor(SpriteManager this);
void SpriteManager_reset(SpriteManager this);
void SpriteManager_sortLayers(SpriteManager this);
void SpriteManager_registerSprite(SpriteManager this, Sprite sprite);
void SpriteManager_unregisterSprite(SpriteManager this, Sprite sprite);
void SpriteManager_processLayers(SpriteManager this);
void SpriteManager_processFreedLayers(SpriteManager this);
void SpriteManager_renderLastLayer(SpriteManager this);
void SpriteManager_render(SpriteManager this);
u8 SpriteManager_getFreeLayer(SpriteManager this);
void SpriteManager_showLayer(SpriteManager this, u8 layer);
void SpriteManager_recoverLayers(SpriteManager this);
Sprite SpriteManager_getSpriteAtLayer(SpriteManager this, u8 layer);
void SpriteManager_deferTextureWriting(SpriteManager this, bool deferTextureWriting);
void SpriteManager_deferAffineTransformations(SpriteManager this, bool deferAffineTransformations);
int SpriteManager_getMaximumAffineRowsToComputePerCall(SpriteManager this);
void SpriteManager_setMaximumAffineRowsToComputePerCall(SpriteManager this, int maximumAffineRowsToComputePerCall);
s8 SpriteManager_getTexturesMaximumRowsToWrite(SpriteManager this);
void SpriteManager_setCyclesToWaitForTextureWriting(SpriteManager this, u8 cyclesToWaitForTextureWriting);
void SpriteManager_setTexturesMaximumRowsToWrite(SpriteManager this, u8 texturesMaximumRowsToWrite);
void SpriteManager_print(SpriteManager this, int x, int y);


#endif
