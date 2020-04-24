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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <TextObjectSprite.h>
#include <ObjectSpriteContainer.h>
#include <SpriteManager.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Camera.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __FLIP_X_DISPLACEMENT	8
#define __FLIP_Y_DISPLACEMENT	8


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Texture;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof						TextObjectSprite
 * @public
 *
 * @param objectSpriteSpec	Sprite spec
 * @param owner						Owner
 */
void TextObjectSprite::constructor(const TextObjectSpriteSpec* textObjectSpriteSpec, Object owner)
{
	Base::constructor(&textObjectSpriteSpec->objectSpriteSpec, owner);

	this->font = textObjectSpriteSpec->font;
	this->text = textObjectSpriteSpec->text;
	this->palette = textObjectSpriteSpec->palette << 14;

	this->printed = false;

	this->totalObjects = NULL == this->text ? 0 : strlen(this->text);
}

/**
 * Class destructor
 *
 * @memberof						TextObjectSprite
 * @public
 */
void TextObjectSprite::destructor()
{
	this->text = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		TextObjectSprite
 * @public
 *
 * @param evenFrame
 */
bool TextObjectSprite::render(u8 worldLayer __attribute__((unused)))
{
	if(!this->positioned)
	{
		return false;
	}

	if(!this->printed)
	{
		TextObjectSprite::out(this);
	}

	int cols = strlen(this->text);

	// TODO: add support for multiline fonts
	int rows = 1;

	int xDirection = this->head & 0x2000 ? -1 : 1;
	int yDirection = this->head & 0x1000 ? -1 : 1;

	int x = this->position.x - this->halfWidth * xDirection + this->displacement.x - (__LEFT == xDirection ? __FLIP_X_DISPLACEMENT : 0);

	// TODO: the halfHeight should be calculted based on the font's height
	int y = this->position.y - this->halfHeight * yDirection + this->displacement.y - (__UP == yDirection ? __FLIP_Y_DISPLACEMENT : 0);

	//FontData* fontData = Printing::getFontByName(Printing::getInstance(), this->font);

	int i = 0;
	u16 secondWordValue = (this->head & __OBJECT_CHAR_SHOW_MASK) | ((this->position.parallax + this->displacement.parallax) & ~__OBJECT_CHAR_SHOW_MASK);
	u16 fourthWordValue = (this->head & 0x3000);

	for(; i < rows; i++)
	{
		// TODO: Account for multiline characters
		//int outputY = y + (i << 3) * yDirection;
		int outputY = y;

		if((unsigned)(outputY - _cameraFrustum->y0 + 4) > (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0))
		{
			int j = 0;
			for(; j < cols; j++)
			{
				s32 objectIndex = (this->objectIndex + j) << 2;

				_objectAttributesBaseAddress[objectIndex + 1] = __OBJECT_CHAR_HIDE_MASK;
			}

			continue;
		}

		int j = 0;

		for(; j < cols; j++)
		{
			s32 objectIndex = (this->objectIndex + j) << 2;

			// TODO: Account for character's size
//			int outputX = x + (j * fontData->fontSpec->fontSize.x) * xDirection;
			int outputX = x + (j * 8) * xDirection;

			// add 8 to the calculation to avoid char's cut off when scrolling hide the object if outside
			// screen's bounds
			if((unsigned)(outputX - _cameraFrustum->x0 + 4) > (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
			{
				_objectAttributesBaseAddress[objectIndex + 1] = __OBJECT_CHAR_HIDE_MASK;
				continue;
			}

			_objectAttributesBaseAddress[objectIndex] = outputX;
			_objectAttributesBaseAddress[objectIndex + 1] = secondWordValue;
			_objectAttributesBaseAddress[objectIndex + 2] = outputY;
			_objectAttributesBaseAddress[objectIndex + 3] |= fourthWordValue;
		}
	}

	return true;
}

void TextObjectSprite::out()
{
	u32 i = 0;
	u32 charOffset = 0, charOffsetX = 0, charOffsetY = 0;

	FontData* fontData = Printing::getFontByName(Printing::getInstance(), this->font);

	if(!fontData)
	{
		return;
	}

	u32 offset = CharSet::getOffset(fontData->charSet);

	// print text
	while(this->text[i])
	{
		switch(this->text[i])
		{
			default:
				{
					for(charOffsetX = 0; charOffsetX < fontData->fontSpec->fontSize.x; charOffsetX++)
					{
						for(charOffsetY = 0; charOffsetY < fontData->fontSpec->fontSize.y; charOffsetY++)
						{
							charOffset = charOffsetX + (charOffsetY * fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x);

							s32 objectIndex = (this->objectIndex + i) << 2;

							u16 charNumber =
								// offset of charset in char memory
								offset +

								// offset of character in charset
								((u8)(this->text[i] - fontData->fontSpec->offset) * fontData->fontSpec->fontSize.x) +

								// additional y offset in charset
								(((u8)(this->text[i] - fontData->fontSpec->offset)
									/ fontData->fontSpec->charactersPerLineInCharset
									* fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x)
										* (fontData->fontSpec->fontSize.y - 1)) +

								// respective char of character
								charOffset;

							_objectAttributesBaseAddress[objectIndex + 3] = this->palette | (charNumber & 0x7FF);
						}
					}
				}

				break;
		}

		i++;
	}

	this->printed = true;
}
