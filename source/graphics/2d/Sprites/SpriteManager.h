/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
	// list of sprites to render
	VirtualList sprites;
	// list of sprites to delete
	VirtualList spritesToDispose;
	// list of object sprite containers
	VirtualList objectSpriteContainers;
	// sorting first node
	VirtualNode zSortingFirstNode;
	// sorting second node
	VirtualNode zSortingSecondNode;
	// sprite's texture writing
	Sprite spritePendingTextureWriting;
	// pixels drawn
	int totalPixelsDrawn;
	// number of rows to write in affine transformations
	s16 maximumParamTableRowsToComputePerCall;
	// semaphore to prevent manipulation of VirtualList during interrupt
	bool lockSpritesLists;
	// Flag to distinguish between even and odd game frames, needed for sprite transparency.
	bool evenFrame;
	// next world layer
	u8 freeLayer;
	// number of cycles that the texture writing is idle
	s8 cyclesToWaitForSpriteTextureWriting;
	// number of rows to write in texture's writing
	s8 texturesMaximumRowsToWrite;
	// flag to control texture's writing deferring
	s8 deferParamTableEffects;
	// delay before writing again
	s8 waitToWriteSpriteTextures;

	/// @publicsection
	static SpriteManager getInstance();
	Sprite createSprite(SpriteSpec* spriteSpec, Object owner);
	void deferParamTableEffects(bool deferAffineTransformations);
	void destructor();
	void disposeSprite(Sprite sprite);
	void disposeSprites();
	u8 getFreeLayer();
	int getMaximumParamTableRowsToComputePerCall();
	Sprite getSpriteAtLayer(u8 layer);
	s8 getTexturesMaximumRowsToWrite();
	void computeTotalPixelsDrawn();
	void print(int x, int y, bool resumed);
	void printObjectSpriteContainersStatus(int x, int y);
	void processFreedLayers();
	void processLayers();
	void recoverLayers();
	void render();
	void renderLastLayer();
	void reset();
	void setCyclesToWaitForTextureWriting(u8 cyclesToWaitForTextureWriting);
	void setMaximumParamTableRowsToComputePerCall(int maximumAffineRowsToComputePerCall);
	void setTexturesMaximumRowsToWrite(u8 texturesMaximumRowsToWrite);
	void setupObjectSpriteContainers(s16 size[__TOTAL_OBJECT_SEGMENTS], s16 z[__TOTAL_OBJECT_SEGMENTS]);
	ObjectSpriteContainer getObjectSpriteContainer(int numberOfObjects, fix10_6 z);
	ObjectSpriteContainer getObjectSpriteContainerBySegment(int segment);
	void showLayer(u8 layer);
	void sortLayers();
	void sortLayersProgressively();
	void writeTextures();
	void prepareAll();
}


#endif
