/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_SPRITE_CONTAINER_H_
#define OBJECT_SPRITE_CONTAINER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Sprite.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class ObjectSprite;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class ObjectSpriteContainer
///
/// Inherits from Sprite
///
/// Manages ObjectSprites that are displayed in the same SPT.
class ObjectSpriteContainer : Sprite
{
	/// @protectedsection

	/// List node used to Z sort object sprites over time
	VirtualNode sortingSpriteNode;

	/// Index of the first OBJECT
	int32 firstObjectIndex;

	/// Index of the last OBJECT
	int32 lastObjectIndex;

	/// SPT index that this container manages
	int32 spt;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Retrieve the basic class of this kind of sprite.
	/// @return ClassPointer the basic class
	override ClassPointer getBasicType();

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the region of DRAM that this sprite manages
	override int16 doRender(int16 index);
	
	/// Retrieve the total number of pixels actually displayed by all the managed sprites.
	/// @return Total number of pixels displayed by all the managed sprites
	override int32 getTotalPixels();
	
	/// Forcefully show the sprites
	override void forceShow();

	/// Forcefully hide the sprites
	override void forceHide();

	/// Z sort over time the managed sprites.
	/// @param complete: Flag to indicate if the sorting must be complete or deferred
	/// @return True if some sprites was moved to another position in the list
	bool sortProgressively(bool complete);

	/// Retrieve the total number of OBJECTs used by all the managed sprites.
	/// @return Total number of OBJECTs used by all the managed sprites
	int32 getTotalUsedObjects();
}

#endif
