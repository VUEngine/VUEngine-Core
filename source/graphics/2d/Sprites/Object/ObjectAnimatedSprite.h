/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_ANIMATED_SPRITE_H_
#define OBJECT_ANIMATED_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/**
 * A ObjectSprite spec
 *
 * @memberof ObjectSprite
 */
typedef struct ObjectAnimatedSpriteSpec
{
	/// it has a Sprite spec at the beginning
	ObjectSpriteSpec objectSpriteSpec;

} ObjectAnimatedSpriteSpec;

typedef const ObjectAnimatedSpriteSpec ObjectAnimatedSpriteROMSpec;


/// @ingroup graphics-2d-sprites-object
class ObjectAnimatedSprite : ObjectSprite
{	
	/// @publicsection
	void constructor(SpatialObject owner, const ObjectAnimatedSpriteSpec* objectAnimatedSpriteSpec);
	override void writeAnimation();
}


#endif
