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

#ifndef TEXT_OBJECT_SPRITE_H_
#define TEXT_OBJECT_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSprite.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A TextObjectSprite spec
 *
 * @memberof TextObjectSprite
 */
typedef struct TextObjectSpriteSpec
{
	/// it has a ObjectSprite spec at the beginning
	ObjectSpriteSpec objectSpriteSpec;

	/// Text
	const char* text;

	/// Font
	const char* font;

	/// Palette
	u8 palette;

} TextObjectSpriteSpec;

/**
 * A TextObjectSprite spec that is stored in ROM
 *
 * @memberof TextObjectSprite
 */
typedef const TextObjectSpriteSpec TextObjectSpriteROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-object
class TextObjectSprite : ObjectSprite
{
	const char* text;
	const char* font;
	u16 palette;
	bool printed;

	/// @publicsection
	void constructor(const TextObjectSpriteSpec* textObjectSpriteSpec, Object owner);

	override void render();
}


#endif
