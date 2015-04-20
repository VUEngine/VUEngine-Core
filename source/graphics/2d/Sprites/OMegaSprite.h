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

#ifndef O_MEGA_SPRITE_H_
#define O_MEGA_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <OSprite.h>
#include <MiscStructs.h>
#include <Texture.h>


//---------------------------------------------------------------------------------------------------------
// 											 MACROS
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define OMegaSprite_METHODS															\
	Sprite_METHODS																	\

// declare the virtual methods which are redefined
#define OMegaSprite_SET_VTABLE(ClassName)											\
	Sprite_SET_VTABLE(ClassName)													\
	__VIRTUAL_SET(ClassName, OMegaSprite, render);									\
	__VIRTUAL_SET(ClassName, OMegaSprite, getPosition);								\
	__VIRTUAL_SET(ClassName, OMegaSprite, setPosition);								\
	__VIRTUAL_SET(ClassName, OMegaSprite, synchronizePosition);						\
	__VIRTUAL_SET(ClassName, OMegaSprite, setDirection);							\
	__VIRTUAL_SET(ClassName, OMegaSprite, calculateParallax);						\
	
#define OMegaSprite_ATTRIBUTES														\
																					\
	/* super's attributes */														\
	Sprite_ATTRIBUTES;																\
																					\
	/* o sprites */																	\
	VirtualList oSprites;															\
																					\
	/* o sprites */																	\
	u16 availableObjects;															\
																					\
	/* o sprites */																	\
	u16 nextAvailableObject;															\
																					\
	/* spt index */																	\
	u8 spt;																			\

// declare a OMegaSprite, which holds a texture and a drawing specification
__CLASS(OMegaSprite);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void OMegaSprite_constructor(OMegaSprite this, u8 spt);
void OMegaSprite_destructor(OMegaSprite this);
s32 OMegaSprite_addOSprite(OMegaSprite this, OSprite oSprite, int numberOfObjects);
void OMegaSprite_removeOSprite(OMegaSprite this, OSprite oSprite, int numberOfObjects);
bool OMegaSprite_hasRoomFor(OMegaSprite this, int numberOfObjects);
void OMegaSprite_setDirection(OMegaSprite this, int axis, int direction);
VBVec2D OMegaSprite_getPosition(OMegaSprite this);
void OMegaSprite_setPosition(OMegaSprite this, VBVec2D position);
void OMegaSprite_synchronizePosition(OMegaSprite this, VBVec3D position3D);
void OMegaSprite_calculateParallax(OMegaSprite this, fix19_13 z);
void OMegaSprite_render(OMegaSprite this);


#endif