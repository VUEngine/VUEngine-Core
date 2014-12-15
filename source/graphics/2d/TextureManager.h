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

#ifndef TEXTURE_MANAGER_H_
#define TEXTURE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Texture.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */
// declare the virtual methods
#define TextureManager_METHODS						\
		Object_METHODS								\


// declare the virtual methods which are redefined
#define TextureManager_SET_VTABLE(ClassName)					\
		Object_SET_VTABLE(ClassName)					\


__CLASS(TextureManager);




//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

// it is a singleton!
TextureManager TextureManager_getInstance();

// class's destructor
void TextureManager_destructor(TextureManager this);

// reset
void TextureManager_reset(TextureManager this);

// deallocate texture from bgmap graphic memory
void TextureManager_free(TextureManager this, Texture texture);

// retrieve free bgmap segment number
u8 TextureManager_getFreeBgmap(TextureManager this);

// allocate bgmap text boxes
// this bgmap segment is handled as one only bgmap defined inside so, only
// TextureManager.xOffset[textbgmap][0] is used
void TextureManager_allocateText(TextureManager this, Texture texture);

// load and retrieve a texture
Texture TextureManager_get(TextureManager this, TextureDefinition* textureDefinition);

// retrieve x offset
u8 TextureManager_getXOffset(TextureManager this, int id);

// retrieve y offset
u8 TextureManager_getYOffset(TextureManager this, int id);

// retrieve bgmap segment
u8 TextureManager_getBgmapSegment(TextureManager this, int id);

// print status
void TextureManager_print(TextureManager this, int x, int y);

#endif /*TextureManager_H_*/
