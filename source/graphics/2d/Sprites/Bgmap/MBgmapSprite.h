/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef M_BGMAP_SPRITE_H_
#define M_BGMAP_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A MBgmapSprite spec
 *
 * @memberof MBgmapSprite
 */
typedef struct MBgmapSpriteSpec
{
	/// the normal sprite spec
	BgmapSpriteSpec bgmapSpriteSpec;

	/// texture to use with the sprite
	TextureSpec** textureSpecs;

	/// SCX/SCY value
	uint32 scValue;

	/// flag to loop the x axis
	int32 xLoop;

	/// flag to loop the y axis
	int32 yLoop;

	/// Bounds the sprite's size to provide culling; if 0, the value is inferred from the texture
	int32 width;

	/// Bounds the sprite's size to provide culling; if 0, the value is inferred from the texture
	int32 height;

} MBgmapSpriteSpec;

/**
 * A MBgmapSprite spec that is stored in ROM
 *
 * @memberof MBgmapSprite
 */
typedef const MBgmapSpriteSpec MBgmapSpriteROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-bgmap
class MBgmapSprite : BgmapSprite
{
	VirtualList textures;
	// pinter to spec
	const MBgmapSpriteSpec* mBgmapSpriteSpec;
	// to speed up rendering
	uint32 textureXOffset;
	// to speed up rendering
	uint32 textureYOffset;

	/// @publicsection
	void constructor(SpatialObject owner, const MBgmapSpriteSpec* mBgmapSpriteSpec);
	override int16 doRender(int16 index);
	override void writeTextures(int16 maximumTextureRowsToWrite);
}


#endif
