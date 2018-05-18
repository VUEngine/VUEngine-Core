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
#include <ObjectSpriteContainerManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

singleton class SpriteManager : Object
{
	/**
	 * @var VirtualList	sprites
	 * @brief 			list of sprites to render
	 * @memberof		SpriteManager
	 */
	VirtualList sprites;
	/**
	 * @var VirtualList	spritesToDispose
	 * @brief 			list of sprites to delete
	 * @memberof		SpriteManager
	 */
	VirtualList spritesToDispose;
	/**
	 * @var VirtualNode	node
	 * @brief 			sorting nodes
	 * @memberof		SpriteManager
	 */
	VirtualNode zSortingFirstNode;
	/**
	 * @var VirtualNode	nextNode
	 * @brief
	 * @memberof		SpriteManager
	 */
	VirtualNode zSortingSecondNode;
	/**
	 * @var Sprite		spritePendingTextureWriting
	 * @brief 			sprite's texture writing
	 * @memberof		SpriteManager
	 */
	Sprite spritePendingTextureWriting;
	/**
	 * @var bool		lockSpritesLists
	 * @brief 			semaphore to prevent manipulation of VirtualList during interrupt
	 * @memberof		SpriteManager
	 */
	bool lockSpritesLists;
	/**
	 * @var bool		evenFrame
	 * @brief 			Flag to distinguish between even and odd game frames, needed for sprite transparency.
	 * @memberof		SpriteManager
	 */
	bool evenFrame;
	/**
	 * @var u8			freeLayer
	 * @brief 			next world layer
	 * @memberof		SpriteManager
	 */
	u8 freeLayer;
	/**
	 * @var s8			cyclesToWaitForSpriteTextureWriting
	 * @brief 			number of cycles that the texture writing is idle
	 * @memberof		SpriteManager
	 */
	s8 cyclesToWaitForSpriteTextureWriting;
	/**
	 * @var s8			texturesMaximumRowsToWrite
	 * @brief 			number of rows to write in texture's writing
	 * @memberof		SpriteManager
	 */
	s8 texturesMaximumRowsToWrite;
	/**
	 * @var s8			maximumParamTableRowsToComputePerCall
	 * @brief 			number of rows to write in affine transformations
	 * @memberof		SpriteManager
	 */
	s8 maximumParamTableRowsToComputePerCall;
	/**
	 * @var s8			deferParamTableEffects
	 * @brief 			flag to control texture's writing deferring
	 * @memberof		SpriteManager
	 */
	s8 deferParamTableEffects;
	/**
	 * @var s8			waitToWriteSpriteTextures
	 * @brief 			delay before writing again
	 * @memberof		SpriteManager
	 */
	s8 waitToWriteSpriteTextures;

	static SpriteManager getInstance();
	Sprite createSprite(SpriteDefinition* spriteDefinition, Object owner);
	void deferParamTableEffects(bool deferAffineTransformations);
	void deferTextureWriting(bool deferTextureWriting);
	void destructor();
	void disposeSprite(Sprite sprite);
	void disposeSprites();
	u8 getFreeLayer();
	int getMaximumParamTableRowsToComputePerCall();
	Sprite getSpriteAtLayer(u8 layer);
	s8 getTexturesMaximumRowsToWrite();
	void print(int x, int y, bool resumed);
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
	void showLayer(u8 layer);
	void sortLayers();
	void sortLayersProgressively();
	void writeTextures();
}


#endif
