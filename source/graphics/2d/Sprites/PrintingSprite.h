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

#ifndef PRINTING_SPRITE_H_
#define PRINTING_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>
#include <MiscStructs.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A PrintingSprite spec
 *
 * @memberof PrintingSprite
 */
typedef struct PrintingSpriteSpec
{
	/// it has a Sprite spec at the beginning
	BgmapSpriteSpec bgmapSpriteSpec;

} PrintingSpriteSpec;

/**
 * A PrintingSprite spec that is stored in ROM
 *
 * @memberof PrintingSprite
 */
typedef const PrintingSpriteSpec PrintingSpriteROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// Sprite which holds a texture and a drawing specification.
/// @ingroup graphics-2d-sprites-bgmap
class PrintingSprite : BgmapSprite
{
	// print WORLD's width
	uint16 w;
	// print WORLD's height
	uint16 h;
	
	/// @publicsection
	void constructor(const PrintingSpriteSpec* printingSpriteSpec, Object owner);
	void reset();

	void setGValues(int16 gx, int16 gy, int16 gp);
	void setMValues(int16 mx, int16 my, int16 mp);
	void setSize(uint16 w, uint16 h);
	void setSize(uint16 w, uint16 h);

	int16 getGX();
	int16 getGY();
	int16 getGP();

	override uint16 doRender(int16 index, bool evenFrame);
}


#endif
