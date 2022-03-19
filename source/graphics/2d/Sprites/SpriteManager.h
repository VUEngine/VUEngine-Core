/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

	/// @publicsection
	static SpriteManager getInstance();

	Sprite createSprite(SpriteSpec* spriteSpec, Object owner);
	bool registerSprite(Sprite sprite, bool hasEffects);
	void unregisterSprite(Sprite sprite, bool hasEffects);
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
	void renderAndDraw();
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
