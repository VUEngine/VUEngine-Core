/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


class MBgmapSprite : BgmapSprite
{
	/**
	* @var VirtualList 			textures
	* @brief						this is our texture
	* @memberof 					MBgmapSprite
	*/
	VirtualList textures;
	/**
	* @var MBgmapSpriteDefinition*	mBgmapSpriteDefinition
	* @brief						pinter to definition
	* @memberof 					MBgmapSprite
	*/
	const MBgmapSpriteDefinition* mBgmapSpriteDefinition;
	/**
	* @var u32 					textureXOffset
	* @brief						to speed up rendering
	* @memberof 					MBgmapSprite
	*/
	u32 textureXOffset;
	/**
	* @var u32 					textureYOffset
	* @brief						to speed up rendering
	* @memberof 					MBgmapSprite
	*/
	u32 textureYOffset;
	/**
	* @var Point 					sizeMultiplier
	* @brief						Multiple BGMAP expansion
	* @memberof 					MBgmapSprite
	*/
	Point sizeMultiplier;

	void constructor(MBgmapSprite this, const MBgmapSpriteDefinition* mBgmapSpriteDefinition, Object owner);
	override void position(MBgmapSprite this, const Vector3D* position);
	override void setPosition(MBgmapSprite this, const PixelVector* position);
	override void render(MBgmapSprite this, bool evenFrame);
	override void addDisplacement(MBgmapSprite this, const PixelVector* displacement);
	override void resize(MBgmapSprite this, Scale scale, fix10_6 z);
	override void setMode(MBgmapSprite this, u16 display, u16 mode);
	override bool writeTextures(MBgmapSprite this);
	override bool areTexturesWritten(MBgmapSprite this);
}


#endif
