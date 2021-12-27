/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

typedef int16 (*ParamTableEffectMethod)(BgmapSprite);

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
	uint16 bgmapMode;

	/// pointer to affine/hbias manipulation function
	ParamTableEffectMethod applyParamTableEffect;

	/// flag to indicate in which display to show the bg texture
	uint16 display;

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
	uint32 param;
	// param table offset
	int16 paramTableRow;

	/// @publicsection
	void constructor(const BgmapSpriteSpec* bgmapSpriteSpec, Object owner);
	DrawSpec getDrawSpec();
	void invalidateParamTable();
	void setDrawSpec(const DrawSpec* const drawSpec);
	int16 getParamTableRow();
	uint32 getParam();
	void setParam(uint32 param);
	void putChar(Point* texturePixel, uint32* newChar);
	void putPixel(Point* texturePixel, Point* charSetPixel, BYTE newPixelColor);
	void processAffineEffects();
	void processHbiasEffects();
	void onTextureRewritten(Object eventFirer);
	void applyAffineTransformations();
	void applyHbiasEffects();

	virtual bool hasSpecialEffects();

	override int16 doRender(int16 index, bool evenFrame);
	override void processEffects();
	override void rotate(const Rotation* rotation);
	override Scale getScale();
	override void resize(Scale scale, fix10_6 z);
	override void setMode(uint16 display, uint16 mode);
	override void registerWithManager();
}


#endif
