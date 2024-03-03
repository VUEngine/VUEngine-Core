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


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

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
	// The unusual order of the attributes is to optimize data packing as much as possible
	// param table offset
	int16 paramTableRow;
	// param table offset
	uint32 param;
	// pointer to function that implements the param table based effects
	ParamTableEffectMethod applyParamTableEffect;
	// bgmap's source coordinates
	BgmapTextureSource bgmapTextureSource;

	/// @publicsection
	void constructor(SpatialObject owner, const BgmapSpriteSpec* bgmapSpriteSpec);
	void setMode(uint16 display, uint16 mode);
	void invalidateParamTable();
	int16 getParamTableRow();
	uint32 getParam();
	void setParam(uint32 param);
	void putChar(Point* texturePixel, uint32* newChar);
	void putPixel(Point* texturePixel, Point* charSetPixel, BYTE newPixelColor);
	bool onTextureRewritten(ListenerObject eventFirer);
	void applyAffineTransformations();
	void applyHbiasEffects();
	bool hasSpecialEffects();

	override void registerWithManager();
	override void unregisterWithManager();
	override int16 doRender(int16 index);
	override void processEffects();
	override void setRotation(const Rotation* rotation);
	override Scale getScale();
	override void setScale(const Scale* scale);
	override int32 getTotalPixels();
}


#endif
