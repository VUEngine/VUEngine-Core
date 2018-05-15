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

#ifndef BGMAP_SPRITE_H_
#define BGMAP_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <AnimationController.h>
#include <MiscStructs.h>
#include <BgmapTexture.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __UPDATE_HEAD	0x0F
#define __UPDATE_G		0x01
#define __UPDATE_PARAM	0x02
#define __UPDATE_SIZE	0x04
#define __UPDATE_M		0x08

#define __G_DISPLACEMENT_BECAUSE_WH_0_EQUALS_1		1
#define __WORLD_SIZE_DISPLACEMENT					1
#define __GX_LIMIT									511
#define __GY_LIMIT									223


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class BgmapSprite;

typedef struct BgmapSpriteDefinition
{
	// it has a Sprite definition at the beginning
	SpriteDefinition spriteDefinition;

	// the display mode (BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;

	// pointer to affine/hbias manipulation function
	s16 (*applyParamTableEffect)(BgmapSprite);

	// flag to indicate in which display to show the bg texture
	u16 display;


} BgmapSpriteDefinition;

typedef const BgmapSpriteDefinition BgmapSpriteROMDef;
typedef s16 (*ParamTableEffectMethod)(BgmapSprite);

class BgmapSprite : Sprite
{
	/**
	* @var DrawSpec 				drawSpec
	* @brief						3d world position
	* @memberof					BgmapSprite
	*/
	DrawSpec drawSpec;
	/**
	* @var u32 					param
	* @brief						param table offset
	* @memberof					BgmapSprite
	*/
	u32 param;
	/**
	* @var s16 					paramTableRow
	* @brief						param table offset
	* @memberof					BgmapSprite
	*/
	s16 paramTableRow;
	/**
	* @var void(*)(BgmapSprite) 	paramTableEffect
	* @brief						pointer to function that implements the param table based effects
	* @memberof					BgmapSprite
	*/
	s16 (*applyParamTableEffect)(BgmapSprite);

	void constructor(BgmapSprite this, const BgmapSpriteDefinition* bgmapSpriteDefinition, Object owner);
	DrawSpec getDrawSpec(BgmapSprite this);
	void invalidateParamTable(BgmapSprite this);
	void setDrawSpec(BgmapSprite this, const DrawSpec* const drawSpec);
	s16 getParamTableRow(BgmapSprite this);
	u32 getParam(BgmapSprite this);
	void setParam(BgmapSprite this, u32 param);
	void putChar(BgmapSprite this, Point* texturePixel, BYTE* newChar);
	void putPixel(BgmapSprite this, Point* texturePixel, Point* charSetPixel, BYTE newPixelColor);
	void processAffineEffects(BgmapSprite this, int gx, int width, int myDisplacement);
	void processHbiasEffects(BgmapSprite this);
	override void render(BgmapSprite this, bool evenFrame);
	override void rotate(BgmapSprite this, const Rotation* rotation);
	override Scale getScale(BgmapSprite this);
	override void applyAffineTransformations(BgmapSprite this);
	override void applyHbiasEffects(BgmapSprite this);
	override void resize(BgmapSprite this, Scale scale, fix10_6 z);
	override void calculateParallax(BgmapSprite this, fix10_6 z);
	override void addDisplacement(BgmapSprite this, const PixelVector* displacement);
	override void setMode(BgmapSprite this, u16 display, u16 mode);
}


#endif
