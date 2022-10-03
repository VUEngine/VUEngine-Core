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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MBgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * A MBgmapAnimatedSprite spec
 *
 * @memberof MBgmapAnimatedSprite
 */
typedef struct MBgmapAnimatedSpriteSpec
{
	/// it has a Sprite spec at the beginning
	MBgmapSpriteSpec mBgmapSpriteSpec;

} MBgmapAnimatedSpriteSpec;

/**
 * A MBgmapAnimatedSprite spec that is stored in ROM
 *
 * @memberof MBgmapAnimatedSprite
 */
typedef const MBgmapAnimatedSpriteSpec MBgmapAnimatedSpriteROMSpec;


/// @ingroup graphics-2d-sprites-bgmap
class MBgmapAnimatedSprite : MBgmapSprite
{
	/// @publicsection
	void constructor(const MBgmapAnimatedSpriteSpec* mBgmapAnimatedSpriteSpec, ListenerObject owner);
	override void writeAnimation();
}


#endif
