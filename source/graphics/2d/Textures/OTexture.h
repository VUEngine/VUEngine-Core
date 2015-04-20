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

#ifndef O_TEXTURE_H_
#define O_TEXTURE_H_


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

#define OTexture_METHODS														\
	Texture_METHODS																\

#define OTexture_SET_VTABLE(ClassName)											\
	Texture_SET_VTABLE(ClassName)												\
	__VIRTUAL_SET(ClassName, OTexture, write);									\

#define OTexture_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Texture_ATTRIBUTES;															\
																				\
	/* object index */															\
	int objectIndex;															\

// A texture which has the logic to be allocated in graphic memory
__CLASS(OTexture);

//use a OTexture when you want to show a static background or a character that must be scaled according
//its deep on the screen so there exists consistency between the deep and the size of the character


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef const TextureDefinition OTextureDefinition;
typedef const OTextureDefinition OTextureROMDef;


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(OTexture, OTextureDefinition* oTextureDefinition, u16 id);

void OTexture_destructor(OTexture this);
void OTexture_write(OTexture this);
void OTexture_setObjectIndex(OTexture this, int objectIndex);

#endif