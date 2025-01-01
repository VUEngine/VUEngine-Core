/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_ANIMATED_SPRITE_H_
#define OBJECT_ANIMATED_SPRITE_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ObjectSprite.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A ObjectSprite spec
/// @memberof ObjectSprite
typedef struct ObjectAnimatedSpriteSpec
{
	/// it has a Sprite spec at the beginning
	ObjectSpriteSpec objectSpriteSpec;

} ObjectAnimatedSpriteSpec;

/// A ObjectAnimatedSpriteSpec spec that is stored in ROM
/// @memberof ObjectAnimatedSprite
typedef const ObjectAnimatedSpriteSpec ObjectAnimatedSpriteROMSpec;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class BgmapAnimatedSprite
///
/// Inherits from BgmapSprite
///
/// Animates a sprite whose graphics are allocated in OBJECT space.
class ObjectAnimatedSprite : ObjectSprite
{	
	/// @publicsection

	/// Class' constructor
	/// @param owner: GameObject to which the sprite attaches to
	/// @param objectAnimatedSpriteSpec: Specification that determines how to configure the sprite
	void constructor(GameObject owner, const ObjectAnimatedSpriteSpec* objectAnimatedSpriteSpec);

	/// Update the animation.
	override void updateAnimation();
}


#endif
