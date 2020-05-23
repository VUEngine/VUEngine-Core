/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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

#define __MINIMUM_BGMAP_SPRITE_WIDTH				0
#define __MINIMUM_BGMAP_SPRITE_HEIGHT				7


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class BgmapSprite;

typedef s16 (*ParamTableEffectMethod)(BgmapSprite);

/**
 * A BgmapSprite spec
 *
 * @memberof BgmapSprite
 */
typedef struct BgmapSpriteSpec
{
	/// it has a Sprite spec at the beginning
	SpriteSpec spriteSpec;

	/// the display mode (BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;

	/// pointer to affine/hbias manipulation function
	ParamTableEffectMethod applyParamTableEffect;

	/// flag to indicate in which display to show the bg texture
	u16 display;

} BgmapSpriteSpec;

/**
 * A BgmapSprite spec that is stored in ROM
 *
 * @memberof BgmapSprite
 */
typedef const BgmapSpriteSpec BgmapSpriteROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// Sprite which holds a texture and a drawing specification.
/// @ingroup graphics-2d-sprites-bgmap
class BgmapSprite : Sprite
{
	// 3d world position
	DrawSpec drawSpec;
	// pointer to function that implements the param table based effects
	ParamTableEffectMethod applyParamTableEffect;
	// param table offset
	u32 param;
	// param table offset
	s16 paramTableRow;

	/// @publicsection
	void constructor(const BgmapSpriteSpec* bgmapSpriteSpec, Object owner);
	DrawSpec getDrawSpec();
	void invalidateParamTable();
	void setDrawSpec(const DrawSpec* const drawSpec);
	s16 getParamTableRow();
	u32 getParam();
	void setParam(u32 param);
	void putChar(Point* texturePixel, BYTE* newChar);
	void putPixel(Point* texturePixel, Point* charSetPixel, BYTE newPixelColor);
	void processAffineEffects(int gx, int width, int myDisplacement);
	void processHbiasEffects();
	void onTextureRewritten(Object eventFirer);
	void applyAffineTransformations();
	void applyHbiasEffects();
	override bool render(u16 index, bool evenFrame);
	override void rotate(const Rotation* rotation);
	override Scale getScale();
	override void resize(Scale scale, fix10_6 z);
	override void addDisplacement(const PixelVector* displacement);
	override void setMode(u16 display, u16 mode);
	override void releaseTexture();
}


#endif
