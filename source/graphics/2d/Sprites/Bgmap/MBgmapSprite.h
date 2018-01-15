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

#ifndef M_BGMAP_SPRITE_H_
#define M_BGMAP_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define MBgmapSprite_METHODS(ClassName)																	\
		BgmapSprite_METHODS(ClassName)																	\

// declare the virtual methods which are redefined
#define MBgmapSprite_SET_VTABLE(ClassName)																\
		BgmapSprite_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, MBgmapSprite, position);												\
		__VIRTUAL_SET(ClassName, MBgmapSprite, setPosition);											\
		__VIRTUAL_SET(ClassName, MBgmapSprite, render);													\
		__VIRTUAL_SET(ClassName, MBgmapSprite, getPosition);											\
		__VIRTUAL_SET(ClassName, MBgmapSprite, addDisplacement);										\
		__VIRTUAL_SET(ClassName, MBgmapSprite, resize);													\
		__VIRTUAL_SET(ClassName, MBgmapSprite, setMode);														\
		__VIRTUAL_SET(ClassName, MBgmapSprite, writeTextures);											\
		__VIRTUAL_SET(ClassName, MBgmapSprite, areTexturesWritten);										\

#define MBgmapSprite_ATTRIBUTES																			\
		BgmapSprite_ATTRIBUTES																			\
		/**
		 * @var VirtualList 			textures
		 * @brief						this is our texture
		 * @memberof 					MBgmapSprite
		 */																								\
		VirtualList textures;																			\
		/**
		 * @var MBgmapSpriteDefinition*	mBgmapSpriteDefinition
		 * @brief						pinter to definition
		 * @memberof 					MBgmapSprite
		 */																								\
		const MBgmapSpriteDefinition* mBgmapSpriteDefinition;											\
		/**
		 * @var u32 					textureXOffset
		 * @brief						to speed up rendering
		 * @memberof 					MBgmapSprite
		 */																								\
		u32 textureXOffset;																				\
		/**
		 * @var u32 					textureYOffset
		 * @brief						to speed up rendering
		 * @memberof 					MBgmapSprite
		 */																								\
		u32 textureYOffset;																				\
		/**
		 * @var Point 					sizeMultiplier
		 * @brief						Multiple BGMAP expansion
		 * @memberof 					MBgmapSprite
		 */																								\
		Point sizeMultiplier;

__CLASS(MBgmapSprite);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct MBgmapSpriteDefinition
{
	// the normal sprite definition
	BgmapSpriteDefinition bgmapSpriteDefinition;

	// texture to use with the sprite
	TextureDefinition** textureDefinitions;

	// SCX/SCY value
	u32 scValue;

	// flag to loop the x axis
	int xLoop;

	// flag to loop the y axis
	int yLoop;

} MBgmapSpriteDefinition;

typedef const MBgmapSpriteDefinition MBgmapSpriteROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(MBgmapSprite, const MBgmapSpriteDefinition* mBgmapSpriteDefinition, Object );

void MBgmapSprite_constructor(MBgmapSprite this, const MBgmapSpriteDefinition* mBgmapSpriteDefinition, Object owner);
void MBgmapSprite_destructor(MBgmapSprite this);

void MBgmapSprite_addDisplacement(MBgmapSprite this, const PixelVector* displacement);
PixelVector MBgmapSprite_getPosition(MBgmapSprite this);
void MBgmapSprite_position(MBgmapSprite this, const Vector3D* position);
void MBgmapSprite_render(MBgmapSprite this);
void MBgmapSprite_resize(MBgmapSprite this, Scale scale, fix10_6 z);
void MBgmapSprite_setPosition(MBgmapSprite this, const PixelVector* position);
void MBgmapSprite_setMode(MBgmapSprite this, u16 display, u16 mode);
bool MBgmapSprite_writeTextures(MBgmapSprite this);
bool MBgmapSprite_areTexturesWritten(MBgmapSprite this);


#endif
