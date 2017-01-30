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

#ifndef OBJECT_TEXTURE_H_
#define OBJECT_TEXTURE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Texture.h>
#include <CharSet.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ObjectTexture_METHODS(ClassName)															    \
    	Texture_METHODS(ClassName)																		\

#define ObjectTexture_SET_VTABLE(ClassName)																\
	    Texture_SET_VTABLE(ClassName)																	\
	    __VIRTUAL_SET(ClassName, ObjectTexture, write);													\

#define ObjectTexture_ATTRIBUTES																		\
        /* super's attributes */																		\
        Texture_ATTRIBUTES																				\
        /* object index */																				\
        int objectIndex;																				\
        /* bgmap displacement */																		\
        int bgmapDisplacement;																			\

// A texture which has the logic to be allocated in graphic memory
__CLASS(ObjectTexture);

//use a ObjectTexture when you want to show a static background or a character that must be scaled according
//its depth on the screen so there exists consistency between the depth and the size of the character


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef const TextureDefinition ObjectTextureDefinition;
typedef const ObjectTextureDefinition ObjectTextureROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ObjectTexture, ObjectTextureDefinition* objectTextureDefinition, u16 id);

void ObjectTexture_destructor(ObjectTexture this);
void ObjectTexture_write(ObjectTexture this);
void ObjectTexture_setObjectIndex(ObjectTexture this, int objectIndex);
void ObjectTexture_resetBgmapDisplacement(ObjectTexture this);
void ObjectTexture_addBgmapDisplacement(ObjectTexture this, int frame);


#endif
