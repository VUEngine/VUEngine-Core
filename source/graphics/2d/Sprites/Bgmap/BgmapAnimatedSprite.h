/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BGMAP_ANIMATED_SPRITE_H_
#define BGMAP_ANIMATED_SPRITE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <BgmapSprite.h>


//=========================================================================================================
// CLASS'S DATA
//=========================================================================================================

/// A BgmapAnimatedSprite spec
/// @memberof BgmapAnimatedSprite
typedef struct BgmapAnimatedSpriteSpec
{
	/// it has a Sprite spec at the beginning
	BgmapSpriteSpec bgmapSpriteSpec;

} BgmapAnimatedSpriteSpec;

/// A BgmapAnimatedSprite spec that is stored in ROM
/// @memberof BgmapAnimatedSprite
typedef const BgmapAnimatedSpriteSpec BgmapAnimatedSpriteROMSpec;

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
class BgmapAnimatedSprite : BgmapSprite
{
	/// @publicsection
	/// Class' constructor
	/// @param owner: SpatialObject to which the sprite attaches to
	/// @param bgmapAnimatedSpriteSpec: Specification that determines how to configure the sprite
	void constructor(SpatialObject owner, const BgmapAnimatedSpriteSpec* bgmapAnimatedSpriteSpec);

	/// 
	override void updateAnimation();
}


#endif
