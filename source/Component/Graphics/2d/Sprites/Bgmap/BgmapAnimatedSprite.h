/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BGMAP_ANIMATED_SPRITE_H_
#define BGMAP_ANIMATED_SPRITE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapSprite.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class BgmapSprite;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class BgmapAnimatedSprite
///
/// Inherits from BgmapSprite
///
/// Animates a sprite whose texture is allocated in BGMAP space.
class BgmapAnimatedSprite : BgmapSprite
{
	/// @publicsection

	/// Class' constructor
	/// @param owner: GameObject to which the sprite attaches to
	/// @param bgmapAnimatedSpriteSpec: Specification that determines how to configure the sprite
	void constructor(GameObject owner, const BgmapAnimatedSpriteSpec* bgmapAnimatedSpriteSpec);

	/// Update the animation.
	override void updateAnimation();
}

#endif
