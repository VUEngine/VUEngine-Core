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
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A ObjectSprite spec
 *
 * @memberof ObjectSprite
 */
typedef struct ObjectSpriteSpec
{
	/// it has a Sprite spec at the beginning
	SpriteSpec spriteSpec;

	/// the display mode (BGMAP, AFFINE, H-BIAS)
	u16 bgmapMode;

	/// flag to indicate in which display to show the bg texture
	u16 display;

} ObjectSpriteSpec;

/**
 * A ObjectSprite spec that is stored in ROM
 *
 * @memberof ObjectSprite
 */
typedef const ObjectSpriteSpec ObjectSpriteROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-object
class ObjectSprite : Sprite
{
	// parent sprite
	ObjectSpriteContainer objectSpriteContainer;
	// number of objects
	s16 totalObjects;

	/// @publicsection
	void constructor(const ObjectSpriteSpec* objectSpriteSpec, Object owner);
	s16 getTotalObjects();
	void invalidateObjectSpriteContainer();
	override u16 doRender(s16 index, bool evenFrame);
	override void rotate(const Rotation* rotation);
	override void setMode(u16 display, u16 mode);
	override void registerSprite();
	override void registerSprite();
}


#endif
