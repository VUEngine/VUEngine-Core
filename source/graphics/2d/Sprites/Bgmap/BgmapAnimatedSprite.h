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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>
#include <AnimationController.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * A BgmapAnimatedSprite spec
 *
 * @memberof BgmapAnimatedSprite
 */
typedef struct BgmapAnimatedSpriteSpec
{
	/// it has a Sprite spec at the beginning
	BgmapSpriteSpec bgmapSpriteSpec;

} BgmapAnimatedSpriteSpec;

/**
 * A BgmapAnimatedSprite spec that is stored in ROM
 *
 * @memberof BgmapAnimatedSprite
 */
typedef const BgmapAnimatedSpriteSpec BgmapAnimatedSpriteROMSpec;

/// @ingroup graphics-2d-sprites-bgmap
class BgmapAnimatedSprite : BgmapSprite
{
	// bgmap's source coordinates
	TextureSource originalTextureSource;

	/// @publicsection
	void constructor(const BgmapAnimatedSpriteSpec* bgmapAnimatedSpriteSpec, ListenerObject owner);
	override void writeAnimation();
}


#endif
