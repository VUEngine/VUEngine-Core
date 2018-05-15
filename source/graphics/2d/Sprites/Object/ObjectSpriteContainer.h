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

class ObjectSpriteContainer : Sprite
{
	/**
	* @var VirtualList	objectSprites
	* @brief			object sprites
	* @memberof		ObjectSpriteContainer
	*/
	VirtualList objectSprites;
	/**
	* @var VirtualNode	node
	* @brief			for z sorting
	* @memberof		ObjectSpriteContainer
	*/
	VirtualNode node;
	/**
	* @var VirtualNode	previousNode
	* @brief			for z sorting
	* @memberof		ObjectSpriteContainer
	*/
	VirtualNode previousNode;
	/**
	* @var VirtualNode	objectSpriteNodeToDefragment
	* @brief			next object sprite node to defragment
	* @memberof		ObjectSpriteContainer
	*/
	VirtualNode objectSpriteNodeToDefragment;
	/**
	* @var int			freedObjectIndex
	* @brief			used for defragmentation
	* @memberof		ObjectSpriteContainer
	*/
	int freedObjectIndex;
	/**
	* @var int			firstObjectIndex
	* @brief			first object index
	* @memberof		ObjectSpriteContainer
	*/
	int firstObjectIndex;
	/**
	* @var int			totalObjects
	* @brief			total objects
	* @memberof		ObjectSpriteContainer
	*/
	int totalObjects;
	/**
	* @var int			availableObjects
	* @brief			OBJs available
	* @memberof		ObjectSpriteContainer
	*/
	int availableObjects;
	/**
	* @var int			spt
	* @brief			spt index
	* @memberof		ObjectSpriteContainer
	*/
	int spt;
	/**
	* @var bool		removingObjectSprite
	* @brief			flag to halt defragmentation while sprite removal is taking place
	* @memberof		ObjectSpriteContainer
	*/
	bool removingObjectSprite;

	void constructor(ObjectSpriteContainer this, int spt, int totalObjects, int firstObjectIndex);
	s32 addObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, int numberOfObjects);
	void calculateParallax(ObjectSpriteContainer this, fix10_6 z);
	int getAvailableObjects(ObjectSpriteContainer this);
	int getFirstObjectIndex(ObjectSpriteContainer this);
	int getLastObjectIndex(ObjectSpriteContainer this);
	int getNextFreeObjectIndex(ObjectSpriteContainer this);
	int getTotalUsedObjects(ObjectSpriteContainer this);
	bool hasRoomFor(ObjectSpriteContainer this, s32 numberOfObjects);
	void position(ObjectSpriteContainer this, const Vector3D* position);
	void print(ObjectSpriteContainer this, int x, int y);
	void removeObjectSprite(ObjectSpriteContainer this, ObjectSprite objectSprite, s32 numberOfObjects);
	override void render(ObjectSpriteContainer this, bool evenFrame);
	override void setPosition(ObjectSpriteContainer this, const PixelVector* position);
	override void show(ObjectSpriteContainer this);
	override void hide(ObjectSpriteContainer this);
	override void addDisplacement(ObjectSpriteContainer this, const PixelVector* displacement);
	override void setMode(ObjectSpriteContainer this, u16 display, u16 mode);
	override bool writeTextures(ObjectSpriteContainer this);
	override bool areTexturesWritten(ObjectSpriteContainer this);
}


#endif
