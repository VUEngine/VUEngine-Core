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

#ifndef OBJECT_SPRITE_H_
#define OBJECT_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Sprite.h>
#include <MiscStructs.h>
#include <Texture.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------

#define __OBJECT_CHAR_SHOW_MASK		0xC000
#define __OBJECT_CHAR_HIDE_MASK		0x0000


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef struct ObjectSpriteDefinition
{
	// it has a Sprite definition at the beginning
	SpriteDefinition spriteDefinition;

	// the display mode (BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;

	// flag to indicate in which display to show the bg texture
	u16 display;

} ObjectSpriteDefinition;

typedef const ObjectSpriteDefinition ObjectSpriteROMDef;

class ObjectSprite : Sprite
{
	/**
	* @var ObjectSpriteContainer 	objectSpriteContainer
	* @brief						parent sprite
	* @memberof					ObjectSprite
	*/
	ObjectSpriteContainer objectSpriteContainer;
	/**
	* @var s16 					objectIndex
	* @brief						object index
	* @memberof					ObjectSprite
	*/
	s16 objectIndex;
	/**
	* @var s16 					totalObjects
	* @brief						number of objects
	* @memberof					ObjectSprite
	*/
	s16 totalObjects;

	void constructor(ObjectSprite this, const ObjectSpriteDefinition* oSpriteDefinition, Object owner);
	s16 getObjectIndex(ObjectSprite this);
	s16 getTotalObjects(ObjectSprite this);
	void setObjectIndex(ObjectSprite this, s16 objectIndex);
	override void render(ObjectSprite this, bool evenFrame);
	override void setPosition(ObjectSprite this, const PixelVector* position);
	override void position(ObjectSprite this, const Vector3D* position3D);
	override void rotate(ObjectSprite this, const Rotation* rotation);
	override void calculateParallax(ObjectSprite this, fix10_6 z);
	override u8 getWorldLayer(ObjectSprite this);
	override void addDisplacement(ObjectSprite this, const PixelVector* displacement);
	override void setMode(ObjectSprite this, u16 display, u16 mode);
}


#endif
