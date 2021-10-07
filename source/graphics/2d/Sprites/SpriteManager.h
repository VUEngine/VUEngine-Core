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

#ifndef SPRITE_MANAGER_H_
#define SPRITE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Sprite.h>
#include <ObjectSpriteContainer.h>



//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __TOTAL_OBJECT_SEGMENTS 	4


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * Sprites List
 *
 * @memberof SpriteManager
 */
typedef struct SpritesList
{
	const void* spriteClassVTable;
	VirtualList sprites;

} SpritesList;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites
singleton class SpriteManager : Object
{
	// Sprites to render
	VirtualList sprites;
	// Object sprite containers
	VirtualList objectSpriteContainers;
	// Sprites with special effects
	VirtualList specialSprites;
	// Textures pending update during writeDRAM
	VirtualList texturesToUpdate;
	// pixels drawn
	int32 totalPixelsDrawn;
	// number of rows to write in affine transformations
	int16 maximumParamTableRowsToComputePerCall;
	// semaphore to prevent manipulation of VirtualList during interrupt
	bool lockSpritesLists;
	// Flag to distinguish between even and odd game frames, needed for sprite transparency.
	bool evenFrame;
	// next world layer
	int16 freeLayer;
	// number of rows to write in texture's writing
	int8 texturesMaximumRowsToWrite;
	// flag to control texture's writing deferring
	int8 deferParamTableEffects;
	// delay before writing again
	int8 waitToWriteSpriteTextures;
	// flag to prevent race conditions on texturesToUpdate list
	bool lockTextureList;

	/// @publicsection
	static SpriteManager getInstance();

	Sprite createSprite(SpriteSpec* spriteSpec, Object owner);
	void registerSprite(Sprite sprite, bool hasEffects);
	void unregisterSprite(Sprite sprite, bool hasEffects);
	void updateTexture(Texture texture);
	void deferParamTableEffects(bool deferAffineTransformations);
	void destructor();
	void disposeSprite(Sprite sprite);
	int8 getFreeLayer();
	int32 getNumberOfSprites();
	int32 getMaximumParamTableRowsToComputePerCall();
	Sprite getSpriteAtPosition(int16 position);
	int16 getSpritePosition(Sprite sprite);
	int8 getTexturesMaximumRowsToWrite();
	void computeTotalPixelsDrawn();
	void print(int32 x, int32 y, bool resumed);
	void printObjectSpriteContainersStatus(int32 x, int32 y);
	void showSprites(Sprite spareSprite, bool showPrinting);
	void hideSprites(Sprite spareSprite, bool hidePrinting);
	void render();
	void stopRendering();
	void reset();
	void setMaximumParamTableRowsToComputePerCall(int32 maximumAffineRowsToComputePerCall);
	void setTexturesMaximumRowsToWrite(uint8 texturesMaximumRowsToWrite);
	void setupObjectSpriteContainers(int16 size[__TOTAL_OBJECT_SEGMENTS], int16 z[__TOTAL_OBJECT_SEGMENTS]);
	ObjectSpriteContainer getObjectSpriteContainer(fix10_6 z);
	ObjectSpriteContainer getObjectSpriteContainerBySegment(int32 segment);
	void sort();
	bool sortProgressively();
	void writeTextures();
	void prepareAll();
	bool isEvenFrame();
	void writeDRAM();
}


#endif
