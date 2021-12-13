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

#include <ObjectSprite.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __AVAILABLE_CHAR_OBJECTS	1024


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-object
class ObjectSpriteContainer : Sprite
{
	// object sprites
	VirtualList objectSprites;
	// first object index
	int32 firstObjectIndex;
	// last rendered object index
	int32 lastObjectIndex;
	// spt index
	int32 spt;
	// semaphore to prevent manipulation of VirtualList during interrupt
	bool lockSpritesLists;
	// flag to override to show / hide sprites
	bool hideSprites;

	/// @publicsection
	static void prepareForRendering();
	static void writeDRAM();

	void constructor();
	bool registerSprite(ObjectSprite objectSprite);
	int32 getAvailableObjects();
	int32 getFirstObjectIndex();
	int32 getLastObjectIndex();
	int32 getNextFreeObjectIndex();
	int32 getTotalUsedObjects();
	bool hasRoomFor(int32 numberOfObjects);
	void sortProgressively();
	void position(const Vector3D* position);
	void unregisterSprite(ObjectSprite objectSprite);
	void showSprites(ObjectSprite spareSprite);
	void hideSprites(ObjectSprite spareSprite);
	override void hideForDebug();
	override void showForDebug();
	override int16 doRender(int16 index, bool evenFrame);
	override void setMode(uint16 display, uint16 mode);
	override bool writeTextures();
	override void print(int32 x, int32 y);
	override int32 getTotalPixels();
}


#endif
