/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_SPRITE_H_
#define OBJECT_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __OBJECT_SPRITE_CHAR_SHOW_MASK			0xC000
#define __OBJECT_SPRITE_CHAR_HIDE_MASK			0x0000

#define __OBJECT_SPRITE_FLIP_X_DISPLACEMENT		8
#define __OBJECT_SPRITE_FLIP_Y_DISPLACEMENT		8

//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A ObjectSprite spec
 *
 * @memberof ObjectSprite
 */
typedef struct ObjectSpriteSpec
{
	/// it has a Sprite spec at the beginning
	SpriteSpec spriteSpec;

	/// the display mode (BGMAP, AFFINE, H-BIAS)
	uint16 bgmapMode;

	/// flag to indicate in which display to show the bg texture
	uint16 display;

} ObjectSpriteSpec;

/**
 * A ObjectSprite spec that is stored in ROM
 *
 * @memberof ObjectSprite
 */
typedef const ObjectSpriteSpec ObjectSpriteROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-object
class ObjectSprite : Sprite
{
	// number of objects
	int16 totalObjects;

	// parent sprite
	ObjectSpriteContainer objectSpriteContainer;
	ObjectTextureSource objectTextureSource;

	// cache some attributes to speed up rendering
	uint16 fourthWordValue;
	uint8 cols;
	uint8 rows;
	int8 xDisplacementIncrement;
	int8 yDisplacementIncrement;
	int8 xDisplacementDelta;
	int8 yDisplacementDelta;

	/// @publicsection
	void constructor(SpatialObject owner, const ObjectSpriteSpec* objectSpriteSpec);
	int16 getTotalObjects();
	void invalidateObjectSpriteContainer();
	void resetTotalObjects();

	override void registerWithManager();
	override void unregisterWithManager();
	override int16 doRender(int16 index, bool evenFrame);
	override void setRotation(const Rotation* rotation);
	override int32 getTotalPixels();
}


#endif
