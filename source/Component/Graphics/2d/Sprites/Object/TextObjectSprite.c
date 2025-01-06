/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <DebugConfig.h>
#include <ObjectSpriteContainer.h>
#include <Printing.h>
#include <Texture.h>
#include <VIPManager.h>

#include "TextObjectSprite.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Texture;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TextObjectSprite::constructor(Entity owner, const TextObjectSpriteSpec* textObjectSpriteSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &textObjectSpriteSpec->objectSpriteSpec);

	this->font = textObjectSpriteSpec->font;
	this->text = textObjectSpriteSpec->text;
	this->palette = textObjectSpriteSpec->palette << 14;

	this->printed = false;

	this->totalObjects = NULL == this->text ? 0 : strlen(this->text);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TextObjectSprite::destructor()
{
	this->text = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 TextObjectSprite::doRender(int16 index)
{
	NM_ASSERT(!isDeleted(this->texture), "TextObjectSprite::doRender: null texture");

	if(!this->printed)
	{
		TextObjectSprite::out(this, index);
	}

	int32 cols = strlen(this->text);

	// TODO: add support for multiline fonts
	int32 rows = 1;

	int32 xDirection = this->head & 0x2000 ? -1 : 1;
	int32 yDirection = this->head & 0x1000 ? -1 : 1;

	int32 x = this->position.x - this->halfWidth * xDirection + this->displacement.x - (__LEFT == xDirection ? __OBJECT_SPRITE_FLIP_X_DISPLACEMENT : 0);

	// TODO: the halfHeight should be calculted based on the font's height
	int32 y = this->position.y - this->halfHeight * yDirection + this->displacement.y - (__UP == yDirection ? __OBJECT_SPRITE_FLIP_Y_DISPLACEMENT : 0);

	//FontData* fontData = Printing::getFontByName(this->font);

	int32 i = 0;
	uint16 secondWordValue = (this->head & __OBJECT_SPRITE_CHAR_SHOW_MASK) | ((this->position.parallax + this->displacement.parallax) & ~__OBJECT_SPRITE_CHAR_SHOW_MASK);
	uint16 fourthWordValue = (this->head & 0x3000);
	ObjectAttributes* objectPointer = NULL;

	for(; i < rows; i++)
	{
		// TODO: Account for multiline characters
		//int32 outputY = y + (i << 3) * yDirection;
		int32 outputY = y;

		if((unsigned)(outputY - _cameraFrustum->y0 + 4) > (unsigned)(_cameraFrustum->y1 - _cameraFrustum->y0))
		{
			int32 j = 0;
			for(; j < cols; j++)
			{
				int32 objectIndex = index + j;

				objectPointer = &_objectAttributesCache[objectIndex];
				objectPointer->head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
			}

			continue;
		}

		int32 j = 0;

		for(; j < cols; j++)
		{
			int32 objectIndex = index + j;

			// TODO: Account for character's size
//			int32 outputX = x + (j * fontData->fontSpec->fontSize.x) * xDirection;
			int32 outputX = x + (j * 8) * xDirection;

			// Add 8 to the calculation to avoid char's cut off when scrolling hide the object if outside
			// Screen's bounds
			if((unsigned)(outputX - _cameraFrustum->x0 + 4) > (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
			{
				objectPointer->head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
				continue;
			}

			objectPointer = &_objectAttributesCache[objectIndex];

			objectPointer->jx = outputX;
			objectPointer->head = secondWordValue;
			objectPointer->jy = outputY;
			objectPointer->tile |= fourthWordValue;
		}
	}

	return index;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TextObjectSprite::out(uint16 index)
{
	uint32 i = 0;
	uint32 charOffset = 0, charOffsetX = 0, charOffsetY = 0;

	FontData* fontData = Printing::getFontByName(this->font);

	if(!fontData)
	{
		return;
	}

	uint32 offset = CharSet::getOffset(fontData->charSet);

	// Print text
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

							int32 objectIndex = index + i;

							uint16 charNumber =
								// Offset of charset in char memory
								offset +

								// Offset of character in charset
								((uint8)(this->text[i] - fontData->fontSpec->offset) * fontData->fontSpec->fontSize.x) +

								// Additional y offset in charset
								(((uint8)(this->text[i] - fontData->fontSpec->offset)
									/ fontData->fontSpec->charactersPerLineInCharset
									* fontData->fontSpec->charactersPerLineInCharset * fontData->fontSpec->fontSize.x)
										* (fontData->fontSpec->fontSize.y - 1)) +

								// Respective char of character
								charOffset;

							ObjectAttributes* objectPointer = &_objectAttributesCache[objectIndex];

							objectPointer->tile = this->palette | (charNumber & 0x7FF);
						}
					}
				}

				break;
		}

		i++;
	}

	this->printed = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
