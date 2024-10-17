/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_SPRITE_CONTAINER_H_
#define OBJECT_SPRITE_CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class ObjectSprite;

/// @ingroup graphics-2d-sprites-object
class ObjectSpriteContainer : Sprite
{
	// object sprites
	VirtualList objectSprites;
	// Node for z sorting
	VirtualNode sortingSpriteNode;
	// first object index
	int32 firstObjectIndex;
	// last rendered object index
	int32 lastObjectIndex;
	// spt index
	int32 spt;
	// flag to override to show / hide sprites
	bool hideSprites;

	/// @publicsection
	static void reset();
	static void prepareForRendering();
	static void finishRendering();
	static void writeDRAM();

	void constructor();
	bool registerSprite(ObjectSprite objectSprite);
	int32 getAvailableObjects();
	int32 getFirstObjectIndex();
	int32 getLastObjectIndex();
	int32 getNextFreeObjectIndex();
	int32 getTotalUsedObjects();
	bool hasRoomFor(int32 numberOfObjects);
	bool sortProgressively(bool complete);
	void position(const Vector3D* position);
	void unregisterSprite(ObjectSprite objectSprite);
	void showSprites(ObjectSprite spareSprite);
	void hideSprites(ObjectSprite spareSprite);
	void renderSprites(bool evenFrame, bool updateAnimations);

	override void registerWithManager();
	override void unregisterWithManager();
	override void hideForDebug();
	override void forceShow();

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the regio of DRAM that this sprite configured
	override int16 doRender(int16 index);
	override void print(int32 x, int32 y);
	override int32 getTotalPixels();
	override void invalidateRendering();
}


#endif
