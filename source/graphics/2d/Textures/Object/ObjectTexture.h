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
