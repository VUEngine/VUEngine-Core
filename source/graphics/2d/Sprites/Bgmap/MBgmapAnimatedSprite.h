/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef M_BGMAP_ANIMATED_SPRITE_H_
#define M_BGMAP_ANIMATED_SPRITE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <MBgmapSprite.h>


//=========================================================================================================
// CLASS'S DATA
//=========================================================================================================

/// A MBgmapAnimatedSprite spec
/// @memberof MBgmapAnimatedSprite
typedef struct MBgmapAnimatedSpriteSpec
{
	MBgmapSpriteSpec mBgmapSpriteSpec;

} MBgmapAnimatedSpriteSpec;

/// A MBgmapAnimatedSprite spec that is stored in ROM
/// @memberof MBgmapAnimatedSprite
typedef const MBgmapAnimatedSpriteSpec MBgmapAnimatedSpriteROMSpec;


//=========================================================================================================
// CLASS'S DECLARATION
//=========================================================================================================

///
/// Class FrameBlendBgmapSprite
///
/// Inherits from BgmapSprite
///
/// Animates a sprite whose texture is allocated in BGMAP space.
/// @ingroup graphics-2d-sprites-bgmap
class MBgmapAnimatedSprite : MBgmapSprite
{
	/// @publicsection

	/// Class' constructor
	/// @param owner: SpatialObject to which the sprite attaches to
	/// @param mBgmapAnimatedSpriteSpec: Specification that determines how to configure the sprite
	void constructor(SpatialObject owner, const MBgmapAnimatedSpriteSpec* mBgmapAnimatedSpriteSpec);

	/// Update the animation
	override void updateAnimation();

	/// Set the current multiframe
	/// @param frame: Current animation frame 
	override void setMultiframe(uint16 frame);
}


#endif
