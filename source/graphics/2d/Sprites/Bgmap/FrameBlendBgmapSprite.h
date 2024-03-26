/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef FRAME_BLEND_BGMAP_SPRITE_H_
#define FRAME_BLEND_BGMAP_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * A FrameBlendBgmapSprite spec
 *
 * @memberof FrameBlendBgmapSprite
 */
typedef struct FrameBlendBgmapSpriteSpec
{
	/// it has a Sprite spec at the beginning
	BgmapSpriteSpec bgmapSpriteSpec;

} FrameBlendBgmapSpriteSpec;

/**
 * A FrameBlendBgmapSprite spec that is stored in ROM
 *
 * @memberof FrameBlendBgmapSprite
 */
typedef const FrameBlendBgmapSpriteSpec FrameBlendBgmapSpriteROMSpec;

/// @ingroup graphics-2d-sprites-bgmap
class FrameBlendBgmapSprite : BgmapSprite
{
	uint8 actualFrame;
	
	/// @publicsection
	void constructor(SpatialObject owner, const FrameBlendBgmapSpriteSpec* frameBlendBgmapSpriteSpec);
	override int16 doRender(int16 index);
}


#endif
