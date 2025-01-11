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

	/// List of managed object sprites
	VirtualList objectSprites;
	
	/// List node used to Z sort object sprites over time
	VirtualNode sortingSpriteNode;

	/// Index of the first OBJECT
	int32 firstObjectIndex;

	/// Index of the last OBJECT
	int32 lastObjectIndex;

	/// SPT index that this container manages
	int32 spt;

	/// @publicsection

	/// Reset the state of the class's attributes.
	static void reset();

	/// Prepare the class' global state for rendering.
	static void prepareForRendering();

	/// Finish rendering.
	static void finishRendering();

	/// Write cached OBJECT settings to DRAM
	static void writeDRAM();

	/// Class' constructor
	void constructor();

	/// Register this sprite with the appropriate sprites manager.
	override void registerWithManager();

	/// Unegister this sprite with the appropriate sprites manager.	
	override void unregisterWithManager();

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

	/// Print the container's statistics.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	override void print(int32 x, int32 y);

	/// Register a sprite to be managed
	/// @param objectSprite: Sprite to be managed
	/// @return True if the sprite was successfully registered; false otherwise
	bool registerSprite(ObjectSprite objectSprite);

	/// Unregister a sprite to be managed
	/// @param objectSprite: Sprite to no longer manage
	void unregisterSprite(ObjectSprite objectSprite);

	/// Z sort over time the managed sprites.
	/// @param complete: Flag to indicate if the sorting must be complete or deferred
	/// @return True if some sprites was moved to another position in the list
	bool sortProgressively(bool complete);

	/// Render the managed sprites
	/// @param evenFrame: Flag to control transparency effects
	/// @param updateAnimations: Flag to allow or prevent animations to be updated
	void renderSprites(bool evenFrame, bool updateAnimations);

	/// Retrieve the total number of OBJECTs used by all the managed sprites.
	/// @return Total number of OBJECTs used by all the managed sprites
	int32 getTotalUsedObjects();
}

#endif
