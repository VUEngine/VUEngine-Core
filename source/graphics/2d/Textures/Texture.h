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

#ifndef TEXTURE_H_
#define TEXTURE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define Texture_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, write);															\
		__VIRTUAL_DEC(ClassName, void, rewrite);														\

#define Texture_SET_VTABLE(ClassName)																	\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Texture, write);														\
		__VIRTUAL_SET(ClassName, Texture, rewrite);														\

#define Texture_ATTRIBUTES																				\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* char group to use int this texture */														\
		CharSet charSet;																				\
		/* pointer to ROM definition */																	\
		TextureDefinition* textureDefinition;															\
		/* array definition of the map */																\
		u32 mapDisplacement;																			\
		/* texture's id */																				\
		u16 id;																							\
		/* color palette */																				\
		u8 palette;																						\
		/* written flag */																				\
		u8 written;																						\

// A texture which has the logic to be allocated in graphic memory
__CLASS(Texture);


//---------------------------------------------------------------------------------------------------------
//											CLASS'S ROM DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines a background in ROM memory
typedef struct TextureDefinition
{
	// pointer to the char definition
	CharSetDefinition* charSetDefinition;

	// pointer to the bgtexture definition in ROM
	BYTE* mapDefinition;

	// x size, 1 column represents 8 pixels
	u32 cols;

	// y size, 1 row represents 8 pixels
	u32 rows;

	// padding for affine/hbias transformations (cols, rows)
	TexturePadding padding;

	// number of frames
	u32 numberOfFrames;

	// palette index to use
	u32 palette;

} TextureDefinition;

typedef const TextureDefinition TextureROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Texture_constructor(Texture this, TextureDefinition* textureDefinition, u16 id);
void Texture_destructor(Texture this);
void Texture_setDefinition(Texture this, TextureDefinition* textureDefinition);
TextureDefinition* Texture_getDefinition(Texture this);
void Texture_releaseCharSet(Texture this);
void Texture_write(Texture this);
void Texture_rewrite(Texture this);
void Texture_writeHBiasMode(Texture this);
int Texture_getNumberOfChars(Texture this);
TextureDefinition* Texture_getTextureDefinition(Texture this);
u32 Texture_getTotalCols(Texture this);
u32 Texture_getTotalRows(Texture this);
u32 Texture_getNumberOfFrames(Texture this);
CharSet Texture_getCharSet(Texture this, u32 loadIfNeeded);
BYTE* Texture_getMapDefinition(Texture this);
void Texture_setPalette(Texture this, u8 palette);
u8 Texture_getPalette(Texture this);
u32 Texture_getRows(Texture this);
u32 Texture_getCols(Texture this);
u16 Texture_getId(Texture this);
void Texture_putChar(Texture this, Point* texturePixel, BYTE* newChar);
void Texture_putPixel(Texture this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor);
bool Texture_isWritten(Texture this);
void Texture_setMapDisplacement(Texture this, u32 mapDisplacement);


#endif
