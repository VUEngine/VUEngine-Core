/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef FRAME_BLEND_BGMAP_SPRITE_H_
#define FRAME_BLEND_BGMAP_SPRITE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <BgmapSprite.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A FrameBlendBgmapSprite spec
/// @memberof FrameBlendBgmapSprite
typedef struct FrameBlendBgmapSpriteSpec
{
	/// it has a Sprite spec at the beginning
	BgmapSpriteSpec bgmapSpriteSpec;

} FrameBlendBgmapSpriteSpec;

/// A FrameBlendBgmapSprite spec that is stored in ROM.
/// @memberof FrameBlendBgmapSprite
typedef const FrameBlendBgmapSpriteSpec FrameBlendBgmapSpriteROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class FrameBlendBgmapSprite
///
/// Inherits from BgmapSprite
///
/// Blends two frames of animation into a single one to achieve hi color images with a single sprite.
/// @ingroup graphics-2d-sprites-bgmap
class FrameBlendBgmapSprite : BgmapSprite
{
	/// The frame to show during the game cycle
	uint8 actualFrame;
	
	/// @publicsection

	/// Class' constructor
	/// @param owner: SpatialObject to which the sprite attaches to
	/// @param frameBlendBgmapSpriteSpec: Specification that determines how to configure the sprite
	void constructor(SpatialObject owner, const FrameBlendBgmapSpriteSpec* frameBlendBgmapSpriteSpec);

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the region of DRAM that this sprite manages
	override int16 doRender(int16 index);
}


#endif
