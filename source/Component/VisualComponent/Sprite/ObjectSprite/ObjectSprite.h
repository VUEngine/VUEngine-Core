/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_SPRITE_H_
#define OBJECT_SPRITE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Sprite.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __OBJECT_SPRITE_CHAR_SHOW_MASK			0xC000
#define __OBJECT_SPRITE_CHAR_HIDE_MASK			0x0000

#define __OBJECT_SPRITE_FLIP_X_DISPLACEMENT		8
#define __OBJECT_SPRITE_FLIP_Y_DISPLACEMENT		8

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A ObjectSprite spec
/// @memberof ObjectSprite
typedef struct ObjectSpriteSpec
{
	/// it has a Sprite spec at the beginning
	SpriteSpec spriteSpec;

	/// Flag to indicate in which display to show the texture (__WORLD_ON, __WORLD_LON or __WORLD_RON)
	uint16 display;

} ObjectSpriteSpec;

/// A ObjectSprite spec that is stored in ROM
/// @memberof ObjectSprite
typedef const ObjectSpriteSpec ObjectSpriteROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class ObjectSprite
///
/// Inherits from Sprite
///
/// Displays a texture in OBJECT space.
class ObjectSprite : Sprite
{
	/// @protectedsection

	/// The number of OBJECTs that the sprite uses
	int16 totalObjects;

	/// Texture's displacement in the map array
	ObjectTextureSource objectTextureSource;

	// Cache some attributes to speed up rendering

	/// Cache the computed value for the 4th entry of OBJECTs
	uint16 fourthWordValue;

	/// Cache of the number of cols for the sprite's texture
	uint8 cols;

	/// Cache of the number of cols for the sprite's texture
	uint8 rows;

	/// Cache of the x displacamente used to flip horizontally the image
	int8 xDisplacementIncrement;

	/// Cache of the y displacamente used to flip vertically the image
	int8 yDisplacementIncrement;

	/// Cache of a displacement that is necessary to flip images horizontally 
	/// while keeping their center at the same position on the screen
	int8 xDisplacementDelta;

	/// Cache of a displacement that is necessary to flip images vertically 
	/// while keeping their center at the same position on the screen
	int8 yDisplacementDelta;

	/// @publicsection

	/// Class' constructor
	/// @param owner: Entity to which the sprite attaches to
	/// @param objectSpriteSpec: Specification that determines how to configure the sprite
	void constructor(Entity owner, const ObjectSpriteSpec* objectSpriteSpec);

	/// Called to release the component's resources.
	override void releaseResources();

	/// Retrieve the basic class of this kind of sprite.
	/// @return ClassPointer the basic class
	override ClassPointer getBasicType();

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the region of DRAM that this sprite manages
	override int16 doRender(int16 index);

	/// Set the current multiframe.
	/// @param frame: Current animation frame 
	override void setMultiframe(uint16 frame);

	/// Set the sprite's rotation.
	/// @param rotation: Rotation to apply to the sprite 
	override void setRotation(const Rotation* rotation);

	/// Retrieve the sprite's total number of pixels actually displayed.
	/// @return Sprite's total number of pixels actually displayed
	override int32 getTotalPixels();

	/// Force the computation of the number of OBJECTs that the sprite uses.
	void resetTotalObjects();

	/// Retrieve the number of OBJECTs that the sprite uses.
	/// @return Number of OBJECTs that the sprite uses
	int16 getTotalObjects();
}

#endif
