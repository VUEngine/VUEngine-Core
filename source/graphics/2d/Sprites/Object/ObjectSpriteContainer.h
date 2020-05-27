/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
	int firstObjectIndex;
	// last rendered object index
	int lastRenderedObjectIndex;
	// total objects
	int totalObjects;
	// OBJs available
	int availableObjects;
	// spt index
	int spt;
	// semaphore to prevent manipulation of VirtualList during interrupt
	bool lockSpritesLists;

	/// @publicsection
	void constructor(int spt, int totalObjects, int firstObjectIndex);
	bool registerSprite(ObjectSprite objectSprite, int numberOfObjects);
	int getAvailableObjects();
	int getFirstObjectIndex();
	int getLastObjectIndex();
	int getNextFreeObjectIndex();
	int getTotalUsedObjects();
	bool hasRoomFor(s32 numberOfObjects);
	void position(const Vector3D* position);
	void unregisterSprite(ObjectSprite objectSprite, s32 numberOfObjects);
	override u16 doRender(u16 index, bool evenFrame);
	override void setPosition(const PixelVector* position);
	override void addDisplacement(const PixelVector* displacement);
	override void setMode(u16 display, u16 mode);
	override bool writeTextures();
	override void print(int x, int y);
	override int getTotalPixels();
}


#endif
